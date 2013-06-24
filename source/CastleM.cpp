/*
#				Orbit-Hopper
#				CastleM.cpp : CastleAttack
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
#include "SDL_mixer.h"



#include "../headers/Game.h"
#include "../headers/Structs.h"
#include "../headers/Utils.h"
#include "../headers/Font.h"
#include "../headers/CastleM.h"
#include "../headers/ModeSelM.h"
#include "../headers/Sound.h"



extern Settings set;
extern GLuint **anims;
extern Gamestate *gamestates[15];
extern Gamestate *old_state;
extern Gamestate *cur_gamestate;
extern unsigned int frames;
extern bool debugmsgs;
extern int animcount;
extern bool animated[NUM_TEXTURES];
extern bool blooming;
#ifndef WIN32
float min(float a, float b);
#endif
void PowerUp(int angle, Vertex pos, float r, float g, float b);
void DrawPowerUp(int x,int y, int type);
extern int origwidth;
extern int origheight;

CastleM::CastleM()
{
	player1goalminindex = 0;
	player2goalmaxindex = 0;
	anglecount = 0;		//current angle of powerups
	num_tiles = 0;		
	floors = NULL;
	starttile = 0;
}

CastleM::~CastleM()
{
	if(floors)
	{
		for(int i = 0;i<num_tiles;i++)
			delete floors[i];
		delete[] floors;
		floors = NULL;
	}
}

void CastleM::LoadOptions()
{
	
	FILE *in = fopen("multiplayer.ini","r");
	
	if(!in)
	{
		fprintf(stderr,"Error: multiplayer.ini not found - using default settings.");	
		lives = lives = 5;
		gravity = gravity = 8;
		powerups = true;
		return;
	}
	
	char temp[64];
	char *val;
	fgets(temp,64,in);
     	
   	fgets(temp,64,in);
   	val = strchr(temp,'=');
    lives = (val ? atoi(val+2): 0 );

	fgets(temp,64,in);
	val = strchr(temp,'=');
   	powerups = (val ? atoi(val+2): 1 );

	fgets(temp,64,in);
	val = strchr(temp,'=');
   	gravity = (val ? atoi(val+2): 1 );
	
   	if(lives <= 0 || lives > 10)
		lives = 5;
	if(gravity < 5 || gravity > 10)
		gravity = 8;
	
	
	fclose(in);
}

//gives us this nice effect of speed
void CastleM::AdjustFov(PlayerPawn &Player,float fov, bool force)
{
	if(fov > 170)
		return;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(fov <= Player.cam.fov-1.5 && !force)
		Player.cam.fov = Player.cam.fov-1.5;
	else
		Player.cam.fov = fov;
	Player.cam.pos.z = Player.self.pos.z-Player.dir*5000+Player.dir*(int)((Player.cam.fov-35)*45);
	Player.cam.pos.y = Player.self.pos.y+1500-(int)((Player.cam.fov-35)*6);

	if(!blooming)
		gluPerspective(Player.cam.fov,(float)(set.width)/(float)(set.height/2),50.5f,400000.0f);
	else
		gluPerspective(Player.cam.fov,(float)(origwidth)/(float)(origheight/2),50.5f,400000.0f);
		
	
	glMatrixMode(GL_MODELVIEW);	
}




void CastleM::ResetPlayer(PlayerPawn &Player,int direction)
{
	
	Player.health = 100;		//restore health completely
	Player.self.f_speed = Player.self.s_speed = Player.self.v_speed = 0; //stop !
	Player.pushtime = 0;		
	Player.dir = direction;
	Player.self.on_floor = Player.last_floor = NORMAL;
	

	for(int h = 0;h<323;h++)
		Player.keys[h] = false;		//no keys are pressed
		
	if(direction == -1) //player 2 - starts at the end of the level
	{
		Player.self.pos.x = floors[starttile]->cube->origin.x-floors[starttile]->cube->ext.x/2;
		Player.self.pos.y = floors[starttile]->cube->origin.y+150;
		Player.self.pos.z =	floors[starttile]->cube->origin.z+floors[starttile]->cube->ext.z-500;
	}
	else //player 1 - starts at the beginning of the level
	{
		Player.self.pos.x = floors[0]->cube->origin.x-floors[0]->cube->ext.x/2;
		Player.self.pos.y = floors[0]->cube->origin.y+150;
		Player.self.pos.z =	floors[0]->cube->origin.z+500;
	}
	
	Player.self.oldpos = Player.self.pos; //no old position - load current one
	
	//camera !!!
	AdjustFov(Player,(float)((float)Player.self.f_speed/8.0f)+35.0f,true);					
	Player.cam.pos.x = Player.self.pos.x;
	Player.cam.pos.y = Player.self.pos.y+1500;
	if(direction == -1)
		Player.cam.pos.z = Player.self.pos.z+5000;
	else
		Player.cam.pos.z = Player.self.pos.z-5000;
	Player.cam.target = &Player.self.pos;
	Player.cam.up.x = 0;
	Player.cam.up.y = 1;
	Player.cam.up.z = 0;
	Player.cam.offsets.x = 0;
	Player.cam.offsets.y = 700;
	Player.laston = NULL;
	if(direction == -1)
		Player.cam.offsets.z = -1000;	
	else
		Player.cam.offsets.z = 1000;
			
	for(int u = 0;u<10;u++) //reset smoke-puffs
	{
		Player.smoke[u].lifetime = 0;
		Player.smoke[u].zs = 1.0f;
	}
	
}


void CastleM::Init()
{
	LoadOptions();
	player1goalminindex = 0;
	player2goalmaxindex = 0;
	anglecount = 0;		
	num_tiles = 0;
	floors = NULL;
	starttile = 0;
	
	LoadCastleMap(((ModeSelM *)gamestates[GS_MODESELM])->camaps[((ModeSelM *)gamestates[GS_MODESELM])->mapsel]);
	Player1.cam.fov = 35;
	Player2.cam.fov = 35;
	Player1.min = lives; 	// score of player 1
	Player2.min = lives; 	// score of player 2
	Player1.sec = 0; 	// number of deaths
	Player2.sec = 0;	// number of deaths
	Player1.tsec = 0;	// powerups used
	Player2.tsec = 0;	// powerups used
	Player1.bumpedtime = Player2.bumpedtime =  0;
	Player1.powerup = 0;
	Player2.powerup = 0;
	Player1.powerupused = 0;
	Player2.powerupused = 0;
	Player1.protectiontime = 0;
	Player2.protectiontime = 0;
	Player1.wallpos.x = Player1.wallpos.y = Player1.wallpos.z = 0;
	Player2.wallpos.x = Player2.wallpos.y = Player2.wallpos.z = 0;
	Player1.started = 0;
	Player2.started = 0;
	
	anglecount = 0;
	
	
	//count goal floors
	int goal = 0;
	for(int i = 0;i<num_tiles*2;i++)
		if(floors[i]->type == GOAL)
			goal++;
	
	//set max and min goal indices to make sure hits are counted correctly
	int g = 0;
	for(int j = 0;j<num_tiles*2;j++)
	{
		if(floors[j]->type == GOAL)
		{
			if(g < goal/2)
			{
				g++;
				player2goalmaxindex = j;
			}
			else
			{
				player1goalminindex = j;
				break;	
			}
		}
	}
	ResetPlayer(Player1,1);
	ResetPlayer(Player2,-1);
}




int CastleM::Input(SDL_Event action,SDL_keysym key)
{
	//player 2 has a key-offset of 7
	if(action.type == SDL_KEYDOWN)
	{
		if(key.sym == SDLK_ESCAPE)
		{
			SwitchGamestate(GS_MODESELM);
		}
		else if(key.sym == set.keys[RESET1])
		{
			ResetPlayer(Player1,Player1.dir);
		}
		else if(key.sym == set.keys[RESET1+7])
		{
			ResetPlayer(Player2,Player2.dir);
		}
		else if(key.sym == set.keys[USE1])
		{
			
			if(Player1.powerup == PU_SHIELD) //Shield Powerup
			{
				Player1.powerupused = Player1.powerup;
				Player1.protectiontime = 1200;
				Player1.powerup = 0;
				Player1.tsec++;
				play(1,S_PUSH);
				
			}
			else if(Player1.powerup == PU_PUSH) //Push powerup
			{
				Player1.powerupused = Player1.powerup;
				Player1.pushtime = 5;
				Player1.powerup = 0;
				Player1.tsec++;
				play(1,S_PUSH);
			}
			else if(Player1.powerup != PU_DJUMP && Player1.self.on_floor >= 0)
			{
				Player1.powerupused = Player1.powerup;
				Player1.powerup = 0;
				Player1.tsec++;
				
				
			}
		}
		else if(key.sym == set.keys[USE1+7])
		{
			
			if(Player2.powerup == PU_SHIELD) //Shield Powerup
			{
				Player2.powerupused = Player2.powerup;
				Player2.protectiontime = 1200;
				Player2.powerup = 0;
				Player2.tsec++;
				
			}
			else if(Player2.powerup == PU_PUSH) //Push powerup
			{
				Player2.powerupused = Player2.powerup;
				Player2.pushtime = 5;
				Player2.powerup = 0;
				Player2.tsec++;
				play(1,S_PUSH);
				
			}
			else if(Player2.powerup != PU_DJUMP && Player2.self.on_floor >= 0)
			{
				Player2.powerupused = Player2.powerup;
				Player2.powerup = 0;
				Player2.tsec++;
				
				
			}
		}
		else if(key.sym == SDLK_BACKQUOTE || key.sym == SDLK_TAB) //opens console
		{
			old_state = cur_gamestate;
			cur_gamestate = gamestates[GS_CONSOLE];
			
		}
		
		
		Player1.keys[key.sym] = true;
		Player2.keys[key.sym] = true;
	}
	if(action.type == SDL_KEYUP)
	{
		Player1.keys[key.sym] = false;
		Player2.keys[key.sym] = false;
	}
	
	return 0;
	
}



//collision detection b/w player and floor
int CastleM::OnFloor(PlayerPawn &player)
{

	int floor_type = -1;
	bool on = false;
	Tile **i = &floors[0];
	Tile **max = i+num_tiles*2;

	int count = 0;
	for(;i<max;i++,count++)
	{
		bool onthis = false;
		switch(player.OnFloor((*i)))
		{
			case SKIP:
				i = max-1;	//skip next floors
			break;
			case OFF:
			case ABOVE:
			break;
			case ON:
				if(player.powerupused == PU_WALL) //places wall
				{
					if((*i)->type != GOAL && i != &floors[0] && count != starttile)
					{
						player.wallpos.x = player.self.pos.x;
						player.wallpos.y = (*i)->cube->origin.y;
						player.wallpos.z = player.self.pos.z-player.dir*120;
						play(-1,S_PLACE);
					}
					else
						player.powerup = player.powerupused;
					player.powerupused = 0;
				}
				if(player.powerupused == PU_ICE) //transforms  into ICE floors
				{
					if((*i)->type != GOAL && i != &floors[0] && count != starttile)
					{
						floor_type = (*i)->cube->new_type = ICE;
						(*i)->cube->new_type_time = 2000;
						play(-1,S_PLACE);
					}
					else
						player.powerup = player.powerupused;
					player.powerupused = 0;
				}
				else if(player.powerupused == PU_HURT) //transforms  into DAMAGE floors
				{
					if((*i)->type != GOAL && i != &floors[0] && count != starttile)
					{
						floor_type = (*i)->cube->new_type = HURT;
						(*i)->cube->new_type_time = 2000;
						play(-1,S_PLACE);
					}
					else
						player.powerup = player.powerupused;
					player.powerupused = 0;
				}
				else if((*i)->cube->new_type_time <= 0)
					floor_type = (*i)->type;
				else
					floor_type = (*i)->cube->new_type;
				
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
				player.sec++;
				play(-1,S_EXPLODE);
				ResetPlayer(player,player.dir);
				return 0;
			break;
			
		};
		if(onthis)//tigger floors
		{
			if(player.laston != (*i))
			{
				if(E_TOUCH((*i)->cube->triggerflag) && T_OTHER((*i)->cube->triggerflag))//triggered on touch
				{
					int j = OTHERINDEX((*i)->cube->triggerflag);
					play(-1,S_PLACE);
	
					while(true)//trigger "remote" floors
					{
						floors[j]->cube->triggered = floors[j]->cube->triggered ? 0 : -1;
						
						if(A_TYPE(floors[j]->cube->triggerflag) && floors[j]->cube->new_type_time == 0)
						{
							floors[j]->cube->new_type = NEWTYPE(floors[j]->cube->triggerflag)+1;
							floors[j]->cube->new_type_time = 99999999;	
						}	
						else if(A_TYPE(floors[j]->cube->triggerflag))
							floors[j]->cube->new_type_time = 0;
							
						if(T_OTHER(floors[j]->cube->triggerflag))
						{
							j = OTHERINDEX(floors[j]->cube->triggerflag);
						}
						else
							break;
					}
			
					
				}
				player.laston = (*i);
			}
		}
	}
	return floor_type;
	
}

void CastleM::Update(unsigned int diff)
{
	UpdatePlayer(Player1,diff);
	UpdatePlayer(Player2,diff);
	
}

void CastleM::UpdatePlayer(PlayerPawn &Player, unsigned int diff)
{
	static int framecount1 = 0;
	static int framecount2 = 0;
	if(Player.dir > 0)
		framecount1 += diff;
	else
		framecount2 += diff;
		
	if(framecount1 > 21 && Player.dir > 0 
	|| framecount2 > 21 && Player.dir < 0)  //need 50+ fps to work the way it should
	{
		int keyadd = 0;
		if(Player.dir < 0)
			keyadd = 7;
			
		if(Player.bumpedtime > 0)
			Player.bumpedtime--;
		if(Player.protectiontime > 0)
			Player.protectiontime --;
		if(Player.pushtime > 0)
			Player.pushtime --;
				
		if(Player1.started == 1 || Player2.started == 1)//exact synchronization of the player's first move
		{
			animcount = 0;
			Player1.started = Player2.started = 2;
			for(int j = 0;j< num_tiles*2;j++)
			{
				if(floors[j]->cube->movetime)
				{
					floors[j]->cube->movecount = 0;
					if(floors[j]->cube->triggered == 1)	//has been triggered
						floors[j]->cube->triggered = 0;	//set to an untriggered state
				}
			}
			
		}
		else if(Player1.started == 2 || Player2.started == 2)
		{
			animcount++;
			for(int j = 0;j< num_tiles*2;j++)
			{
				if(floors[j]->type == HURT)
					floors[j]->cube->red = floors[j]->cube->red > 2.0 ? 0.0 : floors[j]->cube->red+0.007;
				
				
				if(floors[j]->cube->movetime && floors[j]->cube->triggered //moving floor, that is already moving
				&& floors[j]->cube->movecount < floors[j]->cube->movetime*500)	//reset movecount after 10 seconds
				{
					
					if(A_MOVEMENT(floors[j]->cube->triggerflag))//if floor is only supposed to do its movement once -> stop
					{
						int part1 = floors[j]->cube->movecount/(floors[j]->cube->movetime*50);
						int part2 = (floors[j]->cube->movecount+1)/(floors[j]->cube->movetime*50);
						if((int)part2%2 != (int)part1%2)
							floors[j]->cube->triggered = 0;
					}
					floors[j]->cube->movecount++;	
				}
				else if(floors[j]->cube->movecount >= floors[j]->cube->movetime*500)
					floors[j]->cube->movecount = 0;
				
				if(floors[j]->pu_time > 0)
					floors[j]->pu_time--;
				if(floors[j]->cube->new_type_time > 0)
					floors[j]->cube->new_type_time--;
			}
			anglecount ++;
			anglecount %= 360;
		}
		
		
		if(Player.self.v_speed < -440) //player is falling too fast -> die
		{
			Player.sec++;
			play(-1,S_EXPLODE);
			ResetPlayer(Player,Player.dir);
			return;	
		}
		
		
		if(Player.keys[set.keys[JUMP1+keyadd]] 
		&& Player.powerup == PU_DJUMP 
		&& Player.self.on_floor < 0
		&& Player.self.v_speed < 0
		&& Player.self.on_floor != SAND)//use double-jump powerup if available
		{
			Player.powerup = 0;
			Player.tsec++;
			Player.self.v_speed = 125;
			play(-1,S_JUMP);
	
			//place smoke puff
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
		
		if(!powerups)
			Player.powerup = -1;
		int old_on_sth = Player.self.on_floor;
		if((Player.self.on_floor = OnFloor(Player))< 0) //player is airborne
		{
			if(!powerups)
				Player.powerup = 0;
			Player.self.v_speed -= gravity; //8 = gravity -> pulls player down
			
		
			if(Player.keys[set.keys[FORWARD1+keyadd]] 
			&& Player.self.f_speed < MAX_FSPEED)
			{
				Player.self.f_speed +=3;
				//AdjustFov(Player,(float)((float)Player.self.f_speed/8.0f)+35.0f,false);		
			}
			if(Player.keys[set.keys[BACKWARD1+keyadd]]
			&& Player.self.f_speed > -MAX_BSPEED)
			{
		
				Player.self.f_speed -=2;
			
				//AdjustFov(Player,(float)((float)Player.self.f_speed/8.0f)+35.0f,false);		
			}
			if(Player.keys[set.keys[LEFT1+keyadd]]	
			&& Player.self.s_speed < 0	//player is moving from left to right
			&& Player.self.on_floor != SIDE_BLOCK_L)
			{
				if(Player.self.s_speed < -85)
					Player.self.s_speed =-85; //slow down player's side-speed (shortens flight-distance)
			}
			if(Player.keys[set.keys[RIGHT1+keyadd]]
			&& Player.self.s_speed > 0	//player is moving from right to left
			&& Player.self.on_floor != SIDE_BLOCK_R)
			{
				if(Player.self.s_speed > 85)
					Player.self.s_speed = 85; //slow down player's side-speed (shortens flight-distance)
				
			}
			
		}
		else //player touches floor
		{
			if(!powerups)
				Player.powerup = 0;
			//stop player from falling down
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
			if(Player.bumpedtime <= 0)	//player has control of his ship
				Player.self.s_speed = 0;
				
			if(Player.keys[set.keys[FORWARD1+keyadd]] //accelerate
			&& Player.self.f_speed < MAX_FSPEED)
			{
				play(2,S_SPEEDUP);
				Player.self.f_speed +=4;
				
				//AdjustFov(Player,(float)((float)Player.self.f_speed/8.0f)+35.0f,false);		
			}
			if(Player.keys[set.keys[BACKWARD1+keyadd]]	//deaccelerate
			 && Player.self.f_speed > -MAX_BSPEED)
			{
				play(2,S_SLOWDOWN);
				Player.self.f_speed -=3;
				
				//AdjustFov(Player,(float)((float)Player.self.f_speed/8.0f)+35.0f,false);		
			}
			if(Player.keys[set.keys[LEFT1+keyadd]]	//left
			&& Player.self.s_speed < MAX_SSPEED
			&& Player.self.on_floor != SIDE_BLOCK_L)
			{
				
				
				Player.self.s_speed +=(int)min(abs(Player.self.f_speed/4),70);
				Player.self.s_speed +=40;
				
				if(Player.self.s_speed < 0)
					Player.self.s_speed +=4;
			}
			if(Player.keys[set.keys[RIGHT1+keyadd]] //right
			&& Player.self.s_speed > -MAX_SSPEED
			&& Player.self.on_floor != SIDE_BLOCK_R)
			{
				
				
				Player.self.s_speed -=(int)min(abs(Player.self.f_speed/4),70);
				Player.self.s_speed -=40;
				
				if(Player.self.s_speed > 0)
					Player.self.s_speed -=4;
			}
			if(Player.keys[set.keys[JUMP1+keyadd]])//jump
			{
				if(Player.self.on_floor == SIDE_BLOCK_L)
					Player.self.s_speed = -Player.dir*95; 
				if(Player.self.on_floor == SIDE_BLOCK_R)
					Player.self.s_speed = Player.dir*95;
				
				if(Player.self.on_floor != SAND)
				{
					Player.self.v_speed = 125;
					play(-1,S_JUMP);
			
					//smoke puffs
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
			
				
			//affect player based on type of floor he is touching
			switch(Player.self.on_floor)
			{
				case NORMAL:
				break;
				case GOAL:
					//decrease amount of lives of enemy-player
					if(Player.dir > 0 && Player.self.pos.z+1000 >= floors[player1goalminindex]->cube->origin.z)
					{
						Player2.min--;
						if(Player2.min <= 0)
							SwitchGamestate(GS_CAEND);
						play(1,S_HURT);
						ResetPlayer(Player1,Player1.dir);
					}
					else if(Player.dir < 0 && Player.self.pos.z-1000 < floors[player2goalmaxindex]->cube->origin.z+floors[player2goalmaxindex]->cube->ext.z)
					{
						Player1.min--;
						if(Player1.min <= 0)
							SwitchGamestate(GS_CAEND);
						play(1,S_HURT);
						ResetPlayer(Player2,Player2.dir);
					}
				break;
				case SPEED_UP:
					play(2,S_SPEEDUP);
					Player.self.f_speed +=5;
					
					//AdjustFov(Player,(float)((float)Player.self.f_speed/8.0f)+35.0f,false);		
				break;
				case SLOW_DOWN:
					play(2,S_SLOWDOWN);
					Player.self.f_speed -=4;
					//AdjustFov(Player,(float)((float)Player.self.f_speed/8.0f)+35.0f,false);			
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
					if(Player.protectiontime <= 0)
					{
						play(1,S_HURT);
						Player.health -= 3;
					}
						
					if(Player.health <= 0)
					{
						Player.sec ++;
						play(-1,S_EXPLODE);
						ResetPlayer(Player,Player.dir);
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
					
					Player.self.s_speed = old_s_speed;
					
					//influence movement by "counter-movement-actions" of ice floors.	
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
		Player.self.pos.z		+= Player.dir*Player.self.f_speed;
		Player.self.pos.y		+= Player.self.v_speed;
		Player.self.pos.x		+= Player.dir*Player.self.s_speed;
		Player.cam.pos.z		+= Player.dir*Player.self.f_speed;	
		Player.cam.pos.y		+= Player.self.v_speed;	
		Player.cam.pos.x		+= Player.dir*Player.self.s_speed;
			
		if(Player.dir >0)
			framecount1 = framecount1%21;	
		else
			framecount2 = framecount2%21;	
	
	}
	
}


void CastleM::WallCrash(PlayerPawn &Player, Vertex wallpos)
{
	//check whether player crashes into wall
	if(Player.self.pos.x-120 <= wallpos.x+800
	&& Player.self.pos.x+120 >= wallpos.x-800)
	{
		if(Player.self.oldpos.z <= wallpos.z
		&& Player.self.pos.z >= wallpos.z
		|| Player.self.oldpos.z >= wallpos.z
		&& Player.self.pos.z <= wallpos.z)
		{
			if(Player.self.pos.y+190 >= wallpos.y
			&& Player.self.pos.y <= wallpos.y+800)	
			{
				if(wallpos.x || wallpos.y || wallpos.z)
				{
					Player.sec++;
					play(-1,S_EXPLODE);
					ResetPlayer(Player,Player.dir);
				}
			}
		}
	}
	
}

void CastleM::PlayerCrash()
{
	//check whether players collide
	if(Length((float)Player1.self.pos.x,(float)Player1.self.pos.y,(float)Player1.self.pos.z,(float)Player2.self.pos.x,(float)Player2.self.pos.y,(float)Player2.self.pos.z)<500)
	{
		
		//results are really bad ... looked and felt funny -> left it that way
		if(Player1.bumpedtime <=0)
		{
			Player1.bumpedtime = Player2.bumpedtime =  25;
			Player1.self.v_speed = Player2.self.v_speed =15;
			play(-1,S_BUMP);
			if(Player2.self.pos.x < Player1.self.pos.x-60)
			{
				Player2.self.s_speed = abs(Player1.self.f_speed/2);
				Player1.self.s_speed = abs(Player2.self.f_speed/2);
				
				int temp = Player2.self.f_speed;
				Player2.self.f_speed = -Player1.self.f_speed/4;
				Player1.self.f_speed = -temp/4;	
			}
			else if(Player2.self.pos.x > Player1.self.pos.x+60)
			{
				Player2.self.s_speed = -abs(Player1.self.f_speed/2);
				Player1.self.s_speed = -abs(Player2.self.f_speed/2);
				int temp = Player2.self.f_speed;
				Player2.self.f_speed = -Player1.self.f_speed/4;
				Player1.self.f_speed = -temp/4;	
			}
			else
			{
				int temp = Player2.self.f_speed;
				Player2.self.f_speed = -Player1.self.f_speed/2;
				Player1.self.f_speed = -temp/2;	
			}
			
		}
	}
}

void CastleM::PlayerPush()
{
	if(Length((float)Player1.self.pos.x,(float)Player1.self.pos.y,(float)Player1.self.pos.z,(float)Player2.self.pos.x,(float)Player2.self.pos.y,(float)Player2.self.pos.z)<2000)
	{
		//has player just used push-powerup ?
		if(Player1.pushtime > 0 && Player2.protectiontime <= 0 )
		{
			Player2.self.v_speed += 10;
			if(Player2.self.pos.x < Player1.self.pos.x-40)
				Player2.self.s_speed = 100;
			else if(Player2.self.pos.x > Player1.self.pos.x+40)
				Player2.self.s_speed = -100;
			if(Player2.self.pos.y > Player1.self.pos.y)
				Player2.self.v_speed += 10;
		}
		if(Player2.pushtime > 0 && Player1.protectiontime <= 0 )
		{
			Player1.self.v_speed += 10;
			if(Player1.self.pos.x < Player2.self.pos.x-40)
				Player1.self.s_speed = -100;
			else if(Player1.self.pos.x > Player2.self.pos.x+40)
				Player1.self.s_speed = 100;
			if(Player1.self.pos.y > Player2.self.pos.y)
				Player1.self.v_speed += 10;
		}
	}
}

void CastleM::DisplayDebug()
{
	//display debug-output
	
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

void CastleM::Render()
{
	WallCrash(Player1,Player1.wallpos);
	WallCrash(Player1,Player2.wallpos);
	WallCrash(Player2,Player1.wallpos);
	WallCrash(Player2,Player2.wallpos);
	PlayerCrash();
	PlayerPush();
	
	{
		//default direction - player1
		
		
		
			
		glLoadIdentity();		
			
		gluLookAt(Player1.cam.pos.x,Player1.cam.pos.y,Player1.cam.pos.z,																//where we are
  			Player1.cam.target->x+Player1.cam.offsets.x,Player1.cam.target->y+Player1.cam.offsets.y,Player1.cam.target->z+Player1.cam.offsets.z,	//where we look at
  			Player1.cam.up.x,Player1.cam.up.y,Player1.cam.up.z);

		//use lower half of the screen
		
		glViewport(0,0,set.width,set.height/2);

		if(set.speed)	
			AdjustFov(Player1,(float)((float)Player1.self.f_speed/8.0f)+35.0f,false);	
		else
			AdjustFov(Player1,35.0f,false);
	
		int z = Player1.cam.pos.z;
				
		


		//draw space-background
		DrawBackGround(23,1);
		
		

		Tile **u = &floors[num_tiles*2-1];
		for(;u>=&floors[0];u--)
		{
			if((*u)->cube->origin.z+(*u)->cube->ext.z+23000 < z)
				break;
			if((*u)->cube->origin.z-set.drawdist < z)
			{
				(*u)->cube->Draw(false,(*u)->cube->origin.z-Player1.self.pos.z);
				if((*u)->cube->flag1 && EX_POWERUP((*u)->cube->triggerflag))
				{
					Vertex ppos = {(*u)->cube->flag1,(*u)->cube->origin.y+200,(*u)->cube->flag2};
					if((*u)->pu_time <= 0 && powerups)
						PowerUp(anglecount+(*u)->cube->origin.z%70,ppos,1.0,1.0,0.0);
				}
			}
		}

		


		Player1.Draw();
		Player2.Draw();
		
		
		
		//draw walls
		int anim = 0;
		if(animated[30])
			anim = (animcount/set.animspeed)%animated[30];
		glBindTexture(GL_TEXTURE_2D,anims[30][anim]);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);	
			glBegin(GL_QUADS);
				if(!(Player1.wallpos.x == 0 && Player1.wallpos.y == 0 && Player1.wallpos.z == 0))
				{
					glTexCoord2f(0,0);
					glVertex3i(Player1.wallpos.x-800,Player1.wallpos.y,Player1.wallpos.z);
					glTexCoord2f(1,0);
					glVertex3i(Player1.wallpos.x+800,Player1.wallpos.y,Player1.wallpos.z);
					glTexCoord2f(1,1);
					glVertex3i(Player1.wallpos.x+800,Player1.wallpos.y+700,Player1.wallpos.z);
					glTexCoord2f(0,1);
					glVertex3i(Player1.wallpos.x-800,Player1.wallpos.y+700,Player1.wallpos.z);
				}
				if(!(Player2.wallpos.x == 0 && Player2.wallpos.y == 0 && Player2.wallpos.z == 0))
				{
					glTexCoord2f(0,0);
					glVertex3i(Player2.wallpos.x-800,Player2.wallpos.y,Player2.wallpos.z);
					glTexCoord2f(1,0);
					glVertex3i(Player2.wallpos.x+800,Player2.wallpos.y,Player2.wallpos.z);
					glTexCoord2f(1,1);
					glVertex3i(Player2.wallpos.x+800,Player2.wallpos.y+700,Player2.wallpos.z);
					glTexCoord2f(0,1);
					glVertex3i(Player2.wallpos.x-800,Player2.wallpos.y+700,Player2.wallpos.z);
				}
			glEnd();
		glEnable(GL_LIGHTING);	
		glEnable(GL_CULL_FACE);


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
			glEnable(GL_LIGHTING);	
			glDisable(GL_BLEND);
		
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_TEXTURE_2D);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		
			glDisable(GL_STENCIL_TEST);
		}
		//draw glow-effect
		if(set.glow && !blooming)
		{
			glEnable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);	
			Tile **u = &floors[num_tiles*2-1];
			for(;u>=&floors[0];u--)
			{
				if((*u)->cube->origin.z+(*u)->cube->ext.z+23000 < z)
					break;
				if((*u)->cube->origin.z-set.drawdist < z)
				{
					if((*u)->type == HURT || (*u)->cube->new_type== HURT && (*u)->cube->new_type_time > 0)//render red glow effect
					{
						(*u)->cube->ext.y += 300;
						(*u)->cube->ext.x += 300;
						(*u)->cube->ext.z += 300;
						(*u)->cube->origin.x += 150;
						(*u)->cube->origin.y += 150;
						(*u)->cube->origin.z -= 150;
						glColor3f(1.0f,0.0f,0.0f);
						(*u)->cube->Draw(true);
						(*u)->cube->origin.y -= 150;
						(*u)->cube->origin.x -= 150;
						(*u)->cube->origin.z += 150;
						(*u)->cube->ext.y -= 300;
						(*u)->cube->ext.x -= 300;
						(*u)->cube->ext.z -= 300;
						glColor3f(1.0f,1.0f,1.0f);
					}
					else if((*u)->type == ICE || (*u)->cube->new_type== ICE && (*u)->cube->new_type_time > 0)//render blue glow effect
					{
						(*u)->cube->ext.y += 200;
						(*u)->cube->ext.x += 200;
						(*u)->cube->ext.z += 200;
						(*u)->cube->origin.x += 100;
						(*u)->cube->origin.y += 100;
						(*u)->cube->origin.z -= 100;
						glColor3f(0.0f,0.0f,1.0f);
						(*u)->cube->Draw(true);
						(*u)->cube->origin.y -= 100;
						(*u)->cube->origin.x -= 100;
						(*u)->cube->origin.z += 100;
						(*u)->cube->ext.y -= 200;
						(*u)->cube->ext.x -= 200;
						(*u)->cube->ext.z -= 200;
						glColor3f(1.0f,1.0f,1.0f);
					}
					
				}
			}
			glEnable(GL_LIGHTING);
			glEnable(GL_TEXTURE_2D);	
			glDisable(GL_BLEND);
		}
		
		glBindTexture(GL_TEXTURE_2D,anims[31][0]);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glColor3f(1.0,1.0,1.0);
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_QUADS);
			if(Player1.protectiontime > 0)
			{
				//draw shield
				glTexCoord2f(0,1);
				glVertex3f(Player1.self.pos.x+400,Player1.self.pos.y-300,Player1.self.pos.z-400);
				glTexCoord2f(1,1);
				glVertex3f(Player1.self.pos.x-400,Player1.self.pos.y-300,Player1.self.pos.z-400);
				glTexCoord2f(1,0);
				glVertex3f(Player1.self.pos.x-400,Player1.self.pos.y+500,Player1.self.pos.z-400);
				glTexCoord2f(0,0);
				glVertex3f(Player1.self.pos.x+400,Player1.self.pos.y+500,Player1.self.pos.z-400);
			}
			if(Player1.pushtime > 0)
			{
				//draw area of effect of push-powerup
				glTexCoord2f(0,1);
				glVertex3f(Player1.self.pos.x+1200-200*Player1.pushtime,Player1.self.pos.y-1200+200*Player1.pushtime,Player1.self.pos.z-300);
				glTexCoord2f(1,1);
				glVertex3f(Player1.self.pos.x-1200+200*Player1.pushtime,Player1.self.pos.y-1200+200*Player1.pushtime,Player1.self.pos.z-300);
				glTexCoord2f(1,0);
				glVertex3f(Player1.self.pos.x-1200+200*Player1.pushtime,Player1.self.pos.y+1200-200*Player1.pushtime,Player1.self.pos.z-300);
				glTexCoord2f(0,0);
				glVertex3f(Player1.self.pos.x+1200-200*Player1.pushtime,Player1.self.pos.y+1200-200*Player1.pushtime,Player1.self.pos.z-300);
			}
		glEnd();
		glEnable(GL_DEPTH_TEST);

		glBegin(GL_QUADS);
			
			if(Player2.pushtime > 0)
			{
				//draw area of effect of push-powerup
				glTexCoord2f(0,1);
				glVertex3f(Player2.self.pos.x+1200-200*Player2.pushtime,Player2.self.pos.y-1200+200*Player2.pushtime,Player2.self.pos.z-300);
				glTexCoord2f(1,1);
				glVertex3f(Player2.self.pos.x-1200+200*Player2.pushtime,Player2.self.pos.y-1200+200*Player2.pushtime,Player2.self.pos.z-300);
				glTexCoord2f(1,0);
				glVertex3f(Player2.self.pos.x-1200+200*Player2.pushtime,Player2.self.pos.y+1200-200*Player2.pushtime,Player2.self.pos.z-300);
				glTexCoord2f(0,0);
				glVertex3f(Player2.self.pos.x+1200-200*Player2.pushtime,Player2.self.pos.y+1200-200*Player2.pushtime,Player2.self.pos.z-300);
			}
			if(Player2.protectiontime > 0)
			{
				//draw shield
				glTexCoord2f(0,1);
				glVertex3f(Player2.self.pos.x+400,Player2.self.pos.y-300,Player2.self.pos.z-400);
				glTexCoord2f(1,1);
				glVertex3f(Player2.self.pos.x-400,Player2.self.pos.y-300,Player2.self.pos.z-400);
				glTexCoord2f(1,0);
				glVertex3f(Player2.self.pos.x-400,Player2.self.pos.y+500,Player2.self.pos.z-400);
				glTexCoord2f(0,0);
				glVertex3f(Player2.self.pos.x+400,Player2.self.pos.y+500,Player2.self.pos.z-400);
			}
		
		glEnd();
		
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);	
		glEnable(GL_CULL_FACE);
		
		if(Player2.smoke[9].lifetime > SDL_GetTicks())
			DrawEffect(Player2.smoke[9]);	
		if(Player1.smoke[9].lifetime > SDL_GetTicks())
			DrawEffect(Player1.smoke[9]);
			

		//draw big - 3D score of player 2	
		int ix = floors[num_tiles*2-1]->cube->origin.x-floors[num_tiles*2-1]->cube->ext.x/2;
		int iy = floors[num_tiles*2-1]->cube->origin.y+150;
		int iz = floors[num_tiles*2-1]->cube->origin.z+floors[num_tiles*2-1]->cube->ext.z-500;
		
		char score1[10];
		sprintf(score1,"%d",Player2.min);
		if(Player2.min >= 10)
			printtxt3d(Player1.cam.pos.y,Player1.cam.pos.z,ix+1100,iy+6000,iz,1,1,1,8,score1);
		else
			printtxt3d(Player1.cam.pos.y,Player1.cam.pos.z,ix+500,iy+6000,iz,1,1,1,8,score1);
		
		//draw HUD
		DrawPowerUp((int)(0.9*(float)(set.width)),(int)(0.1*(float)(set.height)),Player1.powerup);
		sprintf(score1,"%d",Player1.min);
		printtxt2d((int)(0.95*(float)(set.width)),(int)(0.1*(float)(set.height)),1.0,1.0,1.0,1.0,score1);
		
		
	}
	
	{


		
			
		glLoadIdentity();		
		
		gluLookAt(Player2.cam.pos.x,Player2.cam.pos.y,Player2.cam.pos.z,																//where we are
  			Player2.cam.target->x+Player2.cam.offsets.x,Player2.cam.target->y+Player2.cam.offsets.y,Player2.cam.target->z+Player2.cam.offsets.z,	//where we look at
  			Player2.cam.up.x,Player2.cam.up.y,Player2.cam.up.z);

		//use upper half of screen			
		glViewport(0,set.height/2,set.width,set.height/2);
		//second direction - player2
		if(set.speed)	
			AdjustFov(Player2,(float)((float)Player2.self.f_speed/8.0f)+35.0f,false);	
		else
			AdjustFov(Player2,35.0f,false);	

		DrawBackGround(28,1);

		int z = Player2.cam.pos.z;
		Tile **u = &floors[0];
		Tile **max = &floors[num_tiles*2-1];
		for(;u<=max;u++)
		{
			if((*u)->cube->origin.z-23000 > z)
				break;
			if((*u)->cube->origin.z+(*u)->cube->ext.z+set.drawdist > z)
			{
				(*u)->cube->Draw(false,Player2.self.pos.z-(*u)->cube->origin.z);
				if((*u)->cube->flag1 && EX_POWERUP((*u)->cube->triggerflag))
				{
					Vertex ppos = {(*u)->cube->flag1,(*u)->cube->origin.y+200,(*u)->cube->flag2};
					if((*u)->pu_time <= 0 && powerups)
						PowerUp(anglecount+(*u)->cube->origin.z%70,ppos,1.0,1.0,0.0);
				}
			}
		}


		Player2.Draw();
		Player1.Draw();
		
		
		
		//draw walls
		int anim = 0;
		if(animated[30])
			anim = (animcount/set.animspeed)%animated[30];
		glBindTexture(GL_TEXTURE_2D,anims[30][anim]);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);	
			glBegin(GL_QUADS);
				if(!(Player1.wallpos.x == 0 && Player1.wallpos.y == 0 && Player1.wallpos.z == 0))
				{
					glTexCoord2f(0,0);
					glVertex3i(Player1.wallpos.x-800,Player1.wallpos.y,Player1.wallpos.z);
					glTexCoord2f(1,0);
					glVertex3i(Player1.wallpos.x+800,Player1.wallpos.y,Player1.wallpos.z);
					glTexCoord2f(1,1);
					glVertex3i(Player1.wallpos.x+800,Player1.wallpos.y+700,Player1.wallpos.z);
					glTexCoord2f(0,1);
					glVertex3i(Player1.wallpos.x-800,Player1.wallpos.y+700,Player1.wallpos.z);
				}
				if(!(Player2.wallpos.x == 0 && Player2.wallpos.y == 0 && Player2.wallpos.z == 0))
				{
					glTexCoord2f(0,0);
					glVertex3i(Player2.wallpos.x-800,Player2.wallpos.y,Player2.wallpos.z);
					glTexCoord2f(1,0);
					glVertex3i(Player2.wallpos.x+800,Player2.wallpos.y,Player2.wallpos.z);
					glTexCoord2f(1,1);
					glVertex3i(Player2.wallpos.x+800,Player2.wallpos.y+700,Player2.wallpos.z);
					glTexCoord2f(0,1);
					glVertex3i(Player2.wallpos.x-800,Player2.wallpos.y+700,Player2.wallpos.z);
				}
			glEnd();
		glEnable(GL_LIGHTING);		
		glEnable(GL_CULL_FACE);
		
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
			glEnable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);	
			Tile **u = &floors[0];
			Tile **max = &floors[num_tiles*2-1];
			for(;u<=max;u++)
			{
				if((*u)->cube->origin.z-23000 > z)
					break;
				if((*u)->cube->origin.z+(*u)->cube->ext.z+set.drawdist > z)
				{
					if((*u)->type == HURT || (*u)->cube->new_type== HURT && (*u)->cube->new_type_time > 0)//render red glow effect
					{
						(*u)->cube->ext.y += 300;
						(*u)->cube->ext.x += 300;
						(*u)->cube->ext.z += 300;
						(*u)->cube->origin.x += 150;
						(*u)->cube->origin.y += 150;
						(*u)->cube->origin.z -= 150;
						glColor3f(1.0f,0.0f,0.0f);
						(*u)->cube->Draw(true);
						(*u)->cube->origin.y -= 150;
						(*u)->cube->origin.x -= 150;
						(*u)->cube->origin.z += 150;
						(*u)->cube->ext.y -= 300;
						(*u)->cube->ext.x -= 300;
						(*u)->cube->ext.z -= 300;
						glColor3f(1.0f,1.0f,1.0f);
					}
					else if((*u)->type == ICE || (*u)->cube->new_type== ICE && (*u)->cube->new_type_time > 0)//render blue glow effect
					{
						(*u)->cube->ext.y += 200;
						(*u)->cube->ext.x += 200;
						(*u)->cube->ext.z += 200;
						(*u)->cube->origin.x += 100;
						(*u)->cube->origin.y += 100;
						(*u)->cube->origin.z -= 100;
						glColor3f(0.0f,0.0f,1.0f);
						(*u)->cube->Draw(true);
						(*u)->cube->origin.y -= 100;
						(*u)->cube->origin.x -= 100;
						(*u)->cube->origin.z += 100;
						(*u)->cube->ext.y -= 200;
						(*u)->cube->ext.x -= 200;
						(*u)->cube->ext.z -= 200;
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
			
			
			
		glBindTexture(GL_TEXTURE_2D,anims[31][0]);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);	
		glEnable(GL_BLEND);
		glColor3f(1.0,1.0,1.0);
		glBegin(GL_QUADS);
			if(Player1.protectiontime > 0)
			{
				//draw shield
				glTexCoord2f(0,1);
				glVertex3f(Player1.self.pos.x+400,Player1.self.pos.y-300,Player1.self.pos.z+400);
				glTexCoord2f(1,1);
				glVertex3f(Player1.self.pos.x-400,Player1.self.pos.y-300,Player1.self.pos.z+400);
				glTexCoord2f(1,0);
				glVertex3f(Player1.self.pos.x-400,Player1.self.pos.y+500,Player1.self.pos.z+400);
				glTexCoord2f(0,0);
				glVertex3f(Player1.self.pos.x+400,Player1.self.pos.y+500,Player1.self.pos.z+400);
			}
			if(Player1.pushtime > 0)
			{
				//draw area of effect of push-powerup
				glTexCoord2f(0,1);
				glVertex3f(Player1.self.pos.x+1200-200*Player1.pushtime,Player1.self.pos.y-1200+200*Player1.pushtime,Player1.self.pos.z+300);
				glTexCoord2f(1,1);
				glVertex3f(Player1.self.pos.x-1200+200*Player1.pushtime,Player1.self.pos.y-1200+200*Player1.pushtime,Player1.self.pos.z+300);
				glTexCoord2f(1,0);
				glVertex3f(Player1.self.pos.x-1200+200*Player1.pushtime,Player1.self.pos.y+1200-200*Player1.pushtime,Player1.self.pos.z+300);
				glTexCoord2f(0,0);
				glVertex3f(Player1.self.pos.x+1200-200*Player1.pushtime,Player1.self.pos.y+1200-200*Player1.pushtime,Player1.self.pos.z+300);
			}
			
		glEnd();
		glDisable(GL_DEPTH_TEST);

		glBegin(GL_QUADS);
			if(Player2.protectiontime > 0)
			{
				//draw shield
				glTexCoord2f(0,1);
				glVertex3f(Player2.self.pos.x+400,Player2.self.pos.y-300,Player2.self.pos.z+400);
				glTexCoord2f(1,1);
				glVertex3f(Player2.self.pos.x-400,Player2.self.pos.y-300,Player2.self.pos.z+400);
				glTexCoord2f(1,0);
				glVertex3f(Player2.self.pos.x-400,Player2.self.pos.y+500,Player2.self.pos.z+400);
				glTexCoord2f(0,0);
				glVertex3f(Player2.self.pos.x+400,Player2.self.pos.y+500,Player2.self.pos.z+400);
			}
			if(Player2.pushtime > 0)
			{
				//draw area of effect of push-powerup
				glTexCoord2f(0,1);
				glVertex3f(Player2.self.pos.x+1200-200*Player2.pushtime,Player2.self.pos.y-1200+200*Player2.pushtime,Player2.self.pos.z+300);
				glTexCoord2f(1,1);
				glVertex3f(Player2.self.pos.x-1200+200*Player2.pushtime,Player2.self.pos.y-1200+200*Player2.pushtime,Player2.self.pos.z+300);
				glTexCoord2f(1,0);
				glVertex3f(Player2.self.pos.x-1200+200*Player2.pushtime,Player2.self.pos.y+1200-200*Player2.pushtime,Player2.self.pos.z+300);
				glTexCoord2f(0,0);
				glVertex3f(Player2.self.pos.x+1200-200*Player2.pushtime,Player2.self.pos.y+1200-200*Player2.pushtime,Player2.self.pos.z+300);
			}
		glEnd();
		glEnable(GL_DEPTH_TEST);

		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);	
		glEnable(GL_CULL_FACE);
		
		//draw big - 3D score of player 1		
		int ix = floors[0]->cube->origin.x-floors[0]->cube->ext.x/2;
		int iy = floors[0]->cube->origin.y+150;
		int iz = floors[0]->cube->origin.z+500;
		
		char score1[10];
		sprintf(score1,"%d",Player1.min);
		if(Player1.min >= 10)
			printtxt3d(Player2.cam.pos.y,Player2.cam.pos.z,ix-1100,iy+6000,iz,1,1,1,8,score1);
		else
			printtxt3d(Player2.cam.pos.y,Player2.cam.pos.z,ix-500,iy+6000,iz,1,1,1,8,score1);

		
		//draw HUD
		DrawPowerUp((int)(0.9*(float)(set.width)),(int)(0.1*(float)(set.height)),Player2.powerup);
		sprintf(score1,"%d",Player2.min);
		printtxt2d((int)(0.95*(float)(set.width)),(int)(0.1*(float)(set.height)),1.0,1.0,1.0,1.0,score1);
		
		
	}
	
	glViewport(0,0,set.width,set.height);	
	if(debugmsgs)
		DisplayDebug();	
}





void CastleM::LoadCastleMap(char filen[])
{
	if(floors)
	{
		for(int i = 0;i<num_tiles;i++)
			delete floors[i];
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
	
	floors = new Tile*[num_tiles*2];
	
	int max_z = -999999;
	
	Vertex a = {1,1,1};
	for(int u = 0;u<num_tiles;u++)
	{
		floors[u] = new Tile();
		floors[u]->cube = new Cube(a,a,a,0);
		fread(&floors[u]->type,sizeof(int),1,in);
	
		floors[u]->pu_time = 0;
		floors[u]->cube->new_type_time = 0;
		fread(&floors[u]->cube->origin.x,sizeof(int),1,in);
		fread(&floors[u]->cube->origin.y,sizeof(int),1,in);
		fread(&floors[u]->cube->origin.z,sizeof(int),1,in);
		
		fread(&floors[u]->cube->ext.x,sizeof(int),1,in);
		fread(&floors[u]->cube->ext.y,sizeof(int),1,in);
		fread(&floors[u]->cube->ext.z,sizeof(int),1,in);
		
		if(floors[u]->cube->origin.z+ floors[u]->cube->ext.z> max_z)
			max_z = floors[u]->cube->origin.z+ floors[u]->cube->ext.z;
	
			
		fread(&floors[u]->cube->tiled.x,sizeof(int),1,in);
		fread(&floors[u]->cube->tiled.y,sizeof(int),1,in);
		fread(&floors[u]->cube->tiled.z,sizeof(int),1,in);

		
			
		fread(&floors[u]->cube->tex[0],sizeof(int),1,in);
		fread(&floors[u]->cube->tex[1],sizeof(int),1,in);
			
		fread(&floors[u]->cube->flag1,sizeof(int),1,in);
		fread(&floors[u]->cube->flag2,sizeof(int),1,in);
		fread(&floors[u]->cube->flag3,sizeof(int),1,in);
		fread(&floors[u]->cube->triggerflag,sizeof(int),1,in);
		
		if(floors[u]->cube->flag1 && A_MOVE(floors[u]->cube->triggerflag))//moving floor
			floors[u]->cube->movetime = MOVETIME(floors[u]->cube->triggerflag);
			
		if(!E_START(floors[u]->cube->triggerflag))//needs to be triggered
			floors[u]->cube->triggered = 0;	
		
	}
	fclose(in);
	
	//mirror -map since only first half of it has been built
	int mid_x = floors[0]->cube->origin.x-floors[0]->cube->ext.x/2;

	for(int u = num_tiles*2-1,h = 0;u>=num_tiles && h<num_tiles;u--,h++)
	{
		floors[u] = new Tile();
		floors[u]->cube = new Cube(a,a,a,0);
		floors[u]->type = floors[h]->type;
		floors[u]->pu_time = 0;
		floors[u]->cube->new_type_time = 0;
		floors[u]->cube->origin.y = floors[h]->cube->origin.y;
		floors[u]->cube->origin.x = mid_x-(floors[h]->cube->origin.x-floors[h]->cube->ext.x-mid_x);
		floors[u]->cube->origin.z = max_z-(floors[h]->cube->origin.z+floors[h]->cube->ext.z-max_z);
		
		floors[u]->cube->ext.x = floors[h]->cube->ext.x;
		floors[u]->cube->ext.y = floors[h]->cube->ext.y;
		floors[u]->cube->ext.z = floors[h]->cube->ext.z;
		
		floors[u]->cube->tiled.x = floors[h]->cube->tiled.x;
		floors[u]->cube->tiled.y = floors[h]->cube->tiled.y;
		floors[u]->cube->tiled.z = floors[h]->cube->tiled.z;
		
		floors[u]->cube->tex[0] = floors[h]->cube->tex[0];
		floors[u]->cube->tex[1] = floors[h]->cube->tex[1];
		
		if(floors[h]->cube->flag1 && A_MOVE(floors[h]->cube->triggerflag))//moving floor
		{
			floors[u]->cube->flag1 = mid_x-(floors[h]->cube->flag1-floors[h]->cube->ext.x-mid_x); // = x coord of destination
			floors[u]->cube->flag2 = floors[h]->cube->flag2;									// = y coord of destination
			floors[u]->cube->flag3 = max_z-(floors[h]->cube->flag3+floors[h]->cube->ext.z-max_z);// = z coord of destination
		}
		else if(EX_POWERUP(floors[h]->cube->triggerflag))
		{
			floors[u]->cube->flag1 = mid_x-(floors[h]->cube->flag1-mid_x);
			floors[u]->cube->flag2 = max_z-(floors[h]->cube->flag2-max_z);
			floors[u]->cube->flag3 = floors[h]->cube->flag3; 
		}
		else
		{
			floors[u]->cube->flag1 = floors[h]->cube->flag1; 
			floors[u]->cube->flag2 = floors[h]->cube->flag2; 
			floors[u]->cube->flag3 = floors[h]->cube->flag3; 
		}
		
		floors[u]->cube->triggerflag = floors[h]->cube->triggerflag;
		if(T_OTHER(floors[h]->cube->triggerflag))
		{
			floors[u]->cube->triggerflag &= 0xff0007ff;
			floors[u]->cube->triggerflag |= ((u-(OTHERINDEX(floors[h]->cube->triggerflag)-h))<<11);	
		}
		if(floors[u]->cube->flag1 && A_MOVE(floors[u]->cube->triggerflag))//moving floor
			floors[u]->cube->movetime = MOVETIME(floors[u]->cube->triggerflag);
			
		if(!E_START(floors[u]->cube->triggerflag))//needs to be triggered
			floors[u]->cube->triggered = 0;	
	}

	
	//sort floors in ascending z-order
	for(int p = 0;p < num_tiles*2;p++)
	{
		for(int k = p+1;k < num_tiles*2;k++)
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
	
	for(int p = num_tiles*2-1;p >= 0;p--)
	{
		if(floors[p]->cube->origin.z+floors[p]->cube->ext.z 
		> floors[starttile]->cube->origin.z+floors[starttile]->cube->ext.z)
			starttile = p;
	}
	
}
