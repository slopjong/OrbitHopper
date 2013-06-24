/*
#				Orbit-Hopper
#				Options.cpp
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


#include "../headers/Font.h"
#include "../headers/Structs.h"
#include "../headers/Game.h"
#include "../headers/Utils.h"
#include "../headers/Options.h"
#include "../headers/Sound.h"




Settings set;
extern GLuint titles[1];
extern int num_dir;
extern char ** GetGfxDirs(int &num_dir);
extern bool shadersavailable;

void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b);
int InitTextures();

Options::Options()
{
	directories = NULL;
	selectedopt = 1;
	selectedplayer = 1;
	mapping = false;
}


Options::~Options()
{
	if(directories)
	{
		for(int i = 0;i<num_dir;i++)
			delete[] directories[i];
		delete[] directories;
	}
	directories = NULL;
}

void Options::LoadSettings()
{
	mapping = false;
	selectedopt = 1;
	
	FILE *in = fopen("settings.ini","r");
	
	if(!in)
	{
		fprintf(stderr,"Error: settings.ini not found - using default settings.");	
		set.keys[0] = 119;
		set.keys[1] = 115;
		set.keys[2] = 100;
		set.keys[3] = 97;
		set.keys[4] = 32;
		set.keys[5] = 110;
		set.keys[6] = 101;

		set.keys[7] = 273;
		set.keys[8] = 274;
		set.keys[9] = 275;
		set.keys[10] = 276;
		set.keys[11] = 109;
		set.keys[12] = 111;
		set.keys[13] = 98;
		set.bloomw = set.bloomh = 256;
		set.glow = 1;
		set.bloom = 3;
		set.mipmaps = 1;
		set.textures = 0;
		set.details = 0;
		set.width = 1024;
		set.height = 768;
		set.sounds = 1;
		set.animspeed = 10;
		set.windowed = 0;

		return;
	}
	
	char temp[64];
	char *val;
	fgets(temp,64,in);
    fgets(temp,64,in); 
     	
   	
	for(int i = 0;i<14;i++)
	{
		fgets(temp,64,in);
		val = strchr(temp,'=');
   		set.keys[i] = (val ? atoi(val+2): 0 );

	}
	
	fgets(temp,64,in);
	fgets(temp,64,in);
	fgets(temp,64,in);
	
	fgets(temp,64,in);
	val = strchr(temp,'=');
   	set.glow = (val ? atoi(val+2): 1 );
   	
	fgets(temp,64,in);
	val = strchr(temp,'=');
   	set.bloom = (val ? atoi(val+2): 3 );

	fgets(temp,64,in);
	val = strchr(temp,'=');
   	set.bloomw = (val ? atoi(val+2): 256 );

	fgets(temp,64,in);
	val = strchr(temp,'=');
   	set.bloomh = (val ? atoi(val+2): 256 );

	fgets(temp,64,in);
	val = strchr(temp,'=');
   	set.mipmaps = (val ? atoi(val+2): 1 );
   	
	fgets(temp,64,in);
	val = strchr(temp,'=');
   	set.textures = (val ? (atoi(val+2) < num_dir ? atoi(val+2) : 0): 0 );
   	
   	fgets(temp,64,in);
   	val = strchr(temp,'=');
   	set.details = (val ? atoi(val+2): 0 );
   	
   	fgets(temp,64,in);
   	val = strchr(temp,'=');
   	set.drawdist = (val ? (atoi(val+2) < 100000 ? 100000 : atoi(val+2)): 200000 );

	fgets(temp,64,in);
   	val = strchr(temp,'=');
   	set.shadowdist = (val ? (atoi(val+2) < 5000 ? 5000 : atoi(val+2)): 50000 );
   	
   	fgets(temp,64,in);
   	val = strchr(temp,'=');
   	set.animspeed = (val ? (atoi(val+2) >= 50 ? 2 : 53-atoi(val+2)): 0 );
   	
   	fgets(temp,64,in);
   	val = strchr(temp,'=');
   	set.speed = (val ? atoi(val+2): 1 );

	fgets(temp,64,in);
   	val = strchr(temp,'=');
   	set.shadows = (val ? atoi(val+2): 1 );
   	
  
			
	fgets(temp,64,in);
	fgets(temp,64,in);
	fgets(temp,64,in);
	
	fgets(temp,64,in);
	val = strchr(temp,'=');
   	set.width = (val ? atoi(val+2): 1024 );
   	
   	fgets(temp,64,in);
   	val = strchr(temp,'=');
   	set.height = (val ? atoi(val+2): 768 );
   
	fgets(temp,64,in);
   	val = strchr(temp,'=');
   	set.windowed = (val ? atoi(val+2): 0 );

   	fgets(temp,64,in);
	fgets(temp,64,in);
	fgets(temp,64,in);
	
	fgets(temp,64,in);
	val = strchr(temp,'=');
   	set.sounds = (val ? atoi(val+2): 1 );
   
}

void Options::Init()
{
	if(directories)
	{
		for(int i = 0;i<num_dir;i++)
			delete directories[i];
		delete[] directories;
	}
	directories = NULL;
	
	directories = GetGfxDirs(num_dir);
	
	LoadSettings();
}

void Options::Update(unsigned int diff){}

char* Options::ToKeyName(int key)
{
	//character keys
	if(key >=97 && key <= 122)
	{
		sprintf(keyname,"%c",key);
		return keyname;
	}
	
	//number keys
	if(key >=48 && key<=57)
	{
		sprintf(keyname,"%d",key-48);
		return keyname;
	}
	
	//save some key-names to make it easier to identify keys
	switch(key)
	{
		case 8:
		return "backspace";
		case 13:
		return "return";
		case 32:
		return "space";	
		case 256:
		return "KP_0";
		case 257:
		return "KP_1";
		case 258:
		return "KP_DOWN";
		case 259:
		return "KP_3";
		case 260:
		return "KP_LEFT";
		case 261:
		return "KP_5";
		case 262:
		return "KP_RIGHT";
		case 263:
		return "KP_7";
		case 264:
		return "KP_UP";
		case 265:
		return "KP_9";
		case 273:
		return "up";
		case 274:
		return "down";
		case 275:
		return "right";
		case 276:
		return "left";	
		case 303:
		return "rshift";	
		case 304:
		return "lshift";	
		case 305:
		return "rctrl";	
		case 306:
		return "lctrl";	
		case 271:
		return "KP_RETURN";
		default:
		sprintf(keyname,"key%d",key);
		return keyname;
		
	};
	
}

void Options::Render()
{
	
	glColor3f(0.7,0.7,0.7);	
  	DrawBackGround(23,0);
  	glColor3f(1.0,1.0,1.0);	
  	
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
				glTexCoord2f(1,0.67);	glVertex2i((int)(0.85*set.width), (int)(0.842*set.height));
				glTexCoord2f(1,0.54);	glVertex2i((int)(0.85*set.width), (int)(0.932*set.height));
				glTexCoord2f(0,0.54);	glVertex2i((int)(0.15*set.width), (int)(0.932*set.height));
				glTexCoord2f(0,0.67);	glVertex2i((int)(0.15*set.width), (int)(0.842*set.height));	
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

	printtxt2d((int)(0.204*set.width),(int)(0.782*set.height),1.1f,1.0f,1.0f,1.0f,"Controls:");
	printtxt2d((int)(0.204*set.width),(int)(0.41*set.height),1.1f,1.0f,1.0f,1.0f,"Graphics:");
	
	printtxt2d((int)(0.499*set.width),(int)(0.782*set.height),0.9f,1.0f,1.0f,1.0f,"Player 1");
	printtxt2d((int)(0.669*set.width),(int)(0.782*set.height),0.9f,1.0f,1.0f,1.0f,"Player 2");
	
	char temp[32];
	float g= 1.0f;
	
	//TODO: write a function to wrap all of this c*** up
	
	if(selectedopt == 1)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Forward");
	printtxt2d((int)(0.206*set.width),(int)(0.715*set.height),0.8f,1.0f,g,1.0f,temp);
	
		if(selectedplayer == 1 && selectedopt == 1)
			g = 0.0f;
		else 
			g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[FORWARD1+7]));
		printtxt2d((int)(0.499*set.width),(int)(0.715*set.height),0.8f,1.0f,g,1.0f,temp);
		
		if(selectedplayer == 2 && selectedopt == 1)
			g = 0.0f;
		else 
			g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[FORWARD1]));
		printtxt2d((int)(0.669*set.width),(int)(0.715*set.height),0.8f,1.0f,g,1.0f,temp);	
		
	if(selectedopt==2)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Backward");
	printtxt2d((int)(0.206*set.width),(int)(0.675*set.height),0.8f,1.0f,g,1.0f,temp);
	
		if(selectedplayer == 1 && selectedopt == 2)
				g = 0.0f;
			else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[BACKWARD1+7]));
		printtxt2d((int)(0.499*set.width),(int)(0.675*set.height),0.8f,1.0f,g,1.0f,temp);
		
		if(selectedplayer == 2 && selectedopt == 2)
				g = 0.0f;
			else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[BACKWARD1]));
		printtxt2d((int)(0.669*set.width),(int)(0.675*set.height),0.8f,1.0f,g,1.0f,temp);
	
	if(selectedopt==3)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Right");
	printtxt2d((int)(0.206*set.width),(int)(0.637*set.height),0.8f,1.0f,g,1.0f,temp);
		if(selectedplayer == 1 && selectedopt == 3)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[RIGHT1+7]));
		printtxt2d((int)(0.499*set.width),(int)(0.637*set.height),0.8f,1.0f,g,1.0f,temp);
		
		if(selectedplayer == 2 && selectedopt == 3)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[RIGHT1]));
		printtxt2d((int)(0.669*set.width),(int)(0.637*set.height),0.8f,1.0f,g,1.0f,temp);
	
	if(selectedopt==4)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Left");
	printtxt2d((int)(0.206*set.width),(int)(0.598*set.height),0.8f,1.0f,g,1.0f,temp);
	
		if(selectedplayer == 1 && selectedopt == 4)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[LEFT1+7]));
		printtxt2d((int)(0.499*set.width),(int)(0.598*set.height),0.8f,1.0f,g,1.0f,temp);
		
		if(selectedplayer == 2 && selectedopt == 4)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[LEFT1]));
		printtxt2d((int)(0.669*set.width),(int)(0.598*set.height),0.8f,1.0f,g,1.0f,temp);
	
	if(selectedopt==5)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Jump");
	printtxt2d((int)(0.206*set.width),(int)(0.558*set.height),0.8f,1.0f,g,1.0f,temp);
	
		if(selectedplayer == 1 && selectedopt == 5)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[JUMP1+7]));
		printtxt2d((int)(0.499*set.width),(int)(0.558*set.height),0.8f,1.0f,g,1.0f,temp);
		
		
		if(selectedplayer == 2 && selectedopt == 5)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[JUMP1]));
		printtxt2d((int)(0.669*set.width),(int)(0.558*set.height),0.8f,1.0f,g,1.0f,temp);
		
	if(selectedopt==6)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Reset");
	printtxt2d((int)(0.206*set.width),(int)(0.518*set.height),0.8f,1.0f,g,1.0f,temp);
	
		if(selectedplayer == 1 && selectedopt == 6)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[RESET1+7]));
		printtxt2d((int)(0.499*set.width),(int)(0.518*set.height),0.8f,1.0f,g,1.0f,temp);
		
		
		if(selectedplayer == 2 && selectedopt == 6)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[RESET1]));
		printtxt2d((int)(0.669*set.width),(int)(0.518*set.height),0.8f,1.0f,g,1.0f,temp);
		
	if(selectedopt==7)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Use");
	printtxt2d((int)(0.206*set.width),(int)(0.478*set.height),0.8f,1.0f,g,1.0f,temp);
	
		if(selectedplayer == 1 && selectedopt == 7)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[USE1+7]));
		printtxt2d((int)(0.499*set.width),(int)(0.478*set.height),0.8f,1.0f,g,1.0f,temp);
		
		
		if(selectedplayer == 2 && selectedopt == 7)
				g = 0.0f;
		else 
				g = 1.0f;
		sprintf(temp,"%s",ToKeyName(set.keys[USE1]));
		printtxt2d((int)(0.669*set.width),(int)(0.478*set.height),0.8f,1.0f,g,1.0f,temp);
	
	
	if(selectedopt==8)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Glow");
	printtxt2d((int)(0.206*set.width),(int)(0.351*set.height),0.8f,1.0f,g,1.0f,temp);
	if(!set.glow)
		strcpy(temp,"off");
	else
		strcpy(temp,"on");
	printtxt2d((int)(0.499*set.width),(int)(0.351*set.height),0.8f,1.0f,g,1.0f,temp);
		
	if(selectedopt==9)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Shadows");
	printtxt2d((int)(0.206*set.width),(int)(0.311*set.height),0.8f,1.0f,g,1.0f,temp);
	if(!set.shadows)
		strcpy(temp,"off");
	else
		strcpy(temp,"on");
	printtxt2d((int)(0.499*set.width),(int)(0.311*set.height),0.8f,1.0f,g,1.0f,temp);
	
	if(selectedopt==10)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Mipmaps");
	printtxt2d((int)(0.206*set.width),(int)(0.271*set.height),0.8f,1.0f,g,1.0f,temp);
	if(!set.mipmaps)
		strcpy(temp,"off");
	else
		strcpy(temp,"on");
	printtxt2d((int)(0.499*set.width),(int)(0.271*set.height),0.8f,1.0f,g,1.0f,temp);
	
	if(selectedopt==11)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Style");
	printtxt2d((int)(0.206*set.width),(int)(0.231*set.height),0.8f,1.0f,g,1.0f,temp);
	strcpy(temp,directories[set.textures]);
	printtxt2d((int)(0.499*set.width),(int)(0.231*set.height),0.8f,1.0f,g,1.0f,temp);
	
	if(selectedopt==12)
		g = 0.0f;
	else 
		g = 1.0f;
	strcpy(temp,"Details");
	printtxt2d((int)(0.206*set.width),(int)(0.191*set.height),0.8f,1.0f,g,1.0f,temp);
	if(!set.details)
		strcpy(temp,"low");
	else if(set.details == 1)
		strcpy(temp,"medium");
	else  if(set.details == 2)
		strcpy(temp,"high");
	else  if(set.details == 3 && shadersavailable)
		strcpy(temp,"ultra");
	else if(set.details == 3 && !shadersavailable)
		strcpy(temp,"high");
	printtxt2d((int)(0.499*set.width),(int)(0.191*set.height),0.8f,1.0f,g,1.0f,temp);


	
	strcpy(temp,"Resolution");
	printtxt2d((int)(0.206*set.width),(int)(0.151*set.height),0.8f,0.35f,0.35f,0.35f,temp);
	sprintf(temp,"%d x %d",set.width,set.height);
	printtxt2d((int)(0.499*set.width),(int)(0.151*set.height),0.8f,0.35f,0.35f,0.35f,temp);
	
	
	DrawRectangle((int)(0.029*set.width),(int)(0.82*set.height),(int)(0.941*set.width),(int)(0.751*set.height),0.0f,0.0f,1.0f);
	
}

void Options::SaveSettings()
{
	FILE *out = fopen("settings.ini","w");
	char temp[64];
	fputs("[CONTROLS]\n\n",out);
	
	
	sprintf(temp,"Forward1\t= %d\n",set.keys[FORWARD1]);
	fputs(temp,out);
	
	sprintf(temp,"Backward1\t= %d\n",set.keys[BACKWARD1]);
	fputs(temp,out);
	
	sprintf(temp,"Right1\t\t= %d\n",set.keys[RIGHT1]);
	fputs(temp,out);
	
	sprintf(temp,"Left1\t\t= %d\n",set.keys[LEFT1]);
	fputs(temp,out);
	
	sprintf(temp,"Jump1\t\t= %d\n",set.keys[JUMP1]);
	fputs(temp,out);
	
	sprintf(temp,"Reset1\t\t= %d\n",set.keys[RESET1]);
	fputs(temp,out);
	
	sprintf(temp,"Use1\t\t= %d\n",set.keys[USE1]);
	fputs(temp,out);
	
	sprintf(temp,"Forward2\t= %d\n",set.keys[FORWARD1+7]);
	fputs(temp,out);
	
	sprintf(temp,"Backward2\t= %d\n",set.keys[BACKWARD1+7]);
	fputs(temp,out);
	
	sprintf(temp,"Right2\t\t= %d\n",set.keys[RIGHT1+7]);
	fputs(temp,out);
	
	sprintf(temp,"Left2\t\t= %d\n",set.keys[LEFT1 +7]);
	fputs(temp,out);
	
	sprintf(temp,"Jump2\t\t= %d\n",set.keys[JUMP1+7]);
	fputs(temp,out);
	
	sprintf(temp,"Reset2\t\t= %d\n",set.keys[RESET1+7]);
	fputs(temp,out);
	
	sprintf(temp,"Use2\t\t= %d\n",set.keys[USE1+7]);
	fputs(temp,out);
	
	
	
	fputs("\n[GRAPHICS]\n\n",out);

	sprintf(temp,"Glow\t\t= %d\n",set.glow);
	fputs(temp,out);

	sprintf(temp,"Bloom\t\t= %d\n",set.bloom);
	fputs(temp,out);

	sprintf(temp,"Bloomw\t\t= %d\n",set.bloomw);
	fputs(temp,out);

	sprintf(temp,"Bloomh\t\t= %d\n",set.bloomh);
	fputs(temp,out);
	
	sprintf(temp,"Mipmaps\t\t= %d\n",set.mipmaps);
	fputs(temp,out);
	
	sprintf(temp,"Style\t\t= %d\n",set.textures);
	fputs(temp,out);
	
	sprintf(temp,"Details\t\t= %d\n",set.details);
	fputs(temp,out);
	
	sprintf(temp,"Draw-Dist\t= %d\n",set.drawdist);
	fputs(temp,out);

	sprintf(temp,"Shadow-Dist\t= %d\n",set.shadowdist);
	fputs(temp,out);
	
	sprintf(temp,"AnimSpeed\t= %d\n",53-set.animspeed);
	fputs(temp,out);
	
	sprintf(temp,"Speed\t\t= %d\n",set.speed);
	fputs(temp,out);
	
	sprintf(temp,"Shadows\t= %d\n",set.shadows);
	fputs(temp,out);
	
	fputs("\n[VIDEO]\n\n",out);
	
	sprintf(temp,"width\t\t= %d\n",set.width);
	fputs(temp,out);
	
	sprintf(temp,"height\t\t= %d\n",set.height);
	fputs(temp,out);
	sprintf(temp,"windowed\t= %d\n",set.windowed);
	fputs(temp,out);
	
	fputs("\n[SOUND]\n\n",out);
	
	sprintf(temp,"Sounds\t\t= %d\n",set.sounds);
	fputs(temp,out);
	
	
	fclose(out);
	
}



int Options::Input(SDL_Event action,SDL_keysym key)
{
	if(action.type == SDL_KEYDOWN)
	{
		if(key.sym == SDLK_ESCAPE)
		{
			SaveSettings();
			SwitchGamestate(GS_MENU);	
		}
		
		
		if(selectedopt > 0 && selectedopt < 8) //key mapping
		{
			if(key.sym == SDLK_RETURN)
			{
				mapping = true;	
				play(-1,S_SELECT);
				return 0;	
			}
			
			if(mapping)
			{
				mapping = false;
				set.keys[selectedopt-1+(2-selectedplayer)*7] = key.sym;
				play(-1,S_SELECT);
			}
			else if(key.sym == SDLK_RIGHT)
			{	
				selectedplayer = 2;
			}
			else if(key.sym == SDLK_LEFT)
			{	
				selectedplayer = 1;	
			}	
			
			
		}
		else	//options
		{
			if(key.sym == SDLK_RIGHT)
			{
				if(selectedopt == 8)
				{
					set.glow = !set.glow;
					play(-1,S_SELECT);
					return 0;
				}
				else if(selectedopt == 9)
				{
					set.shadows = !set.shadows;
					play(-1,S_SELECT);
					
					return 0;
				}
				else if(selectedopt == 10)
				{
					set.mipmaps = !set.mipmaps;
					play(-1,S_SELECT);
					InitTextures();
					
					return 0;
				}
				else if(selectedopt == 11)
				{
					set.textures = (set.textures+1)%num_dir;
					play(-1,S_SELECT);
					InitTextures();	//load textures once more to use correct set of textures
					
					return 0;
				}
				else if(selectedopt == 12)
				{
					set.details = (set.details+1)%4;
					play(-1,S_SELECT);
					return 0;
				}
				
			}
			if(key.sym == SDLK_LEFT)
			{
				if(selectedopt == 8)
				{
					set.glow = !set.glow;
					play(-1,S_SELECT);
					return 0;
				}
				else if(selectedopt == 9)
				{
					set.shadows = !set.shadows;
					play(-1,S_SELECT);
					
					return 0;
				}
				else if(selectedopt == 10)
				{
					set.mipmaps = !set.mipmaps;
					play(-1,S_SELECT);
					InitTextures();
		
					return 0;
				}
				else if(selectedopt == 11)
				{
					set.textures = (set.textures+num_dir-1)%num_dir;
					play(-1,S_SELECT);
					InitTextures();	//load textures once more to use correct set of textures
			
					return 0;
				}
				else if(selectedopt == 12)
				{
					set.details = (set.details+2)%3;
					play(-1,S_SELECT);
					return 0;
				}
			}
			
		}
		if(!mapping)
		{
			if(key.sym == SDLK_DOWN)
			{
				
				if(selectedopt < 12)
					selectedopt++;
				else
					selectedopt = 1;
				
			}
			else if(key.sym == SDLK_UP)
			{	
				if(selectedopt > 1)
					selectedopt--;
				else
					selectedopt = 12;
				
			}
		}
		
	}
	return 0;
	
}
