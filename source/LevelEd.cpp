/*
#				Orbit-Hopper
#				LevelEd.cpp
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
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <gl.h>	
#include <glu.h>

#include "../headers/LevelEd.h"
#include "../headers/Structs.h"
#include "../headers/Game.h"
#include "../headers/Font.h"
#include "../headers/Utils.h"
#include "../headers/Sound.h"


#ifndef WIN32
#include <dirent.h>
#endif

extern GLuint **anims;
extern Settings set;
extern bool blooming;


#define CA_MOVE	0x00000004		//moving floor

#define CE_TOUCH	0x00000010	//triggered on touch
#define CE_START	0x00000020	//triggered by start

//extras placed on floor, such as checkpoint, powerup...
#define CEX_POWERUP			0x80000000
#define CEX_LIFE			0x40000000
#define CEX_ENEMY			0x20000000
#define CEX_CHECKPOINT		0x10000000
#define CEX_PONG			0x08000000

void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b);
void PowerUp(int angle, Vertex pos, float r, float g, float b);
void DrawCheckPoint(int xleft,int xright,int y,int z, int playerz);
void DrawPongEnemy(int x,int y,int z);

LevelEd::LevelEd()
{
	file_num = 0;
	file_first = 0;
	file_sel = -1;
	controldown = false;
	selectdifficulty = 0;
	seconds = 0;
	view = false;
	lookat.x = lookat.y = lookat.z = start_resize.x = start_resize.y = start_resize.z= 0;
	cur_tilesel = NORMAL;
	num_tiles = 0;
	cur_floor = -1;
	floors = NULL;
	lmousebuttondown = false;
	drag = false;
	dirs[0] = dirs[1] = dirs[2] = dirs[3] = false;
	dialog = NON;
	files = NULL;
	ntype = -1;
	other = -1;
	movement = false;
	triggerline = 0;
	triggercol = 0;
	sel[0] = sel[1] = sel[2] = 0;
	strcpy(subdir,"");
	types = NULL;
	
}

LevelEd::~LevelEd()
{
	if(floors)
	{
		for(int i = 0;i<num_tiles;i++)
			delete floors[i];
		delete[] floors;
		floors = NULL;	
	}
	if(files)
	{
		delete[] files;
		files = NULL;
	}
	if(types)
	{
		delete[] types;
		types = NULL;
	}
}

void LevelEd::Update(unsigned int diff)
{
	static unsigned int framecount = 0;
	framecount += diff;
	if(framecount > 35)
	{
		if(!dialog)
		{
			if(dirs[0]) //move camera
			{
				if(!view)
				{
					lookat.z 		+= 2000;
					camera2.pos.z 	+= 2000;
				}
				else
				{
					lookat.y 		+= 1500;
					camera2.pos.y 	+= 1500;
				}
				
			}
			if(dirs[1]) //move camera
			{
				if(!view)
				{
					lookat.z 		-= 2000;
					camera2.pos.z 	-= 2000;	
				}
				else
				{
					lookat.y 		-= 1500;
					camera2.pos.y 	-= 1500;
				}
				
			}
			if(dirs[2])//move camera
			{
				if(!view)
				{
					lookat.x 		-= 2000;
					camera2.pos.x 	-= 2000;
				}
				else
				{
					lookat.z 		+= 1500;
					camera2.pos.z 	+= 1500;				
				}
				
			}
			if(dirs[3])//move camera
			{
				if(!view)
				{
					lookat.x 		+= 2000;
					camera2.pos.x 	+= 2000;		
				}
				else
				{
					lookat.z 		-= 1500;
					camera2.pos.z 	-= 1500;
				}
			}
		}
				
		framecount%=30;	
	}

}

#ifdef WIN32
	void LevelEd::GetFileList() //Windows
	{
		_finddata_t fileblock;
		
		file_num = 0;
		int dir_num = 0;
		char temp[128];
		if(strstr(subdir,"camp"))
		{
			strcpy(temp,"maps/");
			strcat(temp,subdir);
			strcat(temp,"/*");
		}
		else
			strcpy(temp,"maps/*");
		
		int file_save = _findfirst(temp,&fileblock);
		if(file_save==-1)
			return;
		if(strstr(fileblock.name,".slv") || (!strstr(fileblock.name,".") && strstr(fileblock.name,"camp")))	//map file or campaign directory
			file_num++;
		while(_findnext(file_save,&fileblock) != -1)
			if(strstr(fileblock.name,".slv") || (!strstr(fileblock.name,".") && strstr(fileblock.name,"camp")))
				file_num++;
		
		_findclose(file_save);
		
		if(strstr(subdir,"camp"))
			file_num++;
			
		if(files)
		{
			delete[] files;
			files = NULL;
		}
		files = new char*[file_num];
		for(int i = 0;i<file_num;i++)
			files[i] = new char[32];
	
		
		
		file_save = _findfirst(temp,&fileblock);
		int i = 0;
		if(strstr(subdir,"camp"))
		{
			strncpy(files[0],"..",32);
			i = 1;
		}	
		if(strstr(fileblock.name,".slv") || (!strstr(fileblock.name,".") && strstr(fileblock.name,"camp")))
		{
			strncpy(files[i],fileblock.name,32);
			i++;	
		}
		
		while(i < file_num)
		{
			if(_findnext(file_save,&fileblock) == -1)
				break;
			if(strstr(fileblock.name,".slv") || (!strstr(fileblock.name,".") && strstr(fileblock.name,"camp")))
			{
				if(!strstr(fileblock.name,"."))
					dir_num++;
				strncpy(files[i],fileblock.name,32);
				i++;
			}
		}
		_findclose(file_save);
		
		
		//sort maps to ensure campaign difficulty rises smoothly
		for(int i = dir_num; i < file_num; i++)
		{
			for(int u = i+1; u < file_num; u++)
			{
				for(int k = 0; files[i][k]; k++)
				{
					if(files[u])
						if(files[i][k] != files[u][k])
						{
							if(files[i][k] > files[u][k])
							{
								char swap[32];
								strncpy(swap,files[u],32);
								strncpy(files[u],files[i],32);
								strncpy(files[i],swap,32);
							}
							break;
						}		
				}
				
			}
		}

		//put directories at the top of our list
		for(int i = 0; i < dir_num; i++)
		{
			for(int u = i+1; u < file_num; u++)
			{
				if(files[u])
					if(files[i]!= files[u])
					{
						if(!strstr(files[u],".") && strstr(files[i],".") && !strstr(files[i],".."))
						{
							char swap[32];
							strncpy(swap,files[u],32);
							strncpy(files[u],files[i],32);
							strncpy(files[i],swap,32);
						}
					}		
				
			}
		}
		if(file_sel == -1)
		{
			file_sel = 0;
			strncpy(filename,files[file_sel],32);
		}
	
	}
#else

	void LevelEd::GetFileList() //linux
	{
		char temp[128];
		strcpy(temp,"maps/");
		if(strstr(subdir,"camp"))
		{
			strcat(temp,subdir);
			strcat(temp,"/");
		}
		
			
		DIR *dir = opendir(temp);
		file_num = 0;
		int dir_num = 0;
		dirent *entry = readdir(dir);
		
			
		while(entry != NULL)
		{
			if(strstr(entry->d_name,".slv") || (!strstr(entry->d_name,".") && strstr(entry->d_name,"camp")))
				file_num++;
			entry = readdir(dir);
		}
	
		closedir(dir);
		
		if(strstr(subdir,"camp"))
			file_num++;
			
		if(files)
		{
			delete[] files;
			files = NULL;
		}
		files = new char*[file_num];
		for(int i = 0;i<file_num;i++)
			files[i] = new char[32];
	
	
		
		dir = opendir(temp);
		
		entry = readdir(dir);
		
		int i = 0;
		if(strstr(subdir,"camp"))
		{
			strncpy(files[0],"..",32);
			i++;
		}
		while(entry != NULL && i < file_num)
		{
			if(strstr(entry->d_name,".slv") || (!strstr(entry->d_name,".") && strstr(entry->d_name,"camp")))
			{
				strncpy(files[i],entry->d_name,32);
				i++;
			}
	
			entry = readdir(dir);
		}
	
		closedir(dir);
		
		
		//sort maps to ensure campaign difficulty rises smoothly
		for(int i = dir_num; i < file_num; i++)
		{
			for(int u = i+1; u < file_num; u++)
			{
				for(int k = 0; files[i][k]; k++)
				{
					if(files[u])
						if(files[i][k] != files[u][k])
						{
							if(files[i][k] > files[u][k])
							{
								char swap[32];
								strncpy(swap,files[u],32);
								strncpy(files[u],files[i],32);
								strncpy(files[i],swap,32);
							}
							break;
						}		
				}
				
			}
		}

		//place directories at the top of our list
		for(int i = 0; i < dir_num; i++)
		{
			for(int u = i+1; u < file_num; u++)
			{
				if(files[u])
					if(files[i]!= files[u])
					{
						if(!strstr(files[u],".") && strstr(files[i],".") && !strstr(files[i],".."))
						{
							char swap[32];
							strncpy(swap,files[u],32);
							strncpy(files[u],files[i],32);
							strncpy(files[i],swap,32);
						}
					}		
				
			}
		}

		if(file_sel == -1)
		{
			file_sel = 0;
			if(file_num > 0)
				strncpy(filename,files[file_sel],32);
		}	
	
	}
#endif

void LevelEd::InitCamera()
{
	if(view)
	{	
		camera2.pos.x = -56000;
		camera2.pos.y = 0;
		camera2.pos.z = 0;
		camera2.target = &lookat;
		camera2.up.x = 0;
		camera2.up.y = 1;
		camera2.up.z = 0;
	}
	else
	{
		camera2.pos.x = 0;
		camera2.pos.y = 48000;
		camera2.pos.z = 0;
		camera2.target = &lookat;
		camera2.up.x = 0;
		camera2.up.y = 0;
		camera2.up.z = 1;
	}
	camera2.offsets.x = camera2.offsets.y = camera2.offsets.z = 0;
}

void LevelEd::Init()
{
	
	SDL_ShowCursor(true); //we need our mouse cursor to show
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40,(float)(set.width)/(float)(set.height),50.5f,400000.0f);
	glMatrixMode(GL_MODELVIEW);		
	
	lookat.x = lookat.y = lookat.z = 0;
	view = false; //default: bird's eye view
	InitCamera();
	time_sel = 0;
	digit = 0;
	time1 = 0;
	time2 = 0;
	time3 = 0;
	cur_tilesel = NORMAL;
	num_tiles = 0;
	cur_floor = -1;
	lmousebuttondown = false;
	drag = false; 
	start_resize.x = start_resize.y = start_resize.z = 0;
	controldown = false;
	seconds = 0;
	other = -1;
	dirs[0] = dirs[1] = dirs[2] = dirs[3] = false;
	dialog = NON;
	triggerline = 0;
	triggercol = 0;
	sel[0] = sel[1] = sel[2] = 0;
	ntype = -1;
	movement = false;
	if(types)
	{
		delete[] types;
		types = NULL;	
	}
	types = new char*[8];
	for(int i = 0;i<8;i++)
		types[i] = new char[6];
		
	//types a floor can change to
	strcpy(types[0],"SPEED");
	strcpy(types[1],"SLOW ");
	strcpy(types[2],"LEFT ");
	strcpy(types[3],"RIGHT");
	strcpy(types[4],"HURT ");
	strcpy(types[5],"ICE  ");
	strcpy(types[6],"CRAZY");
	strcpy(types[7],"HEAL ");
	GetFileList();
	
}


void DrawButton()
{
	glDisable(GL_LIGHTING);
	glBegin(GL_QUADS);	
		glTexCoord2f(0, 0);				glVertex2f(0, 0);
		glTexCoord2f(0, -1.0f);			glVertex2f(0, 0.03516*set.height);
		glTexCoord2f(-1.0f,-1.0f);		glVertex2f(-0.0264*set.width, 0.03516*set.height);
		glTexCoord2f(-1.0f, 0);			glVertex2f(-0.0264*set.width, 0);						
	glEnd();
	
}

void LevelEd::RenderGUI()
{
	
	if(cur_floor != -1)
	{
		char temp[16];
		sprintf(temp,"%d",cur_floor);
		if(floors[cur_floor]->cube->triggerflag)
			printtxt2d((int)(0.5*set.width),(int)(0.981*set.height),1.0f,1.0f,0.0f,0.0f,temp);
		else
		{
			int i = 0;
			for(i=0;i<num_tiles;i++)
				if(OTHERINDEX(floors[i]->cube->triggerflag) == cur_floor && cur_floor)
					break;
			if(i == num_tiles)
				printtxt2d((int)(0.5*set.width),(int)(0.981*set.height),1.0f,1.0f,1.0f,1.0f,temp);
			else
				printtxt2d((int)(0.5*set.width),(int)(0.981*set.height),1.0f,1.0f,0.0f,0.0f,temp);
		}
		glColor3f(1.0f,1.0f,1.0f);
		
	}
	glDisable(GL_DEPTH_TEST);							
		glMatrixMode(GL_PROJECTION);						
		glPushMatrix();									
		glLoadIdentity();									
		gluOrtho2D(0,set.width,0,set.height);							
		glMatrixMode(GL_MODELVIEW);						
		glPushMatrix();									
			glLoadIdentity();	
								
			glTranslated(0.97*(float)(set.width),0.93*(float)(set.height),0);	
			glBindTexture(GL_TEXTURE_2D, anims[16][0]);//button to change perspective
			DrawButton();	
			
			glTranslated(0,-0.053*(float)(set.height),0);	
			glBindTexture(GL_TEXTURE_2D, anims[36][0]);//New
			DrawButton();	
			
			glTranslated(0,-0.053*(float)(set.height),0);	
			glBindTexture(GL_TEXTURE_2D, anims[18][0]);//save
			DrawButton();	
			
			glTranslated(0,-0.053*(float)(set.height),0);	
			glBindTexture(GL_TEXTURE_2D, anims[19][0]);//load
			DrawButton();	
			
			if(!view)
			{
				glTranslated(0,-0.07*(float)(set.height),0);
				if(cur_tilesel == NORMAL)
					glTranslated(0.01*(float)(set.width),0,0);
				glBindTexture(GL_TEXTURE_2D, anims[0][0]);//normal
				DrawButton();	
				if(cur_tilesel == NORMAL)
					glTranslated(-0.01*(float)(set.width),0,0);
					
					
				glTranslated(0,-0.0598*(float)(set.height),0);	
				if(cur_tilesel == SPEED_UP)
					glTranslated(0.01*(float)(set.width),0,0);
				glBindTexture(GL_TEXTURE_2D, anims[1][0]);//speed up
				DrawButton();	
				if(cur_tilesel == SPEED_UP)
					glTranslated(-0.01*(float)(set.width),0,0);
					
					
				glTranslated(0,-0.0598*(float)(set.height),0);	
				if(cur_tilesel == SLOW_DOWN)
					glTranslated(0.01*(float)(set.width),0,0);
				glBindTexture(GL_TEXTURE_2D, anims[2][0]);//slow
				DrawButton();	
				if(cur_tilesel == SLOW_DOWN)
					glTranslated(-0.01*(float)(set.width),0,0);
					
					
				glTranslated(0,-0.0598*(float)(set.height),0);	
				if(cur_tilesel == LEFT_PUSH)
					glTranslated(0.01*(float)(set.width),0,0);
				glBindTexture(GL_TEXTURE_2D, anims[3][0]);//left
				DrawButton();	
				if(cur_tilesel == LEFT_PUSH)
					glTranslated(-0.01*(float)(set.width),0,0);
					
					
				glTranslated(0,-0.0598*(float)(set.height),0);
				if(cur_tilesel == RIGHT_PUSH)
					glTranslated(0.01*(float)(set.width),0,0);	
				glBindTexture(GL_TEXTURE_2D, anims[4][0]);//right
				DrawButton();	
				if(cur_tilesel == RIGHT_PUSH)
					glTranslated(-0.01*(float)(set.width),0,0);
					
						
				glTranslated(0,-0.0598*(float)(set.height),0);
				if(cur_tilesel == HURT)
					glTranslated(0.01*(float)(set.width),0,0);	
				glBindTexture(GL_TEXTURE_2D, anims[5][0]);//hurt
				DrawButton();	
				if(cur_tilesel == HURT)
					glTranslated(-0.01*(float)(set.width),0,0);
					
					
				glTranslated(0,-0.0598*(float)(set.height),0);
				if(cur_tilesel == ICE)
					glTranslated(0.01*(float)(set.width),0,0);	
				glBindTexture(GL_TEXTURE_2D, anims[6][0]);//ice
				DrawButton();	
				if(cur_tilesel == ICE)
					glTranslated(-0.01*(float)(set.width),0,0);	
				
					
				glTranslated(0,-0.0598*(float)(set.height),0);	
				if(cur_tilesel == CRAZY)
					glTranslated(0.01*(float)(set.width),0,0);
				glBindTexture(GL_TEXTURE_2D, anims[15][0]);//crazy
				DrawButton();	
				if(cur_tilesel == CRAZY)
					glTranslated(-0.01*(float)(set.width),0,0);	
				
					
				glTranslated(0,-0.0598*(float)(set.height),0);
				if(cur_tilesel == HEAL)
					glTranslated(0.01*(float)(set.width),0,0);	
				glBindTexture(GL_TEXTURE_2D, anims[20][0]);//heal
				DrawButton();	
				if(cur_tilesel == HEAL)
					glTranslated(-0.01*(float)(set.width),0,0);
				
					
				glTranslated(0,-0.0598*(float)(set.height),0);	
				if(cur_tilesel == JUMP)
					glTranslated(0.01*(float)(set.width),0,0);
				glBindTexture(GL_TEXTURE_2D, anims[21][0]);//jump
				DrawButton();	
				if(cur_tilesel == JUMP)
					glTranslated(-0.01*(float)(set.width),0,0);	
				
					
				glTranslated(0,-0.0598*(float)(set.height),0);	
				if(cur_tilesel == GOAL)
					glTranslated(0.01*(float)(set.width),0,0);
				glBindTexture(GL_TEXTURE_2D, anims[26][0]);//goal
				DrawButton();	
				if(cur_tilesel == GOAL)
					glTranslated(-0.01*(float)(set.width),0,0);	
					
					
				glTranslated(0,-0.0598*(float)(set.height),0);	
				if(cur_tilesel == SAND)
					glTranslated(0.01*(float)(set.width),0,0);
				glBindTexture(GL_TEXTURE_2D, anims[37][0]);//goal
				DrawButton();	
				if(cur_tilesel == SAND)
					glTranslated(-0.01*(float)(set.width),0,0);	
					
					
				if(cur_tilesel == 12)
					glTranslated(0.01*(float)(set.width),0,0);
				glTranslated(-0.924*(float)(set.width),0.882*(float)(set.height),0);
				glBindTexture(GL_TEXTURE_2D, anims[32][0]);//singleplayer icons
				glBegin(GL_QUADS); //checkpoint
							glTexCoord2f(1, 1);				glVertex2f(0, 0);
							glTexCoord2f(1, 0.49f);			glVertex2f(0, 0.03616*set.height);
							glTexCoord2f(0.49f,0.49f);		glVertex2f(-0.0274*set.width, 0.03616*set.height);
							glTexCoord2f(0.49f, 1);			glVertex2f(-0.0274*set.width, 0);		
				glEnd();
				if(cur_tilesel == 12)
					glTranslated(-0.01*(float)(set.width),0,0);
				
				if(cur_tilesel == 13)
					glTranslated(0.01*(float)(set.width),0,0);
				glTranslated(0,-0.0598*(float)(set.height),0);	
				glBegin(GL_QUADS);//pong
							glTexCoord2f(0.49f, 0.49f);			glVertex2f(0, 0);
							glTexCoord2f(0.49f, 0);				glVertex2f(0, 0.03616*set.height);
							glTexCoord2f(0,0);					glVertex2f(-0.0274*set.width, 0.03616*set.height);
							glTexCoord2f(0, 0.49f);				glVertex2f(-0.0274*set.width, 0);		
				glEnd();
				if(cur_tilesel == 13)
					glTranslated(-0.01*(float)(set.width),0,0);	
				
				if(cur_tilesel == 14)
					glTranslated(0.01*(float)(set.width),0,0);
				glTranslated(0,-0.0598*(float)(set.height),0);	
				glBegin(GL_QUADS);//life
							glTexCoord2f(1, 0.49f);				glVertex2f(0, 0);
							glTexCoord2f(1, 0);					glVertex2f(0, 0.03616*set.height);
							glTexCoord2f(0.49f,0);				glVertex2f(-0.0274*set.width, 0.03616*set.height);
							glTexCoord2f(0.49f, 0.49f);			glVertex2f(-0.0274*set.width, 0);		
				glEnd();	
				if(cur_tilesel == 14)
					glTranslated(-0.01*(float)(set.width),0,0);
				
				if(cur_tilesel == 15)
					glTranslated(0.01*(float)(set.width),0,0);
				glTranslated(0,-0.0598*(float)(set.height),0);	
				glBegin(GL_QUADS);//enemy
							glTexCoord2f(0, 1.0f);		glVertex2f(0, 0);
							glTexCoord2f(0, 0.49f);		glVertex2f(0, 0.03616*set.height);
							glTexCoord2f(0.49f,0.49f);	glVertex2f(-0.0274*set.width, 0.03616*set.height);
							glTexCoord2f(0.49f,1.0f);	glVertex2f(-0.0274*set.width, 0);		
				glEnd();
				if(cur_tilesel == 15)
					glTranslated(-0.01*(float)(set.width),0,0);		
					
					
				glTranslated(0,-0.0598*(float)(set.height),0);	
				if(cur_tilesel == 20)
					glTranslated(0.01*(float)(set.width),0,0);
				glBindTexture(GL_TEXTURE_2D, anims[17][0]);//star
				DrawButton();	
				if(cur_tilesel == 20)
					glTranslated(-0.01*(float)(set.width),0,0);
					
				
			}
		glMatrixMode(GL_PROJECTION);						
		glPopMatrix();										
		glMatrixMode(GL_MODELVIEW);							
		glPopMatrix();										
	glEnable(GL_DEPTH_TEST);
	
		
}

void LevelEd::Render()
{
	glDisable(GL_LIGHTING);
	gluLookAt(camera2.pos.x,camera2.pos.y,camera2.pos.z,																//where we are
  			camera2.target->x+camera2.offsets.x,camera2.target->y+camera2.offsets.y,camera2.target->z+camera2.offsets.z,	//where we look at
  			camera2.up.x,camera2.up.y,camera2.up.z);
  		
  	Tile **k = &floors[0];	
  	Tile **max = k+num_tiles;	

	//draw floors
	while(k<max)
	{	
		if(k==&floors[0]+cur_floor) 
			glColor3f(0.5f,0.5f,0.5f);
			
		(*k)->cube->Draw(false);	
		
		if(k==&floors[0]+cur_floor) 
			glColor3f(1.0f,1.0f,1.0f);
				
		k++;
	}
	
	
	glEnable(GL_BLEND);
	glColor3f(0.4f,0.5f,0.4f);
	
	k = &floors[0];
	while(k<max)	
	{	
		if((*k)->cube->flag1 && A_MOVE((*k)->cube->triggerflag))//moving floor
		{
		
			Vertex orig = {(*k)->cube->flag1,(*k)->cube->flag2,(*k)->cube->flag3};
			
			//draw alternative position
			Cube(orig,(*k)->cube->ext,(*k)->cube->tiled,(*k)->type).Draw(false);
			
			//draw movement-lines
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINES);
				glVertex3i((*k)->cube->origin.x-(*k)->cube->ext.x,(*k)->cube->origin.y-(*k)->cube->ext.y,(*k)->cube->origin.z+(*k)->cube->ext.z);
				glVertex3i((*k)->cube->flag1-(*k)->cube->ext.x,(*k)->cube->flag2-(*k)->cube->ext.y,(*k)->cube->flag3+(*k)->cube->ext.z);
					
				glVertex3i((*k)->cube->origin.x-(*k)->cube->ext.x,(*k)->cube->origin.y-(*k)->cube->ext.y,(*k)->cube->origin.z);
				glVertex3i((*k)->cube->flag1-(*k)->cube->ext.x,(*k)->cube->flag2-(*k)->cube->ext.y,(*k)->cube->flag3);
				
				glVertex3i((*k)->cube->origin.x,(*k)->cube->origin.y,(*k)->cube->origin.z);
				glVertex3i((*k)->cube->flag1,(*k)->cube->flag2,(*k)->cube->flag3);
					
				glVertex3i((*k)->cube->origin.x,(*k)->cube->origin.y-(*k)->cube->ext.y,(*k)->cube->origin.z+(*k)->cube->ext.z);
				glVertex3i((*k)->cube->flag1,(*k)->cube->flag2-(*k)->cube->ext.y,(*k)->cube->flag3+(*k)->cube->ext.z);
					
			glEnd();
			glEnable(GL_TEXTURE_2D);
		}
		else if(EX_POWERUP((*k)->cube->triggerflag)) //power up
		{
			Vertex ppos = {(*k)->cube->flag1,(*k)->cube->origin.y+300,(*k)->cube->flag2};
			
			glDisable(GL_BLEND);
			PowerUp(0,ppos,1.0,1.0,0.0);
			glEnable(GL_BLEND);
		}
		else if(EX_CHECKPOINT((*k)->cube->triggerflag)) //checkpoint
		{
			
			glDisable(GL_BLEND);
			DrawCheckPoint((*k)->cube->origin.x,(*k)->cube->origin.x-(*k)->cube->ext.x,(*k)->cube->origin.y,(*k)->cube->flag2,-100000);
			glEnable(GL_BLEND);
		}
		else if(EX_PONG( (*k)->cube->triggerflag)) //pong
		{
			
			glDisable(GL_BLEND);
			DrawPongEnemy((*k)->cube->flag1,(*k)->cube->origin.y,(*k)->cube->flag2);
			glEnable(GL_BLEND);
		}
		else if(EX_LIFE((*k)->cube->triggerflag)) //life
		{
			
			glDisable(GL_BLEND);
			Vertex pos = {(*k)->cube->flag1,(*k)->cube->origin.y+200,(*k)->cube->flag2};
			PowerUp(0,pos,0.6,0.0,0.0);
			glEnable(GL_BLEND);
		}
		else if(EX_ENEMY((*k)->cube->triggerflag)) //enemy
		{
			
			glDisable(GL_BLEND);
			Vertex pos = {(*k)->cube->flag1,(*k)->cube->origin.y+200,(*k)->cube->flag2};
			PlayerModel(1,0,pos,pos,0.0,8,0);
			glEnable(GL_BLEND);
		}
		k++;
	}
	glColor3f(1.0f,1.0f,1.0f);
	
	glDisable(GL_BLEND);
	
	//display dialogs
	char text[64];
	int y = 0;
	float g = 1.0f;
	int i = 0;
	int h = 0;
	switch(dialog)
	{
		case LOAD:
		case SAVE:
			y = (int)(0.78125*set.height);
			for(int u = file_first;u < file_num && u<=file_first+5 ;u++,y-=(int)(0.03947*set.height))
			{
				if(u == file_sel)
					g = 0.0f;
				else
					g = 1.0f;
				printtxt2d((int)(0.28*set.width),y,1.0f,1.0f,g,1.0f,files[u]);
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
				if(file_first+5 >= file_num-1)
					glColor3f(0.2,0.2,0.2);
				else
					glColor3f(1.0,1.0,1.0);


					glBegin(GL_QUADS);
			
						glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.7*set.width), (int)(0.55*set.height));
						glTexCoord2f(0.47,0.7);		glVertex2i((int)(0.7*set.width), (int)(0.585*set.height));
						glTexCoord2f(0.40,0.7);		glVertex2i((int)(0.688*set.width), (int)(0.585*set.height));
						glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.688*set.width), (int)(0.55*set.height));	
										
					glEnd();
		
				if(file_first <= 0)
					glColor3f(0.2,0.2,0.2);
				else
					glColor3f(1.0,1.0,1.0);

					glBegin(GL_QUADS);
			
						glTexCoord2f(0.47,0.7);	glVertex2i((int)(0.7*set.width), (int)(0.765*set.height));
						glTexCoord2f(0.47,0.1);		glVertex2i((int)(0.7*set.width), (int)(0.8*set.height));
						glTexCoord2f(0.40,0.1);		glVertex2i((int)(0.688*set.width), (int)(0.8*set.height));
						glTexCoord2f(0.40,0.7);	glVertex2i((int)(0.688*set.width), (int)(0.765*set.height));	
										
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


			
			printtxt2d((int)(0.28*set.width),(int)(0.521*set.height),1.0f,1.0f,1.0f,1.0f,"Enter filename:");
			snprintf(text,31,"%s",filename);
			printtxt2d((int)(0.28*set.width),(int)(0.48*set.height),1.0f,1.0f,1.0f,1.0f,text);

			DrawRectangle((int)(0.23*set.width),(int)(0.807*set.height),(int)(0.516*set.width),(int)(0.382*set.height),1.0f,1.0f,1.0f);
		break;
			
		case TIMES:
			
			
			printtxt2d((int)(0.35*set.width),(int)(0.781*set.height),1.0f,1.0f,g,1.0f,"Please enter times:");
			printtxt2d((int)(0.27*set.width),(int)(0.707*set.height),1.0f,0.5,0.3,0.3,"Bronze Medal: ");
			printtxt2d((int)(0.27*set.width),(int)(0.647*set.height),1.0f,0.7,0.7,0.7,"Silver Medal: ");
			printtxt2d((int)(0.27*set.width),(int)(0.587*set.height),1.0f,1.0,0.85,0.0,"Gold Medal: ");
			
			if(!time_sel)
			{
				sprintf(text,"%02d: %02d: %02d",time3/60000,(time3%60000)/1000,(time3%1000)/10);
				printtxt2d((int)(0.56*set.width),(int)(0.707*set.height),1.0f,1.0f,0.0f,1.0f,text);
			}
			else
			{
				sprintf(text,"%02d: %02d: %02d",time3/60000,(time3%60000)/1000,(time3%1000)/10);
				printtxt2d((int)(0.56*set.width),(int)(0.707*set.height),1.0f,1.0f,g,1.0f,text);
			}

			if(time_sel == 1)
			{
				sprintf(text,"%02d: %02d: %02d",time2/60000,(time2%60000)/1000,(time2%1000)/10);
				printtxt2d((int)(0.56*set.width),(int)(0.647*set.height),1.0f,1.0f,0.0f,1.0f,text);
			}
			else
			{
				sprintf(text,"%02d: %02d: %02d",time2/60000,(time2%60000)/1000,(time2%1000)/10);
				printtxt2d((int)(0.56*set.width),(int)(0.647*set.height),1.0f,1.0f,g,1.0f,text);
			}
			
			if(time_sel == 2)
			{
				sprintf(text,"%02d: %02d: %02d",time1/60000,(time1%60000)/1000,(time1%1000)/10);
				printtxt2d((int)(0.56*set.width),(int)(0.587*set.height),1.0f,1.0f,0.0f,1.0f,text);
			}
			else
			{
				sprintf(text,"%02d: %02d: %02d",time1/60000,(time1%60000)/1000,(time1%1000)/10);
				printtxt2d((int)(0.56*set.width),(int)(0.587*set.height),1.0f,1.0f,g,1.0f,text);
			}
			

			
			DrawRectangle((int)(0.23*set.width),(int)(0.807*set.height),(int)(0.516*set.width),(int)(0.339*set.height),1.0f,1.0f,1.0f);
		break;
		case MOVE:
		
			sprintf(text,"Triggered by:");
			if(!triggercol)
				printtxt2d((int)(0.19*set.width),(int)(0.781*set.height),1.0f,1.0f,0.0f,1.0f,text);
			else
				printtxt2d((int)(0.19*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,text);
		
			strcpy(text,"Other: ");
			
			for(i=0;i<num_tiles;i++)
				if(OTHERINDEX(floors[i]->cube->triggerflag) == cur_floor  && cur_floor)
				{
					if(strlen(text) < 30)
					{
						if(h != i)
							strcat(text,",");
						char b[2];
						sprintf(b,"%d",i);
						
						strcat(text,b);
					}
					else
						break;
				}
				else
					h++;
			
			if(h < num_tiles)	//floor is triggered by other floor
			{
				
				printtxt2d((int)(0.19*set.width),(int)(0.741*set.height),0.8f,0.7f,0.7f,0.7f,"Start");
				printtxt2d((int)(0.19*set.width),(int)(0.711*set.height),0.8f,0.7f,0.7f,0.7f,"Touch");
				
				//sprintf(text,"Other: %d",i);
				printtxt2d((int)(0.19*set.width),(int)(0.681*set.height),0.8f,0.7f,0.7f,0.7f,text);
				
			}
			else
			{
				if(sel[0] == 0)
					printtxt2d((int)(0.19*set.width),(int)(0.741*set.height),0.8f,1.0f,0.0f,1.0f,"Start");
				else
					printtxt2d((int)(0.19*set.width),(int)(0.741*set.height),0.8f,1.0f,1.0f,1.0f,"Start");
				if(sel[0] == 1)
					printtxt2d((int)(0.19*set.width),(int)(0.711*set.height),0.8f,1.0f,0.0f,1.0f,"Touch");
				else
					printtxt2d((int)(0.19*set.width),(int)(0.711*set.height),0.8f,1.0f,1.0f,1.0f,"Touch");
				sprintf(text,"Other");
				
				
				printtxt2d((int)(0.19*set.width),(int)(0.681*set.height),0.8f,0.7f,0.7f,0.7f,text);
				
					
			}
			
			
			sprintf(text,"Action:");
			if(triggercol == 1)
				printtxt2d((int)(0.43*set.width),(int)(0.781*set.height),1.0f,1.0f,0.0f,1.0f,text);
			else
				printtxt2d((int)(0.43*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,text);
			if(A_MOVE(floors[cur_floor]->cube->triggerflag))
			{
				printtxt2d((int)(0.43*set.width),(int)(0.741*set.height),0.8f,0.7f,0.7f,0.7f,"None");
				sprintf(text,"Move: %d",seconds);
				if(movement)
					strcat(text,"  1");
				else
					strcat(text," oo");
				if(sel[1] == 1)
					printtxt2d((int)(0.43*set.width),(int)(0.711*set.height),0.8f,1.0f,0.0f,1.0f,text);
				else
					printtxt2d((int)(0.43*set.width),(int)(0.711*set.height),0.8f,1.0f,1.0f,1.0f,text);
					
				sprintf(text,"Change Type:");
				printtxt2d((int)(0.43*set.width),(int)(0.681*set.height),0.8f,0.7f,0.7f,0.7f,text);
			}
			else
			{
				if(sel[1] == 0)
					printtxt2d((int)(0.43*set.width),(int)(0.741*set.height),0.8f,1.0f,0.0f,1.0f,"None");
				else
					printtxt2d((int)(0.43*set.width),(int)(0.741*set.height),0.8f,1.0f,1.0f,1.0f,"None");
					
				printtxt2d((int)(0.43*set.width),(int)(0.711*set.height),0.8f,0.7f,0.7f,0.7f,"Move");
				if(ntype != -1)
					sprintf(text,"Change Type: %s",types[ntype]);
				else
					sprintf(text,"Change Type: None");
				if(sel[1] == 2)
					printtxt2d((int)(0.43*set.width),(int)(0.681*set.height),0.8f,1.0f,0.0f,1.0f,text);
				else
					printtxt2d((int)(0.43*set.width),(int)(0.681*set.height),0.8f,1.0f,1.0f,1.0f,text);
			}
			
			
			sprintf(text,"Trigger:");
			if(triggercol == 2)
				printtxt2d((int)(0.68*set.width),(int)(0.781*set.height),1.0f,1.0f,0.0f,1.0f,text);
			else
				printtxt2d((int)(0.68*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,text);
			if(sel[2] == 0)
				printtxt2d((int)(0.68*set.width),(int)(0.741*set.height),0.8f,1.0f,0.0f,1.0f,"None");
			else
				printtxt2d((int)(0.68*set.width),(int)(0.741*set.height),0.8f,1.0f,1.0f,1.0f,"None");
				
			sprintf(text,"Other: %d",other);
			if(sel[2] == 2)
				printtxt2d((int)(0.68*set.width),(int)(0.681*set.height),0.8f,1.0f,0.0f,1.0f,text);
			else
				printtxt2d((int)(0.68*set.width),(int)(0.681*set.height),0.8f,1.0f,1.0f,1.0f,text);
			
			DrawRectangle((int)(0.15*set.width),(int)(0.807*set.height),(int)(0.7*set.width),(int)(0.17*set.height),1.0f,1.0f,1.0f);
		
		
		break;
		
		case ENEMY:
		case PONG:
		case LIFE:
			
			printtxt2d((int)(0.15*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,"Select difficulty: ");
			
			if(!selectdifficulty)
				printtxt2d((int)(0.16*set.width),(int)(0.720*set.height),1.0f,1.0f,0.0f,1.0f,"Easy");
			else
				printtxt2d((int)(0.16*set.width),(int)(0.720*set.height),1.0f,1.0f,1.0f,1.0f,"Easy");
			
			if(selectdifficulty == 1)
				printtxt2d((int)(0.16*set.width),(int)(0.690*set.height),1.0f,1.0f,0.0f,1.0f,"Medium");
			else
				printtxt2d((int)(0.16*set.width),(int)(0.690*set.height),1.0f,1.0f,1.0f,1.0f,"Medium");
			
			if(selectdifficulty == 2)			
				printtxt2d((int)(0.16*set.width),(int)(0.660*set.height),1.0f,1.0f,0.0f,1.0f,"Hard");
			else
				printtxt2d((int)(0.16*set.width),(int)(0.660*set.height),1.0f,1.0f,1.0f,1.0f,"Hard");
			
			
			DrawRectangle((int)(0.12*set.width),(int)(0.807*set.height),(int)(0.35*set.width),(int)(0.2*set.height),1.0f,1.0f,1.0f);
		break;
		
	};
	

	RenderGUI();
	DrawRectangle((int)(0.928*set.width),(int)(0.9896*set.height),(int)(0.0684*set.width),(int)(0.964*set.height),1.0f,1.0f,1.0f);
	if(!view)
		DrawRectangle(0,(int)(0.9896*set.height),(int)(0.0684*set.width),(int)(0.33*set.height),1.0f,1.0f,1.0f);

	if(num_tiles > 0)
	{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_LINES);
			glColor3f(1.0,1.0,1.0);
			glVertex3i(floors[0]->cube->origin.x-floors[0]->cube->ext.x/2,floors[0]->cube->origin.y,10000000);
			glVertex3i(floors[0]->cube->origin.x-floors[0]->cube->ext.x/2,floors[0]->cube->origin.y,-10000000);
		glEnd();
		
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);
	}
	glEnable(GL_LIGHTING);
}


void LevelEd::LoadLevel(char* filen)
{

		
	char temp[64];
	strcpy(temp,"maps/");
	strcat(temp,filen);
	if(!strstr(temp,".slv"))
	{
		if(strlen(temp)<60)
			strcat(temp,".slv");		
	}
	
	FILE *in = fopen(temp,"rb");
	if(in == NULL)
		return;
		
	if(floors)
	{
		for(int i = 0;i<num_tiles;i++)
			delete floors[i];
		delete[] floors;
		floors = NULL;	
	}
	fread(&time1,sizeof(int),1,in);
	if(time1 != -1) //old format
	{
		num_tiles = time1;
		time1 = time2 = time3 = 0;
	}
	else
	{
		fread(&time1,sizeof(int),1,in);
		fread(&time2,sizeof(int),1,in);
		fread(&time3,sizeof(int),1,in);
		fread(&num_tiles,sizeof(int),1,in);
	}

	floors = new Tile*[num_tiles];
	
	
	for(int u = 0;u<num_tiles;u++)
	{
		floors[u] = new Tile();
		floors[u]->cube = new Cube(lookat,lookat,lookat,0);//error ??!	
		floors[u]->cube->new_type_time = 0;
		floors[u]->cube->new_type = 0;	
		fread(&floors[u]->type,sizeof(int),1,in);
	
		
		fread(&floors[u]->cube->origin.x,sizeof(int),1,in);
		fread(&floors[u]->cube->origin.y,sizeof(int),1,in);
		fread(&floors[u]->cube->origin.z,sizeof(int),1,in);
		
		fread(&floors[u]->cube->ext.x,sizeof(int),1,in);
		fread(&floors[u]->cube->ext.y,sizeof(int),1,in);
		fread(&floors[u]->cube->ext.z,sizeof(int),1,in);
		
		fread(&floors[u]->cube->tiled.x,sizeof(int),1,in);
		fread(&floors[u]->cube->tiled.y,sizeof(int),1,in);
		fread(&floors[u]->cube->tiled.z,sizeof(int),1,in);

		fread(&floors[u]->cube->tex[0],sizeof(int),1,in);
		fread(&floors[u]->cube->tex[1],sizeof(int),1,in);
					
		fread(&floors[u]->cube->flag1,sizeof(int),1,in);
		fread(&floors[u]->cube->flag2,sizeof(int),1,in);
		fread(&floors[u]->cube->flag3,sizeof(int),1,in);
		fread(&floors[u]->cube->triggerflag,sizeof(int),1,in);
		
	}
	fclose(in);
	view = false;
	InitCamera();
	lookat.x = lookat.y = lookat.z =0;
	
	
}

void LevelEd::SaveLevel(char* filen)
{
	if(num_tiles <= 0)
		return;
		
	char temp[64];
	strcpy(temp,"maps/");
	strcat(temp,filen);
	if(!strstr(temp,".slv"))
	{
		if(strlen(temp)<60)
			strcat(temp,".slv");		
	}
			
	FILE *out = fopen(temp,"wb");
	if(out == NULL)
		return;
	
	//sort floors in ascending z-order
	for(int p = 0;p < num_tiles;p++)
	{
		for(int k = p+1;k < num_tiles;k++)
		{
			if(floors[p]->cube->origin.z > floors[k]->cube->origin.z)
			{
				for(int u = 0;u<num_tiles;u++)
				{
					if(T_OTHER(floors[u]->cube->triggerflag) && OTHERINDEX(floors[u]->cube->triggerflag) == p)
					{
						floors[u]->cube->triggerflag &= 0xff0007ff;
						floors[u]->cube->triggerflag |= (k<<11);
					}
					else if(T_OTHER(floors[u]->cube->triggerflag) && OTHERINDEX(floors[u]->cube->triggerflag) == k)
					{
						floors[u]->cube->triggerflag &= 0xff0007ff;
						floors[u]->cube->triggerflag |= (p<<11);
					}
				}	
				Tile *temp = floors[p];
				floors[p] = floors[k];
				floors[k] = temp;
			}
		}
	}
	int tmp = -1;
	fwrite(&tmp,sizeof(int),1,out);
	fwrite(&time1,sizeof(int),1,out);
	fwrite(&time2,sizeof(int),1,out);
	fwrite(&time3,sizeof(int),1,out);
	fwrite(&num_tiles,sizeof(int),1,out);
	for(int u = 0;u<num_tiles;u++)
	{
		fwrite(&floors[u]->type,sizeof(int),1,out);
			
		fwrite(&floors[u]->cube->origin.x,sizeof(int),1,out);
		fwrite(&floors[u]->cube->origin.y,sizeof(int),1,out);
		fwrite(&floors[u]->cube->origin.z,sizeof(int),1,out);
		
		fwrite(&floors[u]->cube->ext.x,sizeof(int),1,out);
		fwrite(&floors[u]->cube->ext.y,sizeof(int),1,out);
		fwrite(&floors[u]->cube->ext.z,sizeof(int),1,out);
		
		fwrite(&floors[u]->cube->tiled.x,sizeof(int),1,out);
		fwrite(&floors[u]->cube->tiled.y,sizeof(int),1,out);
		fwrite(&floors[u]->cube->tiled.z,sizeof(int),1,out);
		
		fwrite(&floors[u]->cube->tex[0],sizeof(int),1,out);
		fwrite(&floors[u]->cube->tex[1],sizeof(int),1,out);
		
		fwrite(&floors[u]->cube->flag1,sizeof(int),1,out);
		fwrite(&floors[u]->cube->flag2,sizeof(int),1,out);
		fwrite(&floors[u]->cube->flag3,sizeof(int),1,out);
		fwrite(&floors[u]->cube->triggerflag,sizeof(int),1,out);
		
			
	}
	fclose(out);
	GetFileList();
	
}



void LevelEd::LeMouseClick(SDL_MouseButtonEvent click)
{
	if(dialog)
		return;
		
	if(click.button == SDL_BUTTON_LEFT)
	{
		//right-side panel
		if(click.x > 0.9375*(float)(set.width))
		{
			if(!view) //top-down view: floor selection is visible
			{
				play(-1,S_SELECT);
				int top = (int)(0.261*set.height);
				
				cur_tilesel = (click.y-top)/(int)(0.0586*set.height);
				if(cur_tilesel < 0 )
					cur_tilesel = 0;
				else if(cur_tilesel > 11)
					cur_tilesel = SAND;
			}
			
			if(click.y < 0.25*(float)(set.height))//buttons on upper right corner
			{

				if(click.y > 0.19*(float)(set.height))
				{
					play(-1,S_SELECT);
					dialog = LOAD;	//load	
				}
				else if(click.y > 0.13*(float)(set.height))
				{
					play(-1,S_SELECT);
					dialog = SAVE;//save
				}
				else if(click.y > 0.086*(float)(set.height))
				{
					Init();
				}
				else if(click.y < 0.086*(float)(set.height))
				{
					play(-1,S_SELECT);
					view = !view;
					InitCamera();
					
					
					if(!view)
					{
						camera2.pos.x = lookat.x;
						camera2.pos.y = 48000;
						camera2.pos.z = lookat.z;
					}
					else	
					{
						camera2.pos.x = -56000;
						camera2.pos.y = lookat.y;
						camera2.pos.z = lookat.z;	
					}
				}
				
			}	
			
		}
		else if(!view && click.x < 0.068*(float)(set.width))//single player-buttons
		{
			if(click.y < 0.255*(float)(set.height))//buttons on upper left corner
			{
				play(-1,S_SELECT);
				int top = (int)(0.03*set.height);
				
				int temp = (click.y-top)/(int)(0.0586*set.height)+12;
				if(temp >= 12)
					cur_tilesel = temp;
				
			}
			if(click.y > 0.28*(float)(set.height) && click.y < 0.32*(float)(set.height))//star
			{
				cur_tilesel = 20;
			}
			
		}
		else if(!view)//top-down view
		{
			
			lmousebuttondown = true;
			
			GLdouble d_model[16];
			glGetDoublev(GL_MODELVIEW_MATRIX,d_model);
			GLdouble d_proj[16];
			glGetDoublev(GL_PROJECTION_MATRIX,d_proj);
			GLint i_view[4];
			glGetIntegerv(GL_VIEWPORT,i_view);
			
			GLdouble m_x,m_y,m_z = 0;

			gluProject(0,0,0,d_model,d_proj,i_view,&m_x,&m_y,&m_z);	//project origin once

			//did we click on one of the existing floors ?
			for(int o = 0;o<num_tiles;o++)
			{
				gluProject(floors[o]->cube->origin.x,floors[o]->cube->origin.y,floors[o]->cube->origin.z,d_model,d_proj,i_view,&m_x,&m_y,&m_z);
				if(m_x < click.x && set.height-m_y > click.y)
				{
					gluProject(floors[o]->cube->origin.x-floors[o]->cube->ext.x,floors[o]->cube->origin.y,floors[o]->cube->origin.z+floors[o]->cube->ext.z,d_model,d_proj,i_view,&m_x,&m_y,&m_z);
					if(m_x > click.x && set.height-m_y < click.y)
					{
						//get relative mouse-pos and save to allow smooth dragging without resetting to cube-origin
						GLdouble new_x,new_y,new_z;
						gluUnProject(click.x,set.height-click.y,m_z,d_model,d_proj,i_view,&new_x,&new_y,&new_z);
						start_resize.x = (int)floors[o]->cube->origin.x-(int)new_x;
						start_resize.z = (int)floors[o]->cube->origin.z-(int)new_z;
						
						
						if(cur_tilesel == 20) //power up
						{
							floors[o]->cube->flag1 = (int)new_x;
							floors[o]->cube->flag2 = (int)new_z;
							floors[o]->cube->triggerflag = floors[o]->cube->triggerflag | CEX_POWERUP;
							cur_tilesel = NORMAL; 
							return;	
						}
						else if(cur_tilesel == 12)//checkpoint
						{
							floors[o]->cube->flag1 = (int)new_x;
							floors[o]->cube->flag2 = (int)new_z;
							floors[o]->cube->triggerflag = floors[o]->cube->triggerflag | CEX_CHECKPOINT;
							cur_tilesel = NORMAL; 
							return;	
						}
						else if(cur_tilesel == 13)//pong
						{
							floors[o]->cube->flag1 = (int)new_x;
							floors[o]->cube->flag2 = (int)new_z;
							floors[o]->cube->triggerflag = floors[o]->cube->triggerflag | CEX_PONG;
							selectdifficulty = 0;
							cur_floor = o;
							cur_tilesel = NORMAL; 
							dialog = PONG;
							return;	
						}
						else if(cur_tilesel == 14)//life
						{
							floors[o]->cube->flag1 = (int)new_x;
							floors[o]->cube->flag2 = (int)new_z;
							floors[o]->cube->triggerflag = floors[o]->cube->triggerflag | CEX_LIFE;
							selectdifficulty = 0;
							cur_floor = o;
							cur_tilesel = NORMAL; 
							dialog = LIFE;
							return;	
						}
						else if(cur_tilesel == 15)//enemy
						{
							floors[o]->cube->flag1 = (int)new_x;
							floors[o]->cube->flag2 = (int)new_z;
							floors[o]->cube->triggerflag = floors[o]->cube->triggerflag | CEX_ENEMY;
							selectdifficulty = 0;
							cur_floor = o;
							cur_tilesel = NORMAL; 
							dialog = ENEMY;
							return;	
						}
						drag = true;	//we want to drag this floor
						cur_floor = o;
						
						if(controldown) //user holds [Ctrl] -> create moving floor
						{
							floors[o]->cube->flag1 = floors[o]->cube->origin.x;
							floors[o]->cube->flag2 = floors[o]->cube->origin.y;
							floors[o]->cube->flag3 = floors[o]->cube->origin.z;	
							floors[o]->cube->triggerflag = floors[o]->cube->triggerflag | CA_MOVE;
						}
						return;
					}
				}
			}
			
			if(cur_tilesel == 20 || cur_tilesel >= 12 && cur_tilesel <= 15)
			{
				cur_tilesel = NORMAL; 
				return;	
			}
			
			//nope- no floor hit: create a new one
			cur_floor = num_tiles;
			num_tiles++;
			Tile **temp = new Tile*[num_tiles];
			for(int l = 0;l<num_tiles-1;l++)
				temp[l] = floors[l];
				
			if(floors)
			{
				delete[] floors;
				floors = NULL;	
			}	
			floors = temp;
			
			
			GLdouble new_x,new_y,new_z;
			gluUnProject(click.x, set.height-click.y,m_z,d_model,d_proj,i_view,&new_x,&new_y,&new_z);
			
			
			Vertex origin  = {(int)new_x-((int)new_x)%300,0,(int)new_z-((int)new_z)%300};
			start_resize = origin;
			Vertex ext = {600,500,600};
			Vertex tiled = {1,1,1};
			floors[num_tiles-1] = new Tile();
			floors[num_tiles-1]->type = cur_tilesel;
			floors[num_tiles-1]->cube = new Cube(origin,ext,tiled,cur_tilesel);
			floors[num_tiles-1]->cube->new_type_time = 0;
			floors[num_tiles-1]->cube->new_type = 0;
				
		}
		else	//select cube from side
		{
			lmousebuttondown = true;
			
			GLdouble d_model[16];
			glGetDoublev(GL_MODELVIEW_MATRIX,d_model);
			GLdouble d_proj[16];
			glGetDoublev(GL_PROJECTION_MATRIX,d_proj);
			GLint i_view[4];
			glGetIntegerv(GL_VIEWPORT,i_view);
			GLdouble m_x,m_y,m_z;		
			
			//did we hit an existing floor ?
			for(int o = 0;o<num_tiles;o++)
			{
				gluProject(floors[o]->cube->origin.x,floors[o]->cube->origin.y,floors[o]->cube->origin.z,d_model,d_proj,i_view,&m_x,&m_y,&m_z);
				if(m_x < click.x && m_y >  set.height-click.y)
				{
					gluProject(floors[o]->cube->origin.x-floors[o]->cube->ext.x,floors[o]->cube->origin.y-floors[o]->cube->ext.y,floors[o]->cube->origin.z+floors[o]->cube->ext.z,d_model,d_proj,i_view,&m_x,&m_y,&m_z);
					if(m_x > click.x && m_y <  set.height-click.y)
					{
						drag = true;
						cur_floor = o;
						if(controldown) //user holds [Ctrl] -> create moving floor
						{
							floors[o]->cube->flag1 = floors[o]->cube->origin.x;
							floors[o]->cube->flag2 = floors[o]->cube->origin.y;
							floors[o]->cube->flag3 = floors[o]->cube->origin.z;
							floors[o]->cube->triggerflag = floors[o]->cube->triggerflag | CA_MOVE;
						}
						break;			//skip rest
					}
				}
			}
		
		}
		
	}
	if(click.button == SDL_BUTTON_RIGHT)
	{
		
	}
	
	
}

void LevelEd::LeMouseRelease(SDL_MouseButtonEvent click)
{
	if(click.button == SDL_BUTTON_LEFT)
	{
		
		lmousebuttondown = false;
		
		if(cur_floor > -1 && cur_floor < num_tiles)
		{
			if(floors[cur_floor]->cube->flag1 
			&& A_MOVE(floors[cur_floor]->cube->triggerflag)
			&& drag)
			{
				dialog = MOVE; //user has just placed a moving floor -> open dialog box to enter flags
				seconds = MOVETIME(floors[cur_floor]->cube->triggerflag);
				movement = A_MOVEMENT(floors[cur_floor]->cube->triggerflag);
				if(E_START(floors[cur_floor]->cube->triggerflag))
					sel[0] = 0;
				else if(E_TOUCH(floors[cur_floor]->cube->triggerflag))
					sel[0] = 1;
						
				sel[1] = 1;
						
				if(T_OTHER(floors[cur_floor]->cube->triggerflag))
					sel[2] = 2;
					
				if(T_OTHER(floors[cur_floor]->cube->triggerflag))	
					other = OTHERINDEX(floors[cur_floor]->cube->triggerflag);
				triggercol = 0;
				triggerline = sel[0];
			}
		}
			
		drag = false;
	}
	
}


void LevelEd::LeMouseMove(SDL_MouseMotionEvent move)
{
	if(dialog)
		return;
		
	if(lmousebuttondown == true && cur_floor != -1)
	{ 		
		GLdouble d_model[16];
		glGetDoublev(GL_MODELVIEW_MATRIX,d_model);
		GLdouble d_proj[16];
		glGetDoublev(GL_PROJECTION_MATRIX,d_proj);
		GLint i_view[4];
		glGetIntegerv(GL_VIEWPORT,i_view);
		
		GLdouble m_x,m_y,m_z;
		gluProject(floors[cur_floor]->cube->origin.x,floors[cur_floor]->cube->origin.y,floors[cur_floor]->cube->origin.z,d_model,d_proj,i_view,&m_x,&m_y,&m_z);
		
		
		GLdouble new_x,new_y,new_z;
		gluUnProject(move.x, set.height-move.y,m_z,d_model,d_proj,i_view,&new_x,&new_y,&new_z);

		if(!view)//top-down view
		{
			if(drag)//we are dragging a floor -> adjust its pos
			{
				floors[cur_floor]->cube->origin.x = (int)(new_x+start_resize.x)-(int)(new_x+start_resize.x)%300;
				floors[cur_floor]->cube->origin.z = (int)(new_z+start_resize.z)-(int)(new_z+start_resize.z)%300;
				
			}
			else//we are resizing a floor -> adjust extensions
			{
				Vertex ext;	
				ext.x = (floors[cur_floor]->cube->origin.x-(int)new_x)-((floors[cur_floor]->cube->origin.x-(int)new_x)%600);
				ext.z = ((int)new_z-floors[cur_floor]->cube->origin.z)-((int)new_z-floors[cur_floor]->cube->origin.z)%600;
				ext.y = floors[cur_floor]->cube->ext.y;
				
				
				if(ext.x < 600)
					ext.x = 605;
				if(ext.z < 600)
					ext.z = 605;	
				
				floors[cur_floor]->cube->ext = ext;
				
				//nice one: adjust amount of tiled textures on floor
				Vertex tiled = {floors[cur_floor]->cube->ext.x/600,1,floors[cur_floor]->cube->ext.z/600};
				floors[cur_floor]->cube->tiled = tiled;
			}
		}
		else//viewing from the side
			floors[cur_floor]->cube->origin.y = (int)new_y-(int)new_y%250; //move current floor up/down
				
	}
	
	
	
}




int LevelEd::Input(SDL_Event event, SDL_keysym key)
{
	static bool typing = false;
	switch(event.type)
	{
		case SDL_KEYDOWN:
			
			if(dialog == SAVE || dialog == LOAD)//save/load file
			{
				if(key.sym == SDLK_RETURN)
				{
					if(!strstr(filename,".") && !typing)
					{
						strcpy(subdir,filename);
						file_first = 0;
						file_sel = -1;
				
						GetFileList();
					}
					else if(strstr(filename,".."))
					{
						strcpy(subdir,"");
						file_first = 0;
						file_sel = -1;
						GetFileList();
					}
					else
					{
						char temp[128];
						strcpy(temp,subdir);
						strcat(temp,"/");
						strcat(temp,filename);
						
						if(dialog == SAVE)
						{
							dialog = TIMES;
							digit = 0;
						}
						else if(dialog == LOAD)
						{
							LoadLevel(temp);
							dialog = NON;	
						}	
						
					}
					return 0;
				}
				else if(key.sym == SDLK_BACKSPACE)
				{
					if(strlen(filename)>0)
						filename[strlen(filename)-1] = '\0';
					typing = true;
					
				}
				else if(key.sym == SDLK_UP)
				{
					play(-1,S_SELECT);
					if(file_sel > 0)
					{
						file_sel -= 1;
						if(file_sel < file_first)
							file_first--;
					}
					else	//wrap
					{
						file_sel = file_num-1;
						file_first = file_sel-5 >= 0 ? file_sel-5 : 0;
					}
					strncpy(filename,files[file_sel],32);
					typing  = false;
				}
					
				else if(key.sym == SDLK_DOWN)
				{
					play(-1,S_SELECT);
					if(file_sel < file_num-1)
					{
						file_sel += 1;
						if(file_sel > file_first+5)
							file_first++;
					}
					else	//wrap
					{
						file_sel = 0;
						file_first = 0;
					}
					strncpy(filename,files[file_sel],32);
					typing  = false;
				}
				else if(strlen(filename) < 31 && (int)key.sym >= 32 && (int)key.sym < 126)
				{
					filename[strlen(filename)] = (char)key.sym;
					typing = true;
				}
			
				
				if(key.sym == SDLK_ESCAPE)
					dialog = NON;
				
				return 0;
			}
			else if(dialog == TIMES)//medal times
			{
				if(key.sym == SDLK_ESCAPE)
					dialog = NON;
				if(key.sym == SDLK_RETURN)
				{	
					int tmp = 0;
					if(time3 < time1)
					{
						tmp = time3;
						time3 = time1;
						time1 = tmp;
					}
					if(time2 < time1)
					{
						tmp = time2;
						time2 = time1;
						time1 = tmp;
					}
					if(time3 < time2)
					{
						tmp = time3;
						time3 = time2;
						time2 = tmp;
					}
					char temp[128];
					strcpy(temp,subdir);
					strcat(temp,"/");
					strcat(temp,filename);
							
		
					SaveLevel(temp);
					dialog = NON;	
				}
				if(key.sym == SDLK_UP)
				{
					time_sel = (time_sel+2)%3;
					digit = 0;
				}	
				else if(key.sym == SDLK_DOWN)
				{
					time_sel = (time_sel+1)%3;
					digit = 0;	
				}

				if((int)key.sym >= 48 && (int)key.sym <= 57) //0-9
				{
					if(!time_sel) //bronze
					{
						switch(digit)
						{
							case 0:
								time3 = (time3%600000)+((int)key.sym-48)*600000;
							break;
							case 1:
								time3 = (time3/600000)*600000+(time3%60000)+((int)key.sym-48)*60000;
							break;
							case 2:
								time3 = (time3/60000)*60000+(time3%10000)+((int)key.sym-48)*10000;
							break;
							case 3:
								time3 = (time3/10000)*10000+(time3%1000)+((int)key.sym-48)*1000;
							break;
							case 4:
								time3 = (time3/1000)*1000+(time3%100)+((int)key.sym-48)*100;
							break;
							case 5:
								time3 = (time3/100)*100+(time3%10)+((int)key.sym-48)*10;
							break;
						};
						
					}
					else if(time_sel == 1) //silver
					{
						switch(digit)
						{
							case 0:
								time2 = (time2%600000)+((int)key.sym-48)*600000;
							break;
							case 1:
								time2 = (time2/600000)*600000+(time2%60000)+((int)key.sym-48)*60000;
							break;
							case 2:
								time2 = (time2/60000)*60000+(time2%10000)+((int)key.sym-48)*10000;
							break;
							case 3:
								time2 = (time2/10000)*10000+(time2%1000)+((int)key.sym-48)*1000;
							break;
							case 4:
								time2 = (time2/1000)*1000+(time2%100)+((int)key.sym-48)*100;
							break;
							case 5:
								time2 = (time2/100)*100+(time2%10)+((int)key.sym-48)*10;
							break;
						};
						
					}
					else if(time_sel == 2) //gold
					{
						switch(digit)
						{
							case 0:
								time1 = (time1%600000)+((int)key.sym-48)*600000;
							break;
							case 1:
								time1 = (time1/600000)*600000+(time1%60000)+((int)key.sym-48)*60000;
							break;
							case 2:
								time1 = (time1/60000)*60000+(time1%10000)+((int)key.sym-48)*10000;
							break;
							case 3:
								time1 = (time1/10000)*10000+(time1%1000)+((int)key.sym-48)*1000;
							break;
							case 4:
								time1 = (time1/1000)*1000+(time1%100)+((int)key.sym-48)*100;
							break;
							case 5:
								time1 = (time1/100)*100+(time1%10)+((int)key.sym-48)*10;
							break;
						};
						
					}
					digit = (digit+1)%6;
				
				}
			}
			else if(dialog == MOVE)	//attributes of moving floor need to be set
			{
				
				if(key.sym == SDLK_ESCAPE)
				{
					if(!seconds && A_MOVE(floors[cur_floor]->cube->triggerflag))
						floors[cur_floor]->cube->triggerflag &= 0x00000000;
					dialog = NON;
					sel[0] = sel[1] = sel[2] = 0;	
					triggerline = 0;
					triggercol = 0;
					seconds = 0;
					ntype = -1;
					other = -1;
					movement = false;
				}	
				else if(key.sym == SDLK_w || key.sym == SDLK_UP
				|| key.sym == SDLK_s || key.sym == SDLK_DOWN)
				{
					if(!triggercol)//triggered by
					{
						triggerline = 1-triggerline;
						sel[0] = triggerline;
					}
					else if(triggercol == 1)	//action
					{
						if(A_MOVE(floors[cur_floor]->cube->triggerflag))
						{
							sel[1] = 1;
							movement = !movement;
						}
						else
						{
							if(triggerline != 2 || key.sym == SDLK_w || key.sym == SDLK_UP)
							{
								if(triggerline == 2)
									triggerline = 0;
								else
									triggerline = 2;
							}
							else
							{
								ntype = (ntype+1)%8;	
							}
							sel[1] = triggerline;
						}
					}
					else	//trigger
					{
						if(triggerline == 2)
							triggerline = 0;
						else
							triggerline = 2;
						sel[2] = triggerline;
						
					}
					
				}
				else if(key.sym == SDLK_d || key.sym == SDLK_RIGHT)
				{
					if(!triggercol)
					{
						triggercol++;
						triggerline = sel[1];
						
					}
					else if(triggercol == 1)
					{
						triggercol++;
						triggerline = sel[2];
						
					}
				}
				else if(key.sym == SDLK_a || key.sym == SDLK_LEFT)
				{
					if(triggercol == 1)
					{
						triggercol--;
						triggerline = sel[0];
						
					}
					else if(triggercol == 2)
					{
						triggercol--;
						triggerline = sel[1];
						
					}
				}
				else if(key.sym >= '0' && key.sym <= '9')
				{
					if(triggercol == 1 && triggerline == 1)
				 	{
						if(seconds == 0)
							seconds = (int)key.sym-48;
						else if(seconds*10+((int)key.sym-48) <= 32)//maximum movement = 32 secs
							seconds = seconds*10+((int)key.sym-48);
					}
					else if(triggercol == 2 && triggerline == 2)
					{
						if(other == -1 && (int)key.sym-48 < num_tiles)
							other = (int)key.sym-48;
						else if(other*10+((int)key.sym-48) < num_tiles && 
						other*10+((int)key.sym-48) <= 8192)//maximum index of triggered object = 8192
							other = other*10+((int)key.sym-48);
					}
				}
				else if(key.sym == SDLK_BACKSPACE)
				{
					if(triggercol == 1 && triggerline == 1)
						seconds = seconds/10;
					else if(triggercol == 2 && triggerline == 2)
						other = other/10;
				}
				else if(key.sym == SDLK_RETURN)
				{
					//apply changes
					floors[cur_floor]->cube->triggerflag &= 0x00000000;
					int i = 0;
					for(;i<num_tiles;i++)
						if(OTHERINDEX(floors[i]->cube->triggerflag) == cur_floor && cur_floor != 0)
							break;
					if(i == num_tiles)//floor is not triggered by other floor
					{
						if(sel[0] == 0)
							floors[cur_floor]->cube->triggerflag = floors[cur_floor]->cube->triggerflag | CE_START;
						else  if(sel[0] == 1)
							floors[cur_floor]->cube->triggerflag = floors[cur_floor]->cube->triggerflag | CE_TOUCH;
					}
					
					if(sel[1] == 1)
					{
						floors[cur_floor]->cube->triggerflag = floors[cur_floor]->cube->triggerflag | CA_MOVE;
						if(seconds <= 0)
							seconds++;
						floors[cur_floor]->cube->triggerflag = floors[cur_floor]->cube->triggerflag | (seconds<<6);
						
						if(movement)
							floors[cur_floor]->cube->triggerflag = floors[cur_floor]->cube->triggerflag | CA_MOVEMENT;
					}
					else  if(sel[1] == 2)
					{
						floors[cur_floor]->cube->triggerflag = floors[cur_floor]->cube->triggerflag | CA_TYPE;
						floors[cur_floor]->cube->triggerflag = floors[cur_floor]->cube->triggerflag | (ntype<<24);
					}
					
					if(sel[2] == 2)
					{
						floors[cur_floor]->cube->triggerflag = floors[cur_floor]->cube->triggerflag | CT_OTHER;
						floors[cur_floor]->cube->triggerflag = floors[cur_floor]->cube->triggerflag | (other<<11);
					}
					
					dialog = NON;	
					sel[0] = sel[1] = sel[2] = 0;	
					triggerline = 0;
					seconds = 0;
					triggercol = 0;
					ntype = -1;
					other = -1;
				}
				
					
					
			}
			else if(dialog == LIFE || dialog == ENEMY || dialog == PONG)//select difficulty of life/enemy
			{
				if(key.sym == SDLK_RETURN)
				{
					floors[cur_floor]->cube->flag3 = selectdifficulty;	
					dialog = NON;
				}
				else if(key.sym == SDLK_DOWN)
				{
					if(selectdifficulty < 2)
						selectdifficulty++;
					else
						selectdifficulty = 0;
				}
				else if(key.sym == SDLK_UP)
				{
					if(selectdifficulty > 0)
						selectdifficulty--;
					else
						selectdifficulty = 2;
				}
				
			}
			else if(key.sym == SDLK_e)
			{
				if(cur_floor != -1)
				{
					dialog  = MOVE;	
					if(E_START(floors[cur_floor]->cube->triggerflag))
						sel[0] = 0;
					else if(E_TOUCH(floors[cur_floor]->cube->triggerflag))
						sel[0] = 1;
						
					if(A_TYPE(floors[cur_floor]->cube->triggerflag))
						sel[1] = 2;	
					else if(A_MOVE(floors[cur_floor]->cube->triggerflag))
						sel[1] = 1;
						
					if(T_OTHER(floors[cur_floor]->cube->triggerflag))
						sel[2] = 2;
						
					seconds = MOVETIME(floors[cur_floor]->cube->triggerflag);
					if(A_TYPE(floors[cur_floor]->cube->triggerflag))
						ntype = NEWTYPE(floors[cur_floor]->cube->triggerflag);
					if(T_OTHER(floors[cur_floor]->cube->triggerflag))
						other = OTHERINDEX(floors[cur_floor]->cube->triggerflag);
					triggercol = 0;
					triggerline = sel[0];
					
				}
				
				
			}
			else if(key.sym == SDLK_ESCAPE)//get back to main menu
			{
				if(floors)
				{
					for(int i = 0;i< num_tiles;i++)
						delete floors[i];
					delete[] floors;
				}
				floors = NULL;
				
			
				num_tiles = 0;
				cur_tilesel = NORMAL;
				cur_floor = -1;
			
				SDL_WM_GrabInput(SDL_GRAB_ON);
				SDL_ShowCursor(false);
				SwitchGamestate(GS_MENU);
				return 0;
				
			}
			else if((key.sym == SDLK_PLUS || key.sym == 'm' )&& cur_floor > -1)
			{
				floors[cur_floor]->cube->ext.y += 500;
				floors[cur_floor]->cube->tiled.y +=1;
			}
			else if((key.sym == SDLK_MINUS || key.sym == 'n' ) && cur_floor > -1 && floors[cur_floor]->cube->ext.y > 500)
			{
				floors[cur_floor]->cube->ext.y -= 500;
				floors[cur_floor]->cube->tiled.y -=1;
			}
			else if(key.sym == SDLK_DELETE)//remove currently selected floor
			{
				if(cur_floor != -1)
				{
					num_tiles--;
					Tile **temp = new Tile*[num_tiles];
					for(int l = 0;l<cur_floor;l++)
						temp[l] = floors[l];
					
					for(int u = cur_floor;u<num_tiles;u++)
						temp[u] = floors[u+1];	
						
					for(int i = 0;i<num_tiles;i++)
					{
						if(T_OTHER(temp[i]->cube->triggerflag))
						{
							int j = OTHERINDEX(temp[i]->cube->triggerflag);
							if(j > cur_floor)
							{
								
								temp[i]->cube->triggerflag &= 0xff0007ff;
								temp[i]->cube->triggerflag |= ((j-1)<<11);
							}
							else if(j == cur_floor)
							{
								temp[i]->cube->triggerflag &= 0xff0007f7;
							}
						}
					}
					delete floors[cur_floor];
					
					if(floors)
					{
						delete[] floors;
						floors = NULL;
					}
					
					floors = temp;
						
					cur_floor = num_tiles-1;
				}	
				
			}
			else if(key.sym == SDLK_w || key.sym == SDLK_UP) //move camera
				dirs[0] = true;
			else if(key.sym == SDLK_s || key.sym == SDLK_DOWN) //move camera
				dirs[1] = true;
			else if(key.sym == SDLK_d || key.sym == SDLK_RIGHT)//move camera
				dirs[2] = true;
			else if(key.sym == SDLK_a || key.sym == SDLK_LEFT)//move camera
				dirs[3] = true;
			else if(key.sym == SDLK_LCTRL || key.sym == SDLK_RCTRL) //user is holding [ctrl]
				controldown = true;
				
		break;
		case SDL_MOUSEBUTTONDOWN:
			
			if(event.button.button == SDL_BUTTON_WHEELUP)
			{
				if(view)
				{	
					int middlex = 0;
					if(num_tiles > 0)
						middlex = floors[0]->cube->origin.x-floors[0]->cube->ext.x/2;
		
					if(camera2.pos.x < -16000+middlex)
						camera2.pos.x += 1500;
					
				}
				else
				{
					if(camera2.pos.y > 12000)
						camera2.pos.y -= 1500;
				}
			}
			else if(event.button.button == SDL_BUTTON_WHEELDOWN)
			{
				if(view)
				{
					int middlex = 0;
					if(num_tiles > 0)
						middlex = floors[0]->cube->origin.x-floors[0]->cube->ext.x/2;
					if(camera2.pos.x > -120000+middlex)
						camera2.pos.x -= 1500;
					
				}
				else
				{
					if(camera2.pos.y < 120000)
						camera2.pos.y += 1500;
					
				}
			}
			else if(event.button.button == SDL_BUTTON_MIDDLE)
			{
				if(view)
				{
					camera2.pos.x = -56000;
					lookat.y = camera2.pos.y;
					lookat.z = camera2.pos.z;
					
				}
				else
				{
					lookat.x = camera2.pos.x;
					camera2.pos.y = 48000;
					lookat.z = camera2.pos.z;	
				}
			
			}
			else
				LeMouseClick(event.button);
			
		break;
		case SDL_MOUSEBUTTONUP:
			LeMouseRelease(event.button);
			
		break;
		case SDL_MOUSEMOTION:
			LeMouseMove(event.motion);
			
		break;
		
		case SDL_KEYUP:
			if(key.sym == SDLK_LCTRL || key.sym == SDLK_RCTRL)
				controldown = false;
			if(key.sym == SDLK_w || key.sym == SDLK_UP) //move camera
				dirs[0] = false;
			if(key.sym == SDLK_s || key.sym == SDLK_DOWN) //move camera
				dirs[1] = false;
			if(key.sym == SDLK_d || key.sym == SDLK_RIGHT)//move camera
				dirs[2] = false;
			if(key.sym == SDLK_a || key.sym == SDLK_LEFT)//move camera
				dirs[3] = false;
				
				
		break;
		
		
	}
	
	
	
	
	return 0;
	
}



