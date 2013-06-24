/*
#				Orbit-Hopper
#				MapSel.cpp	: map selection for timeattack mode
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
#include "../headers/Font.h"
#include "../headers/Highscore.h"
#include "../headers/Game.h"
#include "../headers/TimeAttack.h"
#include "../headers/MapSel.h"
#include "../headers/Sound.h"



int map_first = 0;
extern int num_maps;	//from HighScore.cpp
extern HighscoreEntry *HEntries;
extern Settings set;
extern Gamestate *gamestates[15];
extern GLuint **anims;
extern GLuint titles[1];
void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b);

MapSel::MapSel()
{
	cur_level = 0;
	num_levels = 0;
	map_first = 0;
}

MapSel::~MapSel()
{
	
}

void MapSel::Update(unsigned int diff){}

void MapSel::Init()
{
	
	num_levels = GetMapsScore();
	LoadHighScore();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspective(40,(float)(set.width)/(float)(set.height),50.5f,400000.0f);
	glMatrixMode(GL_MODELVIEW);
	
	
	//reinitialize statistics
	total_time = 0;		
	for(int i = 0;i<num_maps;i++)
		total_time += HEntries[i].record_time;
	
	average_time = total_time/num_maps;
}


int MapSel::Input(SDL_Event action, SDL_keysym key)
{
	if(action.type == SDL_KEYDOWN)
	{
		if((key.sym == SDLK_UP || key.sym == SDLK_w)&& cur_level > 0)
		{
			play(-1,S_SELECT);

			if(cur_level >= 0)
				cur_level--;
				
			if(cur_level < map_first)
				map_first--;	
		}
			
		else if((key.sym == SDLK_DOWN || key.sym == SDLK_s) && cur_level < num_maps-1 && cur_level < num_levels-1)
		{
			play(-1,S_SELECT);
			
			if(cur_level < num_maps)
				cur_level++;
			
			if(cur_level > map_first+5)
				map_first++;
		}
		else if(key.sym == SDLK_RETURN)
		{
			play(-1,S_SELECT);
			char temp[70];
			strcpy(temp,"maps/");
			strcat(temp,HEntries[cur_level].levelname);
			((TimeAttack *)(gamestates[GS_TIMEATTACK]))->LoadLevelData(temp);	// load map

			glEnable(GL_LIGHTING);
			SwitchGamestate(GS_TIMEATTACK);
		}
		else if(key.sym == SDLK_ESCAPE)
		{
			SDL_WM_GrabInput(SDL_GRAB_ON);
			SDL_ShowCursor(false);

			glEnable(GL_LIGHTING);
			SwitchGamestate(GS_MENU);
			
		}
	}
	return 0;	
	
}

void MapSel::Render()
{
	

  	glColor3f(0.7,0.7,0.7);	
  	DrawBackGround(23,0);
  	
	{
		glColor3f(0.0,0.0,1.0);	
		glEnable(GL_BLEND);	
		glDisable(GL_LIGHTING);
	
		glDisable(GL_DEPTH_TEST);											
		glBindTexture(GL_TEXTURE_2D,titles[0]);	
		glMatrixMode(GL_PROJECTION);								
		glPushMatrix();												
		glLoadIdentity();											
		gluOrtho2D( 0, set.width ,0, set.height);							
		glMatrixMode(GL_MODELVIEW);									
		glPushMatrix();												
			glLoadIdentity();	
		
			glBegin(GL_QUADS);
				glTexCoord2f(1,0.27);	glVertex2i((int)(0.85*set.width), (int)(0.842*set.height));
				glTexCoord2f(1,0.14);	glVertex2i((int)(0.85*set.width), (int)(0.932*set.height));
				glTexCoord2f(0,0.14);	glVertex2i((int)(0.15*set.width), (int)(0.932*set.height));
				glTexCoord2f(0,0.27);	glVertex2i((int)(0.15*set.width), (int)(0.842*set.height));	
			glEnd();	
		
			glColor3f(1.0f, 1.0f, 1.0f);
		glMatrixMode( GL_PROJECTION );								
		glPopMatrix();												
		glMatrixMode( GL_MODELVIEW );								
		glPopMatrix();
	
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);			
		glDisable(GL_BLEND);
		glColor3f(1.0f,1.0f,1.0f);

	}
			
	printtxt2d((int)(0.0488*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,"Levelname");
	printtxt2d((int)(0.439*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,"Record Holder");
	printtxt2d((int)(0.732*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,"Record Time");
	
	int y = (int)(0.716*set.height);
	float g = 1.0f;


	
	for(int u = map_first;u<num_maps && u < num_levels && u<=map_first+10 ;u++,y-=(int)(0.039*set.height))
	{
			
		char temp[128];
		
		unsigned int tsec = (HEntries[u].record_time%1000)/10;
		unsigned int sec = (HEntries[u].record_time/1000)%60;
		unsigned int min = HEntries[u].record_time/60000;

		if(u == cur_level)
			g = 0.0f;
		else
			g = 1.0f;

		if(HEntries[u].medal == 0)
		{
			glColor3f(1.0,0.85,0.0);
			
		}
		else if(HEntries[u].medal == 1)
		{
			glColor3f(0.7,0.7,0.7);
			
		}
		else if(HEntries[u].medal == 2)
		{
			glColor3f(0.5,0.3,0.3);
			
		}

		if(HEntries[u].medal >= 0)
		{
			glDisable(GL_LIGHTING);	
			glEnable(GL_BLEND);	
			
			glDisable(GL_DEPTH_TEST);											
			glBindTexture(GL_TEXTURE_2D,anims[27][0]);
	
			

			glMatrixMode(GL_PROJECTION);								
			glPushMatrix();												
			glLoadIdentity();											
			gluOrtho2D( 0, set.width ,0, set.height);							
			glMatrixMode(GL_MODELVIEW);									
			glPushMatrix();												
				glLoadIdentity();	
		
				glBegin(GL_QUADS);
					glTexCoord2f(0.995,0.995);	glVertex2i((int)(0.702*set.width), y-(int)(0.022*set.height));
					glTexCoord2f(0.995,0.005);	glVertex2i((int)(0.702*set.width), y);
					glTexCoord2f(0.755,0.005);	glVertex2i((int)(0.685*set.width), y);
					glTexCoord2f(0.755,0.995);	glVertex2i((int)(0.685*set.width), y-(int)(0.022*set.height));	
				glEnd();	
		
			glMatrixMode( GL_PROJECTION );								
			glPopMatrix();												
			glMatrixMode( GL_MODELVIEW );								
			glPopMatrix();	

			glEnable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);			
			glDisable(GL_BLEND);
			glColor3f(1.0f,1.0f,1.0f);
		}

		sprintf(temp,"%2d: %02d: %02d",min,sec,tsec);
		
		
		printtxt2d((int)(0.0488*set.width),y,1.0f,1.0f,g,1.0f,HEntries[u].levelname);
		printtxt2d((int)(0.439*set.width),y,1.0f,1.0f,g,1.0f,HEntries[u].playername);
		printtxt2d((int)(0.732*set.width),y,1.0f,1.0f,g,1.0f,temp);
	}

	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);		
	glDisable(GL_DEPTH_TEST);											
	glBindTexture(GL_TEXTURE_2D,anims[27][0]);	

	glMatrixMode(GL_PROJECTION);								
	glPushMatrix();												
	glLoadIdentity();											
	gluOrtho2D( 0, set.width ,0, set.height);							
	glMatrixMode(GL_MODELVIEW);									
	glPushMatrix();												
		glLoadIdentity();	
		if(map_first+10 >= num_maps-1)
			glColor3f(0.2,0.2,0.2);
		else
			glColor3f(0.2,0.2,1.0);


			glBegin(GL_QUADS);
			
				glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.93*set.width), (int)(0.3*set.height));
				glTexCoord2f(0.47,0.7);		glVertex2i((int)(0.93*set.width), (int)(0.335*set.height));
				glTexCoord2f(0.40,0.7);		glVertex2i((int)(0.918*set.width), (int)(0.335*set.height));
				glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.918*set.width), (int)(0.3*set.height));	
										
			glEnd();
		
		if(map_first <= 0)
			glColor3f(0.2,0.2,0.2);
		else
			glColor3f(0.2,0.2,1.0);

			glBegin(GL_QUADS);
			
				glTexCoord2f(0.47,0.7);	glVertex2i((int)(0.93*set.width), (int)(0.70*set.height));
				glTexCoord2f(0.47,0.1);		glVertex2i((int)(0.93*set.width), (int)(0.735*set.height));
				glTexCoord2f(0.40,0.1);		glVertex2i((int)(0.918*set.width), (int)(0.735*set.height));
				glTexCoord2f(0.40,0.7);	glVertex2i((int)(0.918*set.width), (int)(0.70*set.height));	
										
			glEnd();	
		
		glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode( GL_PROJECTION );								
	glPopMatrix();												
	glMatrixMode( GL_MODELVIEW );								
	glPopMatrix();	

	glEnable(GL_DEPTH_TEST);			
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glColor3f(1.0f,1.0f,1.0f);
	
	char temp[128];
	
	sprintf(temp,"Number of maps: %d",num_maps);
	
	printtxt2d((int)(0.070*set.width),(int)(0.22*set.height),0.8f,1.0f,1.0f,1.0f,temp);
	
	sprintf(temp,"Total: %d: %02d: %02d",total_time/60000,(total_time/1000)%60,(total_time%1000)/10);
		
	printtxt2d((int)(0.370*set.width),(int)(0.22*set.height),0.8f,1.0f,1.0f,1.0f,temp);
	
	sprintf(temp,"Average: %d: %02d: %02d",average_time/60000,(average_time/1000)%60,(average_time%1000)/10);
	printtxt2d((int)(0.632*set.width),(int)(0.22*set.height),0.8f,1.0f,1.0f,1.0f,temp);	
	
	DrawRectangle((int)(0.029*set.width),(int)(0.82*set.height),(int)(0.941*set.width),(int)(0.651*set.height),0.0f,0.0f,1.0f);
	
}
