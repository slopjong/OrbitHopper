/*
#				Orbit-Hopper
#				SinglePlCSel.cpp : campaign selection
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
#include "../headers/SinglePlCSel.h"
#include "../headers/Game.h"
#include "../headers/Single.h"
#include "../headers/Utils.h"
#include "../headers/Sound.h"



#ifndef WIN32
#include <dirent.h>
#endif 

 
extern Settings set;
extern Gamestate *gamestates[15];
extern GLuint titles[1];
extern GLuint **anims;


void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b);

SinglePlCSel::SinglePlCSel()
{
	campaigns = NULL;
	num_camp = 0;
	campfirst = 0;
	campsel = 0;
	dirname = NULL;
	campaignname = NULL;
}

SinglePlCSel::~SinglePlCSel()
{
	
	if(campaigns)
	{
		for(int i = 0;i<num_camp;i++)
			delete[] campaigns[i];
		delete[] campaigns;
	}
	campaigns = NULL;

	if(dirname)
		delete[] dirname;
	dirname = NULL;
	if(campaignname)
		delete[] campaignname;
	campaignname = NULL;
}

void SinglePlCSel::Update(unsigned int diff)
{
	
}

void SinglePlCSel::Init()
{
	
	if(campaigns)
	{
		for(int i = 0;i<num_camp;i++)
			delete[] campaigns[i];
		delete[] campaigns;
	}
	campaigns = NULL;

	if(dirname)
		delete[] dirname;
	dirname = NULL;
	if(campaignname)
		delete[] campaignname;
	campaignname = NULL;
	GetCampaigns();	
	
	
}

void SinglePlCSel::LoadDescription(int i)
{
	char  b[64];
	sprintf(b,"maps/%s/description.txt",campaigns[i]);
	
	FILE *in = fopen(b,"r");
	
	if(!in)
	{
		strcat(campaigns[i],"#No name#Unknown number of levels# No date#Anonymous#No description available#");
		return;
	}
	
	char temp1[32];
	fgets(temp1,32,in);
   	fgets(temp1,32,in);	//Campaign name
   	
   	strcat(campaigns[i],"#");
    	strncat(campaigns[i],temp1,32);
     	
     	
    fgets(temp1,32,in);	//empty
    fgets(temp1,32,in);
    char temp2[16];
    fgets(temp2,16,in);//number of levels
    
    strcat(campaigns[i],"#");
    strncat(campaigns[i],temp2,4);
    
    fgets(temp1,32,in);	//empty
    fgets(temp1,32,in);
    fgets(temp2,16,in);//Date
    
    strcat(campaigns[i],"#");
    strncat(campaigns[i],temp2,16);
    
    fgets(temp1,32,in);	//empty
    fgets(temp1,32,in);
    char temp3[16];
    fgets(temp3,16,in);//Author
    
    strcat(campaigns[i],"#");
    strncat(campaigns[i],temp3,16);
    
    fgets(temp1,32,in);	//empty
    fgets(temp1,32,in);
    char temp4[171];
    fgets(temp4,171,in);//Description
    
    strcat(campaigns[i],"#");
    strncat(campaigns[i],temp4,171);
    strcat(campaigns[i],"#");
 
}

#ifdef WIN32
	void SinglePlCSel::GetCampaigns() //Windows
	{
		_finddata_t fileblock;
		num_camp = 0;
		
		int dir = _findfirst("maps/*",&fileblock);
		if(dir==-1)
		{
			campaigns = new char*[1];
			campaigns[0] = new char[512];
			strcpy(campaigns[0],"No campaign found.");
			
			return;
		}
		if(!strstr(fileblock.name,".") && strstr(fileblock.name,"camp"))	//fileblock is a directory
			num_camp++;
		
		while(_findnext(dir,&fileblock) != -1)
		{
			if(!strstr(fileblock.name,".") && strstr(fileblock.name,"camp"))	//fileblock is a campaign-directory
				num_camp++;
			
		}
		
		if(!num_camp)//no directories found
		{
			campaigns = new char*[1];
			campaigns[0] = new char[512];
			strcpy(campaigns[0],"No campaign found.");
			
			return;
		}
		
		_findclose(dir);
		
		campaigns = new char*[num_camp];
		for(int i = 0;i<num_camp;i++)
			campaigns[i] = new char[512];
			
		num_camp = 0;
		
		dir = _findfirst("maps/*",&fileblock);
		
		if(!strstr(fileblock.name,".") && strstr(fileblock.name,"camp"))	//fileblock is a directory
		{
			num_camp++;
			strncpy(campaigns[0],fileblock.name,16);
		}
		
		while(_findnext(dir,&fileblock) != -1)
		{
			if(!strstr(fileblock.name,".")&& strstr(fileblock.name,"camp"))	//fileblock is a campaign-directory
			{
				strncpy(campaigns[num_camp],fileblock.name,16);
				num_camp++;
			}
			
		}
		
		_findclose(dir);
		for(int i = 0;i<num_camp;i++)
			LoadDescription(i);
			
	}
#else

	void SinglePlCSel::GetCampaigns() //linux
	{
		DIR *dir = opendir("maps/");
		
		num_camp = 0;
	
		dirent *entry = readdir(dir);
	
		while(entry != NULL)
		{
			if(!strstr(entry->d_name,".") && strstr(entry->d_name,"camp"))
				num_camp++;
			entry = readdir(dir);
		}
	
		closedir(dir);
		
		campaigns = new char*[num_camp];
		for(int i = 0;i<num_camp;i++)
			campaigns[i] = new char[512];
	
		

		num_camp = 0;
		dir = opendir("maps/");
		entry = readdir(dir);
	
		while(entry != NULL)
		{
			if(!strstr(entry->d_name,".") && strstr(entry->d_name,"camp"))
			{
				strncpy(campaigns[num_camp],entry->d_name,16);
				num_camp++;
			}
			entry = readdir(dir);
		}
	
		closedir(dir);

		

		for(int i = 0;i<num_camp;i++)
			LoadDescription(i);
			
	}
#endif


int SinglePlCSel::Input(SDL_Event action, SDL_keysym key)
{
	if(action.type == SDL_KEYDOWN )
	{
		if(key.sym == SDLK_UP && campsel > 0)
		{
			play(-1,S_SELECT);

			if(campsel >= 0)
				campsel--;
				
			if(campsel < campfirst)
				campfirst--;	
		}
			
		else if(key.sym == SDLK_DOWN && campsel < num_camp-1)
		{
			play(-1,S_SELECT);
			
			if(campsel < num_camp)
				campsel++;
			
			if(campsel > campfirst+1)
				campfirst++;
		}
		else if(key.sym == SDLK_RETURN && num_camp)
		{
			play(-1,S_SELECT);
			
			char *temp = new char[512];
			strncpy(temp,campaigns[campsel],512);
			if(dirname)
				delete[] dirname;
			dirname = new char[32];
			strncpy(dirname,strtok(temp,"#"),32);
			if(campaignname)
				delete[] campaignname;
			campaignname = new char[32];
			strncpy(campaignname,strtok(NULL,"#"),32);
			delete[] temp;
			
			
			SwitchGamestate(GS_SINGLESEL);
			return 0;
		}
		else if(key.sym == SDLK_ESCAPE)
		{
			SDL_WM_GrabInput(SDL_GRAB_ON);
			SDL_ShowCursor(false);
			//SaveSingleProgress();
			SwitchGamestate(GS_MENU);
			
		}
	}	
	return 0;
	
}



void SinglePlCSel::Render()
{		
	
	glColor3f(0.7,0.7,0.7);	
  	DrawBackGround(28,0);
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
				glTexCoord2f(1,0.40);	glVertex2i((int)(0.85*set.width), (int)(0.842*set.height));
				glTexCoord2f(1,0.27);	glVertex2i((int)(0.85*set.width), (int)(0.932*set.height));
				glTexCoord2f(0,0.27);	glVertex2i((int)(0.15*set.width), (int)(0.932*set.height));
				glTexCoord2f(0,0.40);	glVertex2i((int)(0.15*set.width), (int)(0.842*set.height));	
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
  	
				
	int y = (int)(0.750*set.height);
	float g = 1.0f;
	
	if(!num_camp)
		printtxt2d((int)(0.1*set.width),y,1.0f,1.0f,g,1.0f,"No campaigns found.");
		
	for(int u = campfirst;u<num_camp && u<=campfirst+1 ;u++,y-=(int)(0.300*set.height))
	{
		char *temp = new char[512];
		strcpy(temp,campaigns[u]);
		char *tok = strtok(temp,"#");
		
		
		if(u == campsel)
			g = 0.0f;
		else
			g = 1.0f;
		
		tok = strtok(NULL,"#");	
		printtxt2d((int)(0.1*set.width),y,1.0f,1.0f,g,1.0f,tok);
		
		tok = strtok(NULL,"#");
		
		sprintf(temp,"%s levels",tok);
		printtxt2d((int)(0.77*set.width-strlen(temp)*0.0091*set.width),y,0.8f,1.0f,g,1.0f,temp);
		
		char *tok1 = strtok(NULL,"#");
		char *tok2 = strtok(NULL,"#");
		sprintf(temp,"%s by %s",tok1,tok2);
		printtxt2d((int)(0.77*set.width-strlen(temp)*0.0091*set.width),y-(int)(0.039*set.height),0.8f,1.0f,g,1.0f,temp);
		
		tok = strtok(NULL,"#");
		if(strlen(tok) > 57)
		{
			for(int i = 1;(i-1)*57 < (int)strlen(tok);i++)
			{
				char line[58];
				memset(line,0,sizeof(char)*58);
				strncpy(line,tok+(i-1)*57,57);
				printtxt2d((int)(0.1*set.width),y-(1+i)*(int)(0.039*set.height),0.8f,1.0f,g,1.0f,line);
			
			}
		}
		else
			printtxt2d((int)(0.1*set.width),y-2*(int)(0.039*set.height),0.8f,1.0f,g,1.0f,tok);
		delete[] temp;	
		
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
		if(campfirst+1 >= num_camp-1)
			glColor3f(0.2,0.2,0.2);
		else
			glColor3f(1.0,1.0,0.0);


			glBegin(GL_QUADS);
			
				glTexCoord2f(0.47,0.1);	glVertex2i((int)(0.9*set.width), (int)(0.25*set.height));
				glTexCoord2f(0.47,0.7);		glVertex2i((int)(0.9*set.width), (int)(0.285*set.height));
				glTexCoord2f(0.40,0.7);		glVertex2i((int)(0.888*set.width), (int)(0.285*set.height));
				glTexCoord2f(0.40,0.1);	glVertex2i((int)(0.888*set.width), (int)(0.25*set.height));	
										
			glEnd();
		
		if(campfirst <= 0)
			glColor3f(0.2,0.2,0.2);
		else
			glColor3f(1.0,1.0,0.0);

			glBegin(GL_QUADS);
			
				glTexCoord2f(0.47,0.7);	glVertex2i((int)(0.9*set.width), (int)(0.72*set.height));
				glTexCoord2f(0.47,0.1);		glVertex2i((int)(0.9*set.width), (int)(0.755*set.height));
				glTexCoord2f(0.40,0.1);		glVertex2i((int)(0.888*set.width), (int)(0.755*set.height));
				glTexCoord2f(0.40,0.7);	glVertex2i((int)(0.888*set.width), (int)(0.72*set.height));	
										
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

	char temp[64];
	
	sprintf(temp,"Number of campaigns: %d",num_camp);
	
	printtxt2d((int)(0.070*set.width),(int)(0.22*set.height),0.8f,1.0f,1.0f,1.0f,temp);
	
	
	DrawRectangle((int)(0.029*set.width),(int)(0.82*set.height),(int)(0.941*set.width),(int)(0.651*set.height),1.0f,1.0f,0.0f);
	
}
