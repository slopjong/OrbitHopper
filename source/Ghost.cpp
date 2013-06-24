/*
#				Orbit-Hopper
#				Ghost.cpp : Replay
#
#				Copyright (C) 2004-2008 Harald Obermaier
#
# This file is part of Orbit-Hopper.
#
#   Orbit-Hopper is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   Orbit-Hopper is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Orbit-Hopper; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
*/

#include "../headers/Structs.h"
#include "../headers/MapSel.h"
#include "../headers/Game.h"



bool ghost= false;
Vertex *ghostpos = NULL;
Vertex *ownpos = NULL;

int poscount = 250;
int gh_entries = 0;
int cur_poscount = 0;

extern HighscoreEntry *HEntries;
extern Gamestate *gamestates[15];

void LoadGhost(char *filename)
{
	gh_entries = 0;
	cur_poscount = 0;

	char *temp = new char[64];
	strncpy(temp,filename,64);
	
	char *ending = strchr(temp,'.');
	if(ending)
	{
		
		strcpy(ending,".gho");
		
		FILE *gho = fopen(temp,"rb");
		if(gho == NULL)
		{
			ghost = false;
			delete[] temp;
			temp = NULL;
			return;
		}
		
		fread(&gh_entries,sizeof(int),1,gho);
		if(ghostpos)
		{
			delete[] ghostpos;
			ghostpos = NULL;
		}
		ghostpos = new Vertex[gh_entries];
		
		if(ghostpos != NULL)
		{
			for(int i = 0;i<gh_entries;i++)
			{
				fread(&ghostpos[i].x,sizeof(int),1,gho);
				fread(&ghostpos[i].y,sizeof(int),1,gho);
				fread(&ghostpos[i].z,sizeof(int),1,gho);
			
			}
			
		}
		
		
		fclose(gho);
		ghost = true;
	}
	delete[] temp;
	temp = NULL;
}


void SaveGhost()
{
	char *temp = new char[64];
	strcpy(temp,"maps/");
	strcat(temp,HEntries[((MapSel *)gamestates[GS_MAPSEL])->cur_level].levelname);
	
	char *ending = strchr(temp,'.');
	if(ending)
	{
		
		strcpy(ending,".gho");
	
		FILE *out = fopen(temp,"wb");
		if(out == NULL)
		{
			delete[] temp;
			temp = NULL;
			return;
		}
			
		cur_poscount++;
		fwrite(&cur_poscount,sizeof(int),1,out);
		for(int u = 0;u<cur_poscount;u++)
		{
			fwrite(&ownpos[u].x,sizeof(int),1,out);
			fwrite(&ownpos[u].y,sizeof(int),1,out);	
			fwrite(&ownpos[u].z,sizeof(int),1,out);		
						
		}
		fclose(out);
	}
	delete[] temp;
	temp = NULL;
}
