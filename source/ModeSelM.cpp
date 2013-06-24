/*
#				Orbit-Hopper
#				ModeSelM.cpp : multiplayer menu
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
#ifndef WIN32
#include <dirent.h> //linux only
#endif

#include "../headers/Font.h"
#include "../headers/Game.h"
#include "../headers/Structs.h"
#include "../headers/Utils.h"
#include "../headers/ModeSelM.h"
#include "../headers/Sound.h"


#include "SDL_image.h"

extern Settings set;
extern GLuint titles[1];
void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b);
extern GLuint **anims;
extern bool blooming;

ModeSelM::ModeSelM()
{
	modesel = 0;
	mapsel = 0;
	start_index = 0;
	camaps = NULL;
	ramaps = NULL;	
	pics = NULL;
	
}

ModeSelM::~ModeSelM()
{
	if(camaps)
	{
		delete[] camaps;
		camaps = NULL;
	}
	if(ramaps)
	{
		delete[] ramaps;
		ramaps = NULL;
	}
	if(pics)
	{
		delete[] pics;
		pics = NULL;	
	}
}

void ModeSelM::Update(unsigned int diff){}

#ifndef WIN32
	void ModeSelM::GetCAMaps() //linux
	{
		DIR *dir = opendir("maps/");
		num_camaps = 0;
		dirent *entry = readdir(dir);
	
		while(entry)
		{
			if(strstr(entry->d_name,".slv"))
			{
				if(!strncmp(entry->d_name,"ca-",3) )
					num_camaps++;
			}
	
			entry = readdir(dir);
		}
	
		closedir(dir);
		
		if(camaps)
		{
			delete[] camaps;
			camaps = NULL;
		}
		camaps = new char*[num_camaps];
		for(int i = 0;i<num_camaps;i++)
			camaps[i] = new char[32];
			
		dir = opendir("maps/");
		entry = readdir(dir);
	
		int i = 0;
		while(entry && i < num_camaps)
		{
			if(strstr(entry->d_name,".slv"))
			{
				if(!strncmp(entry->d_name,"ca-",3) )
				{
					strncpy(camaps[i],entry->d_name,32);
					i++;
				}
			}
	
			entry = readdir(dir);
		}
	
		closedir(dir);


		for(i = 0; i < num_camaps; i++)
		{
			for(int u = i+1; u < num_camaps; u++)
			{
				for(int k = 0; camaps[i][k]; k++)
				{
					if(camaps[u])
						if(camaps[i][k] != camaps[u][k])
						{
							if(camaps[i][k] > camaps[u][k])
							{
								char swap[32];
								strncpy(swap,camaps[u],32);
								strncpy(camaps[u],camaps[i],32);
								strncpy(camaps[i],swap,32);
							}
							break;
						}		
				}
				
			}
		}
	}
#else
	void ModeSelM::GetCAMaps()//windows
	{
		_finddata_t fileblock;
		num_camaps = 0;
		
		int file_save = _findfirst("maps/*.slv",&fileblock);
		if(file_save==-1)
			return;
			
		if(!strncmp(fileblock.name,"ca-",3) )
			num_camaps++;	
		while(_findnext(file_save,&fileblock) != -1)
		{	
			if(!strncmp(fileblock.name,"ca-",3) )
				num_camaps++;
		}
		_findclose(file_save);
		
		
		if(camaps)
		{
			delete[] camaps;
			camaps = NULL;
		}
		camaps = new char*[num_camaps];
		for(int i = 0;i<num_camaps;i++)
			camaps[i] = new char[32];
			
		file_save = _findfirst("maps/*.slv",&fileblock);
		
		int i = 0;
		if(!strncmp(fileblock.name,"ca-",3) )
		{
			strncpy(camaps[0],fileblock.name,32);
			i++;
		}
		while(i < num_camaps)
		{
			if(_findnext(file_save,&fileblock) == -1)
				break;
				
			if(!strncmp(fileblock.name,"ca-",3) )
			{	
				strncpy(camaps[i],fileblock.name,32);
				i++;
			}
		}
		_findclose(file_save);

		for(i = 0; i < num_camaps; i++)
		{
			for(int u = i+1; u < num_camaps; u++)
			{
				for(int k = 0; camaps[i][k]; k++)
				{
					if(camaps[u])
						if(camaps[i][k] != camaps[u][k])
						{
							if(camaps[i][k] > camaps[u][k])
							{
								char swap[32];
								strncpy(swap,camaps[u],32);
								strncpy(camaps[u],camaps[i],32);
								strncpy(camaps[i],swap,32);
							}
							break;
						}		
				}
				
			}
		}
		
	}
#endif



#ifndef WIN32
	void ModeSelM::GetRAMaps() //linux
	{
		DIR *dir = opendir("maps/");
		num_ramaps = 0;
		dirent *entry = readdir(dir);
	
		while(entry)
		{
			if(strstr(entry->d_name,".slv"))
			{
				if(!strncmp(entry->d_name,"ra-",3) )
					num_ramaps++;
			}
	
			entry = readdir(dir);
		}
		
		if(ramaps)
		{
			delete[] ramaps;
			ramaps = NULL;
		}
		ramaps = new char*[num_ramaps];
		for(int i = 0;i<num_ramaps;i++)
			ramaps[i] = new char[32];
		
		dir = opendir("maps/");
		entry = readdir(dir);
	
		int i = 0;
		while(entry && i < num_ramaps)
		{
			if(strstr(entry->d_name,".slv"))
			{
				if(!strncmp(entry->d_name,"ra-",3) )
				{
					strncpy(ramaps[i],entry->d_name,32);
					i++;
				}
			}
	
			entry = readdir(dir);
		}
	
		closedir(dir);

		for(i = 0; i < num_ramaps; i++)
		{
			for(int u = i+1; u < num_ramaps; u++)
			{
				for(int k = 0; ramaps[i][k]; k++)
				{
					if(ramaps[u])
						if(ramaps[i][k] != ramaps[u][k])
						{
							if(ramaps[i][k] > ramaps[u][k])
							{
								char swap[32];
								strncpy(swap,ramaps[u],32);
								strncpy(ramaps[u],ramaps[i],32);
								strncpy(ramaps[i],swap,32);
							}
							break;
						}		
				}
				
			}
		}
	}
#else
	void ModeSelM::GetRAMaps()//windows
	{
		_finddata_t fileblock;
		num_ramaps = 0;
		
		int file_save = _findfirst("maps/*.slv",&fileblock);
		if(file_save==-1)
			return;
			
		if(!strncmp(fileblock.name,"ra-",3) )
			num_ramaps++;
		while(_findnext(file_save,&fileblock) != -1)
		{
				
			if(!strncmp(fileblock.name,"ra-",3) )
				num_ramaps++;
		}
		_findclose(file_save);
		
		
		if(ramaps)
		{
			delete[] ramaps;
			ramaps = NULL;
		}
		ramaps = new char*[num_ramaps];
		for(int i = 0;i<num_ramaps;i++)
			ramaps[i] = new char[32];
		
		file_save = _findfirst("maps/*.slv",&fileblock);
		
		int i = 0;
		if(!strncmp(fileblock.name,"ra-",3) )
		{
			strncpy(ramaps[0],fileblock.name,32);
			i++;
		}
			
	
		while(i < num_ramaps)
		{
			if(_findnext(file_save,&fileblock) == -1)
				break;
				
			if(!strncmp(fileblock.name,"ra-",3) )
			{	
				strncpy(ramaps[i],fileblock.name,32);
				i++;
			}
		}
		_findclose(file_save);

		for(i = 0; i < num_ramaps; i++)
		{
			for(int u = i+1; u < num_ramaps; u++)
			{
				for(int k = 0; ramaps[i][k]; k++)
				{
					if(ramaps[u])
						if(ramaps[i][k] != ramaps[u][k])
						{
							if(ramaps[i][k] > ramaps[u][k])
							{
								char swap[32];
								strncpy(swap,ramaps[u],32);
								strncpy(ramaps[u],ramaps[i],32);
								strncpy(ramaps[i],swap,32);
							}
							break;
						}		
				}
				
			}
		}
		
	}
#endif

void ModeSelM::getPics()
{
	if(pics)
	{
		delete[] pics;
	}
	
	int num_texts = num_camaps+num_ramaps;
	pics = new GLuint[num_texts];
	
	SDL_Surface *TempTex[num_texts]; 
	memset(TempTex,0,sizeof(void *)*num_texts);   
	
	for(int i = 0;i<num_camaps;i++)
	{
		char filename[64];
		strcpy(filename,"maps/");
		strncat(filename,camaps[i],64);
		char *a = strstr(filename,".slv");
		if(a)
			strcpy(a,".png");
		if(!(TempTex[i] = IMG_Load(filename)))
		{
			TempTex[i] = IMG_Load("maps/nopic.png");
		}
	}
	for(int i = 0;i<num_ramaps;i++)
	{
		char filename[64];
		
		strcpy(filename,"maps/");
		strncat(filename,ramaps[i],64);
		char *a = strstr(filename,".slv");
		if(a)
			strcpy(a,".png");
		if(!(TempTex[i+num_camaps] = IMG_Load(filename)))
		{
			TempTex[i+num_camaps] = IMG_Load("maps/nopic.png");
		}
	}	
	
	glGenTextures(num_texts, pics);
	for(int p = 0;p < num_texts;p++)
	{
		glBindTexture(GL_TEXTURE_2D, pics[p]);
		
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB, TempTex[p]->w, TempTex[p]->h, 0, GL_RGB, GL_UNSIGNED_BYTE, TempTex[p]->pixels);
				
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
					
		SDL_FreeSurface(TempTex[p]);	
		
	}
		
	
}

void ModeSelM::GetMultSettings()
{
	
	FILE *in = fopen("multiplayer.ini","r");
	
	if(!in)
	{
		fprintf(stderr,"Error: multiplayer.ini not found - using default settings.");	
		calives = ralives = 5;
		cagravity = ragravity = 8;
		powerups = true;
		return;
	}
	
	char temp[64];
	char *val;
	fgets(temp,64,in);
     	
   	fgets(temp,64,in);
   	val = strchr(temp,'=');
    calives = (val ? atoi(val+2): 0 );

	fgets(temp,64,in);
	val = strchr(temp,'=');
   	powerups = (val ? atoi(val+2): 1 );

	fgets(temp,64,in);
	val = strchr(temp,'=');
   	cagravity = (val ? atoi(val+2): 1 );

	fgets(temp,64,in);
	fgets(temp,64,in);
	fgets(temp,64,in);
	val = strchr(temp,'=');
   	ralives = (val ? atoi(val+2): 1 );

	fgets(temp,64,in);
	val = strchr(temp,'=');
   	ragravity = (val ? atoi(val+2): 1 );
	
   	if(ralives <= 0 || ralives > 10)
		ralives = 5;
	if(calives <= 0 || calives > 10)
		calives = 5;
	if(cagravity < 5 || cagravity > 10)
		cagravity = 8;
	if(ragravity < 5 || ragravity > 10)
		ragravity = 8;
	
	fclose(in);
}


void ModeSelM::WriteMultSettings()
{
	FILE *out = fopen("multiplayer.ini","w");
	char temp[64];
	fputs("[CASTLE-ATTACK]\n",out);
	
	sprintf(temp,"lives = %d\n",calives);
	fputs(temp,out);
	
	sprintf(temp,"powerups = %d\n",powerups);
	fputs(temp,out);
	
	sprintf(temp,"gravity = %d\n\n",cagravity);
	fputs(temp,out);
	
	fputs("[RACE]\n",out);
	sprintf(temp,"lives = %d\n",ralives);
	fputs(temp,out);
	
	
	
	sprintf(temp,"gravity = %d\n\n",ragravity);
	fputs(temp,out);
	
	
	
	fclose(out);
}
void ModeSelM::Init()
{
	cagravity = ragravity = 8;
	calives = ralives = 5;
	powerups = true;
	GetCAMaps();
	GetRAMaps();
	GetMultSettings();
	getPics();
	//modesel = 0;
	//mapsel = 0;
	//start_index = 0;
}

int ModeSelM::Input(SDL_Event action,SDL_keysym key)
{
	if(action.type == SDL_KEYDOWN)
	{
		if(key.sym == SDLK_RIGHT || key.sym == SDLK_d)
		{
			play(-1,S_SELECT);
			modesel = (modesel+1)%3;
			mapsel = 0;
			start_index = 0;
		}	
		else if(key.sym == SDLK_LEFT || key.sym == SDLK_a)
		{
			play(-1,S_SELECT);
			modesel = (modesel+2)%3;
			mapsel = 0;
			start_index = 0;		
		}
		else if(key.sym == SDLK_UP || key.sym == SDLK_w)
		{
		
			play(-1,S_SELECT);
			
			if(modesel != SM_OPTIONS)
			{
				if(mapsel >0)
					mapsel--;
				if(start_index+2 > mapsel && start_index > 0)
					start_index--;
			}
			else
				mapsel = (mapsel+4)%5;
			
			
		}
		else if(key.sym == SDLK_DOWN || key.sym == SDLK_s)
		{
		
			play(-1,S_SELECT);
			
			if(modesel != SM_OPTIONS)
			{
				if(mapsel < num_camaps-1 && !modesel
				|| mapsel < num_ramaps-1)
					mapsel++;
				
				if(start_index+2 < mapsel)
					start_index++;
			}
			else
				mapsel = (mapsel+1)%5;
		}
		else if(key.sym == SDLK_RETURN)
		{
			
			if(modesel == SM_CASTLE)
			{
				WriteMultSettings();
				play(-1,S_SELECT);
				SwitchGamestate(GS_CASTLEM);
			}
			else if(modesel == SM_RACE)
			{
				WriteMultSettings();
				play(-1,S_SELECT);
				SwitchGamestate(GS_RACEM);
			}
			else if(modesel == SM_OPTIONS)
			{
				play(-1,S_SELECT);
				switch(mapsel)
				{
					case 0:
						calives = (calives)%10+1;
					break;
					case 1:
						powerups = !powerups;
					break;
					case 2:
						cagravity = (cagravity+1-5)%6+5;
					break;
					case 3:
						ralives = (ralives)%8+1;
					break;
					case 4:
						ragravity = (ragravity+1-5)%6+5;
					break;
				};
			}
		}
		else if(key.sym == SDLK_SPACE)
		{
			if(modesel == SM_OPTIONS)
			{
				play(-1,S_SELECT);
				switch(mapsel)
				{
					case 0:
						calives = (calives)%10+1;
					break;
					case 1:
						powerups = !powerups;
					break;
					case 2:
						cagravity = (cagravity+1-5)%6+5;
					break;
					case 3:
						ralives = (ralives)%8+1;
					break;
					case 4:
						ragravity = (ragravity+1-5)%6+5;
					break;
				};
			}

		}
		else if(key.sym == SDLK_ESCAPE)
		{
			SwitchGamestate(GS_MENU);
		}
	}	
	return 0;
	
}

void ModeSelM::drawPic(int i)
{
	if(blooming)
		return;

	glColor3f(1.0f,1.0f,1.0f);
	glDisable(GL_DEPTH_TEST);											
	glBindTexture(GL_TEXTURE_2D,pics[i]);	
	glDisable(GL_LIGHTING);	
	glMatrixMode(GL_PROJECTION);								
	glPushMatrix();												
	glLoadIdentity();											
	gluOrtho2D( 0, set.width ,0, set.height);							
	glMatrixMode(GL_MODELVIEW);									
	glPushMatrix();												
		glLoadIdentity();	
		
		glBegin(GL_QUADS);
			//upper-left corner
			glTexCoord2f(0,0);	glVertex2f(0.4*(float)(set.width), 0.5*(float)(set.height));
			glTexCoord2f(0,1);	glVertex2f(0.4*(float)(set.width), (0.5-0.2)*(float)(set.height));
			glTexCoord2f(1,1);	glVertex2f((0.4+0.2)*(float)(set.width), (0.5-0.2)*(float)(set.height));
			glTexCoord2f(1,0);	glVertex2f((0.4+0.2)*(float)(set.width), 0.5*(float)(set.height));							
		glEnd();	
		
	glMatrixMode( GL_PROJECTION );								
	glPopMatrix();												
	glMatrixMode( GL_MODELVIEW );								
	glPopMatrix();	
	glEnable(GL_LIGHTING);	
	glEnable(GL_DEPTH_TEST);			

	
}

void ModeSelM::Render()
{
	
	glColor3f(0.7,0.7,0.7);	
  	DrawBackGround(35,0);
  	glColor3f(1.0,1.0,1.0);	
  	
	{
		glColor3f(0.0,1.0,1.0);	
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
				glTexCoord2f(1,0.53);	glVertex2i((int)(0.85*set.width), (int)(0.842*set.height));
				glTexCoord2f(1,0.405);	glVertex2i((int)(0.85*set.width), (int)(0.932*set.height));
				glTexCoord2f(0,0.405);	glVertex2i((int)(0.15*set.width), (int)(0.932*set.height));
				glTexCoord2f(0,0.53);	glVertex2i((int)(0.15*set.width), (int)(0.842*set.height));	
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
			
	printtxt2d((int)(0.049*(float)(set.width)),(int)(0.78*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Selected Mode:");
	float g = 0.0;
	float y = 0.491*(float)(set.height);
	
	//TODO: improve appearance of that page	
	switch(modesel)
	{
		case SM_CASTLE:
			printtxt2d((int)(0.049*(float)(set.width)),(int)(0.716*(float)(set.height)),1.0f,1.0f,0.0f,1.0f,"Castle Attack");
			printtxt2d((int)(0.30*(float)(set.width)),(int)(0.716*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Race");
			printtxt2d((int)(0.42*(float)(set.width)),(int)(0.716*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Options");
			printtxt2d((int)(0.049*(float)(set.width)),(int)(0.664*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"Decrease your enemy's castle-health by entering his castle.");
			printtxt2d((int)(0.049*(float)(set.width)),(int)(0.638*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"Try to get hold of some of the power-ups placed on the maps;");
			printtxt2d((int)(0.049*(float)(set.width)),(int)(0.612*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"things should be easier then.");
			printtxt2d((int)(0.049*(float)(set.width)),(int)(0.546*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Maps:");
	
			
			for(int u = start_index;u<num_camaps && u<=start_index+5 ;u++,y-=0.039*(float)(set.height))
			{
				if(u == mapsel)
					g = 0.0f;
				else
					g = 1.0f;
				printtxt2d((int)(0.049*(float)(set.width)),(int)(y),0.8f,1.0f,g,1.0f,camaps[u]);
				
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
				if(start_index+5 >= num_camaps-1)
					glColor3f(0.2,0.2,0.2);
				else
					glColor3f(0.0,1.0,1.0);


					glBegin(GL_QUADS);
			
						glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.3*set.width), (int)(0.26*set.height));
						glTexCoord2f(0.47,0.7);		glVertex2i((int)(0.3*set.width), (int)(0.295*set.height));
						glTexCoord2f(0.40,0.7);		glVertex2i((int)(0.288*set.width), (int)(0.295*set.height));
						glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.288*set.width), (int)(0.26*set.height));	
										
					glEnd();
		
				if(start_index <= 0)
					glColor3f(0.2,0.2,0.2);
				else
					glColor3f(0.0,1.0,1.0);

					glBegin(GL_QUADS);
			
						glTexCoord2f(0.47,0.7);	glVertex2i((int)(0.3*set.width), (int)(0.475*set.height));
						glTexCoord2f(0.47,0.1);		glVertex2i((int)(0.3*set.width), (int)(0.51*set.height));
						glTexCoord2f(0.40,0.1);		glVertex2i((int)(0.288*set.width), (int)(0.51*set.height));
						glTexCoord2f(0.40,0.7);	glVertex2i((int)(0.288*set.width), (int)(0.475*set.height));	
										
					glEnd();	

				glColor3f(0.0,1.0,1.0);
					glBegin(GL_QUADS);
		
						glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.281*set.width), (int)(0.697*set.height));
						glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.281*set.width), (int)(0.711*set.height));
						glTexCoord2f(0.47,0.7);	glVertex2i((int)(0.261*set.width), (int)(0.711*set.height));
						glTexCoord2f(0.40,0.7);	glVertex2i((int)(0.261*set.width), (int)(0.697*set.height));	
									
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


			drawPic(mapsel);

			
			
		break;
		
		case SM_RACE:

			printtxt2d((int)(0.049*(float)(set.width)),(int)(0.716*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Castle Attack");
			printtxt2d((int)(0.30*(float)(set.width)),(int)(0.716*(float)(set.height)),1.0f,1.0f,0.0f,1.0f,"Race");
			printtxt2d((int)(0.42*(float)(set.width)),(int)(0.716*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Options");
			printtxt2d((int)(0.049*(float)(set.width)),(int)(0.664*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"Earn points by reaching the goal.");
			printtxt2d((int)(0.049*(float)(set.width)),(int)(0.546*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Maps:");
	
			for(int u = start_index;u<num_ramaps && u<=start_index+5 ;u++,y-=0.039*(float)(set.height))
			{
				if(u == mapsel)
					g = 0.0f;
				else
					g = 1.0f;
				printtxt2d((int)(0.049*(float)(set.width)),(int)(y),0.8f,1.0f,g,1.0f,ramaps[u]);
				
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
				if(start_index+5 >= num_ramaps-1)
					glColor3f(0.2,0.2,0.2);
				else
					glColor3f(0.0,1.0,1.0);


					glBegin(GL_QUADS);
			
						glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.3*set.width), (int)(0.26*set.height));
						glTexCoord2f(0.47,0.7);		glVertex2i((int)(0.3*set.width), (int)(0.295*set.height));
						glTexCoord2f(0.40,0.7);		glVertex2i((int)(0.288*set.width), (int)(0.295*set.height));
						glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.288*set.width), (int)(0.26*set.height));	
										
					glEnd();
		
				if(start_index <= 0)
					glColor3f(0.2,0.2,0.2);
				else
					glColor3f(0.0,1.0,1.0);

					glBegin(GL_QUADS);
			
						glTexCoord2f(0.47,0.7);	glVertex2i((int)(0.3*set.width), (int)(0.475*set.height));
						glTexCoord2f(0.47,0.1);		glVertex2i((int)(0.3*set.width), (int)(0.51*set.height));
						glTexCoord2f(0.40,0.1);		glVertex2i((int)(0.288*set.width), (int)(0.51*set.height));
						glTexCoord2f(0.40,0.7);	glVertex2i((int)(0.288*set.width), (int)(0.475*set.height));	
										
					glEnd();	
				glColor3f(0.0,1.0,1.0);
					glBegin(GL_QUADS);
		
						glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.401*set.width), (int)(0.697*set.height));
						glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.401*set.width), (int)(0.711*set.height));
						glTexCoord2f(0.47,0.7);	glVertex2i((int)(0.381*set.width), (int)(0.711*set.height));
						glTexCoord2f(0.40,0.7);	glVertex2i((int)(0.381*set.width), (int)(0.697*set.height));	
									
					glEnd();

				glColor3f(0.0,1.0,1.0);
					glBegin(GL_QUADS);
		
						glTexCoord2f(0.40,0.7);	glVertex2i((int)(0.281*set.width), (int)(0.697*set.height));
						glTexCoord2f(0.47,0.7);	glVertex2i((int)(0.281*set.width), (int)(0.711*set.height));
						glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.261*set.width), (int)(0.711*set.height));
						glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.261*set.width), (int)(0.697*set.height));	
									
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



			drawPic(mapsel+num_camaps);
			
		break;
		case SM_OPTIONS:
			printtxt2d((int)(0.049*(float)(set.width)),(int)(0.716*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Castle Attack");
			printtxt2d((int)(0.30*(float)(set.width)),(int)(0.716*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Race");
			printtxt2d((int)(0.42*(float)(set.width)),(int)(0.716*(float)(set.height)),1.0f,1.0f,0.0f,1.0f,"Options");

			
			printtxt2d((int)(0.36*(float)(set.width)),(int)(0.616*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Castle Attack");
			
			char temp[32];
			sprintf(temp,"%d",calives);
			if(!mapsel)
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.566*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,"Lives:");
				printtxt2d((int)(0.56*(float)(set.width)),(int)(0.566*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,temp);
			}
			else
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.566*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"Lives:");
				printtxt2d((int)(0.56*(float)(set.width)),(int)(0.566*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,temp);
			}

			
			if(mapsel == 1)
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.516*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,"Powerups:");
				if(powerups)
					printtxt2d((int)(0.56*(float)(set.width)),(int)(0.516*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,"on");
				else
					printtxt2d((int)(0.56*(float)(set.width)),(int)(0.516*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,"off");
			}
			else
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.516*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"Powerups:");
				if(powerups)
					printtxt2d((int)(0.56*(float)(set.width)),(int)(0.516*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"on");
				else
					printtxt2d((int)(0.56*(float)(set.width)),(int)(0.516*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"off");
			}


			sprintf(temp,"%d",cagravity);
			if(mapsel == 2)
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.466*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,"Gravity:");
				printtxt2d((int)(0.56*(float)(set.width)),(int)(0.466*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,temp);
			}
			else
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.466*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"Gravity:");
				printtxt2d((int)(0.56*(float)(set.width)),(int)(0.466*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,temp);
			}


			printtxt2d((int)(0.36*(float)(set.width)),(int)(0.406*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,"Race");
			sprintf(temp,"%d",ralives);
			if(mapsel == 3)
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.356*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,"Lives:");
				printtxt2d((int)(0.56*(float)(set.width)),(int)(0.356*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,temp);
			}
			else
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.356*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"Lives:");
				printtxt2d((int)(0.56*(float)(set.width)),(int)(0.356*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,temp);
			}

			sprintf(temp,"%d",ragravity);
			if(mapsel == 4)
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.306*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,"Gravity:");
				printtxt2d((int)(0.56*(float)(set.width)),(int)(0.306*(float)(set.height)),0.8f,1.0f,0.0f,1.0f,temp);
			}
			else
			{
				printtxt2d((int)(0.36*(float)(set.width)),(int)(0.306*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,"Gravity:");
				printtxt2d((int)(0.56*(float)(set.width)),(int)(0.306*(float)(set.height)),0.8f,1.0f,1.0f,1.0f,temp);
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
				
				glColor3f(0.0,1.0,1.0);
					glBegin(GL_QUADS);
		
						glTexCoord2f(0.40,0.7);	glVertex2i((int)(0.401*set.width), (int)(0.697*set.height));
						glTexCoord2f(0.47,0.7);	glVertex2i((int)(0.401*set.width), (int)(0.711*set.height));
						glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.381*set.width), (int)(0.711*set.height));
						glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.381*set.width), (int)(0.697*set.height));	
									
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
		break;
	}
		
	
	DrawRectangle((int)(0.029*(float)(set.width)),(int)(0.82*(float)(set.height)),(int)(0.941*(float)(set.width)),(int)(0.651*(float)(set.height)),0.0f,1.0f,1.0f);
	
	
}

