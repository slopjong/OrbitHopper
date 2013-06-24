/*
#				Orbit-Hopper
#				LevelEnd.cpp : Timeattack statistics
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

#include "../headers/Font.h"
#include "../headers/Structs.h"
#include "../headers/Ghost.h"
#include "../headers/Game.h"
#include "../headers/MapSel.h"
#include "../headers/Highscore.h"
#include "../headers/TimeAttack.h"
#include "../headers/LevelEnd.h"


#include <cstring>

extern HighscoreEntry *HEntries;
extern Settings set;
extern Gamestate *gamestates[15];
extern GLuint titles[1];
void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b);

LevelEnd::LevelEnd()
{
	
}

LevelEnd::~LevelEnd()
{
	if(HEntries)
	{
		delete[] HEntries;
		HEntries = NULL;
	}
}

void LevelEnd::Update(unsigned int diff){}

void LevelEnd::Init()
{
	glDisable(GL_LIGHTING);
	memset(playername,0,sizeof(char)*64);
}

void LevelEnd::Render()
{
	
	

	glColor3f(0.7,0.7,0.7);	
  	DrawBackGround(23,0);
  	glColor3f(1.0,1.0,1.0);	
  	
	int cur_level = ((MapSel *)gamestates[GS_MAPSEL])->cur_level;
	if(((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.fastest)
	{

		{
			glColor3f(0.0,1.0,0.0);	
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
					glTexCoord2f(1,0.84);	glVertex2i((int)(0.906*set.width), (int)(0.845*set.height));
					glTexCoord2f(1,0.77);	glVertex2i((int)(0.906*set.width), (int)(0.902*set.height));
					glTexCoord2f(0,0.77);	glVertex2i((int)(0.206*set.width), (int)(0.902*set.height));
					glTexCoord2f(0,0.84);	glVertex2i((int)(0.206*set.width), (int)(0.845*set.height));	
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
		//printtxt2d((int)(0.22*set.width),(int)(0.872*set.height),1.3f,0.0f,1.0f,0.0f,"Whohoooo - that was damn fast!");

		if(((TimeAttack *)gamestates[GS_TIMEATTACK])->time1 >= ((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.min*60000 
		+((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.sec*1000+((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.tsec)
		{
			printtxt2d((int)(0.307*set.width),(int)(0.75*set.height),1.2f,1.0f,0.85f,0.0f,"You won a GOLD medal!");
			HEntries[cur_level].medal = 0;
		}
		else if (((TimeAttack *)gamestates[GS_TIMEATTACK])->time2 >= ((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.min*60000
		+((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.sec*1000+((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.tsec)
		{
			printtxt2d((int)(0.31*set.width),(int)(0.75*set.height),1.1f,0.7f,0.7f,0.7f,"You won a SILVER medal!");
			HEntries[cur_level].medal = 1;
		}
		else if (((TimeAttack *)gamestates[GS_TIMEATTACK])->time3 >= ((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.min*60000
		+((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.sec*1000+((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.tsec)
		{
			printtxt2d((int)(0.327*set.width),(int)(0.75*set.height),1.0f,0.5f,0.3f,0.3f,"You won a BRONZE medal!");
			HEntries[cur_level].medal = 2;
		}
		else
			HEntries[cur_level].medal = -1;

		char time[128];

		sprintf(time,"Your time was:       %2d:%02d:%02d",((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.min,((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.sec,((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.tsec/10);
		printtxt2d((int)(0.197*set.width),(int)(0.65*set.height),1.0f,1.0f,1.0f,1.0f,time);
		
		unsigned int tsec = (HEntries[cur_level].record_time%1000)/10;
		unsigned int sec = (HEntries[cur_level].record_time/1000)%60;
		unsigned int min = HEntries[cur_level].record_time/60000;
				
		sprintf(time,"Old record time was: %2d:%02d:%02d",min,sec,tsec);
		printtxt2d((int)(0.197*set.width),(int)(0.586*set.height),1.0f,1.0f,1.0f,1.0f,time);
		
		sprintf(time,"That's a difference of %d milliseconds.",((HEntries[cur_level].record_time%1000)-((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.tsec)+1000*(sec-((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.sec)+60000*(min-((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.min));
		printtxt2d((int)(0.197*set.width),(int)(0.495*set.height),1.0f,1.0f,1.0f,1.0f,time);
		
		snprintf(time,127,"Enter your name: %s",playername);
		printtxt2d((int)(0.197*set.width),(int)(0.40*set.height),1.0f,1.0f,1.0f,1.0f,time);
		
		DrawRectangle((int)(0.029*set.width),(int)(0.82*set.height),(int)(0.941*set.width),(int)(0.651*set.height),0.0f,1.0f,0.0f);
	}
	else
	{
		//printtxt2d((int)(0.244*set.width),(int)(0.87*set.height),1.3f,1.0f,0.0f,0.0f,"Well...better luck next time!");

		{
			glColor3f(1.0,0.0,0.0);	
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
					glTexCoord2f(1,0.76);	glVertex2i((int)(0.86*set.width), (int)(0.842*set.height));
					glTexCoord2f(1,0.66);	glVertex2i((int)(0.86*set.width), (int)(0.932*set.height));
					glTexCoord2f(0,0.66);	glVertex2i((int)(0.16*set.width), (int)(0.932*set.height));
					glTexCoord2f(0,0.76);	glVertex2i((int)(0.16*set.width), (int)(0.842*set.height));	
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
		char time[128];

		sprintf(time,"Your time was:  %2d:%02d:%02d",((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.min,((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.sec,((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.tsec/10);
		printtxt2d((int)(0.198*set.width),(int)(0.65*set.height),1.0f,1.0f,1.0f,1.0f,time);
		
		unsigned int tsec = (HEntries[cur_level].record_time%1000)/10;
		unsigned int sec = (HEntries[cur_level].record_time/1000)%60;
		unsigned int min = HEntries[cur_level].record_time/60000;
				
		sprintf(time,"Record time is: %2d:%02d:%02d  by %s",min,sec,tsec,HEntries[cur_level].playername);
		printtxt2d((int)(0.198*set.width),(int)(0.586*set.height),1.0f,1.0f,1.0f,1.0f,time);
		
		sprintf(time,"That's a difference of %d milliseconds.",(-(HEntries[cur_level].record_time%1000)+((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.tsec)+1000*(-sec+((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.sec)+60000*(-min+((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.min));
		printtxt2d((int)(0.198*set.width),(int)(0.521*set.height),1.0f,1.0f,1.0f,1.0f,time);
		
		DrawRectangle((int)(0.029*set.width),(int)(0.82*set.height),(int)(0.941*set.width),(int)(0.651*set.height),1.0f,0.0f,0.0f);
	}	
	
	
	
}

int LevelEnd::Input(SDL_Event action,SDL_keysym key)
{
	int cur_level = ((MapSel *)gamestates[GS_MAPSEL])->cur_level;
	if(action.type == SDL_KEYDOWN)
	{
		if(key.sym == SDLK_RETURN)
		{
			
			if(((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.fastest)
			{	
				//new highscore -> Player has been faster than old record-holder
				HEntries[cur_level].record_time = ((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.run_time;
				strncpy(HEntries[cur_level].playername,playername,32);
				SaveHighScore();
				SaveGhost();
			}
			
			SwitchGamestate(GS_TIMEATTACK);	
		}
		else if(key.sym == SDLK_ESCAPE)
		{
			
			if(((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.fastest)
			{	
				//new highscore -> Player has been faster than old record-holder
				HEntries[cur_level].record_time = ((TimeAttack *)gamestates[GS_TIMEATTACK])->Player.run_time;
				strncpy(HEntries[cur_level].playername,playername,32);
				SaveHighScore();
				SaveGhost();
			}
			
			SwitchGamestate(GS_MAPSEL);
			
		}
		else if(key.sym == SDLK_BACKSPACE)
		{
			if(strlen(playername) > 0)
				playername[strlen(playername)-1] = '\0';
			
		}
		else if(strlen(playername) < 16)
		{
			if((char)key.unicode >= ' ' && (char)key.unicode <= '}')
				playername[strlen(playername)] = (char)key.unicode;
			
		}
	}
	return 0;	
}
