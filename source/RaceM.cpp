/*
#				Orbit-Hopper
#				RaceM.cpp : Race
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
#include "../headers/Structs.h"
#include "../headers/Utils.h"
#include "../headers/Font.h"
#include "../headers/RaceM.h"
#include "../headers/ModeSelM.h"
#include "../headers/Sound.h"


#include <iostream>
using namespace std;


extern Gamestate *gamestates[15];
extern Settings set;
extern Settings set;
extern GLuint **anims;
extern Gamestate *old_state;
extern Gamestate *cur_gamestate;
extern unsigned int frames;
extern bool debugmsgs;
extern int animcount;
extern bool blooming;
void LoadRaceMap(char filen[]);
float min(float a, float b);

extern int origwidth;
extern int origheight;

RaceM::RaceM()
{
	num_tiles = 0;
	floors = NULL;	
}

RaceM::~RaceM()
{
	if(floors)
	{
		delete[] floors;
		floors = NULL;
	}
}

void RaceM::LoadOptions()
{
	FILE *in = fopen("multiplayer.ini","r");
	
	if(!in)
	{
		fprintf(stderr,"Error: multiplayer.ini not found - using default settings.");	
		lives = 5;
		gravity = 8;
		return;
	}
	
	char temp[64];
	char *val;
	fgets(temp,64,in);
   	fgets(temp,64,in);
	fgets(temp,64,in);
	fgets(temp,64,in);
	fgets(temp,64,in);
	fgets(temp,64,in);
	fgets(temp,64,in);
	val = strchr(temp,'=');
   	lives = (val ? atoi(val+2): 1 );

	fgets(temp,64,in);
	val = strchr(temp,'=');
   	gravity = (val ? atoi(val+2): 1 );
	
   	if(lives <= 0 || lives > 10)
		lives = 5;
	if(gravity < 5 || gravity > 10)
		gravity = 8;
	
	
	fclose(in);

}


void RaceM::AdjustFov(PlayerPawn &Player,float fov, bool force)
{
	if(fov > 170)
		return;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(fov <= Player.cam.fov-1.5 && !force)
		Player.cam.fov = Player.cam.fov-1.5;
	else
		Player.cam.fov = fov;
	Player.cam.pos.z = Player.self.pos.z-5000+(int)((Player.cam.fov-35)*45);
	Player.cam.pos.y = Player.self.pos.y+1500-(int)((Player.cam.fov-35)*6);
	if(!blooming)
		gluPerspective(Player.cam.fov,(float)(set.width)/(float)(set.height/2),50.5f,400000.0f);
	else
		gluPerspective(Player.cam.fov,(float)(origwidth)/(float)(origheight/2),50.5f,400000.0f);
	glMatrixMode(GL_MODELVIEW);		
	
}


void RaceM::RestartPlayer(PlayerPawn &Player,int direction)
{
	
	Player.health = 100;
	Player.self.f_speed = Player.self.s_speed = Player.self.v_speed = 0;
	Player.pushtime = 0;
	Player.dir = direction;
	Player.self.on_floor = Player.last_floor = NORMAL;
	Player.laston = NULL;

	for(int h = 0;h<323;h++)
		Player.keys[h] = false;
		
	
	Player.self.pos.x = floors[0].cube->origin.x-floors[0].cube->ext.x/2;
	if(Player.dir != 1)
		Player.self.pos.x += 250;
	else
		Player.self.pos.x -= 250;
	Player.self.pos.y = floors[0].cube->origin.y+150;
	Player.self.pos.z =	floors[0].cube->origin.z+500;
	
	Player.self.oldpos = Player.self.pos;
	
	//camera !!!
	AdjustFov(Player,(float)((float)Player.self.f_speed/8.0f)+35.0f,true);					
	Player.cam.pos.x = Player.self.pos.x;
	Player.cam.pos.y = Player.self.pos.y+1500;
	
	Player.cam.pos.z = Player.self.pos.z-5000;
	
	Player.cam.target = &Player.self.pos;
	Player.cam.up.x = 0;
	Player.cam.up.y = 1;
	Player.cam.up.z = 0;
	Player.cam.offsets.x = 0;
	Player.cam.offsets.y = 700;
	
	Player.cam.offsets.z = 1000;
			
	for(int u = 0;u<10;u++)
	{
		Player.smoke[u].lifetime = 0;
		Player.smoke[u].zs = 1.0f;
	}
	
}

void RaceM::Init()
{
	Player1.cam.fov = 35;
	Player2.cam.fov = 35;
	LoadOptions();
	LoadRaceMap(((ModeSelM *)gamestates[GS_MODESELM])->ramaps[((ModeSelM *)gamestates[GS_MODESELM])->mapsel]);
	Player1.min = 0; 		// number of points earned
	Player2.min = 0;		// number of points earned
	Player1.sec = lives; 		// amount of lives
	Player2.sec = lives;		// amount of lives
	Player1.tsec = 0;		// distance driven
	Player2.tsec = 0;		// distance driven
	Player1.bumpedtime = Player2.bumpedtime =  0;
	Player1.powerup = -99999999; 	// longest distance driven
	Player2.powerup = -99999999; 	// longest distance driven
	Player1.powerupused = 0;
	Player2.powerupused = 0;
	Player1.protectiontime = 0;
	Player2.protectiontime = 0;
	Player1.wallpos.x = 0;
	Player1.wallpos.y = 0;
	Player1.wallpos.z = 0;
	Player2.wallpos.x = 0;
	Player2.wallpos.y = 0;
	Player2.wallpos.z = 0;
	Player1.started = 0;
	Player2.started = 0;
	
	RestartPlayer(Player1,1);
	RestartPlayer(Player2,2);
	
}




int RaceM::Input(SDL_Event action,SDL_keysym key)
{
	
	if(action.type == SDL_KEYDOWN)
	{
		if(key.sym == SDLK_ESCAPE)
		{
			SwitchGamestate(GS_MODESELM);
		}
		else if(key.sym == set.keys[RESET1])
		{
			RestartPlayer(Player1,Player1.dir);
		}
		else if(key.sym == set.keys[RESET1+7])
		{
			RestartPlayer(Player2,Player2.dir);
		}
		
		
		Player1.keys[key.sym] = true;
		Player2.keys[key.sym] = true;
	}
	if(action.type == SDL_KEYUP)
	{
		Player1.keys[key.sym] = false;
		Player2.keys[key.sym] = false;
	}
	else if(key.sym == SDLK_BACKQUOTE  || key.sym == SDLK_TAB)
	{
		old_state = cur_gamestate;
		cur_gamestate = gamestates[GS_CONSOLE];
			
	}
	
	return 0;
}



int RaceM::OnFloor(PlayerPawn &player)
{
	
	int floor_type = -1;
	bool on = false;
		
	Tile *i = &floors[0];
	Tile *max = i+num_tiles;
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
					floor_type = i->cube->new_type;	//save type of floor we are on
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
				player.sec--;
				player.tsec += player.self.pos.z-floors[0].cube->origin.z;
				if(player.self.pos.z-floors[0].cube->origin.z > player.powerup)
				{
					player.powerup = player.self.pos.z-floors[0].cube->origin.z;
					player.powerupused = player.self.pos.y;
					player.protectiontime = player.self.pos.x;
				}
				
				play(-1, S_EXPLODE);
				RestartPlayer(player,player.dir);
				return 0;
			break;
			
		};
		if(onthis)//do some triggering
		{
			if(player.laston != i)
			{
				if(E_TOUCH(i->cube->triggerflag) && T_OTHER(i->cube->triggerflag))
				{
					int j = OTHERINDEX(i->cube->triggerflag);
					
					play(-1, S_PLACE);
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


void RaceM::Update(unsigned int diff)
{
	
	UpdatePlayer(Player1,diff);
	UpdatePlayer(Player2,diff);
	
	
}

void RaceM::UpdatePlayer(PlayerPawn &Player, unsigned int diff)
{
	
	static int framecount1 = 0;
	static int framecount2 = 0;
	if(Player.dir == 1)
		framecount1 += diff;
	else
		framecount2 += diff;
		
	if(Player.sec <= 0) //player is dead <- no more lives left
		return;
		
	if(framecount1 > 21 && Player.dir == 1 
	|| framecount2 > 21 && Player.dir != 1)  //need 50+ fps to work the way it should
	{
		int keyadd = 0;
		if(Player.dir != 1)
			keyadd = 7;
		if(Player.bumpedtime > 0)
			Player.bumpedtime--;
				
		if(Player1.started == 1 || Player2.started == 1)//synchronize first movement of player
		{
			Player1.started = Player2.started = 2;
			animcount = 0;
			for(int j = 0;j< num_tiles;j++)
			{
				if(floors[j].cube->movetime)
				{
					floors[j].cube->movecount = 0;
					if(floors[j].cube->triggered == 1)	//has been triggered
						floors[j].cube->triggered = 0;
				}
			}
			
		}
		else if(Player1.started == 2 || Player2.started == 2)
		{
			animcount++;
			for(int j = 0;j< num_tiles;j++)
			{
				if(floors[j].type == HURT)
					floors[j].cube->red = floors[j].cube->red > 2.0 ? 0.0 : floors[j].cube->red+0.007;
				
				
				if(floors[j].cube->movetime && floors[j].cube->triggered //floor is moving
				&& floors[j].cube->movecount < floors[j].cube->movetime*500)	//resets movecount after 10 seconds
				{
					
					if(A_MOVEMENT(floors[j].cube->triggerflag))	//if floor is only supposed to do its movement once -> stop
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
			Player.sec--;
			Player.tsec += Player.self.pos.z-floors[0].cube->origin.z;
			if(Player.self.pos.z-floors[0].cube->origin.z > Player.powerup)
			{
				Player.powerup = Player.self.pos.z-floors[0].cube->origin.z;
				Player.powerupused = Player.self.pos.y;
				Player.protectiontime = Player.self.pos.x;
			}
			play(-1,S_EXPLODE);
			RestartPlayer(Player,Player.dir);
			return;	
		}
		
		int old_on_sth = Player.self.on_floor;
		if((Player.self.on_floor = OnFloor(Player))< 0)
		{
			Player.self.v_speed -= gravity; //8 = gravity
			
			if(Player.keys[set.keys[FORWARD1+keyadd]] 	//forward
			&& Player.self.f_speed < MAX_FSPEED)
			{
				Player.self.f_speed +=3;	
			}
			if(Player.keys[set.keys[BACKWARD1+keyadd]]	//backward
			&& Player.self.f_speed > -MAX_BSPEED)
			{
				Player.self.f_speed -=2;	
			}
			if(Player.keys[set.keys[LEFT1+keyadd]]		//left
			&& Player.self.s_speed < 0	
			&& Player.self.on_floor != SIDE_BLOCK_L)
			{
				if(Player.self.s_speed < -85)
					Player.self.s_speed =-85;
			}
			if(Player.keys[set.keys[RIGHT1+keyadd]] 	//right
			&& Player.self.s_speed > 0	
			&& Player.self.on_floor != SIDE_BLOCK_R)
			{
				if(Player.self.s_speed > 85)
					Player.self.s_speed = 85; 
				
			}
			
				
		}
		else
		{
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
				
			if(Player.keys[set.keys[FORWARD1+keyadd]] 	//forward
			&& Player.self.f_speed < MAX_FSPEED)
			{
				play(2,S_SPEEDUP);
				Player.self.f_speed +=4;	
			}
			if(Player.keys[set.keys[BACKWARD1+keyadd]]	//backward
			 && Player.self.f_speed > -MAX_BSPEED)
			{
				play(2,S_SLOWDOWN);
				Player.self.f_speed -=3;
			}
			if(Player.keys[set.keys[LEFT1+keyadd]]		//left
			&& Player.self.s_speed < MAX_SSPEED
			&& Player.self.on_floor != SIDE_BLOCK_L)
			{
				
				
				Player.self.s_speed +=min(abs(Player.self.f_speed/4),70);
				Player.self.s_speed +=40;
				
				if(Player.self.s_speed < 0)
					Player.self.s_speed +=4;
			}
			if(Player.keys[set.keys[RIGHT1+keyadd]] 	//right
			&& Player.self.s_speed > -MAX_SSPEED
			&& Player.self.on_floor != SIDE_BLOCK_R)
			{
				
				
				Player.self.s_speed -=min(abs(Player.self.f_speed/4),70);
				Player.self.s_speed -=40;
				
				if(Player.self.s_speed > 0)
					Player.self.s_speed -=4;
			}
			if(Player.keys[set.keys[JUMP1+keyadd]])//jump
			{
				if(Player.self.on_floor == SIDE_BLOCK_L)
					Player.self.s_speed = -95; 
				if(Player.self.on_floor == SIDE_BLOCK_R)
					Player.self.s_speed = 95;
				
				if(Player.self.on_floor != SAND)
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
					Player.tsec += Player.self.pos.z-floors[0].cube->origin.z;
					if(Player.self.pos.z-floors[0].cube->origin.z > Player.powerup)
					{
						Player.powerup = Player.self.pos.z-floors[0].cube->origin.z;
						Player.powerupused = Player.self.pos.y;
						Player.protectiontime = Player.self.pos.x;
					}
					Player.min++;
					play(1,S_HURT);
					RestartPlayer(Player,Player.dir);
					
				break;
				case SPEED_UP:
					play(2,S_SPEEDUP);
					Player.self.f_speed +=5;
				break;
				case SLOW_DOWN:
					play(2,S_SLOWDOWN);
					Player.self.f_speed -=4;			
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
						Player.sec--;
						Player.tsec += Player.self.pos.z-floors[0].cube->origin.z;
						if(Player.self.pos.z-floors[0].cube->origin.z > Player.powerup)
						{
							Player.powerup = Player.self.pos.z-floors[0].cube->origin.z;
							Player.powerupused = Player.self.pos.y;
							Player.protectiontime = Player.self.pos.x;
						}
						play(-1,S_EXPLODE);
						RestartPlayer(Player,Player.dir);
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
						
					if(Player.keys[set.keys[FORWARD1+keyadd]] && Player.self.f_speed < MAX_FSPEED)//forward
						Player.self.f_speed -=3;
					if(Player.keys[set.keys[BACKWARD1+keyadd]] && Player.self.f_speed > -MAX_BSPEED)//backward
						Player.self.f_speed +=2;
					if(Player.keys[set.keys[LEFT1+keyadd]] && Player.self.s_speed > -MAX_SSPEED)
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
					if(Player.keys[set.keys[RIGHT1+keyadd]] && Player.self.s_speed < MAX_SSPEED)
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
						Player.cam.up.y = -Player.cam.up.y;
					
				break;		
				
			};
			Player.last_floor = Player.self.on_floor;
		}
		if(Player.self.f_speed != 0 && !Player.started)
			Player.started = 1;
	
		Player.self.oldpos = Player.self.pos;
		Player.self.pos.z		+= Player.self.f_speed;
		Player.self.pos.y		+= Player.self.v_speed;
		Player.self.pos.x		+= Player.self.s_speed;
		Player.cam.pos.z		+= Player.self.f_speed;	
		Player.cam.pos.y		+= Player.self.v_speed;	
		Player.cam.pos.x		+= Player.self.s_speed;
			
		if(Player.dir == 1)
			framecount1 = framecount1%21;	
		else
			framecount2 = framecount2%21;	
	
	}
	
}


//collision detection between players
void RaceM::PlayerCrashRace()
{
	
	if(Length((float)Player1.self.pos.x,(float)Player1.self.pos.y,(float)Player1.self.pos.z,(float)Player2.self.pos.x,(float)Player2.self.pos.y,(float)Player2.self.pos.z)<440)
	{
		
		if(Player1.bumpedtime <= 0 && Player2.bumpedtime <= 0)
		{
			play(-1,S_BUMP);
			if(Player2.self.pos.x < Player1.self.pos.x) //player 2 is left
			{
				
				//speed up slower player
				int speed = Player2.self.f_speed > Player1.self.f_speed ? Player2.self.f_speed : Player1.self.f_speed;
				Player1.self.f_speed += (speed-Player1.self.f_speed)/2;
				Player2.self.f_speed += (speed-Player2.self.f_speed)/2;
				
				int offset = (Player1.self.pos.x-221)-(Player2.self.pos.x+221);
				Player1.self.pos.x -= offset/2;
				Player1.cam.pos.x -= offset/2;
				Player2.self.pos.x += offset/2;
				Player2.cam.pos.x += offset/2;
				
			}
			else if(Player2.self.pos.x >= Player1.self.pos.x)//player 2 is right
			{
				
				//accelerate slower player
				int speed = Player2.self.f_speed > Player1.self.f_speed ? Player2.self.f_speed : Player1.self.f_speed;
				Player1.self.f_speed += (speed-Player1.self.f_speed)/2;
				Player2.self.f_speed += (speed-Player2.self.f_speed)/2;
				
				int offset = (Player2.self.pos.x-221)-(Player1.self.pos.x+221);
				Player2.self.pos.x -= offset/2;
				Player2.cam.pos.x -= offset/2;
				Player1.self.pos.x += offset/2;
				Player1.cam.pos.x += offset/2;
				
				
			}
			
		}
		
	}
}

void RaceM::DisplayDebug()
{
	
	
	char temp[64];
	sprintf(temp,"f_speed 1|2:   %d | %d",Player2.self.f_speed, Player1.self.f_speed);
	printtxt2d(0,(int)(0.977*set.height),0.6f,1.0f,1.0f,0.0f,temp);
	
	sprintf(temp,"s_speed 1|2:   %d | %d",Player2.self.s_speed,Player1.self.s_speed);
	printtxt2d(0,(int)(0.957*set.height),0.6f,1.0f,1.0f,0.0f,temp);
	
	sprintf(temp,"v_speed 1|2:   %d | %d",Player2.self.v_speed,Player1.self.v_speed);
	printtxt2d(0,(int)(0.937*set.height),0.6f,1.0f,1.0f,0.0f,temp);
	sprintf(temp,"ypos 1|2:      %d | %d",Player2.self.pos.y,Player1.self.pos.y);
	printtxt2d(0,(int)(0.918*set.height),0.6f,1.0f,1.0f,0.0f,temp);
	
	sprintf(temp,"health 1|2:    %d | %d",Player2.health,Player1.health);
	printtxt2d(0,(int)(0.898*set.height),0.6f,1.0f,1.0f,0.0f,temp);
	
	sprintf(temp,"fps:           %d",frames);
	printtxt2d(0,(int)(0.879*set.height),0.6f,1.0f,1.0f,0.0f,temp);
		
	
}

void RaceM::Render()
{
	
	PlayerCrashRace();
	if(Player1.sec <= 0 && Player2.sec <= 0)
	{
		SwitchGamestate(GS_RACEEND);
		return;
	}

	int zet = Player1.powerup+floors[0].cube->origin.z;
	if(Player2.powerup+floors[0].cube->origin.z > zet)
		zet = Player2.powerup+floors[0].cube->origin.z;
		
	if(Player1.self.pos.z > zet)
		zet = Player1.self.pos.z;
	if(Player2.self.pos.z > zet)
		zet = Player2.self.pos.z;
	
	{
		//default direction: player 1
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();				
		if(set.speed)	
			AdjustFov(Player1,(float)((float)Player1.self.f_speed/8.0f)+35.0f,false);	
		else
			AdjustFov(Player1,35.0f,false);				
		
		glMatrixMode(GL_MODELVIEW);	
		glLoadIdentity();		
			
		gluLookAt(Player1.cam.pos.x,Player1.cam.pos.y,Player1.cam.pos.z,																//where we are
  			Player1.cam.target->x+Player1.cam.offsets.x,Player1.cam.target->y+Player1.cam.offsets.y,Player1.cam.target->z+Player1.cam.offsets.z,	//where we look at
  			Player1.cam.up.x,Player1.cam.up.y,Player1.cam.up.z);
  			
		int z = Player1.cam.pos.z;
		glColor3f(1.0f,1.0f,1.0f);				
		glViewport(0,0,set.width,set.height/2);
		
		DrawBackGround(35,1);
		bool blue = true;

		

		Tile *u = &floors[num_tiles-1];
		glColor3f(0.3,0.3,0.3);
		for(;u>=&floors[0];u--)
		{
			if(u->cube->origin.z+u->cube->ext.z+23000 < z)
				break;
			if(blue && u->cube->origin.z <= zet)
			{
				blue = false;
				glColor3f(1.0,1.0,1.0);
			}
			 
			if(u->cube->origin.z-set.drawdist < z)
				u->cube->Draw(false, u->cube->origin.z-Player1.self.pos.z);
			
		}
		

		Player1.Draw();
		Player2.Draw();
		

		if(set.shadows && !blooming && set.details >= 2)
		{	
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthMask(GL_FALSE);
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0.0f, 100.0f);

			
			float lightdir[3] = {0,-1,0};
	
			Player1.CreateShadow(lightdir);
			Player2.CreateShadow(lightdir);
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
		//glow effect
		if(set.glow && !blooming)
		{
			glEnable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);	
			Tile *u = &floors[num_tiles-1];
			for(;u>=&floors[0];u--)
			{
				if(u->cube->origin.z+u->cube->ext.z+23000 < z)
					break;
				if(u->cube->origin.z-set.drawdist < z)
				{
					if(u->type == HURT || u->cube->new_type== HURT && u->cube->new_type_time > 0)//render red glow effect
					{
						u->cube->ext.y += 300;
						u->cube->ext.x += 300;
						u->cube->ext.z += 300;
						u->cube->origin.x += 150;
						u->cube->origin.y += 150;
						u->cube->origin.z -= 150;
						glColor3f(1.0f,0.0f,0.0f);
						u->cube->Draw(true);
						u->cube->origin.y -= 150;
						u->cube->origin.x -= 150;
						u->cube->origin.z += 150;
						u->cube->ext.y -= 300;
						u->cube->ext.x -= 300;
						u->cube->ext.z -= 300;
						glColor3f(1.0f,1.0f,1.0f);
					}
					else if(u->type == ICE || u->cube->new_type== ICE && u->cube->new_type_time > 0)//render blue glow effect
					{
						u->cube->ext.y += 200;
						u->cube->ext.x += 200;
						u->cube->ext.z += 200;
						u->cube->origin.x += 100;
						u->cube->origin.y += 100;
						u->cube->origin.z -= 100;
						glColor3f(0.0f,0.0f,1.0f);
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
			}
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_LIGHTING);	
			glDisable(GL_BLEND);
		}
		
		
		if(Player2.smoke[9].lifetime > SDL_GetTicks())
			DrawEffect(Player2.smoke[9]);	
		if(Player1.smoke[9].lifetime > SDL_GetTicks())
			DrawEffect(Player1.smoke[9]);

		
		char score1[32];
		sprintf(score1,"Points: %d",Player1.min);
		printtxt2d((int)(0.75*set.width),(int)(0.1*set.height),0.9,1.0,1.0,1.0,score1);
		sprintf(score1,"Distance: %d",(Player1.tsec+Player1.self.pos.z-floors[0].cube->origin.z)/500);
		printtxt2d((int)(0.75*set.width),(int)(0.05*set.height),0.9,1.0,1.0,1.0,score1);
		
			
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);									
			glMatrixMode(GL_PROJECTION);						
			glPushMatrix();									
			glLoadIdentity();									
			gluOrtho2D(0,set.width,0,set.height);							
			glMatrixMode(GL_MODELVIEW);						
			glPushMatrix();									
				glLoadIdentity();						
					
					glColor3f(0.8f,0.0f,0.1f);
					glBegin(GL_QUADS);
						for(int u = 0;u<Player1.sec-1;u++)
						{	
							glVertex2i((int)(0.03*set.width*u + 0.82*set.width),(int)(0.95*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.83*set.width),(int)(0.96*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.82*set.width),(int)(0.97*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.81*set.width),(int)(0.96*set.height));
								
						}	
					glEnd();							
					glEnable(GL_BLEND);
					glBegin(GL_QUADS);
						for(int u = 0;u<Player1.sec-1;u++)
						{	
							glVertex2i((int)(0.03*set.width*u + 0.82*set.width),(int)(0.945*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.835*set.width),(int)(0.96*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.82*set.width),(int)(0.975*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.805*set.width),(int)(0.96*set.height));
								
						}	
					glEnd();
											
					glDisable(GL_BLEND);			
			glMatrixMode(GL_PROJECTION);						
			glPopMatrix();										
			glMatrixMode(GL_MODELVIEW);							
			glPopMatrix();	
		glEnable(GL_LIGHTING);											
		glEnable(GL_DEPTH_TEST);								
		glEnable(GL_TEXTURE_2D);
	
		
	}
	
	{
		glColor3f(1.0f,1.0f,1.0f);		
		//alternative direction: player 2
		if(set.speed)	
			AdjustFov(Player2,(float)((float)Player2.self.f_speed/8.0f)+35.0f,false);	
		else
			AdjustFov(Player2,35.0f,false);		
		glLoadIdentity();		
			
		gluLookAt(Player2.cam.pos.x,Player2.cam.pos.y,Player2.cam.pos.z,																//where we are
  			Player2.cam.target->x+Player2.cam.offsets.x,Player2.cam.target->y+Player2.cam.offsets.y,Player2.cam.target->z+Player2.cam.offsets.z,	//where we look at
  			Player2.cam.up.x,Player2.cam.up.y,Player2.cam.up.z);
  			
		int z = Player2.cam.pos.z;
					
		glViewport(0,set.height/2,set.width,set.height/2);
		
		DrawBackGround(35,1);
		bool blue = true;

		

		Tile *u = &floors[num_tiles-1];
		glColor3f(0.3,0.3,0.3);
		for(;u>=&floors[0];u--)
		{
			if(u->cube->origin.z+u->cube->ext.z+23000 < z)
				break;
			if(blue && u->cube->origin.z <= zet)
			{
				blue = false;
				glColor3f(1.0,1.0,1.0);
			}
			 
			if(u->cube->origin.z-set.drawdist < z)
				u->cube->Draw(false, Player2.self.pos.z-u->cube->origin.z);
			
		}
		

		Player2.Draw();
		Player1.Draw();
		
		if(set.shadows && !blooming && set.details >= 2)
		{	
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthMask(GL_FALSE);
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0.0f, 100.0f);

			
			float lightdir[3] = {0,-1,0};
	
			Player2.CreateShadow(lightdir);
			Player1.CreateShadow(lightdir);
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
			glEnable(GL_LIGHTING);	
			glDisable(GL_BLEND);
		
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_TEXTURE_2D);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		
			glDisable(GL_STENCIL_TEST);
		}
		if(set.glow && !blooming)
		{
			glDisable(GL_LIGHTING);	
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			
			Tile *u = &floors[num_tiles-1];
			for(;u>=&floors[0];u--)
			{
				if(u->cube->origin.z+u->cube->ext.z+23000 < z)
					break;
				if(u->cube->origin.z-set.drawdist < z)
				{
					if(u->type == HURT || u->cube->new_type== HURT && u->cube->new_type_time > 0)//render red glow effect
					{
						u->cube->ext.y += 300;
						u->cube->ext.x += 300;
						u->cube->ext.z += 300;
						u->cube->origin.x += 150;
						u->cube->origin.y += 150;
						u->cube->origin.z -= 150;
						glColor3f(1.0f,0.0f,0.0f);
						u->cube->Draw(true);
						u->cube->origin.y -= 150;
						u->cube->origin.x -= 150;
						u->cube->origin.z += 150;
						u->cube->ext.y -= 300;
						u->cube->ext.x -= 300;
						u->cube->ext.z -= 300;
						glColor3f(1.0f,1.0f,1.0f);
					}
					else if(u->type == ICE || u->cube->new_type== ICE && u->cube->new_type_time > 0)//render blue glow effect
					{
						u->cube->ext.y += 200;
						u->cube->ext.x += 200;
						u->cube->ext.z += 200;
						u->cube->origin.x += 100;
						u->cube->origin.y += 100;
						u->cube->origin.z -= 100;
						glColor3f(0.0f,0.0f,1.0f);
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
			}
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_LIGHTING);	
			glDisable(GL_BLEND);
		}
		
		if(Player2.smoke[9].lifetime > SDL_GetTicks())
			DrawEffect(Player2.smoke[9]);	
		if(Player1.smoke[9].lifetime > SDL_GetTicks())
			DrawEffect(Player1.smoke[9]);	

		
		char score1[32];
		sprintf(score1,"Points: %d",Player2.min);
		printtxt2d((int)(0.75*set.width),(int)(0.1*set.height),0.9,1.0,1.0,1.0,score1);
		sprintf(score1,"Distance: %d",(Player2.tsec+Player2.self.pos.z-floors[0].cube->origin.z)/500);
		printtxt2d((int)(0.75*set.width),(int)(0.05*set.height),0.9,1.0,1.0,1.0,score1);
		
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);									
			glMatrixMode(GL_PROJECTION);						
			glPushMatrix();									
			glLoadIdentity();									
			gluOrtho2D(0,set.width,0,set.height);							
			glMatrixMode(GL_MODELVIEW);						
			glPushMatrix();									
				glLoadIdentity();						
					glColor3f(0.8f,0.0f,0.1f);
					glBegin(GL_QUADS);
						for(int u = 0;u<Player2.sec-1;u++)
						{	
							glVertex2i((int)(0.03*set.width*u + 0.82*set.width),(int)(0.95*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.83*set.width),(int)(0.96*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.82*set.width),(int)(0.97*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.81*set.width),(int)(0.96*set.height));
								
						}	
					glEnd();
					glEnable(GL_BLEND);
					glBegin(GL_QUADS);
						for(int u = 0;u<Player2.sec-1;u++)
						{	
							glVertex2i((int)(0.03*set.width*u + 0.82*set.width),(int)(0.945*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.835*set.width),(int)(0.96*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.82*set.width),(int)(0.975*set.height));
							glVertex2i((int)(0.03*set.width*u + 0.805*set.width),(int)(0.96*set.height));
								
						}	
					glEnd();
												
					glDisable(GL_BLEND);						
			glMatrixMode(GL_PROJECTION);						
			glPopMatrix();										
			glMatrixMode(GL_MODELVIEW);							
			glPopMatrix();	
		glEnable(GL_LIGHTING);											
		glEnable(GL_DEPTH_TEST);								
		glEnable(GL_TEXTURE_2D);
		
		

	}
	
	glViewport(0,0,set.width,set.height);		
	if(debugmsgs)
		DisplayDebug();
	
}



void RaceM::LoadRaceMap(char filen[])
{
	if(floors)
	{
		delete[] floors;
		floors = NULL;
	}
		
	char temp[64];
	strcpy(temp,"maps/");
	strcat(temp,filen);
	
	FILE *in = fopen(temp,"rb");
	if(in == NULL)
		return;

	

	fread(&num_tiles,sizeof(int),1,in);
	if(num_tiles == -1)//new type
	{
		fread(&num_tiles,sizeof(int),1,in);
		fread(&num_tiles,sizeof(int),1,in);
		fread(&num_tiles,sizeof(int),1,in);
		fread(&num_tiles,sizeof(int),1,in);
	}
	
	if(floors)
	{
		delete[] floors;
		floors = NULL;	
	}

	

	floors = new Tile[num_tiles];
	
	Vertex a = {1,1,1};
	for(int u = 0;u<num_tiles;u++)
	{
		
		floors[u].cube = new Cube(a,a,a,0);
		fread(&floors[u].type,sizeof(int),1,in);
	
		floors[u].pu_time = 0;
		floors[u].cube->new_type_time = 0;
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
	
	
}
