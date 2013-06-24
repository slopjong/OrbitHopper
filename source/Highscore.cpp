/*
#				Orbit-Hopper
#				Highscore.cpp
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
#include "../headers/Game.h"

#ifndef WIN32
#include <dirent.h> //linux only
#endif

HighscoreEntry *HEntries = NULL;
int num_maps = 0;
char playername[16];
extern Settings set;


void LoadHighScore()
{
	char file[32];
	
	strcpy(file,"highscore.hsc");

	//default: Zneaker wins :-)
	for(int l = 0;l<num_maps;l++)
	{
		HEntries[l].record_time = 999999;
		HEntries[l].medal = -1;
		strcpy(HEntries[l].playername,"Zneaker");
	}

	FILE *in = fopen(file,"rb");
	if(in == NULL)
		return;
		
	int num_entries = 0;
	fread(&num_entries,sizeof(int),1,in);
	
	
	
	for(int u = 0;u<num_entries && u<num_maps;u++)
	{
		char tempname[64];
		char playername[32];
		time_t temptime;
		unsigned int temprecord;
		int tempmedal;
		fread(&tempname,sizeof(char[64]),1,in);
		fread(&temptime,sizeof(time_t),1,in);
		fread(&temprecord,sizeof(unsigned int),1,in);
		fread(&tempmedal,sizeof(int),1,in);
		fread(&playername,sizeof(char[32]),1,in);
		
		for(int k = 0;k < num_maps;k++)
			if(!strcmp(HEntries[k].levelname,tempname) 
			&& HEntries[k].write_time == temptime)
			{
				HEntries[k].record_time = temprecord;
				HEntries[k].medal = tempmedal;
				strncpy(HEntries[k].playername,playername,32);
				break;
			}
	}
	fclose(in);
	
}

void SaveHighScore()
{
	char file[32];
	
	strcpy(file,"highscore.hsc");
	FILE *out = fopen(file,"wb");
	
	fwrite(&num_maps,sizeof(int),1,out);
	for(int u = 0;u<num_maps;u++)
	{
		fwrite(&HEntries[u].levelname,sizeof(char[64]),1,out);
		fwrite(&HEntries[u].write_time,sizeof(time_t),1,out);
		fwrite(&HEntries[u].record_time,sizeof(unsigned int),1,out);
		fwrite(&HEntries[u].medal,sizeof(int),1,out);
		fwrite(&HEntries[u].playername,sizeof(char[32]),1,out);
			
	}
	fclose(out);
	
}

#ifndef WIN32
	int GetMapsScore() //linux
	{
		DIR *dir = opendir("maps/");
		num_maps = 0;
		dirent *entry = readdir(dir);
	
		while(entry)
		{
			if(strstr(entry->d_name,".slv"))
			{
				if(strncmp(entry->d_name,"ca-",3) 
				&& strncmp(entry->d_name,"s-",2))
					num_maps++;
			}
	
			entry = readdir(dir);
		}
	
		closedir(dir);
		
		if(HEntries)
		{
			delete[] HEntries;
			HEntries = NULL;
		}
			
		HEntries = new HighscoreEntry[num_maps];
		
		dir = opendir("maps/");
		entry = readdir(dir);
		int i = 0;
		while(entry && i< num_maps)
		{
			if(strstr(entry->d_name,".slv"))
			{
				if(strncmp(entry->d_name,"ca-",3) 
				&& strncmp(entry->d_name,"s-",2))
				{
					strncpy(HEntries[i].levelname,entry->d_name,64);
					HEntries[i].write_time = 10;
					i++;	
				}
			}
	
			entry = readdir(dir);
		}
		
		closedir(dir);

		
		for(i = 0; i < num_maps; i++)
		{
			for(int u = i+1; u < num_maps; u++)
			{
				for(int k = 0; HEntries[i].levelname[k]; k++)
				{
					if(HEntries[u].levelname)
						if(HEntries[i].levelname[k] != HEntries[u].levelname[k])
						{
							if(HEntries[i].levelname[k] > HEntries[u].levelname[k])
							{
								char swap[32];
								strncpy(swap,HEntries[u].levelname,32);
								strncpy(HEntries[u].levelname,HEntries[i].levelname,32);
								strncpy(HEntries[i].levelname,swap,32);
							}
							break;
						}		
				}
				
			}
		}
	
		return num_maps;
	}
#else
	int GetMapsScore()//windows
	{
		_finddata_t fileblock;
		num_maps = 0;
		
		int file_save = _findfirst("maps/*.slv",&fileblock);
		if(file_save==-1)
			return 0;
			
		if(strncmp(fileblock.name,"ca-",3) 
		&& strncmp(fileblock.name,"s-",2))
			num_maps++;
			
		while(_findnext(file_save,&fileblock) != -1)
		{	
			if(strncmp(fileblock.name,"ca-",3) 
			&& strncmp(fileblock.name,"s-",2))
				num_maps++;
		}
		_findclose(file_save);
		
		
		
		if(HEntries)
		{
			delete[] HEntries;
			HEntries = NULL;
		}
			
		HEntries = new HighscoreEntry[num_maps];
		
		
		file_save = _findfirst("maps/*.slv",&fileblock);
		
		int i = 0;
		if(strncmp(fileblock.name,"ca-",3) 
		&& strncmp(fileblock.name,"s-",2))
		{	
			strncpy(HEntries[0].levelname,fileblock.name,64);
			HEntries[0].write_time = fileblock.time_write;
			i++;
		}
		while(i < num_maps)
		{
			if(_findnext(file_save,&fileblock) == -1)
				break;
				
			if(strncmp(fileblock.name,"ca-",3) 
			&& strncmp(fileblock.name,"s-",2))
			{	
				strncpy(HEntries[i].levelname,fileblock.name,64);
				HEntries[i].write_time = fileblock.time_write;
	
				i++;
			}
		}
		_findclose(file_save);
		
		for(i = 0; i < num_maps; i++)
		{
			for(int u = i+1; u < num_maps; u++)
			{
				for(int k = 0; HEntries[i].levelname[k]; k++)
				{
					if(HEntries[u].levelname)
						if(HEntries[i].levelname[k] != HEntries[u].levelname[k])
						{
							if(HEntries[i].levelname[k] > HEntries[u].levelname[k])
							{
								char swap[32];
								strncpy(swap,HEntries[u].levelname,32);
								strncpy(HEntries[u].levelname,HEntries[i].levelname,32);
								strncpy(HEntries[i].levelname,swap,32);
							}
							break;
						}		
				}
				
			}
		}
		return num_maps;
		
	}
#endif
