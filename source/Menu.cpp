/*
#				Orbit-Hopper
#				Menu.cpp : main menu
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
#include "../headers/Game.h"
#include "../headers/Menu.h"
#include "../headers/Sound.h"


#include "math.h"

extern GLuint **anims;
extern Settings set;
extern Gamestate *gamestates[15];
extern Gamestate *old_state;
extern Gamestate *cur_gamestate;
extern unsigned int frames;
extern bool debugmsgs;
extern GLuint titles[1];

void DrawEdge(int dir1, int dir2,int dir3, Vertex start, float length,int tiles);
void DrawCorner(int dir1, int dir2,int dir3, Vertex middle);



Menu::Menu()
{
	menu_index = 0;
	rot_angle = 0;
	rot_dir = 0;
	v_speed = 0;
	
}

Menu::~Menu()
{
	
}

void Menu::Update(unsigned int diff)
{
	static unsigned int framec = 0;
	framec += diff;
	
	if(framec >= 21)
	{
		if(rot_dir)
		{
			if(rot_dir==1)
				rot_angle += 5;
			else
				rot_angle += -5;	
				
			if(!(rot_angle%60))
			{
				if(rot_dir == 1)
				{
					menu_index--;
					if(menu_index < 0)
						menu_index = 5;
				}
				else
				{
					menu_index++;
					if(menu_index > 5)
						menu_index = 0;
				}
				rot_dir = 0;
				
			}
		}
		
		v_speed -= 10;
			
		oldPpos = Ppos;
		
		Ppos.y += v_speed;
		if(Ppos.y <= 500)
		{
			Ppos.y = 500;
			v_speed = 0;
		}
		framec = framec%21;
	}
}

void Menu::Init()
{
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45,(float)(set.width)/(float)(set.height),50.5f,400000.0f);
	glMatrixMode(GL_MODELVIEW);	
	
	Ppos.x = oldPpos.x = 20;
	Ppos.y = oldPpos.y= 500;
	Ppos.z = oldPpos.z= -5250;
	
}

int Menu::Input(SDL_Event action,SDL_keysym key)
{
	switch(action.type)
	{
		case SDL_KEYDOWN:	
			if(key.sym == SDLK_d || key.sym == SDLK_RIGHT)
			{
				play(-1,S_MENU);
				if(rot_dir == 1)
					menu_index--;
				rot_dir = 2;
				if(Ppos.y == 500)
					v_speed = 120;
			}
			else if(key.sym == SDLK_a || key.sym == SDLK_LEFT)
			{
				play(-1,S_MENU);
				if(rot_dir == 2)
					menu_index++;
				rot_dir = 1;	
				if(Ppos.y == 500)
					v_speed = 120;
			}
			else if(key.sym == SDLK_ESCAPE)
			{
				SDL_Quit();
				return -1;
			}	
			else if(key.sym == SDLK_RETURN)
			{
				play(-1,S_SELECT);
				if(menu_index == 0)
				{
					SwitchGamestate(GS_MAPSEL);
				}
				if(menu_index == 1)
				{
					SwitchGamestate(GS_SINGLECSEL);
				}
				if(menu_index == 2)
				{
					SwitchGamestate(GS_MODESELM);	
				}
				if(menu_index == 3)
				{
					SwitchGamestate(GS_OPTIONS);
				}
				else if(menu_index == 4)
				{
					SwitchGamestate(GS_LEVELED);
				}
				else if(menu_index == 5)
				{
					SDL_Quit();
					return -1;	
				}
			}
			else if(key.sym == SDLK_BACKQUOTE  || key.sym == SDLK_TAB)//opens console
			{
				old_state = cur_gamestate;
				cur_gamestate = gamestates[GS_CONSOLE];
				
			}
		break;
	}
	return 0;
	
}


void Menu::DisplayDebug()
{
 	char temp[64];
 	sprintf(temp,"selection:   %d",menu_index);
	printtxt2d(0,(int)(0.977*set.height),0.6f,1.0f,1.0f,0.0f,temp);
		
	sprintf(temp,"fps:         %d",frames);
	printtxt2d(0,(int)(0.957*set.height),0.6f,1.0f,1.0f,0.0f,temp);
		
		
	
}

#define DETAIL_OFFSET 	50
void DrawSection(int text)
{
	Vertex ext = {1800,500,2000};
	int x1 = 900;
	int x2 = 0;
	int x3 = -5000;
	

	if(!set.details)//draw low detail cubes
	{
		glBindTexture(GL_TEXTURE_2D, anims[0][0]);
		glBegin(GL_QUADS);
		
			// Right face
			glNormal3f(1.0,0.0,0.0);
			glTexCoord2f(0, 0);		glVertex3i(x1, x2,  		x3+ext.z);
			glTexCoord2f(0, 1);		glVertex3i(x1, x2-ext.y, 	x3+ext.z);
			glTexCoord2f(1, 1);		glVertex3i(x1, x2-ext.y,  	x3);
			glTexCoord2f(1, 0);		glVertex3i(x1, x2,  		x3);
		
			// Left Face
			glNormal3f(-1.0,0.0,0.0);
			glTexCoord2f(0, 0);		glVertex3i(x1-ext.x, x2,  		x3);
			glTexCoord2f(0, 1);		glVertex3i(x1-ext.x, x2-ext.y,  x3);
			glTexCoord2f(1, 1);		glVertex3i(x1-ext.x, x2-ext.y,  x3+ext.z);
			glTexCoord2f(1,0);		glVertex3i(x1-ext.x, x2,  		x3+ext.z);
		glEnd();
		
		glBindTexture(GL_TEXTURE_2D, anims[text][0]);
		glBegin(GL_QUADS);
		// Back Face
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f(0, 0);		glVertex3i(x1-ext.x, 	x2,  		x3+ext.z);
			glTexCoord2f(0, 1);		glVertex3i(x1-ext.x, 	x2-ext.y, x3+ext.z);
			glTexCoord2f(1,1);		glVertex3i(x1, 		x2-ext.y, x3+ext.z);
			glTexCoord2f(1, 0);		glVertex3i(x1, 		x2,  		x3+ext.z);
		
		// Front Face
			glNormal3f(0.0,0.0,-1.0);
			glTexCoord2f(0, 0);		glVertex3i(x1, 		x2,  		x3);
			glTexCoord2f(0, 1);		glVertex3i(x1, 		x2-ext.y,x3);
			glTexCoord2f(1, 1);		glVertex3i(x1-ext.x, 	x2-ext.y, x3);
			glTexCoord2f(1, 0);		glVertex3i(x1-ext.x, 	x2,  		x3);
		
		glEnd();
	
	
	}
	else if(set.details == 1)//draw medium detail -cubes
	{
		glBindTexture(GL_TEXTURE_2D, anims[0][0]);
		glBegin(GL_QUADS);
			//bottom face
			glNormal3f(0.0,-1.0,0.0);
			glTexCoord2f(0, 0);		glVertex3i(x1-ext.x, 	x2-ext.y,  x3+ext.z);
			glTexCoord2f(0, 1);		glVertex3i(x1-ext.x, 	x2-ext.y,  x3);
			glTexCoord2f(1, 1);		glVertex3i(x1, 			x2-ext.y,  x3);
			glTexCoord2f(1, 0);		glVertex3i(x1, 			x2-ext.y,  x3+ext.z);
			
			//left face
			//upper edge
			glNormal3f(1.0,1.0,0.0);
			glTexCoord2f(0.1, 0.99);	glVertex3i(x1, x2,  		x3+ext.z);	
			glTexCoord2f(0.1, 1);		glVertex3i(x1+DETAIL_OFFSET, x2-DETAIL_OFFSET, 	x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(0.11, 1);		glVertex3i(x1+DETAIL_OFFSET, x2-DETAIL_OFFSET,  	x3+DETAIL_OFFSET);
			glTexCoord2f(0.11, 0.99);	glVertex3i(x1, x2,  		x3);

			glNormal3f(1.0,0.0,0.0);
			glTexCoord2f(0, 0);		glVertex3i(x1+DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(0, 1);		glVertex3i(x1+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET, 	x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(1, 1);		glVertex3i(x1+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  	x3+DETAIL_OFFSET);
			glTexCoord2f(1, 0);		glVertex3i(x1+DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3+DETAIL_OFFSET);
		
			//lower edge
			glNormal3f(1.0,-1.0,0.0);
			glTexCoord2f(0.1, 0.99);	glVertex3i(x1+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  		x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);		glVertex3i(x1, x2-ext.y, 	x3+ext.z);
			glTexCoord2f(0.11, 1);		glVertex3i(x1, x2-ext.y,  	x3);
			glTexCoord2f(0.11, 0.99);	glVertex3i(x1+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  		x3+DETAIL_OFFSET);
		
			//right face
			//upper edge
			glNormal3f(-1.0,1.0,0.0);
			glTexCoord2f(0, 0.99);		glVertex3i(x1-ext.x, x2,  		x3);
			glTexCoord2f(0, 1);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-DETAIL_OFFSET,  x3+DETAIL_OFFSET);
			glTexCoord2f(0.01, 1);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-DETAIL_OFFSET,  x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(0.01,0.99);	glVertex3i(x1-ext.x, x2,  		x3+ext.z);
		
			glNormal3f(-1.0,0.0,0.0);
			glTexCoord2f(0, 0);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3+DETAIL_OFFSET);
			glTexCoord2f(0, 1);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3+DETAIL_OFFSET);
			glTexCoord2f(1, 1);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(1,0);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3+ext.z-DETAIL_OFFSET);
			
			//lower edge
			glNormal3f(-1.0,-1.0,0.0);
			glTexCoord2f(0.99, 0.99);	glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  		x3+DETAIL_OFFSET);
			glTexCoord2f(0.99, 1);		glVertex3i(x1-ext.x, x2-ext.y,  x3);
			glTexCoord2f(1, 1);			glVertex3i(x1-ext.x, x2-ext.y,  x3+ext.z);
			glTexCoord2f(1,0.99);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  		x3+ext.z-DETAIL_OFFSET);
	
			//back face
			//upper edge
			glNormal3f(0.0,1.0,1.0);
			glTexCoord2f(0.1, 0);		glVertex3i(x1-ext.x, 	x2,  		x3+ext.z);
			glTexCoord2f(0.1, 0.01);	glVertex3i(x1-ext.x+DETAIL_OFFSET, 	x2-DETAIL_OFFSET,  	x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0.11,0.01);	glVertex3i(x1-DETAIL_OFFSET, 			x2-DETAIL_OFFSET,  	x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0.11, 0);		glVertex3i(x1, 			x2,  		x3+ext.z);
					
			//lower edge
			glNormal3f(0.0,-1.0,1.0);
			glTexCoord2f(0.1, 0.99);	glVertex3i(x1-ext.x+DETAIL_OFFSET, 	x2-ext.y+DETAIL_OFFSET,  		x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);		glVertex3i(x1-ext.x, 	x2-ext.y,  	x3+ext.z);
			glTexCoord2f(0.11,1);		glVertex3i(x1, 			x2-ext.y,  	x3+ext.z);
			glTexCoord2f(0.11, 0.99);	glVertex3i(x1-DETAIL_OFFSET, 			x2-ext.y+DETAIL_OFFSET,  		x3+ext.z+DETAIL_OFFSET);
			
			//right edge
			glNormal3f(-1.0,0.0,1.0);
			glTexCoord2f(0, 0);		glVertex3i(x1-DETAIL_OFFSET, 	x2-DETAIL_OFFSET,  		x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(1, 0);		glVertex3i(x1-DETAIL_OFFSET, 	x2-ext.y+DETAIL_OFFSET,  	x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(1,0);		glVertex3i(x1+DETAIL_OFFSET, 		x2-ext.y+DETAIL_OFFSET,  	x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(0, 0);		glVertex3i(x1+DETAIL_OFFSET, 		x2-DETAIL_OFFSET,  		x3+ext.z-DETAIL_OFFSET);
		
		
			//left edge
			glNormal3f(1.0,0.0,1.0);	
			glTexCoord2f(0,0);		glVertex3i(x1-ext.x-DETAIL_OFFSET, 	x2-DETAIL_OFFSET,  		x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(1, 0);		glVertex3i(x1-ext.x-DETAIL_OFFSET, 	x2-ext.y+DETAIL_OFFSET,  	x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(1,0);		glVertex3i(x1-ext.x+DETAIL_OFFSET, 			x2-ext.y+DETAIL_OFFSET,  	x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0, 0);		glVertex3i(x1-ext.x+DETAIL_OFFSET, 			x2-DETAIL_OFFSET,  		x3+ext.z+DETAIL_OFFSET);
	
	
			//front face
			//upper edge
			glNormal3f(0.0,1.0,-1.0);
			glTexCoord2f(0.1, 0.99);		glVertex3i(x1, 		 x2,  	x3);
			glTexCoord2f(0.1, 1);	glVertex3i(x1-DETAIL_OFFSET, 		 x2-DETAIL_OFFSET, x3-DETAIL_OFFSET);
			glTexCoord2f(0.11, 1);	glVertex3i(x1-ext.x+DETAIL_OFFSET, x2-DETAIL_OFFSET,  x3-DETAIL_OFFSET);
			glTexCoord2f(0.11, 0.99);		glVertex3i(x1-ext.x, x2,  	x3);
			
		
			//lower edge
			glNormal3f(0.0,-1.0,-1.0);
			glTexCoord2f(0.1, 0.99);	glVertex3i(x1-DETAIL_OFFSET, 		 x2-ext.y+DETAIL_OFFSET,  		x3-DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);		glVertex3i(x1, 		 x2-ext.y,  x3);
			glTexCoord2f(0.11, 1);		glVertex3i(x1-ext.x, x2-ext.y,  x3);
			glTexCoord2f(0.11, 0.99);	glVertex3i(x1-ext.x+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  		x3-DETAIL_OFFSET);
		
			//left edge
			glNormal3f(1.0,0.0,-1.0);
			glTexCoord2f(0, 0);		glVertex3i(x1+DETAIL_OFFSET, 		 x2-DETAIL_OFFSET,  		x3+DETAIL_OFFSET);
			glTexCoord2f(1, 0);		glVertex3i(x1+DETAIL_OFFSET, 		 x2-ext.y+DETAIL_OFFSET,  x3+DETAIL_OFFSET);
			glTexCoord2f(1, 0);		glVertex3i(x1-DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3-DETAIL_OFFSET);
			glTexCoord2f(0, 0);		glVertex3i(x1-DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3-DETAIL_OFFSET);
		
			//right edge
			glNormal3f(-1.0,0.0,-1.0);
			glTexCoord2f(0,0);		glVertex3i(x1-ext.x+DETAIL_OFFSET, 		x2-DETAIL_OFFSET,  		x3-DETAIL_OFFSET);
			glTexCoord2f(1, 0);		glVertex3i(x1-ext.x+DETAIL_OFFSET, 		x2-ext.y+DETAIL_OFFSET,  x3-DETAIL_OFFSET);
			glTexCoord2f(1, 0);		glVertex3i(x1-ext.x-DETAIL_OFFSET, 	x2-ext.y+DETAIL_OFFSET,  x3+DETAIL_OFFSET);
			glTexCoord2f(0, 0);		glVertex3i(x1-ext.x-DETAIL_OFFSET, 	x2-DETAIL_OFFSET,  		x3+DETAIL_OFFSET);
		
		glEnd();
		
		
		glBindTexture(GL_TEXTURE_2D, anims[text][0]);
		glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0,0.0,-1.0);
			glTexCoord2f(0, 0.1);		glVertex3i(x1-DETAIL_OFFSET, 		 x2-DETAIL_OFFSET,  		x3-DETAIL_OFFSET);
			glTexCoord2f(0, 0.9);		glVertex3i(x1-DETAIL_OFFSET, 		 x2-ext.y+DETAIL_OFFSET,  x3-DETAIL_OFFSET);
			glTexCoord2f(1, 0.9);		glVertex3i(x1-ext.x+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3-DETAIL_OFFSET);
			glTexCoord2f(1, 0.1);		glVertex3i(x1-ext.x+DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3-DETAIL_OFFSET);
			
			//back face
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f(-1,0.1 );		glVertex3i(x1-ext.x+DETAIL_OFFSET, 	x2-DETAIL_OFFSET,  		x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(-1, 0.9);		glVertex3i(x1-ext.x+DETAIL_OFFSET, 	x2-ext.y+DETAIL_OFFSET,  	x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0,0.9);		glVertex3i(x1-DETAIL_OFFSET, 			x2-ext.y+DETAIL_OFFSET,  	x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0, 0.1);		glVertex3i(x1-DETAIL_OFFSET, 			x2-DETAIL_OFFSET,  		x3+ext.z+DETAIL_OFFSET);
	
		glEnd();
		
		glBindTexture(GL_TEXTURE_2D, anims[0][0]);
		//corners
		glBegin(GL_TRIANGLES);
			//front
			//upper-right corner
			glNormal3f(-1.0,1.0,-1.0);
			glTexCoord2f(0.11, 1);	glVertex3i(x1-ext.x+DETAIL_OFFSET, x2-DETAIL_OFFSET,  x3-DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);	glVertex3i(x1-ext.x-DETAIL_OFFSET, 		 x2-DETAIL_OFFSET, x3+DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);		glVertex3i(x1-ext.x, 		 x2,  	x3);
			
			//upper-left corner
			glNormal3f(1.0,1.0,-1.0);
			glTexCoord2f(0.11, 1);	glVertex3i(x1+DETAIL_OFFSET, x2-DETAIL_OFFSET,  x3+DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);	glVertex3i(x1-DETAIL_OFFSET, 		 x2-DETAIL_OFFSET, x3-DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);		glVertex3i(x1, 		 x2,  	x3);
			

			//lower-right corner
			glNormal3f(-1.0,-1.0,-1.0);
			glTexCoord2f(0.1, 1);	glVertex3i(x1-ext.x, 		 x2-ext.y,  	x3);
			glTexCoord2f(0.1, 1);		glVertex3i(x1-ext.x-DETAIL_OFFSET, 		 x2-ext.y+DETAIL_OFFSET, x3+DETAIL_OFFSET);
			glTexCoord2f(0.11, 1);		glVertex3i(x1-ext.x+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3-DETAIL_OFFSET);
			
			//lower-left corner
			glNormal3f(1.0,-1.0,-1.0);
			glTexCoord2f(0.1, 1);	glVertex3i(x1, 		 x2-ext.y,  	x3);
			glTexCoord2f(0.1, 1);		glVertex3i(x1-DETAIL_OFFSET, 		 x2-ext.y+DETAIL_OFFSET, x3-DETAIL_OFFSET);
			glTexCoord2f(0.11, 1);		glVertex3i(x1+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3+DETAIL_OFFSET);
		
	
	
	
			//back
			//upper left corner
			glNormal3f(1.0,1.0,1.0);
			glTexCoord2f(0.1, 1);		glVertex3i(x1-ext.x, 		 x2,  	x3+ext.z);
			glTexCoord2f(0.1, 1);	glVertex3i(x1-ext.x-DETAIL_OFFSET, 		 x2-DETAIL_OFFSET, x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(0.11, 1);	glVertex3i(x1-ext.x+DETAIL_OFFSET, x2-DETAIL_OFFSET,  x3+ext.z+DETAIL_OFFSET);
			
			//upper-right corner
			glNormal3f(-1.0,1.0,1.0);
			glTexCoord2f(0.1, 1);		glVertex3i(x1, 		 x2,  	x3+ext.z);
			glTexCoord2f(0.1, 1);	glVertex3i(x1-DETAIL_OFFSET, 		 x2-DETAIL_OFFSET, x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0.11, 1);	glVertex3i(x1+DETAIL_OFFSET, x2-DETAIL_OFFSET,  x3+ext.z-DETAIL_OFFSET);
			


			//lower-left corner
			glNormal3f(1.0,-1.0,1.0);
			glTexCoord2f(0.11, 1);		glVertex3i(x1-ext.x+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);		glVertex3i(x1-ext.x-DETAIL_OFFSET, 		 x2-ext.y+DETAIL_OFFSET, x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);	glVertex3i(x1-ext.x, 		 x2-ext.y,  	x3+ext.z);

	
			
			//lower-right corner
			glNormal3f(-1.0,-1.0,1.0);
			glTexCoord2f(0.1, 1);	glVertex3i(x1, 		 x2-ext.y,  	x3+ext.z);
			glTexCoord2f(0.11, 1);		glVertex3i(x1+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3+ext.z-DETAIL_OFFSET);
			glTexCoord2f(0.1, 1);		glVertex3i(x1-DETAIL_OFFSET, 		 x2-ext.y+DETAIL_OFFSET, x3+ext.z+DETAIL_OFFSET);	
		
		glEnd();
				
		
	}
	else//High detail cubes 
	{
		glBindTexture(GL_TEXTURE_2D, anims[0][0]);
		glBegin(GL_QUADS);
			//bottom face
			glNormal3f(0.0,-1.0,0.0);
			glTexCoord2f(0, 0);			glVertex3i(x1-ext.x, 	x2-ext.y,  x3+ext.z);
			glTexCoord2f(0, 1);			glVertex3i(x1-ext.x, 	x2-ext.y,  x3);
			glTexCoord2f(1, 1);			glVertex3i(x1, 			x2-ext.y,  x3);
			glTexCoord2f(1, 0);			glVertex3i(x1, 			x2-ext.y,  x3+ext.z);
		
			//left
			glNormal3f(1.0,0.0,0.0);
			glTexCoord2f(0, 0);			glVertex3i(x1+DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3+ext.z);
			glTexCoord2f(0, 1);			glVertex3i(x1+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET, 	x3+ext.z);
			glTexCoord2f(1, 1);			glVertex3i(x1+DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  	x3);
			glTexCoord2f(1, 0);			glVertex3i(x1+DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3);
		
		
			//right
			glNormal3f(-1.0,0.0,0.0);
			glTexCoord2f(0, 0);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3);
			glTexCoord2f(0, 1);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3);
			glTexCoord2f(1, 1);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-ext.y+DETAIL_OFFSET,  x3+ext.z);
			glTexCoord2f(1,0);		glVertex3i(x1-ext.x-DETAIL_OFFSET, x2-DETAIL_OFFSET,  		x3+ext.z);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, anims[text][0]);
		glBegin(GL_QUADS);	
			//back
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f(-1,0.1 );		glVertex3i(x1-ext.x, 	x2-DETAIL_OFFSET,  		x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(-1, 0.9);		glVertex3i(x1-ext.x, 	x2-ext.y+DETAIL_OFFSET,  	x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0,0.9);		glVertex3i(x1, 			x2-ext.y+DETAIL_OFFSET,  	x3+ext.z+DETAIL_OFFSET);
			glTexCoord2f(0, 0.1);		glVertex3i(x1, 			x2-DETAIL_OFFSET,  		x3+ext.z+DETAIL_OFFSET);
			
			//front
			glNormal3f(0.0,0.0,-1.0);
			glTexCoord2f(0, 0.1);		glVertex3i(x1, 		 x2-DETAIL_OFFSET,  		x3-DETAIL_OFFSET);
			glTexCoord2f(0, 0.9);		glVertex3i(x1, 		 x2-ext.y+DETAIL_OFFSET,  x3-DETAIL_OFFSET);
			glTexCoord2f(1, 0.9);		glVertex3i(x1-ext.x, x2-ext.y+DETAIL_OFFSET,  x3-DETAIL_OFFSET);
			glTexCoord2f(1, 0.1);		glVertex3i(x1-ext.x, x2-DETAIL_OFFSET,  		x3-DETAIL_OFFSET);
		glEnd();
		
		glBindTexture(GL_TEXTURE_2D, anims[0][0]);
		{
			//upper front edge
			Vertex left = {x1, x2-DETAIL_OFFSET,  x3};
			DrawEdge(0,0,0,left,ext.x,1);
		}
		
		{
			//lower front edge
			Vertex left = {x1,  x2-ext.y+DETAIL_OFFSET,  x3};
			DrawEdge(0,0,-1,left,ext.x,1);
		}
		{
			//upper back edge
			Vertex left = {x1, 	x2-DETAIL_OFFSET,  x3+ext.z};
			DrawEdge(0,0,1,left,ext.x,1);
		}
		{
			//lower back edge
			Vertex left = {x1, 	x2-ext.y+DETAIL_OFFSET,  x3+ext.z};
			DrawEdge(0,0,2,left,ext.x,1);
		}
		
		{
			//upper right edge
			Vertex left = {x1-ext.x, x2-DETAIL_OFFSET,  		x3};
			DrawEdge(1,0,0,left,ext.z,1);
		}
		{
			//lower right edge
			Vertex left = {x1-ext.x, x2-ext.y+DETAIL_OFFSET,  		x3};
			DrawEdge(1,1,0,left,ext.z,1);
		}
		
		{
			//upper left edge
			Vertex left = {x1, x2-DETAIL_OFFSET,  		x3+ext.z};
			DrawEdge(-1,0,0,left,ext.z,1);
		}
		{
			//lower right edge
			Vertex left = {x1, x2-ext.y+DETAIL_OFFSET,  		x3+ext.z};
			DrawEdge(-1,-1,0,left,ext.z,1);
		}
		
		{
			//front right edge
			Vertex left = {x1-ext.x, x2-DETAIL_OFFSET,  		x3};
			DrawEdge(0,1,0,left,ext.y-2*DETAIL_OFFSET,1);
		}
		
		{
			//front left edge
			Vertex left = {x1, x2-ext.y+DETAIL_OFFSET,    x3};
			DrawEdge(0,-1,0,left,ext.y-2*DETAIL_OFFSET,1);
		}
		
		{
			//back left edge
			Vertex left = {x1, x2-DETAIL_OFFSET,  		x3+ext.z};
			DrawEdge(2,-1,0,left,ext.y-2*DETAIL_OFFSET,1);
		}
		
		{
			//back right edge
			Vertex left = {x1-ext.x, x2-ext.y+DETAIL_OFFSET,    x3+ext.z};
			DrawEdge(2,1,0,left,ext.y-2*DETAIL_OFFSET,1);
		}
		
		{
			//upper right front 
			Vertex middle = {x1-ext.x, x2-DETAIL_OFFSET,  x3};
			DrawCorner(0,0,0,middle);
		}
		{
			//lower right front
			Vertex middle = {x1-ext.x, x2-ext.y+DETAIL_OFFSET,  x3};
			DrawCorner(0,1,0,middle);
		}
		
		{
			//upper left front
			Vertex middle = {x1, x2-DETAIL_OFFSET,  x3};
			DrawCorner(-1,0,0,middle);
		}
		
		{
			//lower left front
			Vertex middle = {x1, x2-ext.y+DETAIL_OFFSET,  x3};
			DrawCorner(0,2,0,middle);
		}
		
		{
			//lower left back
			Vertex middle = {x1, x2-ext.y+DETAIL_OFFSET,  x3+ext.z};
			DrawCorner(1,2,0,middle);
		}
		
		{
			//upper left back
			Vertex middle = {x1, x2-DETAIL_OFFSET,  x3+ext.z};
			DrawCorner(2,0,0,middle);
		}
		
		{
			//upper right back
			Vertex middle = {x1-ext.x, x2-DETAIL_OFFSET,  x3+ext.z};
			DrawCorner(1,0,0,middle);
		}
		
		{
			//lower right back
			Vertex middle = {x1-ext.x, x2-ext.y+DETAIL_OFFSET,  x3+ext.z};
			DrawCorner(-2,2,0,middle);
		}
		
		
	}
	
	
	glBindTexture(GL_TEXTURE_2D, anims[9][0]);
	glBegin(GL_QUADS);
			// top face
		glNormal3f(0.0,1.0,0.0);
		glTexCoord2f(0,0);		glVertex3i(x1, 			x2, 	x3+ext.z);
		glTexCoord2f(0, 1);		glVertex3i(x1, 			x2,  	x3);
		glTexCoord2f(1, 1);		glVertex3i(x1-ext.x, 	x2,  	x3);
		glTexCoord2f(1, 0);		glVertex3i(x1-ext.x, 	x2,  	x3+ext.z);
		
	glEnd();
	
}


void Menu::Render()
{
	
	gluLookAt(0,2000,-12000,	
  			0,0,0,				
  			0,1,0);				
	
	glColor3f(1.0f,1.0f,1.0f);
	
 	

	glRotated(rot_angle,0.0f,1.0f,0.0f);
	
	glBindTexture(GL_TEXTURE_2D, anims[25][0]);
	glBegin(GL_QUAD_STRIP);
		glTexCoord2f(0, 0);	glVertex3i(-15000,10000,-15000);
		glTexCoord2f(0, 1);	glVertex3i(-15000,-20000,-15000);
		
		glTexCoord2f(0.25, 0);	glVertex3i(15000,10000,-15000);
		glTexCoord2f(0.25, 1);	glVertex3i(15000,-20000,-15000);
		
		glTexCoord2f(0.5, 0);	glVertex3i(15000,10000,15000);
		glTexCoord2f(0.5, 1);	glVertex3i(15000,-20000,15000);
		
		glTexCoord2f(0.75, 0);	glVertex3i(-15000,10000,15000);
		glTexCoord2f(0.75, 1);	glVertex3i(-15000,-20000,15000);
		
		glTexCoord2f(1, 0);	glVertex3i(-15000,10000,-15000);
		glTexCoord2f(1, 1);	glVertex3i(-15000,-20000,-15000);
	glEnd();
	
		DrawSection(10);
		
	
	glRotated(60,0.0f,1.0f,0.0f);
		DrawSection(11);
	

	glRotated(60,0.0f,1.0f,0.0f);
		DrawSection(12);
		
	
	glRotated(60,0.0f,1.0f,0.0f);
		DrawSection(13);
	
	glRotated(60,0.0f,1.0f,0.0f);
		DrawSection(14);
	
	glRotated(60,0.0f,1.0f,0.0f);
		DrawSection(24);
	
	glRotated(-300-rot_angle,0.0f,1.0f,0.0f);
	PlayerModel(1,v_speed,oldPpos,Ppos,0,32,0);
		
	glEnable(GL_BLEND);		
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);											
	glBindTexture(GL_TEXTURE_2D,titles[0]);	
	glMatrixMode(GL_PROJECTION);								
	glPushMatrix();												
	glLoadIdentity();											
	gluOrtho2D( 0, set.width ,0, set.height);							
	glMatrixMode(GL_MODELVIEW);									
	glPushMatrix();												
		glLoadIdentity();	
		
		glBegin(GL_QUADS);
			glTexCoord2f(1,0.13);	glVertex2i((int)(0.9*set.width), (int)(0.842*set.height));
			glTexCoord2f(1,0.0);	glVertex2i((int)(0.9*set.width), (int)(0.942*set.height));
			glTexCoord2f(0,0.0);	glVertex2i((int)(0.1*set.width), (int)(0.942*set.height));
			glTexCoord2f(0,0.13);	glVertex2i((int)(0.1*set.width), (int)(0.842*set.height));	
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

	//printtxt2d((int)(0.28*set.width),(int)(set.height-0.09*set.height),2.5f,1.0f,1.0f,1.0f,"Orbit-Hopper");
	printtxt2d((int)(0.685*set.width),(int)(set.height-0.165*set.height),0.8f,1.0f,1.0f,1.0f,"Version 1.16c");
	printtxt2d((int)(0.8*set.width),20,0.5f,1.0f,1.0f,1.0f,"by Harald Obermaier");
	
	if(debugmsgs)
		DisplayDebug();	

}
