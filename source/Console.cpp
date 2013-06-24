/*
#				Orbit-Hopper
#				Console.cpp
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

#include "../headers/Game.h"
#include "../headers/Font.h"
#include "../headers/Structs.h"
#include "../headers/Init.h"
#include "../headers/Console.h"
#include "../headers/Utils.h"


#include "SDL.h"
//#include <cstring>

extern Gamestate* cur_gamestate;
extern Gamestate **gamestates;
extern bool wireframemode;
extern bool debugmsgs;
extern Settings set;
extern Gamestate *old_state;

Console::Console()
{
	
}

Console::~Console()
{
	old_state = NULL;
}

void Console::Init()
{
	old_l = 0;	
	memset(cur_line,0,sizeof(char)*128);
	for(int l = 19;l>=0;l--)
		memset(old_lines[l],0,sizeof(char)*128);
		
	
}

void Console::AddConsoleEntry(char *entry)
{
	//move old lines
	for(int l = old_l;l>0;l--)
		strcpy(old_lines[l],old_lines[l-1]);
				
	//add new line
	strncpy(old_lines[0],(const char *)entry,128);
	
	if(old_l < 19)
		old_l++;
	
}

int Console::Input(SDL_Event action, SDL_keysym key)
{
	if(action.type==SDL_KEYDOWN)
	{
		if(key.sym == SDLK_BACKQUOTE  || key.sym == SDLK_TAB)//closes console
		{
			cur_gamestate = old_state; //return to saved gamestate
		}
		else if(key.sym == SDLK_BACKSPACE)
		{
			if(strlen(cur_line) > 0)
				cur_line[strlen(cur_line)-1] = '\0'; //remove last character
			
		}
		if(strlen(cur_line) < 128 && (char)key.unicode >= ' ' && (char)key.unicode <= '}')
		{
			cur_line[strlen(cur_line)] = (char)key.unicode;	
		}
		else if(key.sym == SDLK_RETURN)
		{
			if(!strcmp(cur_line,"wireframe on"))
			{
				wireframemode = true;	
				glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_CULL_FACE);
				AddConsoleEntry("Wireframemode now on");
			}
			else if(!strcmp(cur_line,"wireframe off"))
			{
				wireframemode = false;	
				glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
				glEnable(GL_TEXTURE_2D);
				glEnable(GL_CULL_FACE);
				AddConsoleEntry("Wireframemode now off");
			
			}
			else if(!strcmp(cur_line,"debug off"))
			{
				debugmsgs = false;
				AddConsoleEntry("Debugmode now off");
			}
			else if(!strcmp(cur_line,"debug on"))
			{
				debugmsgs = true;
				AddConsoleEntry("Debugmode now on");
			}
			else if(!strcmp(cur_line,"speed off"))
			{
				set.speed = false;
				AddConsoleEntry("Speed-effect now off");
			}
			else if(!strcmp(cur_line,"speed on"))
			{
				set.speed = true;
				AddConsoleEntry("Speed-effect now on");
			}
			else if(!strcmp(cur_line,"glow off"))
			{
				set.glow = false;
				AddConsoleEntry("Glow effect now off");
			}
			else if(!strcmp(cur_line,"shadows off"))
			{
				set.shadows = false;
				AddConsoleEntry("Shadows now off");
			}
			else if(!strcmp(cur_line,"shadows on"))
			{
				set.shadows = true;
				AddConsoleEntry("Shadows now on");
			}
			else if(!strcmp(cur_line,"glow on"))
			{
				set.glow = true;
				AddConsoleEntry("Glow effect now on");
			}
			else if(!strcmp(cur_line,"details 0"))
			{
				set.details = 0;
				AddConsoleEntry("Details are now on low");
			}
			else if(!strcmp(cur_line,"details 1"))
			{
				set.details = 1;
				AddConsoleEntry("Details are now on medium");
			}
			else if(!strcmp(cur_line,"details 2"))
			{
				set.details = 2;
				AddConsoleEntry("Details are now on high");
			}
			else if(!strcmp(cur_line,"details 3"))
			{
				set.details = 3;
				AddConsoleEntry("Details are now on ultra");
			}
			else if(strstr(cur_line,"texset"))
			{
				set.textures = atoi(strstr(cur_line,"texset")+6);
				InitTextures();
				AddConsoleEntry("Changed set of textures");
			}
			else if(strstr(cur_line,"animspeed"))
			{
				
				set.animspeed = 53-atoi(strstr(cur_line,"animspeed")+10);
				AddConsoleEntry("Speed adjusted");
			}
			else //invalid command was entered
			{
				AddConsoleEntry("Available commands:");
				AddConsoleEntry("   texset [0-?]                Activates set of textures x");
				AddConsoleEntry("   details [0-2]            	Changes detail settings");
				AddConsoleEntry("   glow [on/off]               Activates/Deactivates glow effect");
				AddConsoleEntry("   debug [on/off]              Activates/Deactivates debug output");
				AddConsoleEntry("   wireframe [on/off]          Activates/Deactivates wireframe mode");
				AddConsoleEntry("   speed [on/off]          	Activates/Deactivates speed-effect");
				AddConsoleEntry("   shadows [on/off]          	Activates/Deactivates shadows");
				AddConsoleEntry("   animspeed [0-50]          	Adjusts speed of animations");
				
			}
		}
	}
	return 0;
}


void Console::Render()
{
	glColor3f(0.5f,0.5f,1.0f);
	
	old_state->Render();
	
	if(wireframemode)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
		glEnable(GL_CULL_FACE);	
	}
	
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);		
	glDisable(GL_TEXTURE_2D);					
		glMatrixMode(GL_PROJECTION);						
		glPushMatrix();									
		glLoadIdentity();									
		gluOrtho2D(0,set.width,0,set.height);							
		glMatrixMode(GL_MODELVIEW);						
		glPushMatrix();
		glLoadIdentity();
			glColor3f(0.25f,0.25f,0.25f);										
			glBegin(GL_QUADS);
				//upper part -> contents
				glVertex2i(0, (int)(0.6992*(float)(set.height)));
				glVertex2i(set.width, (int)(0.6992*(float)(set.height)));
				glVertex2i(set.width, set.height);
				glVertex2i(0, set.height);
				
			glEnd();
			
			glColor3f(0.35f,0.35f,0.35f);										
			glBegin(GL_QUADS);
				//lower part -> input
				glVertex2i(0, (int)(0.681*(float)(set.height)));
				glVertex2i(set.width, (int)(0.681*(float)(set.height)));
				glVertex2i(set.width, (int)(0.6992*(float)(set.height)));
				glVertex2i(0, (int)(0.6992*(float)(set.height)));
				
			glEnd();	
				
		glMatrixMode(GL_PROJECTION);						
		glPopMatrix();										
		glMatrixMode(GL_MODELVIEW);							
		glPopMatrix();	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);		
	
	if(wireframemode)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
	}	
	
	//draw contents
	for(int j = 19;j>=0;j--)
		printtxt2d(0,(int)(0.977*(float)(set.height)-(19-j)*0.013*(float)(set.height)),0.5f,0.8f,0.8f,0.8f,old_lines[j]);
	
	//draw input
	printtxt2d(0,(int)(0.697*(float)(set.height)),0.5f,0.8f,0.8f,0.8f,cur_line);
		  
}

void Console::Update(unsigned int diff){}


