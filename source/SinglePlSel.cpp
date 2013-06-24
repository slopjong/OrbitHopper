/*
#				Orbit-Hopper
#				SinglePlSel.cpp : map selection
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
#include "../headers/SinglePlSel.h"
#include "../headers/SinglePlCSel.h"
#include "../headers/Game.h"
#include "../headers/Single.h"
#include "../headers/Utils.h"
#include "../headers/Sound.h"




#ifndef WIN32
#include <dirent.h>
#endif 


extern Settings set;
extern GLuint titles[1];
SingleData *data = NULL;
extern Gamestate *gamestates[15];
extern GLuint **anims;
void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b);

SinglePlSel::SinglePlSel()
{
	levelsel = 0;
	mapfirst = 0;
	reached = 0;
	num_skipped = 0;
	num_levels = 0;
}

SinglePlSel::~SinglePlSel()
{
	if(data)
	{
		delete[] data;
		data = NULL;
	}
}

void SinglePlSel::Update(unsigned int diff){}

void SinglePlSel::GetSingleProgress()
{
	char file[64];
	num_skipped = 0;
	sprintf(file,"maps/%s/single.stat",((SinglePlCSel*)gamestates[GS_SINGLECSEL])->dirname);
	
	FILE *in = fopen(file,"rb");
	for(int l = 0;l<num_levels;l++)
	{
		data[l].msecs = 99999999;
		data[l].skipped = false;
		data[l].lives = 0;
	}
	
	if(in == NULL)
	{
		reached = 0;
		return;
	}
	int num_entries = 0;
	fread(&num_entries,sizeof(int),1,in);
	
	fread(&reached,sizeof(int),1,in);
	
	for(int u = 0;u<num_entries && u<num_levels;u++)
	{
		char tempname[64];
		unsigned int temprecord;
		bool tempskipped;
		int templives;
		fread(&tempname,sizeof(char[64]),1,in);
		fread(&tempskipped,sizeof(bool),1,in);
		fread(&temprecord,sizeof(unsigned int),1,in);
		fread(&templives,sizeof(int),1,in);
		
		for(int k = 0;k<num_levels;k++)
			if(!strcmp(data[k].levelname,tempname))
			{
				data[k].msecs = temprecord;
				data[k].skipped = tempskipped;
				data[k].lives = templives;
				if(tempskipped)
					num_skipped++;
				break;
			}
	}
	fclose(in);
	
}

void SinglePlSel::SaveSingleProgress()
{
	char file[64];
	sprintf(file,"maps/%s/single.stat",((SinglePlCSel*)gamestates[GS_SINGLECSEL])->dirname);
	
	
	FILE *out = fopen(file,"wb");
	
	fwrite(&num_levels,sizeof(int),1,out);
	fwrite(&reached,sizeof(int),1,out);
	for(int u = 0;u<num_levels;u++)
	{
		fwrite(&data[u].levelname,sizeof(char[64]),1,out);
		fwrite(&data[u].skipped,sizeof(bool),1,out);
		fwrite(&data[u].msecs,sizeof(unsigned int),1,out);
		fwrite(&data[u].lives,sizeof(int),1,out);	
	}
	fclose(out);
	
}

#ifndef WIN32
	void SinglePlSel::GetMaps() //linux
	{
		char dirn[32];
		sprintf(dirn,"maps/%s/",((SinglePlCSel*)gamestates[GS_SINGLECSEL])->dirname);
	
		DIR *dir = opendir(dirn);
		num_levels = 0;
		dirent *entry = readdir(dir);
	
		while(entry)
		{
			if(strstr(entry->d_name,".slv"))
			{
				if(!strncmp(entry->d_name,"s-",2) )
					num_levels++;
			}
	
			entry = readdir(dir);
		}
	
		closedir(dir);
		
		if(data)
		{
			delete[] data;
			data = NULL;
		}
		data = new SingleData[num_levels];
		
		dir = opendir(dirn);
		entry = readdir(dir);
		int i = 0;
		while(entry && i < num_levels)
		{
			if(strstr(entry->d_name,".slv"))
			{
				if(!strncmp(entry->d_name,"s-",2) )
				{
					strncpy(data[i].levelname,entry->d_name,32);
					i++;
				}
			}
	
			entry = readdir(dir);
		}
	
		closedir(dir);
		
		//sort maps to insure campaign diffculty rises smoothly
		for(i = 0; i < num_levels; i++)
		{
			for(int u = i+1; u < num_levels; u++)
			{
				for(int k = 0; data[i].levelname[k]; k++)
				{
					if(data[u].levelname)
						if(data[i].levelname[k] != data[u].levelname[k])
						{
							if(data[i].levelname[k] > data[u].levelname[k])
							{
								char swap[32];
								strncpy(swap,data[u].levelname,32);
								strncpy(data[u].levelname,data[i].levelname,32);
								strncpy(data[i].levelname,swap,32);
							}
							break;
						}		
				}
				
			}
		}
	}
#else
	void SinglePlSel::GetMaps()//windows
	{
		_finddata_t fileblock;
		num_levels = 0;
		
		char dirn[32];
		sprintf(dirn,"maps/%s/*.slv",((SinglePlCSel*)gamestates[GS_SINGLECSEL])->dirname);
		
		int file_save = _findfirst(dirn,&fileblock);
		if(file_save==-1)
			return;
			
		if(!strncmp(fileblock.name,"s-",2) )
				num_levels++;
				
		while(_findnext(file_save,&fileblock) != -1)
		{
			if(!strncmp(fileblock.name,"s-",2) )
				num_levels++;
			
		}
		_findclose(file_save);
		
		
		if(data)
		{
			delete[] data;
			data = NULL;
		}
		data = new SingleData[num_levels];
		
		file_save = _findfirst(dirn,&fileblock);
			
		int i = 0;	
		if(!strncmp(fileblock.name,"s-",2) )
		{
			strncpy(data[0].levelname,fileblock.name,32);
			i++;
		}
		while(i < num_levels)
		{
			if(_findnext(file_save,&fileblock) == -1)
				break;
				
			if(!strncmp(fileblock.name,"s-",2) )
			{	
				strncpy(data[i].levelname,fileblock.name,32);
				i++;
			}
		}
		_findclose(file_save);
		
		//sort maps to insure campaign diffculty rises smoothly
		for(i = 0; i < num_levels; i++)
		{
			for(int u = i+1; u < num_levels; u++)
			{
				for(int k = 0; data[i].levelname[k]; k++)
				{
					if(data[u].levelname)
						if(data[i].levelname[k] != data[u].levelname[k])
						{
							if(data[i].levelname[k] > data[u].levelname[k])
							{
								char swap[32];
								strncpy(swap,data[u].levelname,32);
								strncpy(data[u].levelname,data[i].levelname,32);
								strncpy(data[i].levelname,swap,32);
							}
							break;
						}		
				}
				
			}
		}
		
	}
#endif


void SinglePlSel::Init()
{
	
	int oldnum_levels = num_levels;
	GetMaps();
	GetSingleProgress();
	if(reached != reached && reached != reached +1 || oldnum_levels != num_levels)
	{
		levelsel = 0;
		mapfirst = 0;
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspective(40,(float)(set.width)/(float)(set.height),50.5f,400000.0f);
	glMatrixMode(GL_MODELVIEW);		
	
	

}


int SinglePlSel::Input(SDL_Event action, SDL_keysym key)
{
	if(action.type == SDL_KEYDOWN )
	{
		if(key.sym == SDLK_UP && levelsel > 0)
		{
			play(-1,S_SELECT);

			if(levelsel >= 0)
				levelsel--;
				
			if(levelsel < mapfirst)
				mapfirst--;	
		}
			
		else if(key.sym == SDLK_DOWN && levelsel < num_levels-1)
		{
			play(-1,S_SELECT);
			
			if(levelsel < num_levels && levelsel < reached)
				levelsel++;
			
			if(levelsel > mapfirst+5)
				mapfirst++;
		}
		else if(key.sym == SDLK_s &&  num_skipped < num_levels/6 && levelsel == reached)
		{
			play(-1,S_HURT);
			
			data[levelsel].skipped = true;
			
			num_skipped++;
			reached++;
			SaveSingleProgress();
		}
		else if(key.sym == SDLK_RETURN)
		{
			play(-1,S_SELECT);
				
			char temp[128];
			sprintf(temp,"maps/%s/",((SinglePlCSel*)gamestates[GS_SINGLECSEL])->dirname);
			strcat(temp,data[levelsel].levelname);
			((Single *)(gamestates[GS_SINGLE]))->LoadLevelData(temp);
			
			SwitchGamestate(GS_SINGLE);
			return 0;
		}
		else if(key.sym == SDLK_ESCAPE)
		{
			SDL_WM_GrabInput(SDL_GRAB_ON);
			SDL_ShowCursor(false);
			//SaveSingleProgress();
			SwitchGamestate(GS_SINGLECSEL);
			
		}
	}	
	return 0;
	
}

void SinglePlSel::Render()
{		
	
	glColor3f(0.7,0.7,0.7);	
  	DrawBackGround(28,0);
  	glColor3f(1.0,1.0,1.0);	
  	
	{
		glColor3f(1.0,1.0,0.0);	
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
				glTexCoord2f(1,0.40);	glVertex2i((int)(0.8*set.width), (int)(0.862*set.height));
				glTexCoord2f(1,0.27);	glVertex2i((int)(0.8*set.width), (int)(0.942*set.height));
				glTexCoord2f(0,0.27);	glVertex2i((int)(0.2*set.width), (int)(0.942*set.height));
				glTexCoord2f(0,0.40);	glVertex2i((int)(0.2*set.width), (int)(0.862*set.height));	
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

  	float x_offs = (strlen(((SinglePlCSel*)gamestates[GS_SINGLECSEL])->campaignname)/2)*0.029;
	printtxt2d((int)((0.695-x_offs)*set.width),(int)(0.856*set.height),1.0f,1.0f,1.0f,0.0f,((SinglePlCSel*)gamestates[GS_SINGLECSEL])->campaignname);
			
	printtxt2d((int)(0.0488*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,"Levelname");
	printtxt2d((int)(0.5*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,"Lives left");
	printtxt2d((int)(0.732*set.width),(int)(0.781*set.height),1.0f,1.0f,1.0f,1.0f,"Record Time");
	
	int y = (int)(0.716*set.height);
	float g = 1.0f;
	int numlives = 0;
	for(int k = 0;k<reached;k++)
		numlives += data[k].lives;
		
	for(int u = mapfirst;u<=reached && u<num_levels && u<=mapfirst+10 ;u++,y-=(int)(0.039*set.height))
	{
			
		char temp[128];
		
		unsigned int tsec = (data[u].msecs%1000)/10;
		unsigned int sec = (data[u].msecs/1000)%60;
		unsigned int min = data[u].msecs/60000; 
		
		sprintf(temp,"%d: %02d: %02d",min,sec,tsec);
		
	
			
		if(u == levelsel)
			g = 0.0f;
		else
			g = 1.0f;
			
		if(data[u].skipped)
			printtxt2d((int)(0.348*set.width),y,1.0f,1.0f,0.0f,1.0f-g,"SKIPPED");
		printtxt2d((int)(0.732*set.width),y,1.0f,1.0f,g,1.0f,temp);
		sprintf(temp,"%d",data[u].lives);
		printtxt2d((int)(0.5*set.width),y,1.0f,1.0f,g,1.0f,temp);	
		printtxt2d((int)(0.0488*set.width),y,1.0f,1.0f,g,1.0f,data[u].levelname);
		
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
		if(mapfirst+10 >= num_levels-1)
			glColor3f(0.2,0.2,0.2);
		else
			glColor3f(1.0,1.0,0.0);


			glBegin(GL_QUADS);
			
				glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.93*set.width), (int)(0.3*set.height));
				glTexCoord2f(0.47,0.7);		glVertex2i((int)(0.93*set.width), (int)(0.335*set.height));
				glTexCoord2f(0.40,0.7);		glVertex2i((int)(0.918*set.width), (int)(0.335*set.height));
				glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.918*set.width), (int)(0.3*set.height));	
										
			glEnd();
		
		if(mapfirst <= 0)
			glColor3f(0.2,0.2,0.2);
		else
			glColor3f(1.0,1.0,0.0);

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
	sprintf(temp,"Number of skips left: %d", num_levels/6-num_skipped);
	printtxt2d((int)(0.0488*set.width),(int)(0.22*set.height),0.8f,1.0f,1.0f,1.0f,temp);
	int lost = (reached)*5-numlives >= 0 ? (reached)*5-numlives : 0;
	
	//what skill-level do you have ?
	char score[32] = "Newbie"; //Lowskiller ;-P
	if(!lost)
	{
	
		if(reached > 4)
			strcpy(score,"Godlike");
		else if(reached > 2)
			strcpy(score,"Progamer");
		else if(reached > 0)
			strcpy(score,"Veteran");
		
	}
	else if((float)(reached)/(float)(lost) >= 3.5f)
	{
		strcpy(score,"Godlike"); //= Y0u R th3 1337-roxxor
	}
	else if((float)(reached)/(float)(lost) >= 1.7f)
	{
		strcpy(score,"Progamer");//just couldn't resist... ;-)
	}
	else if((float)(reached)/(float)(lost) >= 1.0f)
	{
		strcpy(score,"Veteran");
	}
	else if((float)(reached)/(float)(lost) >= 0.75f)
	{
		strcpy(score,"Highly Skilled");
	}
	else if((float)(reached)/(float)(lost) >= 0.5f)
	{
		strcpy(score,"Skilled");
	}
	else if((float)(reached)/(float)(lost) >= 0.3f)
	{
		strcpy(score,"Not Bad");
	}
	else if((float)(reached)/(float)(lost) >= 0.25f)
	{
		strcpy(score,"Poor");
	}
	
	sprintf(temp,"Number of lives lost: %d - %s", lost, score);
	printtxt2d((int)(0.4*set.width),(int)(0.22*set.height),0.8f,1.0f,1.0f,1.0f,temp);
	
	DrawRectangle((int)(0.029*set.width),(int)(0.82*set.height),(int)(0.941*set.width),(int)(0.651*set.height),1.0f,1.0f,0.0f);
	
}
