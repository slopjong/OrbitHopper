/*
#				Orbit-Hopper
#				TimeAttack.cpp
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

#include "../headers/Game.h"
#include "../headers/Structs.h"
#include "../headers/Ghost.h"
#include "../headers/Utils.h"
#include "../headers/Font.h"
#include "../headers/Console.h"
#include "../headers/SinglePlSel.h"
#include "../headers/TimeAttack.h"
#include "../headers/MapSel.h"
#include "../headers/Sound.h"



extern char debugmsg[64];
extern GLuint **anims;
#ifndef WIN32
float min(float a, float b);
#endif
// ghost stuff
extern bool ghost;
extern Vertex *ghostpos;
extern Vertex *ownpos;
extern int poscount;
extern int gh_entries;
extern int cur_poscount;
Vertex cur_ghostpos;
Vertex old_ghostpos;
extern bool blooming;

extern SingleData *data;
extern HighscoreEntry *HEntries;
extern Settings set;
extern unsigned int frames;
extern Gamestate *cur_gamestate;
extern Gamestate *gamestates[15];
extern int animcount;

int background[4] = {23,28,34,35};
int back = 0;
Gamestate *old_state = NULL;
bool wireframemode = false;
bool debugmsgs = false;


TimeAttack::TimeAttack()
{
	num_tiles = 0;
	floors = NULL; 	
}

TimeAttack::~TimeAttack()
{
	num_tiles = 0;
	
	if(floors)
	{
		delete[] floors;	
		floors = NULL; 
	}
	if(ownpos)
	{
		delete[] ownpos;
		ownpos = NULL;
	}
	
	if(ghostpos)
	{
		delete[] ghostpos;
		ghostpos = NULL;
	}
	
}

void TimeAttack::CleanUpLevel()
{
	if(floors)
	{
		delete[] floors;
		floors = NULL;
	}
	if(ghost)
	{
		delete[] ghostpos;
		ghostpos = NULL;
	}
	if(ownpos)
	{
		delete[] ownpos;
		ownpos = NULL;
	}
	
}

void TimeAttack::AdjustFov(float fov, bool force)
{
	if(fov > 170 || (!set.speed && fov != 35))
		return;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(fov <= Player.cam.fov-1.5 && !force)
		Player.cam.fov = Player.cam.fov-1.5;
	else
		Player.cam.fov = fov;
	Player.cam.pos.z = Player.self.pos.z-6000+(int)((Player.cam.fov-35)*40);
	Player.cam.pos.y = Player.self.pos.y+1800-(int)((Player.cam.fov-35)*6);
	gluPerspective(Player.cam.fov,(float)(set.width)/(float)(set.height),50.5f,400000.0f);
	glMatrixMode(GL_MODELVIEW);		
	
}


void TimeAttack::NewRun()
{	
	
	poscount = 250;
	cur_poscount = 0;
	
	
	if(ownpos)
	{
		delete[] ownpos;
		ownpos = NULL;	
	}
	ownpos = new Vertex[poscount];
	
	if(((MapSel *)gamestates[GS_MAPSEL])->cur_level >= 0)
	{
		char temp[70];
		strcpy(temp,"maps/");
		strcat(temp,HEntries[((MapSel *)gamestates[GS_MAPSEL])->cur_level].levelname);
		LoadGhost(temp);
	}
	
	Player.health = 100;
	Player.started = 0;
	Player.dir = 1;
	
	
	Player.tsec = Player.sec = Player.min = Player.run_time = 0;
	
		
	Player.self.f_speed = Player.self.s_speed = Player.self.v_speed =  0;
	Player.self.on_floor = Player.last_floor = NORMAL;
	Player.bumpedtime = 0;
	for(int h = 0;h<323;h++)
		Player.keys[h] = false;
		
	
	Player.self.pos.x = floors[0].cube->origin.x-floors[0].cube->ext.x/2;
	Player.self.pos.y = floors[0].cube->origin.y+150;
	Player.self.pos.z =	floors[0].cube->origin.z+500;
	Player.powerupused = 0;
	Player.powerup = 0;
	
	Player.self.oldpos = Player.self.pos;
	
	Player.fastest = false;
	
	//camera !!!
	
	AdjustFov((float)((float)Player.self.f_speed/8.0f)+35.0f,true);					
	Player.cam.pos.x = Player.self.pos.x;
	Player.cam.pos.y = Player.self.pos.y+1800;
	Player.cam.pos.z = Player.self.pos.z-6000;
	Player.cam.target = &Player.self.pos;
	Player.cam.up.x = 0;
	Player.cam.up.y = 1;
	Player.cam.up.z = 0;
	Player.cam.offsets.x = 0;
	Player.cam.offsets.y = 700;
	Player.cam.offsets.z = 1000;	
	Player.laston = NULL;
	for(int u = 0;u<10;u++)
	{
		Player.smoke[u].lifetime = 0;
		Player.smoke[u].zs = 1.0f;
	}
	animcount = 0;
	for(int j = 0;j< num_tiles;j++)
	{
		if(floors[j].cube->movetime)
			floors[j].cube->movecount = 0;
		if(!E_START(floors[j].cube->triggerflag))
		{
			floors[j].cube->triggered = 0;
			floors[j].cube->movecount = 0;
			floors[j].cube->new_type_time = 0;
		}
		
	}	
}


void TimeAttack::Init()
{
	//set background-image based on levelname
	back = (HEntries[((MapSel *)gamestates[GS_MAPSEL])->cur_level].levelname[4]+((MapSel *)gamestates[GS_MAPSEL])->cur_level)%4;
	//start new run
	Player.cam.fov = 35;
 	NewRun();	
 	strcpy(debugmsg,"");
}





int TimeAttack::LoadLevelData(char *filename)
{
	
	Vertex ori =  {-1750,0,0};
	
	FILE *in = fopen(filename,"rb");
	if(in == NULL)
	{
		fprintf(stderr, "Level not found.");
		return -1;	
	}
	
	fread(&num_tiles,sizeof(int),1,in);
	time1 = 0;
	time2 = 0;
	time3 = 0;
	if(num_tiles == -1) //new file
	{
		fread(&time1,sizeof(int),1,in);	//gold
		fread(&time2,sizeof(int),1,in); //silver
		fread(&time3,sizeof(int),1,in); //bronze
		fread(&num_tiles,sizeof(int),1,in);
	}		
	if(floors)
	{
		delete[] floors;
		floors = NULL;	
	}
	floors = new Tile[num_tiles];
	
	for(int u = 0;u<num_tiles;u++)
	{
		
		floors[u].cube = new Cube(ori,ori,ori,0);	
		floors[u].cube->new_type = 0;
		fread(&floors[u].type,sizeof(int),1,in);
		
		fread(&floors[u].cube->origin.x,sizeof(int),1,in);
		fread(&floors[u].cube->origin.y,sizeof(int),1,in);
		fread(&floors[u].cube->origin.z,sizeof(int),1,in);
		
		fread(&floors[u].cube->ext.x,sizeof(int),1,in);
		fread(&floors[u].cube->ext.y,sizeof(int),1,in);
		fread(&floors[u].cube->ext.z,sizeof(int),1,in);
		
		fread(&floors[u].cube->tiled.x,sizeof(int),1,in);
		fread(&floors[u].cube->tiled.y,sizeof(int),1,in);
		fread(&floors[u].cube->tiled.z,sizeof(int),1,in);

			
		fread(&floors[u].cube->tex[0],sizeof(int),1,in);
		fread(&floors[u].cube->tex[1],sizeof(int),1,in);
			
		fread(&floors[u].cube->flag1,sizeof(int),1,in);
		fread(&floors[u].cube->flag2,sizeof(int),1,in);
		fread(&floors[u].cube->flag3,sizeof(int),1,in);
		fread(&floors[u].cube->triggerflag,sizeof(int),1,in);
		
		if(floors[u].cube->flag1 && A_MOVE(floors[u].cube->triggerflag))//moving floor
			floors[u].cube->movetime = MOVETIME(floors[u].cube->triggerflag);
			
		if(!E_START(floors[u].cube->triggerflag))//needs to be triggered
			floors[u].cube->triggered = 0;	
		
	}
	fclose(in);
	LoadGhost(filename);
	return 0;
}



int TimeAttack::Input(SDL_Event action, SDL_keysym key)
{
	if(action.type==SDL_KEYDOWN)
	{
		if(key.sym == SDLK_ESCAPE)
		{
			CleanUpLevel();
		
			SwitchGamestate(GS_MAPSEL);
			
		}	
		else if(key.sym == set.keys[RESET1+7])
			NewRun();
		else if(key.sym == set.keys[JUMP1+7])
		{
			if(Player.self.on_floor >= 0  && Player.self.on_floor != SAND)
			{
				Player.self.v_speed = 125;
				play(-1,S_JUMP);
				
				Player.smoke[9].lifetime = SDL_GetTicks()+randomi(300,0,0,900);
				Player.smoke[9].origin = Player.self.pos;
				Player.smoke[9].origin.y+=100;
				Player.smoke[9].origin.x+=200;
				Player.smoke[9].type = 0;
				Player.smoke[9].r = Player.smoke[9].g = 0.75f;
				Player.smoke[9].b = 1.0f;
				Player.smoke[9].xs = 2.5f+((float)randomi(0,0,0,7))/10.0f;
				Player.smoke[9].ys = 2.5f+((float)randomi(0,0,0,7))/10.0f;
			}
		
		}
		else if(key.sym == SDLK_BACKQUOTE  || key.sym == SDLK_TAB)
		{
			old_state = cur_gamestate;
			cur_gamestate = gamestates[GS_CONSOLE];
			
		}
		else if(!Player.started)
			Player.started = 1;
			
		Player.keys[key.sym] = true;
	}
	else if(action.type == SDL_KEYUP)
	{
		Player.keys[key.sym] = false;
	}
	return 0;
}

void TimeAttack::DisplayHud()
{
	
	char temp[64];
	sprintf(temp,"%2d:",Player.min);
	printtxt2d((int)(0.78*set.width),(int)(0.06*set.height),1.0f,0.9f,0.9f,0.9f,temp);
	
	sprintf(temp,"%2d:",Player.sec);
	printtxt2d((int)(0.84*set.width),(int)(0.06*set.height),1.0f,0.9f,0.9f,0.9f,temp);
	
	sprintf(temp,"%2d",Player.tsec/10);
	printtxt2d((int)(0.9*set.width),(int)(0.06*set.height),1.0f,0.9f,0.9f,0.9f,temp);
	
	if(debugmsgs)
	{
 		char temp[64];
 		sprintf(temp,"f_speed:    %d",Player.self.f_speed);
		printtxt2d(0,(int)(0.977*set.height),0.6f,1.0f,1.0f,0.0f,temp);
		
		sprintf(temp,"s_speed:    %d",Player.self.s_speed);
		printtxt2d(0,(int)(0.957*set.height),0.6f,1.0f,1.0f,0.0f,temp);
		
		sprintf(temp,"v_speed:    %d",Player.self.v_speed);
		printtxt2d(0,(int)(0.937*set.height),0.6f,1.0f,1.0f,0.0f,temp);
		sprintf(temp,"ypos:       %d",Player.self.pos.y);
		printtxt2d(0,(int)(0.918*set.height),0.6f,1.0f,1.0f,0.0f,temp);
		
		sprintf(temp,"health:     %d",Player.health);
		printtxt2d(0,(int)(0.898*set.height),0.6f,1.0f,1.0f,0.0f,temp);
		
		sprintf(temp,"fps:        %d",frames);
		printtxt2d(0,(int)(0.879*set.height),0.6f,1.0f,1.0f,0.0f,temp);
		
		sprintf(temp,"Msg:        %s",debugmsg);
		printtxt2d(0,(int)(0.850*set.height),0.6f,1.0f,1.0f,0.0f,temp);
	}
	
}

extern GLuint sphere[1];
void TimeAttack::Render()
{
	gluLookAt(Player.cam.pos.x,Player.cam.pos.y,Player.cam.pos.z,																//where we are
  			Player.cam.target->x+Player.cam.offsets.x,Player.cam.target->y+Player.cam.offsets.y,Player.cam.target->z+Player.cam.offsets.z,	//where we look at
  			Player.cam.up.x,Player.cam.up.y,Player.cam.up.z);	
  				 		
	DrawBackGround(background[back],0);
	


	Tile *u = &floors[0]+num_tiles-1;
	while(u >= &floors[0] && (u->cube->origin.z+u->cube->ext.z+23000 >= Player.self.pos.z || u->cube->flag1))
	{
		if(u->cube->origin.z-set.drawdist < Player.self.pos.z && (u->cube->origin.z+u->cube->ext.z+5000 > Player.self.pos.z || u->cube->flag1))
			u->cube->Draw(false, u->cube->origin.z-Player.self.pos.z);
		u--;
	}

	

	Player.Draw();
	
	if(set.glow)
	{
		glEnable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		Tile *u = &floors[0]+num_tiles-1;
		while(u >= &floors[0] && u->cube->origin.z+u->cube->ext.z+23000 >= Player.self.pos.z)
		{
			if(u->cube->origin.z-set.drawdist < Player.self.pos.z)
			{
				
				if(u->type == HURT || (u->cube->new_type == HURT && u->cube->new_type_time > 0))//render red glow effect
				{
					u->cube->ext.y += 200;
					u->cube->ext.x += 200;
					u->cube->ext.z += 200;
					u->cube->origin.x += 100;
					u->cube->origin.y += 100;
					u->cube->origin.z -= 100;
					glColor4f(0.8f,0.0f,0.0f,0.3f);
					u->cube->Draw(true);
					u->cube->origin.y -= 100;
					u->cube->origin.x -= 100;
					u->cube->origin.z += 100;
					u->cube->ext.y -= 200;
					u->cube->ext.x -= 200;
					u->cube->ext.z -= 200;
					glColor3f(1.0f,1.0f,1.0f);
				}
				else if(u->type == ICE || (u->cube->new_type == ICE && u->cube->new_type_time > 0))//render blue glow effect
				{
					u->cube->ext.y += 200;
					u->cube->ext.x += 200;
					u->cube->ext.z += 200;
					u->cube->origin.x += 100;
					u->cube->origin.y += 100;
					u->cube->origin.z -= 100;
					glColor4f(0.0f,0.0f,1.0f,0.2f);
					u->cube->Draw(true);
					u->cube->origin.y -= 100;
					u->cube->origin.x -= 100;
					u->cube->origin.z += 100;
					u->cube->ext.y -= 200;
					u->cube->ext.x -= 200;
					u->cube->ext.z -= 200;
					glColor3f(1.0f,1.0f,1.0f);
				}
				
				
			}
			u--;
		}
		glEnable(GL_LIGHTING);
		if(!blooming)
			glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
	}

	if(set.shadows && !blooming)
	{	
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0f, 100.0f);

		u = &floors[0]+num_tiles-1;
		float lightdir[3] = {0,-1,0};
	
		while(u >= &floors[0] && (u->cube->origin.z+u->cube->ext.z+23000 >= Player.self.pos.z || u->cube->flag1))
		{
			if(u->cube->origin.z-set.shadowdist < Player.self.pos.z && (u->cube->origin.z+u->cube->ext.z+5000 > Player.self.pos.z || u->cube->flag1))
				u->cube->CreateShadow(lightdir);
			u--;
		}

		Player.CreateShadow(lightdir);
		glDepthMask(GL_TRUE);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		
		//zeichne folgendes nur überall dort, wo stencil buffer wert != 0 ist
		glStencilFunc(GL_NOTEQUAL, 0x0, 0xff);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		
		//zeichne eigentlichen Schatten - grau
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, 1, 1, 0, 0, 1);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		
		glEnable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glBlendFunc(GL_SRC_COLOR,GL_SRC_ALPHA);
		glColor4f(0.2f, 0.2f, 0.2f, 0.6f);
		glBegin(GL_QUADS);
			glVertex2i(0, 0);
			glVertex2i(0, 1);
			glVertex2i(1, 1);
			glVertex2i(1, 0);
		glEnd();
		glBlendFunc(GL_ONE,GL_ONE);
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		
		glDisable(GL_STENCIL_TEST);
	}

	if(Player.health < 95 && Player.self.on_floor != SAND)
	{
		unsigned int now = SDL_GetTicks();
		for(int k = 0;k<(95-Player.health)/9;k++)
		{
			if(Player.smoke[k].lifetime < now)
			{
				for(int l = k;l>0;l--)
					Player.smoke[l] = Player.smoke[l-1];
					
				Player.smoke[0].lifetime = now+randomi(250,0,0,850);
				Player.smoke[0].origin = Player.self.pos;
				Player.smoke[0].origin.x-=randomi(-240,0,0,80);
				Player.smoke[0].origin.y+=randomi(70,0,0,120);
				Player.smoke[0].origin.z-=120;
				Player.smoke[0].type = randomi(0,0,0,3);
				Player.smoke[0].r = Player.smoke[0].g = Player.smoke[0].b = (float)randomi(2,2,2,10)/10.0f;
				Player.smoke[0].xs = 1.0f+((float)randomi(0,0,0,7))/10.0f;
				Player.smoke[0].ys = 1.0f+((float)randomi(0,0,0,7))/10.0f;
				
			}		
		}
		for(int k = 0;k<(95-Player.health)/9;k++)
			DrawEffect(Player.smoke[k]);		
	}
	else if(Player.self.on_floor == SAND)
	{
		unsigned int now = SDL_GetTicks();
		for(int k = 0;k<10;k++)
		{
			if(Player.smoke[k].lifetime < now)
			{
				for(int l = k;l>0;l--)
					Player.smoke[l] = Player.smoke[l-1];
					
				Player.smoke[0].lifetime = now+randomi(100,0,0,330);
				Player.smoke[0].origin = Player.self.pos;
				Player.smoke[0].origin.x-=randomi(-280,0,0,-100);
				Player.smoke[0].origin.y+=randomi(100,0,0,160);
				Player.smoke[0].origin.z-=120;
				Player.smoke[0].type = randomi(0,0,0,3);
				Player.smoke[0].r = Player.smoke[0].g =  (float)randomi(20,0,0,70)/100.0f;
				Player.smoke[0].b = 0;
				Player.smoke[0].xs = 1.6f+((float)randomi(0,0,0,7))/10.0f;
				Player.smoke[0].ys = 1.6f+((float)randomi(0,0,0,7))/10.0f;
				
			}		
		}
		for(int k = 0;k<10;k++)
			DrawEffect(Player.smoke[k]);
				
	}

	
	if(Player.smoke[9].lifetime > SDL_GetTicks())
		DrawEffect(Player.smoke[9]);		
	
	if(ghost && Player.started)
	{
		glEnable(GL_BLEND);
		PlayerModel(1,0,old_ghostpos,cur_ghostpos,1.0f,8.0f,0,true);
		glDisable(GL_BLEND);
	}

	DisplayHud();
	
}

int TimeAttack::OnFloor(PlayerPawn &player)
{
	Pawn pawn = player.self;
	
	int floor_type = -1;
	bool on = false;
	Tile *i = &floors[0];
	Tile *max = &floors[0]+num_tiles;
	for(;i<max;i++)
	{
		bool onthis = false;
		switch(player.OnFloor(i))
		{
			case SKIP:
				i = max-1;	//skip next floors
			break;
			case OFF:
			case ABOVE:
			break;
			case ON:
				 if(i->cube->new_type_time <= 0)
					floor_type = i->type;
				else
					floor_type = i->cube->new_type;
				onthis = on = true;
			break;
			case LEFT_ON:
				floor_type = SIDE_BLOCK_R;	
				if(!on)
					player.self.v_speed -= 3;
				onthis = on = true;
			break;
			case RIGHT_ON:
				floor_type = SIDE_BLOCK_L;	
				if(!on)
					player.self.v_speed -= 3;
				onthis = on = true;
			break;
			case CRASH:						//we crashed into a floor
				play(-1,S_EXPLODE);
			
				char temp[64];
				sprintf(temp,"Died after %2d:%02d:%02d",player.min,player.sec,player.tsec/10);
				((Console *)gamestates[GS_CONSOLE])->AddConsoleEntry(temp);
				NewRun();
				return 0;
			break;
			
		};
		if(onthis)
		{
			if(player.laston != i)
			{
				if(E_TOUCH(i->cube->triggerflag) && T_OTHER(i->cube->triggerflag))
				{
					play(-1,S_PLACE);
					int j = OTHERINDEX(i->cube->triggerflag);
					while(true)
					{
						floors[j].cube->triggered = floors[j].cube->triggered ? 0 : -1;
						
						if(A_TYPE(floors[j].cube->triggerflag) && floors[j].cube->new_type_time == 0)
						{
							floors[j].cube->new_type = NEWTYPE(floors[j].cube->triggerflag)+1;
							floors[j].cube->new_type_time = 99999999;	
						}	
						else if(A_TYPE(floors[j].cube->triggerflag))
							floors[j].cube->new_type_time = 0;
							
						if(T_OTHER(floors[j].cube->triggerflag))
						{
							j = OTHERINDEX(floors[j].cube->triggerflag);
						}
						else
							break;
					}
			
					
				}
				player.laston = i;
			}
		}
		
			
	}
	return floor_type;
	
}

void TimeAttack::Update(unsigned int diff)
{
	static int framecount = 0;
	static int recordcount = 0;
	framecount += diff;
	
	if(Player.started)
	{
		Player.run_time +=diff;
		Player.tsec+=diff;
		if(Player.tsec >= 1000)
		{
			Player.tsec -=1000;
			Player.sec++;
			if(Player.sec >= 60)
			{
				Player.sec -= 60;	
				Player.min++;
			}
		}
		
	}
	
	if(framecount > 21)  //need 50+ fps to work the way it should
	{
		recordcount++;
		if(Player.started == 1)//synchronize first move
		{
			animcount = 0;
			Player.started = 2;
			Player.run_time = Player.tsec = Player.sec = Player.min = 0;
			recordcount = 0;
		}
		else if(Player.started == 2)
		{
			animcount++;
			for(int j = 0;j< num_tiles;j++)
			{
				if(floors[j].type == HURT)
				{
					floors[j].cube->red = floors[j].cube->red > 2.0 ? 0.0 : floors[j].cube->red+0.015;
				}
				
				
				if(floors[j].cube->movetime && floors[j].cube->triggered //floors should move
				&& floors[j].cube->movecount < floors[j].cube->movetime*500)	//reset movecount after 10 seconds
				{	
					if(A_MOVEMENT(floors[j].cube->triggerflag))//if floor is only supposed to do its movement once -> stop
					{
						int part1 = floors[j].cube->movecount/(floors[j].cube->movetime*50);
						int part2 = (floors[j].cube->movecount+1)/(floors[j].cube->movetime*50);
						if((int)part2%2 != (int)part1%2)
							floors[j].cube->triggered = 0;
					}
					floors[j].cube->movecount++;
				}
				else if(floors[j].cube->movecount >= floors[j].cube->movetime*500)
					floors[j].cube->movecount = 0;
				
			}
		}
	
			
		if(Player.self.v_speed < -440)
		{
			char temp[64];
			sprintf(temp,"Died after %2d:%02d:%02d",Player.min,Player.sec,Player.tsec/10);
			((Console *)gamestates[GS_CONSOLE])->AddConsoleEntry(temp);
			play(-1,S_EXPLODE);
			NewRun();
			return;	
		}
				
		int old_on_sth = Player.self.on_floor;
		if((Player.self.on_floor = OnFloor(Player))< 0)
		{
			Player.self.v_speed -= 8; //8 = gravity


			if(Player.keys[set.keys[FORWARD1+7]] 	//forward
			&& Player.self.f_speed < MAX_FSPEED
			&& Player.bumpedtime != -10000)
			{
				Player.self.f_speed +=3;
				AdjustFov((float)((float)Player.self.f_speed/8.0f)+35.0f,false);			
			}
			if(Player.keys[set.keys[BACKWARD1+7]]	//backward
			&& Player.self.f_speed > -MAX_BSPEED
			&& Player.bumpedtime != -10000)
			{
		
				Player.self.f_speed -=2;
			
				AdjustFov((float)((float)Player.self.f_speed/8.0f)+35.0f,false);	
			}
			if(Player.keys[set.keys[LEFT1+7]]		//left
			&& Player.self.on_floor != SIDE_BLOCK_L
			&& Player.self.on_floor != SIDE_BLOCK_R)
			{
				if(Player.self.s_speed < -85)
					Player.self.s_speed =-85; //slow down side-speed
			}
			if(Player.keys[set.keys[RIGHT1+7]] 	//right
			&& Player.self.on_floor != SIDE_BLOCK_R
			&& Player.self.on_floor != SIDE_BLOCK_L)
			{
				if(Player.self.s_speed > 85)
					Player.self.s_speed = 85; //slow down side-speed
					
			}

			
			
		}
		else
		{
			if(Player.bumpedtime == -10) //player left a pong match
			{
				if(Player.powerup != 100)//player is not on a pong-floor
					Player.bumpedtime = 0;
				Player.powerup = 0;
			}
			
			if(Player.self.v_speed < 0 
			&& Player.self.on_floor != SIDE_BLOCK_L
			&& Player.self.on_floor != SIDE_BLOCK_R)
			{
				play(2,S_BUMP);
				Player.self.v_speed = 0;
				if(old_on_sth < 0 &&  old_on_sth != Player.self.on_floor) //slow down right/left speed
					Player.self.s_speed = Player.self.s_speed/2;
			}
		
			
			int old_s_speed = Player.self.s_speed;
			if(Player.bumpedtime <= 0)
				Player.self.s_speed = 0;
				
			if(Player.keys[set.keys[FORWARD1+7]]
			&& Player.self.f_speed < MAX_FSPEED
			&& Player.bumpedtime != -10000) //player is not in a pong match
			{
				play(2,S_SPEEDUP);
				Player.self.f_speed +=4;
				
				
				AdjustFov((float)((float)Player.self.f_speed/8.0f)+35.0f,false);		
			}
			if(Player.keys[set.keys[BACKWARD1+7]]
			 && Player.self.f_speed > -MAX_BSPEED
			 && Player.bumpedtime != -10000)//player is not in a pong match
			{
				play(2,S_SLOWDOWN);
				Player.self.f_speed -=3;
				
				AdjustFov((float)((float)Player.self.f_speed/8.0f)+35.0f,false);		
			}
			if(Player.keys[set.keys[LEFT1+7]]	
			&& Player.self.s_speed < MAX_SSPEED
			&& Player.self.on_floor != SIDE_BLOCK_L
			&& Player.bumpedtime <= 0)
			{
				
				
				Player.self.s_speed +=(int)min(abs(Player.self.f_speed/4),70);
				Player.self.s_speed +=40;
				
				if(Player.self.s_speed < 0)
					Player.self.s_speed +=4;
			}
			if(Player.keys[set.keys[RIGHT1+7]] 
			&& Player.self.s_speed > -MAX_SSPEED
			&& Player.self.on_floor != SIDE_BLOCK_R
			&& Player.bumpedtime <= 0)
			{
				
				
				Player.self.s_speed -=(int)min(abs(Player.self.f_speed/4),70);
				Player.self.s_speed -=40;
				
				if(Player.self.s_speed > 0)
					Player.self.s_speed -=4;
			}
			if(Player.keys[set.keys[JUMP1+7]])
			{
				if(Player.self.on_floor == SIDE_BLOCK_L)
					Player.self.s_speed = -95; 
				if(Player.self.on_floor == SIDE_BLOCK_R)
					Player.self.s_speed = 95; 
				if(Player.self.v_speed < 100 && Player.self.on_floor != SAND)
				{
					Player.self.v_speed = 125;
					play(-1,S_JUMP);
					
					Player.smoke[9].lifetime = SDL_GetTicks()+randomi(300,0,0,900);
					Player.smoke[9].origin = Player.self.pos;
					Player.smoke[9].origin.y+=100;
					Player.smoke[9].origin.x+=200;
					Player.smoke[9].type = 0;
					Player.smoke[9].r = Player.smoke[9].g = 0.75f;
					Player.smoke[9].b = 1.0f;
					Player.smoke[9].xs = 2.5f+((float)randomi(0,0,0,7))/10.0f;
					Player.smoke[9].ys = 2.5f+((float)randomi(0,0,0,7))/10.0f;	
				}
			}
			
				
			switch(Player.self.on_floor)
			{
				case NORMAL:
				break;
				case GOAL:
					
					if(Player.run_time < HEntries[((MapSel *)gamestates[GS_MAPSEL])->cur_level].record_time)
						Player.fastest = true;
					SwitchGamestate(GS_LEVELEND);
					
					return;	
				break;
				case SPEED_UP:
					play(2,S_SPEEDUP);
					Player.self.f_speed +=5;
					
					AdjustFov((float)((float)Player.self.f_speed/8.0f)+35.0f,false);		
				break;
				case SLOW_DOWN:
					play(2,S_SLOWDOWN);
					Player.self.f_speed -=4;
					AdjustFov((float)((float)Player.self.f_speed/8.0f)+35.0f,false);			
				break;
				case RIGHT_PUSH:
					if(Player.self.on_floor == SIDE_BLOCK_R)
						break;
				
					Player.self.s_speed -=60;
					
				break;
				case LEFT_PUSH:
					if(Player.self.on_floor == SIDE_BLOCK_L)
						break;
					
						Player.self.s_speed +=60;

				break;
				case JUMP:
					play(-1,S_JUMP);
					Player.self.v_speed = (int)((float)abs(Player.self.f_speed)/1.9f);
				break;
				case HURT:
				{
					play(1,S_HURT);
					Player.health -= 3;
						
					if(Player.health <= 0)
					{
						play(-1,S_EXPLODE);
						NewRun();
						return;	
					}
				}
				break;
				case HEAL:
					if(Player.health < 100)
					{
						Player.health += 2;	
						play(1,S_HEAL);
					}
				break;
				case ICE:
					
					Player.self.s_speed =old_s_speed;
						
					if(Player.keys[set.keys[FORWARD1+7]] && Player.self.f_speed < MAX_FSPEED && Player.bumpedtime != -10000)//forward
						Player.self.f_speed -=3;
					if(Player.keys[set.keys[BACKWARD1+7]] && Player.self.f_speed > -MAX_BSPEED && Player.bumpedtime != -10000)//backward
						Player.self.f_speed +=2;
					if(Player.keys[set.keys[LEFT1+7]] && Player.self.s_speed > -MAX_SSPEED)
					{
						if(Player.self.s_speed < 100)
						{
							
							Player.self.s_speed +=abs(Player.self.f_speed/55);
							if(Player.self.s_speed > 0)
								Player.self.s_speed += 15;
							else
								Player.self.s_speed += 8;
						}
						else 
							Player.self.s_speed -= 7;
					}
					if(Player.keys[set.keys[RIGHT1+7]] && Player.self.s_speed < MAX_SSPEED)
					{
						if(Player.self.s_speed > -100)
						{
							
							Player.self.s_speed -=abs(Player.self.f_speed/55);
							if(Player.self.s_speed < 0)
								Player.self.s_speed -= 15;
							else
								Player.self.s_speed -= 8;
						}
						else 
							Player.self.s_speed +=7;
					}
				break;
				case CRAZY:
					if(Player.last_floor != CRAZY)
					{
						
						Player.cam.up.y = -Player.cam.up.y;
						
					}
					
				break;		
				case SAND:
				
				break;
				
			};
			Player.last_floor = Player.self.on_floor;
		}
		if(Player.self.f_speed != 0 && !Player.started)
			Player.started = 1;
		Player.self.oldpos = Player.self.pos;
		Player.self.pos.z	+= Player.self.f_speed;
		Player.self.pos.y	+= Player.self.v_speed;
		Player.self.pos.x	+= Player.self.s_speed;
		Player.cam.pos.z	+= Player.self.f_speed;	
		Player.cam.pos.y	+= Player.self.v_speed;	
		Player.cam.pos.x	+= Player.self.s_speed;

		if(Player.started)
		{
			//replay
			if(ghost) //do some replay smoothing
			{
				if(cur_poscount+1 < gh_entries)
				{
					double xoffs = ghostpos[cur_poscount+1].x - ghostpos[cur_poscount].x;
					double yoffs = ghostpos[cur_poscount+1].y - ghostpos[cur_poscount].y;
					double zoffs = ghostpos[cur_poscount+1].z - ghostpos[cur_poscount].z;
				
					double part = (double)recordcount/6.0;
				 
					old_ghostpos = cur_ghostpos;
					cur_ghostpos.x = ghostpos[cur_poscount].x+(int)(part*xoffs);
					cur_ghostpos.y = ghostpos[cur_poscount].y+(int)(part*yoffs);
					cur_ghostpos.z = ghostpos[cur_poscount].z+(int)(part*zoffs);
					
				}	
			}	

			if(recordcount > 5)
			{
				int oldpos = cur_poscount;
				cur_poscount = oldpos+1; 
			
				if(cur_poscount > poscount-1 	//need larger array to hold pos
				&& poscount < 7500) 			//7500 = 5 min ->cancel recording of ghost
				{
					poscount += 750;			//extend ghostarray for another 30 secs
					Vertex *temp1 = new Vertex[poscount];
					for(int j = 0;j<poscount-750;j++)
						temp1[j] = ownpos[j];
					delete[] ownpos;
					ownpos = temp1;

				}
		
				if(cur_poscount > oldpos && cur_poscount < poscount)
				{
					ownpos[cur_poscount] = Player.self.pos;	
					
				}
				recordcount = 0;
			}

		}
		else if(ghost)
		{
			cur_ghostpos = old_ghostpos = ghostpos[0];
			
		}	
		framecount = framecount%21;	
		
	}
	
	
		
		
			
	
}




