/*
#				Orbit-Hopper
#				AI.cpp : AI routines
#			
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


#include "../headers/Game.h"
#include "../headers/Structs.h"
#include "../headers/Single.h"
#include "../headers/Utils.h"
#include "../headers/AI.h"	
#include "../headers/Sound.h"



extern Settings set; 
extern bool debugmsgs;
extern bool wireframemode;
extern Gamestate *gamestates[15];

extern char debugmsg[64];
bool enemy = false;
bool life = false;

PlayerPawn Enemy;
PongBall *pBall = NULL;
PlayerPawn Life;

Tile *next1 = NULL;
Tile *next2 = NULL;
Tile *obstacles[8];
int pongx = -1000;
int jumpdistz,jumpdistxl,jumpdistxr;//Debug	


	
void PowerUp(int angle, Vertex pos, float r, float g, float b);
void DrawPongEnemy(int x,int y,int z);


void InitAI()
{
	pongx = -1000;
	
	for(int u = 0;u<8;u++)
		obstacles[u] = NULL;	
		
	enemy = life = false;

	if(pBall)
		delete pBall;
	pBall = NULL;
}


void DestroyAI()
{
	if(pBall)
		delete pBall;
	pBall = NULL;
	
	//if(next1)
	//	delete next1;
	next1 = NULL;
	
	//if(next2)
	//	delete next2;
	next2 = NULL;
		
}

//create pong ball
void CreatepBall(Tile* i)
{
	if(!pBall)
	{
		pBall = new PongBall;
		//note: flag1 and flag2-values of tile represent x and z position of pong enemy
		pBall->pos.x = i->cube->flag1;
		pBall->pos.y = i->cube->origin.y+200;
		pBall->pos.z = i->cube->flag2-700;	//place ball in front of enemy
		
		for(int j = 0;j<5;j++)
		{
			pBall->oldpos[j].x = i->cube->flag1;
			pBall->oldpos[j].y = i->cube->origin.y+200;
			pBall->oldpos[j].z = i->cube->flag2-700;
		}
		pBall->drall = pBall->speed.x = pBall->speed.y = pBall->speed.z = 0;	//no movement
		pBall->floor = i;
		pongx = i->cube->flag1;
			
	}	
}


void CreateEnemy(Tile *k)
{
	enemy = true;
	Enemy.self.pos.x = k->cube->flag1;
	Enemy.self.pos.y = k->cube->origin.y+200;
	Enemy.self.pos.z = k->cube->flag2;
	Enemy.protectiontime = 0;
		
	Enemy.health = 100;
	Enemy.started = k->cube->flag3;// ~ difficulty
	Enemy.dir = 99; //~ Bot
		
	Enemy.self.f_speed = Enemy.self.s_speed = Enemy.self.v_speed =  0;
	Enemy.self.on_floor = Enemy.last_floor = NORMAL;
	Enemy.bumpedtime = 0;
	//do not press any keys
	for(int h = 0;h<323;h++)
		Enemy.keys[h] = false;
		
	Enemy.powerupused = 0;	
	
	Enemy.self.oldpos = Enemy.self.pos;
	
	Enemy.fastest = false;
	//remove old obstacles
	for(int u = 0;u<8;u++)
		obstacles[u] = NULL;	
		
	//camera settings			
	Enemy.cam.pos.x = -1;
	Enemy.cam.pos.y = Enemy.self.pos.y+2000;
	Enemy.cam.pos.z = Enemy.self.pos.z-7000;
	Enemy.cam.target = &Enemy.self.pos;
	Enemy.cam.up.x = 0;
	Enemy.cam.up.y = 1;
	Enemy.cam.up.z = 0;
	Enemy.cam.offsets.x = 0;
	Enemy.cam.offsets.y = 700;
	Enemy.cam.offsets.z = 1000;	
	
	//effect settings
	for(int u = 0;u<10;u++)
	{
		Enemy.smoke[u].lifetime = 0;
		Enemy.smoke[u].zs = 1.0f;
	}
	
}

//pretty much the same as the CreateEnemy fct
void CreateLife(Tile *k)
{

	life = true;
	Life.self.pos.x = k->cube->flag1;
	Life.self.pos.y = k->cube->origin.y+200;
	Life.self.pos.z = k->cube->flag2;
	Life.protectiontime = 0;
		
	Life.health = 100;
	Life.started = k->cube->flag3;// ~ difficulty
	Life.dir = 200; //~ life
		
	Life.self.f_speed = Life.self.s_speed = Life.self.v_speed =  0;
	Life.self.on_floor = Life.last_floor = NORMAL;
	Life.bumpedtime = 0;
	Life.pushtime = 0;
	
	for(int h = 0;h<323;h++)
		Life.keys[h] = false;
		
	Life.powerupused = 0;	
	
	Life.self.oldpos = Life.self.pos;
	
	Life.fastest = false;
	
	for(int u = 0;u<8;u++)
		obstacles[u] = NULL;		
				
	Life.cam.pos.x = -1;
	Life.cam.pos.y = Life.self.pos.y+2000;
	Life.cam.pos.z = Life.self.pos.z-7000;
	Life.cam.target = &Life.self.pos;
	Life.cam.up.x = 0;
	Life.cam.up.y = 1;
	Life.cam.up.z = 0;
	Life.cam.offsets.x = 0;
	Life.cam.offsets.y = 700;
	Life.cam.offsets.z = 1000;	
	
	for(int u = 0;u<10;u++)
	{
		Life.smoke[u].lifetime = 0;
		Life.smoke[u].zs = 1.0f;
	}
}


void DrawAI(Tile *floors)
{
	//draw pong enemy & ball
	if(pBall)
	{
		//pongx is the only dynamic parameter
		DrawPongEnemy(pongx,pBall->floor->cube->origin.y,pBall->floor->cube->flag2);
		pBall->Draw();	
	}
		
	//draw enemy
	if(enemy)
	{
		//enemy is hurt - draw smoke
		if(Enemy.health < 95)
		{
			unsigned int now = SDL_GetTicks();
			for(int k = 0;k<(95-Enemy.health)/9;k++)
			{
				if(Enemy.smoke[k].lifetime < now)
				{
					for(int l = k;l>0;l--)
						Enemy.smoke[l] = Enemy.smoke[l-1];
						
					//smoke puffs
					Enemy.smoke[0].lifetime = now+randomi(250,0,0,850);
					Enemy.smoke[0].origin = Enemy.self.pos;
					Enemy.smoke[0].origin.x-=randomi(-240,0,0,80);
					Enemy.smoke[0].origin.y+=randomi(70,0,0,120);
					Enemy.smoke[0].origin.z-=120;
					Enemy.smoke[0].type = randomi(0,0,0,3);
					Enemy.smoke[0].r = Enemy.smoke[0].g = Enemy.smoke[0].b = (float)randomi(2,2,2,10)/10.0f;
					Enemy.smoke[0].xs = 1.0f+((float)randomi(0,0,0,7))/10.0f;
					Enemy.smoke[0].ys = 1.0f+((float)randomi(0,0,0,7))/10.0f;
					
				}		
			}
			for(int k = 0;k<(95-Enemy.health)/9;k++)
				DrawEffect(Enemy.smoke[k]);		
		}
		if(Enemy.smoke[9].lifetime > SDL_GetTicks())
			DrawEffect(Enemy.smoke[9]);	
	
		Enemy.Draw();
		
		if(debugmsgs)
		{
			glDisable(GL_TEXTURE_2D);
			//draw directional lines and key-press lines
			glBegin(GL_LINES);
				if(Enemy.keys[set.keys[LEFT1]])
				{
					glVertex3f(Enemy.self.pos.x,Enemy.self.pos.y,Enemy.self.pos.z);
					glVertex3f(Enemy.self.pos.x+1500,Enemy.self.pos.y,Enemy.self.pos.z);
				}
				if(Enemy.keys[set.keys[RIGHT1]])
				{
					glVertex3f(Enemy.self.pos.x,Enemy.self.pos.y,Enemy.self.pos.z);
					glVertex3f(Enemy.self.pos.x-1500,Enemy.self.pos.y,Enemy.self.pos.z);
				}
				if(Enemy.keys[set.keys[JUMP1]])
				{
					glVertex3f(Enemy.self.pos.x,Enemy.self.pos.y,Enemy.self.pos.z);
					glVertex3f(Enemy.self.pos.x,Enemy.self.pos.y+1500,Enemy.self.pos.z);
				}
				glColor3f(0.0f,1.0f,0.0f);
				glVertex3f(Enemy.self.pos.x,Enemy.self.pos.y,Enemy.self.pos.z);
				glVertex3f(Enemy.self.pos.x,Enemy.self.pos.y,Enemy.self.pos.z+jumpdistz);
				
				
				glVertex3f(Enemy.self.pos.x,Enemy.self.pos.y,Enemy.self.pos.z);
				glVertex3f(Enemy.self.pos.x+jumpdistxl,Enemy.self.pos.y,Enemy.self.pos.z+jumpdistz);
				
				glVertex3f(Enemy.self.pos.x,Enemy.self.pos.y,Enemy.self.pos.z);
				glVertex3f(Enemy.self.pos.x+jumpdistxr,Enemy.self.pos.y,Enemy.self.pos.z+jumpdistz);
				
			glEnd();
		}
	}

	//draw life
	if(life)
	{
		if(Life.pushtime > 0) //transparent -> not collectable
			glEnable(GL_BLEND);
		
		PowerUp(Life.self.s_speed/2,Life.self.pos,0.6,0.0,0.0);
			
		if(Life.pushtime > 0 && !wireframemode) //not collectable
			glDisable(GL_BLEND);
		
	}


	if(enemy || life)
	{	
		if(debugmsgs)//draw some outlines...
		{
			//highlight next floors
			glDisable(GL_CULL_FACE);
			if(Enemy.cam.pos.x > 0)
			{
				glColor3f(1.0f,1.0f,0.0f);
				glBegin(GL_QUAD_STRIP);
				
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x-floors[Enemy.cam.pos.x].cube->ext.x,floors[Enemy.cam.pos.x].cube->origin.y,floors[Enemy.cam.pos.x].cube->origin.z+floors[Enemy.cam.pos.x].cube->ext.z);
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x-floors[Enemy.cam.pos.x].cube->ext.x,floors[Enemy.cam.pos.x].cube->origin.y+100,floors[Enemy.cam.pos.x].cube->origin.z+floors[Enemy.cam.pos.x].cube->ext.z);
					
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x-floors[Enemy.cam.pos.x].cube->ext.x,floors[Enemy.cam.pos.x].cube->origin.y,floors[Enemy.cam.pos.x].cube->origin.z);
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x-floors[Enemy.cam.pos.x].cube->ext.x,floors[Enemy.cam.pos.x].cube->origin.y+150,floors[Enemy.cam.pos.x].cube->origin.z);
					
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x,floors[Enemy.cam.pos.x].cube->origin.y,floors[Enemy.cam.pos.x].cube->origin.z);
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x,floors[Enemy.cam.pos.x].cube->origin.y+150,floors[Enemy.cam.pos.x].cube->origin.z);
					
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x,floors[Enemy.cam.pos.x].cube->origin.y,floors[Enemy.cam.pos.x].cube->origin.z+floors[Enemy.cam.pos.x].cube->ext.z);
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x,floors[Enemy.cam.pos.x].cube->origin.y+150,floors[Enemy.cam.pos.x].cube->origin.z+floors[Enemy.cam.pos.x].cube->ext.z);
					
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x-floors[Enemy.cam.pos.x].cube->ext.x,floors[Enemy.cam.pos.x].cube->origin.y,floors[Enemy.cam.pos.x].cube->origin.z+floors[Enemy.cam.pos.x].cube->ext.z);
					glVertex3f(floors[Enemy.cam.pos.x].cube->origin.x-floors[Enemy.cam.pos.x].cube->ext.x,floors[Enemy.cam.pos.x].cube->origin.y+100,floors[Enemy.cam.pos.x].cube->origin.z+floors[Enemy.cam.pos.x].cube->ext.z);
					
				glEnd();
				
			}
			if(next1)
			{
				glColor3f(0.0f,1.0f,0.0f);
				glBegin(GL_QUAD_STRIP);
				
					glVertex3f(next1->cube->origin.x-next1->cube->ext.x,next1->cube->origin.y,next1->cube->origin.z+next1->cube->ext.z);
					glVertex3f(next1->cube->origin.x-next1->cube->ext.x,next1->cube->origin.y+100,next1->cube->origin.z+next1->cube->ext.z);
					
					glVertex3f(next1->cube->origin.x-next1->cube->ext.x,next1->cube->origin.y,next1->cube->origin.z);
					glVertex3f(next1->cube->origin.x-next1->cube->ext.x,next1->cube->origin.y+150,next1->cube->origin.z);
					
					glVertex3f(next1->cube->origin.x,next1->cube->origin.y,next1->cube->origin.z);
					glVertex3f(next1->cube->origin.x,next1->cube->origin.y+150,next1->cube->origin.z);
					
					glVertex3f(next1->cube->origin.x,next1->cube->origin.y,next1->cube->origin.z+next1->cube->ext.z);
					glVertex3f(next1->cube->origin.x,next1->cube->origin.y+150,next1->cube->origin.z+next1->cube->ext.z);
					
					glVertex3f(next1->cube->origin.x-next1->cube->ext.x,next1->cube->origin.y,next1->cube->origin.z+next1->cube->ext.z);
					glVertex3f(next1->cube->origin.x-next1->cube->ext.x,next1->cube->origin.y+100,next1->cube->origin.z+next1->cube->ext.z);
					
				glEnd();
			}
			if(next2)
			{
				
				glColor3f(0.0f,0.6f,0.0f);
				glBegin(GL_QUAD_STRIP);
				
					glVertex3f(next2->cube->origin.x-next2->cube->ext.x,next2->cube->origin.y,next2->cube->origin.z+next2->cube->ext.z);
					glVertex3f(next2->cube->origin.x-next2->cube->ext.x,next2->cube->origin.y+150,next2->cube->origin.z+next2->cube->ext.z);
					
					glVertex3f(next2->cube->origin.x-next2->cube->ext.x,next2->cube->origin.y,next2->cube->origin.z);
					glVertex3f(next2->cube->origin.x-next2->cube->ext.x,next2->cube->origin.y+150,next2->cube->origin.z);
					
					glVertex3f(next2->cube->origin.x,next2->cube->origin.y,next2->cube->origin.z);
					glVertex3f(next2->cube->origin.x,next2->cube->origin.y+150,next2->cube->origin.z);
					
					glVertex3f(next2->cube->origin.x,next2->cube->origin.y,next2->cube->origin.z+next2->cube->ext.z);
					glVertex3f(next2->cube->origin.x,next2->cube->origin.y+150,next2->cube->origin.z+next2->cube->ext.z);
					
					glVertex3f(next2->cube->origin.x-next2->cube->ext.x,next2->cube->origin.y,next2->cube->origin.z+next2->cube->ext.z);
					glVertex3f(next2->cube->origin.x-next2->cube->ext.x,next2->cube->origin.y+150,next2->cube->origin.z+next2->cube->ext.z);
					
				glEnd();
			}
			
			
			glColor3f(1.0f,1.0f,1.0f);
			for(int k = 0;k<8;k++)
			{
				if(obstacles[k])
				{
					glBegin(GL_QUAD_STRIP);
					
						glVertex3f(obstacles[k]->cube->origin.x-obstacles[k]->cube->ext.x,obstacles[k]->cube->origin.y,obstacles[k]->cube->origin.z+obstacles[k]->cube->ext.z);
						glVertex3f(obstacles[k]->cube->origin.x-obstacles[k]->cube->ext.x,obstacles[k]->cube->origin.y+150,obstacles[k]->cube->origin.z+obstacles[k]->cube->ext.z);
						
						glVertex3f(obstacles[k]->cube->origin.x-obstacles[k]->cube->ext.x,obstacles[k]->cube->origin.y,obstacles[k]->cube->origin.z);
						glVertex3f(obstacles[k]->cube->origin.x-obstacles[k]->cube->ext.x,obstacles[k]->cube->origin.y+150,obstacles[k]->cube->origin.z);
						
						glVertex3f(obstacles[k]->cube->origin.x,obstacles[k]->cube->origin.y,obstacles[k]->cube->origin.z);
						glVertex3f(obstacles[k]->cube->origin.x,obstacles[k]->cube->origin.y+150,obstacles[k]->cube->origin.z);
						
						glVertex3f(obstacles[k]->cube->origin.x,obstacles[k]->cube->origin.y,obstacles[k]->cube->origin.z+obstacles[k]->cube->ext.z);
						glVertex3f(obstacles[k]->cube->origin.x,obstacles[k]->cube->origin.y+150,obstacles[k]->cube->origin.z+obstacles[k]->cube->ext.z);
						
						glVertex3f(obstacles[k]->cube->origin.x-obstacles[k]->cube->ext.x,obstacles[k]->cube->origin.y,obstacles[k]->cube->origin.z+obstacles[k]->cube->ext.z);
						glVertex3f(obstacles[k]->cube->origin.x-obstacles[k]->cube->ext.x,obstacles[k]->cube->origin.y+150,obstacles[k]->cube->origin.z+obstacles[k]->cube->ext.z);
						
					glEnd();
					}
			}
			
			glEnable(GL_CULL_FACE);
			glEnable(GL_TEXTURE_2D);
			glColor3f(1.0f,1.0f,1.0f);
		}
	}	
}

//ball "physics"
void Bounce(Tile *k)
{
	if(k != pBall->floor) 
	{
		int x1 = k->cube->origin.x;
		int x3 = k->cube->origin.z;
	
		//update position of moving floor
		if(k->cube->movetime && k->cube->movecount)
		{
			float part = (float)k->cube->movecount/(float)(k->cube->movetime*50);
			
			if((int)part%2) //on the way back
				part = 1.0f-(part-(int)part);
			else	
				part -=(int)part;	
			
			x1 += (int)(part*(k->cube->flag1-k->cube->origin.x));
			x3 += (int)(part*(k->cube->flag3-k->cube->origin.z));	
		}
		
		//ball hits the floor from the left
		if(pBall->pos.x-200 < x1
		&& pBall->oldpos[0].x-200 >= x1
		&& pBall->pos.z-200 <= x3+k->cube->ext.z
		&& pBall->pos.z+200 >= x3)
		{
			
			pBall->pos.x = x1+200;
			pBall->speed.x = -pBall->speed.x;	
			play(-1,S_PONG);
		}	
		//ball hits floor from the right side
		if(pBall->pos.x+200 > x1-k->cube->ext.x
		&& pBall->oldpos[0].x+200 <= x1-k->cube->ext.x
		&& pBall->pos.z-200 <= x3+k->cube->ext.z
		&& pBall->pos.z+200 >= x3)
		{
			
			pBall->pos.x = x1-k->cube->ext.x-200;
			pBall->speed.x = -pBall->speed.x;
			play(-1,S_PONG);
		}
		//ball hits front side of floor
		if(pBall->pos.z+200 > x3
		&& pBall->oldpos[0].z+200 <= x3
		&& pBall->pos.x-200 <= x1
		&& pBall->pos.x+200 >= x1-k->cube->ext.x)
		{
			
			pBall->pos.z = x3-200;
			pBall->speed.z = -pBall->speed.z;
			play(-1,S_PONG);
		}
		//ball hits back side of floor
		if(pBall->pos.z+200 <x3+k->cube->ext.z
		&& pBall->oldpos[0].z+200 >= x3+k->cube->ext.z
		&& pBall->pos.x-200 <= x1
		&& pBall->pos.x+200 >= x1-k->cube->ext.x)
		{
			
			pBall->pos.z = x3+k->cube->ext.z-200;
			pBall->speed.z = -pBall->speed.z;
			play(-1,S_PONG);	
		}
	}	
	
}


//simulate a crash between player and enemy - results are rather poor
//maybe i should allow the player to push the enemy as well...
void Crash(PlayerPawn *Player)
{
	if(Length((float)Player->self.pos.x,(float)Player->self.pos.y,(float)Player->self.pos.z,(float)Enemy.self.pos.x,(float)Enemy.self.pos.y,(float)Enemy.self.pos.z)<500)
	{
		if(Player->self.pos.x > Enemy.self.pos.x)
		{
			int diff = Enemy.self.pos.x+500-Player->self.pos.x;
			Player->self.pos.x = Enemy.self.pos.x+500;	
			Player->cam.pos.x += diff;
		}
		else
		{
			int diff = Enemy.self.pos.x-500-Player->self.pos.x;
			Player->self.pos.x = Enemy.self.pos.x-500;	
			Player->cam.pos.x += diff;
		}
		
		if(Player->self.pos.z > Enemy.self.pos.z)
		{
			int diff = Enemy.self.pos.z+500-Player->self.pos.z;
			Player->self.pos.z = Enemy.self.pos.z+500;	
			Player->cam.pos.z += diff;
		}
		else
		{
			int diff = Enemy.self.pos.z-500-Player->self.pos.z;
			Player->self.pos.z = Enemy.self.pos.z-500;	
			Player->cam.pos.z += diff;
		}
		
		if(!Player->bumpedtime)
		{
			Player->bumpedtime = 20;
			Enemy.bumpedtime =  (3-Enemy.started)*50-30;
			play(-1,S_BUMP);
			
			if(Enemy.self.pos.x > Player->self.pos.x)//enemy is to the player's left
			{
				if(Enemy.self.s_speed < 0) //enemy is going right
				{
					if(Player->self.s_speed > -200)
						Player->self.s_speed += Enemy.self.s_speed;	 
				} 
				else if(Player->self.s_speed > 0) //player is going left
				{
					Enemy.self.s_speed = Player->self.s_speed;
				}
			}
			else//enemy is to the player's right side
			{
				if(Enemy.self.s_speed > 0) //enemy is going left
				{
					if(Player->self.s_speed < 200)
						Player->self.s_speed += Enemy.self.s_speed;
					 
				}
				else if(Player->self.s_speed < 0) //player is going right
				{
					Enemy.self.s_speed = Player->self.s_speed;
				} 
			}
			
			
		}
	}
	
	
}


//let AI decide whether to attack the player
void RamPlayer(PlayerPawn *Player,PlayerPawn *Enemy,Tile * floors)
{
	if(Enemy->keys[RIGHT1] || Enemy->keys[LEFT1])	//attack was already started, or we are navigating
		return;
		
	if(Player->self.pos.z+100 > Enemy->self.pos.z &&
	Player->self.pos.z-100 < Enemy->self.pos.z && Enemy->cam.pos.x >= 0 && next1) //both are at the same z-position
	{
		if(abs(Player->self.pos.x - Enemy->self.pos.x)<2000)
		{
			if(abs(Player->self.pos.y - Enemy->self.pos.y)<150)
			{
				//don't, if we have to jump soon
				if(!((floors[Enemy->cam.pos.x].cube->origin.z+floors[Enemy->cam.pos.x].cube->ext.z+500 <= next1->cube->origin.z//distance between current and next floor is bigger than 500
				|| floors[Enemy->cam.pos.x].cube->origin.x < next1->cube->origin.x-next1->cube->ext.x	//jump to the left
				|| floors[Enemy->cam.pos.x].cube->origin.x-floors[Enemy->cam.pos.x].cube->ext.x > next1->cube->origin.x //jump to the right
				|| next1->cube->origin.y > floors[Enemy->cam.pos.x].cube->origin.y)//jump upwards
				&& Enemy->self.pos.z+200+jumpdistz+1500 >= next1->cube->origin.z)) //we will jump soon
				{
	
					//player wants to ram enemy
					if((Player->self.pos.x + Player->self.s_speed*5 > Enemy->self.pos.x && Player->self.pos.x < Enemy->self.pos.x)
					|| (Player->self.pos.x + Player->self.s_speed*5 < Enemy->self.pos.x && Player->self.pos.x > Enemy->self.pos.x))
					{
						Enemy->keys[set.keys[JUMP1]] = true;//jump ! - this is not very useful - enemy tends to fall off the edge
						
					}
					
					if(Enemy->powerupused <= 0)//powerupused represents ramming-delay
					{
						//let's attack!
				 		if(Enemy->self.pos.x > Player->self.pos.x)
				 			Enemy->keys[set.keys[RIGHT1]] = true;
				 		else
				 			Enemy->keys[set.keys[LEFT1]] = true;
				 			
						Enemy->powerupused = (3-Enemy->started)*50; //Enemy->started ~= difficulty
					}
				}
				
				//player comes from the right side to ram us -> steer to the right
				else if(Player->self.pos.x + Player->self.s_speed*7 > Enemy->self.pos.x && Player->self.pos.x < Enemy->self.pos.x)
				{
					Enemy->keys[set.keys[RIGHT1]] = true;
						
				}
				//player comes from the left side to ram us -> steer to the left
				else if(Player->self.pos.x + Player->self.s_speed*7 < Enemy->self.pos.x && Player->self.pos.x > Enemy->self.pos.x)
				{
					Enemy->keys[set.keys[LEFT1]] = true;
				}
			}
		}
		
	}
	
	
	
}
//return number of update-frames that are lost by jumping a given (height) distance
//needed to time jumps
int HeightFrameLoss(int heightdist)
{
	if(heightdist >= 980)
		return 15;
	if(heightdist >= 740)
		return  8;
	if(heightdist >= 480)
		return  4;
	if(heightdist >= 240)
		return  2;
	if(heightdist <= -240)
		return  -2;
	if(heightdist <= -480)
		return  -4;
	if(heightdist <= -740)
		return  -8;
	if(heightdist <= -980)
		return  -15;
	if(heightdist <= -1240)
		return  -23;
	if(heightdist <= -1440)
		return  -31;
	if(heightdist <= -1740)
		return  -38;	
	return 0;			
}

//decide if enemy has to jump;  index = index of current floor
bool hasToJump(int index,Tile * floors)
{
	return (floors[index].cube->origin.z+floors[index].cube->ext.z+250 <= next1->cube->origin.z//distance between current and next floor is bigger than 250
		|| floors[index].cube->origin.x < next1->cube->origin.x-next1->cube->ext.x	//jump left
		|| floors[index].cube->origin.x-floors[index].cube->ext.x > next1->cube->origin.x //jump right
		|| next1->cube->origin.y > floors[index].cube->origin.y);
}

//collision detection
int OnFloor(PlayerPawn *enem,Tile *floors, int num_tiles, PongBall *pBall)
{
	Pawn pawn = enem->self;
	
	enem->cam.pos.x = -1;
	
	int floor_type = -1;
	bool on = false;
	Tile *i = &floors[0];
	Tile *max = &floors[0]+num_tiles;
	int counter = 0;
	for(;i<max;i++,counter++)
	{
		bool onthis = false;
		switch(enem->OnFloor(i))//structs.cpp
		{
			case SKIP:
				i = max-1;	//skip next floors
			break;
			case OFF:
			break;
			case ABOVE:
			break;
			case ON:
				
				enem->cam.pos.x = counter;
				 if(i->cube->new_type_time <= 0)
					floor_type = i->type;
				else
					floor_type = i->cube->new_type;	//save type of floor we are on
					
				enem->self.v_speed = 0;	//causes trouble, if not set to 0
				onthis = on = true;
			break;
			case LEFT_ON:	//wall jump
				floor_type = SIDE_BLOCK_R;	
				if(!on)
					enem->self.v_speed -= 3;
				onthis = on = true;
			break;
			case RIGHT_ON: //wall jump
				floor_type = SIDE_BLOCK_L;	
				if(!on)
					enem->self.v_speed -= 3;
				onthis = on = true;
			break;
			case CRASH:						//we crashed into a floor
				play(-1,S_EXPLODE);
				
				if(enem->dir == 99)	//bot was enemy
					enemy = false;
				else if(enem->dir == 200)//bot was life
					 life = false;
				
				return 0;
			break;
			
		};
		if(onthis)//allow enemy to trigger floors
		{
			if(enem->laston != i)
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
				enem->laston = i;
			}
		}
		
			
	}
	
	return floor_type;
	
}



void ChooseWay(PlayerPawn *enem,int old_on_index,Tile *floors, int num_tiles)
{
	//find next floors, since we just entered a new one
	if(old_on_index != enem->cam.pos.x)
	{
		bool reusenext2 = false;
		int index = 0;
		int *next  = NULL;		//holds floors, that might be chosen as new target floors
		int n_possnext = 0;	//number of new target floors
		
		int side = 0;	
		if(floors[enem->cam.pos.x].type == LEFT_PUSH || floors[enem->cam.pos.x].type == RIGHT_PUSH)
			side = 60; //get side-acceleration of floor
			
		for(int u = enem->cam.pos.x+1;u<num_tiles;u++)
		{
			//distance bot can jump forward to reach this floor
			int jumpdist1 = (MAX_FSPEED)*(30-HeightFrameLoss(floors[u].cube->origin.y-floors[enem->cam.pos.x].cube->origin.y));
			//distance bot can jump sidewards to reach this floor
			int jumpdist2 = (95+side)*(30-HeightFrameLoss(floors[u].cube->origin.y-floors[enem->cam.pos.x].cube->origin.y));
			
			if(floors[u].cube->origin.z < floors[enem->cam.pos.x].cube->origin.z+floors[enem->cam.pos.x].cube->ext.z+jumpdist1
			&& (floors[u].cube->origin.x > floors[enem->cam.pos.x].cube->origin.x-floors[enem->cam.pos.x].cube->ext.x-jumpdist2
			&& floors[u].cube->origin.x-floors[u].cube->ext.x < floors[enem->cam.pos.x].cube->origin.x+jumpdist2)
			&& floors[u].cube->origin.y+floors[u].cube->ext.y-1100 < floors[enem->cam.pos.x].cube->origin.y+floors[enem->cam.pos.x].cube->ext.y)
				n_possnext++;	//count this block, since it can be theoretically be reached from the floor the bot is currently on
		}
		
		if(n_possnext)	//aaaaah - we found floors - now let's save them
		{
			next = new int[n_possnext];
			
			//same as above - but now we are saving indices
			int j = 0;
			for(int u = enem->cam.pos.x+1;u<num_tiles;u++)
			{
				//distance bot can jump forward to reach this floor
				int jumpdist1 = (MAX_FSPEED)*(30-HeightFrameLoss(floors[u].cube->origin.y-floors[enem->cam.pos.x].cube->origin.y));
				//distance bot can jump sidewards to reach this floor
				int jumpdist2 = (95+side)*(30-HeightFrameLoss(floors[u].cube->origin.y-floors[enem->cam.pos.x].cube->origin.y));
		
				if(floors[u].cube->origin.z < floors[enem->cam.pos.x].cube->origin.z+floors[enem->cam.pos.x].cube->ext.z+jumpdist1
				&& (floors[u].cube->origin.x > floors[enem->cam.pos.x].cube->origin.x-floors[enem->cam.pos.x].cube->ext.x-jumpdist2
				&& floors[u].cube->origin.x-floors[u].cube->ext.x < floors[enem->cam.pos.x].cube->origin.x+jumpdist2)
				&& floors[u].cube->origin.y+floors[u].cube->ext.y-1100 < floors[enem->cam.pos.x].cube->origin.y+floors[enem->cam.pos.x].cube->ext.y)
				{
					next[j] = u; //save this index
					if(&floors[u] == next2)
					{
						reusenext2 = true;
						index = j;
					}
					j++;
				}	
			}
			
			//now check for all of the saved floors, whether there's another floor that can be reached from it
			//since we don't want to get stuck in a dead end ;)
			for(int i = 0;i<n_possnext;i++)
			{
				side = 0;
				if(floors[next[i]].type == LEFT_PUSH || floors[next[i]].type == RIGHT_PUSH)
					side = 60;
						
				int n_poss = 0;
				for(int u = next[i]+1;u<num_tiles;u++)
				{
					int jumpdist1 = (MAX_FSPEED)*(30-HeightFrameLoss(floors[u].cube->origin.y-floors[next[i]].cube->origin.y));
					int jumpdist2 = (95+side)*(30-HeightFrameLoss(floors[u].cube->origin.y-floors[next[i]].cube->origin.y));
					
					if(floors[u].cube->origin.z < floors[next[i]].cube->origin.z+floors[next[i]].cube->ext.z+jumpdist1
					&& (floors[u].cube->origin.x > floors[next[i]].cube->origin.x-floors[next[i]].cube->ext.x-jumpdist2
					&& floors[u].cube->origin.x-floors[u].cube->ext.x < floors[next[i]].cube->origin.x+jumpdist2)
					&& floors[u].cube->origin.y+floors[u].cube->ext.y-1100 < floors[next[i]].cube->origin.y+floors[next[i]].cube->ext.y)
						n_poss++;	
				}
				
				if(!n_poss)//this target floor seems to be a dead end -> delete its index
				{
					if(&floors[i] == next2)
						reusenext2 = false;
					if(n_possnext > 1)
					{
						int *temp = new int[n_possnext-1];
						for(int j = 0;j<n_possnext-1;j++)
						{
							if( j>= i) 
								temp[j] = next[j+1];
							else
								temp[j] = next[j];
								
						}	
						n_possnext--;
						if(next)
						{
							delete[] next;
							next = NULL;
						}
						if(index > i)
							index--;
						next = temp; //size of "next" array has been decreased by one
						
					}
					else
						n_possnext = 0;
				}
			}
		}
		
		index +=1;
		//we have found a chain of at least two new target floors - find next one
		if(n_possnext)
		{
			if(reusenext2)
			{
				if(next2)
					next1 = next2;//take old next2 floor as new next1
				else
					reusenext2 = false;
			}
			if(!reusenext2)
			{
				for(index = 0;index<n_possnext;index++)
				{
					//select a floor that is farther away -> usually gets better navigational results
					if(floors[next[index]].cube->origin.z+5 >= floors[enem->cam.pos.x].cube->origin.z+floors[enem->cam.pos.x].cube->ext.z)
					{
						index++;
						break;
					}
				}
				next1 = &floors[next[index-1]]; //this is the nearest floor, which is not too close to our current floor
			}
			//same as above - now using next1 as starting-block
			
			//note: we now know that this block is not a dead end and can be reached by the bot
			//now we need to find out, which floor will be used after this one (helps us evaluate some strategies,
			//since some constellations require early reactions)
			side = 0;
			if(next1->type == LEFT_PUSH || next1->type == RIGHT_PUSH)
				side = 60;
					
			int n_possn = 0;
			
			for(int u = next[index-1]+1;u<num_tiles;u++)
			{
				int jumpdist1 = (MAX_FSPEED)*(30)-(MAX_FSPEED)*HeightFrameLoss(floors[u].cube->origin.y-next1->cube->origin.y);	
				int jumpdist2 = (95+side)*(30)-(95+side)*HeightFrameLoss(floors[u].cube->origin.y-next1->cube->origin.y);
				
				if(floors[u].cube->origin.z < next1->cube->origin.z+next1->cube->ext.z+jumpdist1
				&& (floors[u].cube->origin.x > next1->cube->origin.x-next1->cube->ext.x-jumpdist2
				&& floors[u].cube->origin.x-floors[u].cube->ext.x < next1->cube->origin.x+jumpdist2
				&& floors[u].cube->origin.y+floors[u].cube->ext.y-1100 < next1->cube->origin.y+next1->cube->ext.y)
				)
					n_possn++;	
			}
			
			if(n_possn)
			{
				int *nextn = new int[n_possn];
				int p = 0;
				side = 0;
				if(next1->type == LEFT_PUSH || next1->type == RIGHT_PUSH)
					side = 60;
					
				for(int u = next[index-1]+1;u<num_tiles;u++)
				{
					int jumpdist1 = (MAX_FSPEED)*(30-HeightFrameLoss(floors[u].cube->origin.y-next1->cube->origin.y));
					int jumpdist2 = (95+side)*(30-HeightFrameLoss(floors[u].cube->origin.y-next1->cube->origin.y));
					
					if(floors[u].cube->origin.z < next1->cube->origin.z+next1->cube->ext.z+jumpdist1
					&& (floors[u].cube->origin.x > next1->cube->origin.x-next1->cube->ext.x-jumpdist2
					&& floors[u].cube->origin.x-floors[u].cube->ext.x < next1->cube->origin.x+jumpdist2)
					&& floors[u].cube->origin.y+floors[u].cube->ext.y-1100 < next1->cube->origin.y+next1->cube->ext.y)
					{
						nextn[p] = u;
						p++;
					}	
				}
				
				
				for(int i = 0;i<n_possn;i++)
				{
					int n_poss = 0;
					side = 0;
					if(floors[nextn[i]].type == LEFT_PUSH || floors[nextn[i]].type == RIGHT_PUSH)
						side = 60;
							
					for(int u = nextn[i]+1;u<num_tiles;u++)
					{
						int jumpdist1 = (MAX_FSPEED)*(30-HeightFrameLoss(floors[u].cube->origin.y-floors[nextn[i]].cube->origin.y));
						int jumpdist2 = (95+side)*(30-HeightFrameLoss(floors[u].cube->origin.y-floors[nextn[i]].cube->origin.y));
						
						if(floors[u].cube->origin.z < floors[nextn[i]].cube->origin.z+floors[nextn[i]].cube->ext.z+jumpdist1
						&& (floors[u].cube->origin.x > floors[nextn[i]].cube->origin.x-floors[nextn[i]].cube->ext.x-jumpdist2
						&& floors[u].cube->origin.x-floors[u].cube->ext.x < floors[nextn[i]].cube->origin.x+jumpdist2)
						&& floors[u].cube->origin.y+floors[u].cube->ext.y-1100 < floors[nextn[i]].cube->origin.y+floors[nextn[i]].cube->ext.y)
							n_poss++;	
					}
					
					if(!n_poss && n_possn > 1)
					{
						int *temp = new int[n_possn-1];
						for(int j = 0;j<n_possn-1;j++)
						{
							if( j>= i) 
								temp[j] = nextn[j+1];
							else
								temp[j] = nextn[j];
								
						}	
						n_possn--;
						if(nextn)
						{
							delete[] nextn;
							nextn = NULL;
						}
						nextn = temp;
					}
				}
				
				for(index = 0;index<n_possn;index++)
				{
					if(floors[nextn[index]].cube->origin.z+5 >= next1->cube->origin.z+next1->cube->ext.z)
					{
						index++;
						break;
					}
				}
				next2 = &floors[nextn[index-1]];
				if(nextn)
				{
					delete[] nextn;
					nextn = NULL;
				}
			}
			else
			{
				
				for(index = 0;index<n_possnext;index++)
				{
					//select a floor that is farther away -> usually gets better navigational results
					if(floors[next[index]].cube->origin.z+5 >= floors[enem->cam.pos.x].cube->origin.z+floors[enem->cam.pos.x].cube->ext.z)
					{
						index++;
						break;
					}
				}
				next2 = &floors[next[index-1]]+1;
			}
			
			
		}
		else //no target floors have been found -> choose next two floors based on their indices (that's dumb !!)
		{
			next1 = &floors[enem->cam.pos.x+1];
			next2 = &floors[enem->cam.pos.x+2];	
		}
		
		if(next)
		{
			delete[] next;
			next = NULL;
		}
		
		
		for(int j = 0;j<8;j++)
			obstacles[j] = NULL; //no obstacles have been found
			

		//check, whether there are some obstacles in our way (between current and next floor)
		Tile* start = (&floors[enem->cam.pos.x+1]);
		Tile* end = next1;
		while(start<end)
		{
			if(start->cube->origin.y > floors[enem->cam.pos.x].cube->origin.y
			&& start->cube->origin.y-start->cube->ext.y-750 < floors[enem->cam.pos.x].cube->origin.y)
			{
				if(start->cube->origin.x > floors[enem->cam.pos.x].cube->origin.x-floors[enem->cam.pos.x].cube->ext.x
				&& start->cube->origin.x-start->cube->ext.x < floors[enem->cam.pos.x].cube->origin.x)	
				{
					
					for(int k = 0;k<4;k++)
						if(!obstacles[k])
						{
							obstacles[k] = start;//found an obstacle (finds a maximum of 4 here)
							break;
						} 	
				}
			}
			start++;	
		}
		
		//check, whether there are some obstacles in our way (between next and the floor after that one)
		start = next1+1;
		end = next2;
		while(start<end)
		{
				
			if(start->cube->origin.y > next1->cube->origin.y
			&& start->cube->origin.y-start->cube->ext.y-750 < next1->cube->origin.y)
			{
				if(start->cube->origin.x > next1->cube->origin.x-next1->cube->ext.x
				&& start->cube->origin.x-start->cube->ext.x < next1->cube->origin.x)	
				{
					for(int k = 4;k<8;k++)
						if(!obstacles[k])
						{
							obstacles[k] = start;//found an obstacle (finds a maximum of another 4 here -> max total of 8 obstacles can be found)
							break;
						} 	
				}
			}
			start++;	
		}
		
		
	}
	
	//- haven't found any bugs yet - bot always seems to choose
	//target blocks wisely :-)
	
	
}


//real AI calculations happen here
void NavigateAI(PlayerPawn *enem,int old_on_index,PlayerPawn * Player,Tile *floors, int num_tiles)
{

	//get side-accelerations coming from the environment
	int side = 0;
	if(enem->self.v_speed == 0 && enem->self.s_speed != 0)
	{
		if(!enem->keys[set.keys[LEFT1]]
		&& !enem->keys[set.keys[RIGHT1]])
			side = enem->self.s_speed;
		else if(enem->keys[set.keys[LEFT1]])
			side = enem->self.s_speed-95;
		else if(enem->keys[set.keys[RIGHT1]])
			side = 95+enem->self.s_speed;
	}
	
	//don't let AI press any buttons !!  %-)
	for(int i = 0;i<323;i++)
	{
		if((i == set.keys[RIGHT1] || i == set.keys[LEFT1]) && enem->cam.pos.x >=0)
			if(enem->powerupused > (3-enem->started)*50-30) //if player rams us - don't set side-speed to 0
				if((enem->self.pos.x-300 > floors[enem->cam.pos.x].cube->origin.x-floors[enem->cam.pos.x].cube->ext.x || enem->keys[set.keys[3]])
				&& (enem->self.pos.x+300 < floors[enem->cam.pos.x].cube->origin.x || enem->keys[set.keys[RIGHT1]]))	//don't fall off the edge if ramming player
					continue;
				
		enem->keys[i] = false;
	}
	
	if(enemy)//lives don't care that much about player's speed....
	{
		
		//try to keep up with player -> regulate f_speed
		if(Player->self.pos.z+200 < enem->self.pos.z)//player is behind bot
		{
			if(Player->self.pos.z+2500 < enem->self.pos.z)//player is far behind bot
			{
				if(enem->self.f_speed >= Player->self.f_speed-55)
					enem->keys[set.keys[BACKWARD1]] = true; //slow down
				else if(Player->self.f_speed-90 > enem->self.f_speed)
					enem->keys[set.keys[FORWARD1]] = true; //accelerate
			
			}
			else
			{
				if(enem->self.f_speed >= Player->self.f_speed-35)
					enem->keys[set.keys[BACKWARD1]] = true; //slow down
				else if(Player->self.f_speed-50 > enem->self.f_speed)
					enem->keys[set.keys[FORWARD1]] = true; //accelerate
				
			}
		}
		else if(Player->self.pos.z-200 > enem->self.pos.z)//bot is behind player
		{
			if(Player->self.pos.z-2500 > enem->self.pos.z)//bot is far behind player
			{
				if(enem->self.f_speed <= Player->self.f_speed+55)
					enem->keys[set.keys[FORWARD1]] = true;
				else if(Player->self.f_speed+90 < enem->self.f_speed)
						enem->keys[set.keys[BACKWARD1]] = true;
		
			}
			else
			{
				if(enem->self.f_speed <= Player->self.f_speed+35)
					enem->keys[set.keys[FORWARD1]] = true;
				else if(Player->self.f_speed+50 < enem->self.f_speed)
					enem->keys[set.keys[BACKWARD1]] = true;
			}
		}
	}
	else if(life)
	{
		if(Player->self.pos.z+200 < enem->self.pos.z)//player is behind life
		{
			if(Player->self.pos.z+2500 < enem->self.pos.z)//player is far behind life
			{
				if(enem->self.f_speed >= Player->self.f_speed-15)
					enem->keys[set.keys[BACKWARD1]] = true; //slow down
				else if(Player->self.f_speed-20 >= enem->self.f_speed)
					enem->keys[set.keys[FORWARD1]] = true;
			
			}
			else
			{
				if(enem->self.f_speed >= Player->self.f_speed-5)
					enem->keys[set.keys[BACKWARD1]] = true; //slow down
				else if(Player->self.f_speed-5 > enem->self.f_speed)
					enem->keys[set.keys[FORWARD1]] = true;
				
			}
		}
		else if(Player->self.pos.z-200 > enem->self.pos.z)//life is behind player
		{
			if(Player->self.pos.z-2500 > enem->self.pos.z)//life is far behind player
			{
				if(enem->self.f_speed <= Player->self.f_speed+15)
					enem->keys[set.keys[FORWARD1]] = true;
				else if(Player->self.f_speed+20 < enem->self.f_speed)
						enem->keys[set.keys[BACKWARD1]] = true;
		
			}
			else
			{
				if(enem->self.f_speed <= Player->self.f_speed+5)
					enem->keys[set.keys[FORWARD1]] = true;
				else if(Player->self.f_speed+5 < enem->self.f_speed)
					enem->keys[set.keys[BACKWARD1]] = true;
			}
		}
		
	}
	
	
	//enem->cam.pos.x = index of the bot's current floor
	int current = enem->cam.pos.x;
	if(current < num_tiles-2 && current >= 0) //don't try to find next floors, if level is ending soon
	{
		ChooseWay(enem,old_on_index,floors,num_tiles);
		
		
		//target floors have been found -> use this knowledge to do the navigation stuff now
		
		
		int add = 0;
		if(enem->keys[set.keys[FORWARD1]] && enem->self.f_speed < MAX_FSPEED && enem->self.v_speed == 0)	//are we accelerating ? if yes, add some speed to our calculations
			add = 4;
		else if(enem->keys[set.keys[BACKWARD1]] && enem->self.f_speed > -MAX_BSPEED && enem->self.v_speed == 0)//are we deaccelerating ? if yes, subtract some speed from our calculations
			add = -3;
		
		int heightdist = next1->cube->origin.y-floors[current].cube->origin.y;
		int heightminus = HeightFrameLoss(heightdist);	
		
		jumpdistxl = (95+side)*(30-heightminus);		//how far can we jump to the left? (30 = maximum amount of frames we can stay airborne if jumping on "ground-level")
		jumpdistxr = (-95+side)*(30-heightminus);	//how far can we jump to the right?
		jumpdistz = (enem->self.f_speed+add)*(30-heightminus);//how far can we jump along the z-axis ?
		
		
		//are we too slow to get this jump ? -> accelerate
		if(floors[enem->cam.pos.x].cube->origin.z+floors[current].cube->ext.z+jumpdistz < next1->cube->origin.z)
			enem->keys[set.keys[FORWARD1]] = true;
		//are we too fast for this jump ? -> slow down a bit
		else if((next1->cube->origin.z+500+jumpdistz > next2->cube->origin.z+next2->cube->ext.z
		&& next2->cube->origin.z - (next1->cube->origin.z+next1->cube->ext.z) > 500)
		|| floors[current].cube->origin.z+floors[current].cube->ext.z+jumpdistz > next1->cube->origin.z+next1->cube->ext.z)
			enem->keys[set.keys[BACKWARD1]] = true;
		
		//do we need to steer some more to the left to reach next floor ?
		if(next1->cube->origin.x-next1->cube->ext.x > enem->self.pos.x)
		{ 
			if(side > -95)
			{
				if((enem->self.pos.x+120 < floors[current].cube->origin.x && !(enem->self.pos.z+200+jumpdistz >= next1->cube->origin.z && hasToJump(current,floors)))
				|| (enem->self.pos.z+200+jumpdistz >= next1->cube->origin.z //we need to jump 
				&& enem->self.pos.x+180+jumpdistxl < next1->cube->origin.x)) //we don't jump too far to the left
				{
					 enem->keys[set.keys[LEFT1]] = true;
					 enem->keys[set.keys[RIGHT1]] = false;
				}
			}
		}
		else if(next1->cube->origin.x < enem->self.pos.x)//do we need to steer some more to the right to reach next floor ?
		{		
			if(side < 95)
			{
				if((enem->self.pos.x-120 > floors[current].cube->origin.x-floors[current].cube->ext.x && !(enem->self.pos.z+200+jumpdistz >= next1->cube->origin.z && hasToJump(current,floors)))
				|| (enem->self.pos.z+200+jumpdistz >= next1->cube->origin.z//we need to jump
				&& enem->self.pos.x+180+jumpdistxr > next1->cube->origin.x-next1->cube->ext.x)) //we don't jump too far to the right
				{
					enem->keys[set.keys[RIGHT1]] = true;
					enem->keys[set.keys[LEFT1]] = false;
				}
			}
		}
		else //no need to go left/right to reach next floor - what about the second target floor ?:
		{
			
			if(next2->cube->origin.x-next2->cube->ext.x > enem->self.pos.x)
			{
				if(enem->self.pos.x+120 < next1->cube->origin.x
				&& (enem->self.pos.x+130 < floors[current].cube->origin.x && !(enem->self.pos.z+200+jumpdistz >= next1->cube->origin.z))
				|| (enem->self.pos.z+200+jumpdistz >= next1->cube->origin.z && hasToJump(current,floors) 	//we need to jump 
				&& enem->self.pos.x+180+jumpdistxl < next1->cube->origin.x)) //we don't jump too far to the left
				{
				 	enem->keys[set.keys[LEFT1]] = true;
				 	enem->keys[set.keys[RIGHT1]] = false;
				}
			}
			else if(next2->cube->origin.x < enem->self.pos.x)
			{
				if(enem->self.pos.x-120 > next1->cube->origin.x-next1->cube->ext.x
				&& (enem->self.pos.x-130 > floors[current].cube->origin.x-floors[current].cube->ext.x && !(enem->self.pos.z+200+jumpdistz >= next1->cube->origin.z))
				|| (enem->self.pos.z+200+jumpdistz >= next1->cube->origin.z && hasToJump(current,floors)//we need to jump 
				&& enem->self.pos.x+180+jumpdistxr > next1->cube->origin.x-next1->cube->ext.x)) //we don't jump too far to the right
				{
					enem->keys[set.keys[RIGHT1]] = true;
					enem->keys[set.keys[LEFT1]] = false;
				}
			}
		}
		
		//something affects our s_speed -> do some movements to lessen the effect
		if((floors[current].type == ICE || floors[current].type == LEFT_PUSH || floors[current].type == RIGHT_PUSH)
		&&( enem->self.pos.z+200+jumpdistz < next1->cube->origin.z || (floors[current].cube->origin.z+floors[current].cube->ext.z+10 < next1->cube->origin.z && !hasToJump(current,floors))))
		{
			if((!enem->keys[set.keys[RIGHT1]] || enem->self.pos.x-320 < floors[current].cube->origin.x-floors[current].cube->ext.x) && enem->self.s_speed < 0)
			{
				enem->keys[set.keys[LEFT1]] = true; 
				enem->keys[set.keys[RIGHT1]] = false;
			}
			else if((!enem->keys[set.keys[LEFT1]] || enem->self.pos.x+320 > floors[current].cube->origin.x) && enem->self.s_speed > 0)
			{
				enem->keys[set.keys[RIGHT1]] = true;
				enem->keys[set.keys[LEFT1]] = false;
			}
		}
			
		
		//do we need to jump ? 
		if(enem->self.v_speed == 0 //we are not airborne
		&& hasToJump(current,floors))	//we need to jump
		{
			strcpy(debugmsg,"JUUUUUMP !!!");
			if((enem->keys[set.keys[RIGHT1]] || enem->self.s_speed < 0)
			&& ((floors[current].cube->origin.x-floors[current].cube->ext.x)-next1->cube->origin.x > 200
			|| next1->cube->origin.z-(floors[current].cube->origin.z+floors[current].cube->ext.z) > 200)
			&& enem->self.pos.x+20 < (floors[current].cube->origin.x-floors[current].cube->ext.x))
			{
				enem->keys[set.keys[JUMP1]] = true;
				strcpy(debugmsg,"JUMP 1");
			}
			else if((enem->keys[set.keys[LEFT1]] || enem->self.s_speed < 0)
			&& ((next1->cube->origin.x-next1->cube->ext.x)-floors[current].cube->origin.x > 200
			|| next1->cube->origin.z-(floors[current].cube->origin.z+floors[current].cube->ext.z) > 200)
			&& enem->self.pos.x-20 > floors[current].cube->origin.x)
			{
				enem->keys[set.keys[JUMP1]] = true;
				strcpy(debugmsg,"JUMP 2");
				
			}
			else if((next1->cube->origin.z-(floors[current].cube->origin.z+floors[current].cube->ext.z) > 200
			|| enem->self.pos.x-next1->cube->origin.x  > 300
			|| next1->cube->origin.x-next1->cube->ext.x-enem->self.pos.x  > 300)
			&& enem->self.pos.z+enem->self.f_speed > (floors[current].cube->origin.z+floors[current].cube->ext.z))
			{
				enem->keys[set.keys[JUMP1]] = true;
				strcpy(debugmsg,"JUMP 3");
			}		
			else if(enem->self.pos.z+200+jumpdistz >= next1->cube->origin.z)	//we can reach the next floor by jumping now
			{
				if(enem->keys[set.keys[LEFT1]])
				{
					//jump to the left would be far enough, but not too far
					if(enem->self.pos.x+180+jumpdistxl > next1->cube->origin.x-next1->cube->ext.x 
					&& enem->self.pos.x-180+jumpdistxl < next1->cube->origin.x)
					{
						int u = 0;
						for(;u<4;u++)
						{
							if(obstacles[u])
							{
								int sider = (obstacles[u]->cube->origin.x-obstacles[u]->cube->ext.x-(enem->self.pos.x+200))-(95*((obstacles[u]->cube->origin.z+obstacles[u]->cube->ext.z)-(enem->self.pos.z-220)))/enem->self.f_speed; //können wir daran vorbeispringen
				
								if(obstacles[u]->cube->origin.z < enem->self.pos.z+220
								&& obstacles[u]->cube->origin.z+obstacles[u]->cube->ext.z > enem->self.pos.z-220
								&& sider < 0
								&& obstacles[u]->cube->origin.y-floors[enem->cam.pos.x].cube->origin.y > 1000)
									break;	//an obstacle might block our jump
							}
							
						}
						if(u==4)
						{
							enem->keys[set.keys[JUMP1]] = true; //jump !!
							strcpy(debugmsg,"JUMP 4");
						}
					}
				}
				else if(enem->keys[set.keys[RIGHT1]])
				{
					//jump to the right would be far enough, but not too far
					if(enem->self.pos.x-180+jumpdistxr < next1->cube->origin.x
					&& enem->self.pos.x+180+jumpdistxr > next1->cube->origin.x-next1->cube->ext.x)
					{
						
						int u = 0;
						for(;u<4;u++)
						{
							if(obstacles[u])
							{
								int sidel = (enem->self.pos.x-200-obstacles[u]->cube->origin.x)-(95*((obstacles[u]->cube->origin.z+obstacles[u]->cube->ext.z)-(enem->self.pos.z-220)))/enem->self.f_speed; //können wir daran vorbeispringen
								
								if(obstacles[u]->cube->origin.z < enem->self.pos.z+220
								&& obstacles[u]->cube->origin.z+obstacles[u]->cube->ext.z > enem->self.pos.z-220
								&& sidel < 0
								&& obstacles[u]->cube->origin.y-floors[enem->cam.pos.x].cube->origin.y >1000)
									break; //an obstacle might block our jump
							}
							
						}
						if(u==4)
						{
							enem->keys[set.keys[JUMP1]] = true; //jump !!
							strcpy(debugmsg,"JUMP 5");
						}
					}
				}
				else if(enem->self.pos.x+200 > next1->cube->origin.x-next1->cube->ext.x 
				&& enem->self.pos.x-200 < next1->cube->origin.x)	//no need to steer left/right - do a straight jump
				{
					enem->keys[set.keys[JUMP1]] = true; //jump !!
					strcpy(debugmsg,"JUMP 6");
				}
				else
				{
					//enem->keys[set.keys[JUMP1]] = true;
					strcpy(debugmsg,"JUMP 7");
				}
					
			}
			else
				enem->keys[set.keys[JUMP1]] = false;
		}
		else if(!(enem->self.pos.x+200 > next1->cube->origin.x-next1->cube->ext.x 
		&& enem->self.pos.x-200 < next1->cube->origin.x))	//we didn't manage to get to the left/right far enough
		{
			if(enem->self.pos.z+100 > floors[current].cube->origin.z+floors[current].cube->ext.z //do a jump, if we are about to drop
			|| enem->self.pos.z+jumpdistz+600 > next1->cube->origin.z+next1->cube->ext.z)//do a jump, if we are about to jump too far
				enem->keys[set.keys[JUMP1]] = true;	
			
		}
		
		
		if(floors[current].type == HURT)
		{
			if(!hasToJump(current,floors))
			{
				enem->keys[set.keys[JUMP1]] = true;
				enem->keys[set.keys[RIGHT1]] = enem->keys[set.keys[LEFT1]] = false;
			}
		}
		
		
		
		
		//avoid obstacles
		for(int j = 0;j<4;j++)
		{
			if(!obstacles[j])
				break;
				
			//obstacle is in our way
			if(obstacles[j]->cube->origin.x > enem->self.pos.x-250 
			&& obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x < enem->self.pos.x+250
			&& obstacles[j]->cube->origin.z > enem->self.pos.z)
			{
				if(obstacles[j]->cube->origin.y-1000 > floors[current].cube->origin.y //we have to dodge
				|| (obstacles[j]->cube->origin.z - enem->self.pos.z)*95/enem->self.f_speed > obstacles[j]->cube->origin.x-enem->self.pos.x
				|| (obstacles[j]->cube->origin.z - enem->self.pos.z)*95/enem->self.f_speed > enem->self.pos.x-(obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x)) //we are able to dodge this
				{
					//TODO: RIGHT/LEFT_PUSH floors affect s_speed....
					int mside = (95*(floors[current].cube->origin.z+floors[current].cube->origin.z-(obstacles[j]->cube->origin.z+obstacles[j]->cube->ext.z)))/enem->self.f_speed; //distance we can drive along the x-axis after having passed the obstacle
					
					
					int poss_sidel = obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x-200+mside; //position, we can drive to the left, after getting around the obstacle's right side
					int poss_sider = obstacles[j]->cube->origin.x+200-mside; //position, we can drive to the right, after getting around the obstacle's left side
					
					
					if(obstacles[j]->cube->origin.x-enem->self.pos.x < enem->self.pos.x-(obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x)) //it's shorter to take the left alternative
					{
						if(poss_sider < next1->cube->origin.x //we are able to get around the left side and still get back (right) far enough
						&& floors[current].cube->origin.x-obstacles[j]->cube->origin.x >= 500) //there's enough space for us to get around the left side
						{
							if(enem->self.pos.x+200 < floors[current].cube->origin.x)
							{
								enem->keys[set.keys[LEFT1]] = true;
								enem->keys[set.keys[RIGHT1]] = false;
							}	
			
						}
						else if(poss_sidel > next1->cube->origin.x-next1->cube->ext.x  //we are able to get around the right side and still get back far enough
						&& (obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x)-(floors[current].cube->origin.x-floors[current].cube->ext.x) >= 500)//there's enough space for us to get around the left side
						{
							if(enem->self.pos.x-200 > floors[current].cube->origin.x-floors[current].cube->ext.x)
							{		
								enem->keys[set.keys[RIGHT1]] = true;
								enem->keys[set.keys[LEFT1]] = false;
							}
							
						}	
					}
					else //same as above - just the other way around
					{
						if(poss_sidel > next1->cube->origin.x-next1->cube->ext.x  
						&& (obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x)-(floors[current].cube->origin.x-floors[current].cube->ext.x) >= 500)
						{
							if(enem->self.pos.x-200 > floors[current].cube->origin.x-floors[current].cube->ext.x)
							{
								enem->keys[set.keys[RIGHT1]] = true;
								enem->keys[set.keys[LEFT1]] = false;
							}
							
						}
						else if(poss_sider < next1->cube->origin.x 
						&& floors[current].cube->origin.x-obstacles[j]->cube->origin.x >= 500)
						{
							if(enem->self.pos.x+200 < floors[current].cube->origin.x)
							{
								enem->keys[set.keys[LEFT1]] = true;
								enem->keys[set.keys[RIGHT1]] = false;
							}
			
						}		
						
					}
					//TODO: RIGHT/LEFT_PUSH: react to speed accelerations from environment
					int nside = (95*(obstacles[j]->cube->origin.z-(enem->self.pos.z+200)))/enem->self.f_speed;
					
					
					if(enem->self.pos.x-200+nside < obstacles[j]->cube->origin.x
					&& enem->self.pos.x+200-nside > obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x)
					{
					
						enem->keys[set.keys[BACKWARD1]] = true;
						enem->keys[set.keys[FORWARD1]] = true;
					}
					enem->keys[set.keys[JUMP1]] = false;	
				}	
				else //jump over it
				{
					int heightm = HeightFrameLoss(floors[current].cube->origin.y-obstacles[j]->cube->origin.y);	
					int jumpd = (enem->self.f_speed)*(30)-(enem->self.f_speed)*heightm;
					
					if(enem->self.pos.z+200+jumpd >= obstacles[j]->cube->origin.z)//we can jump high enough
					{
						enem->keys[set.keys[JUMP1]] = true;
					}
				}
				break; //dodge this obstacle first before reacting to others
			}
				
		}
		
		//react to obstacles that are still a bit away from us ( no need to get into a hurry )
		//very similar to the stuff done above
		for(int j = 4;j<8;j++)
		{
			if(!obstacles[j])
				break;
				
			if(obstacles[j]->cube->origin.x > enem->self.pos.x-250 
			&& obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x < enem->self.pos.x+250)
			{
				if(obstacles[j]->cube->origin.z-next1->cube->origin.z < 5000
				&& obstacles[j]->cube->origin.y-next1->cube->origin.y > 1000)//it might be a bit narrow there....
				{
					//TODO: RIGHT/LEFT_PUSH floors affect s_speed....
					int mside = (95*(next1->cube->origin.z+next1->cube->origin.z-(obstacles[j]->cube->origin.z+obstacles[j]->cube->ext.z)))/enem->self.f_speed; 
					
					int poss_sidel = obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x-200+mside; 
					int poss_sider = obstacles[j]->cube->origin.x+200-mside; 
				
					if(poss_sider < next2->cube->origin.x)
					{
						if(obstacles[j]->cube->origin.x-enem->self.pos.x < enem->self.pos.x-(obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x)) 
						{
							
							if(enem->self.pos.x+200 < floors[current].cube->origin.x)
							{
								enem->keys[set.keys[LEFT1]] = true;
								enem->keys[set.keys[RIGHT1]] = false;
							}
							
						}
						else
						{
							
							if(enem->self.pos.x-200 > floors[current].cube->origin.x-floors[current].cube->ext.x)
							{
								enem->keys[set.keys[RIGHT1]] = true;
								enem->keys[set.keys[LEFT1]] = false;
							}
							
						}
					}
					else if(poss_sidel > next2->cube->origin.x-next1->cube->ext.x)
					{
						if(obstacles[j]->cube->origin.x-enem->self.pos.x > enem->self.pos.x-(obstacles[j]->cube->origin.x-obstacles[j]->cube->ext.x))
						{
							
							if(enem->self.pos.x-200 > floors[current].cube->origin.x-floors[current].cube->ext.x)
							{
								enem->keys[set.keys[RIGHT1]] = true;
								enem->keys[set.keys[LEFT1]] = false;
							}
							
						}
						else
						{
							
							if(enem->self.pos.x+200 < floors[current].cube->origin.x)
							{
								enem->keys[set.keys[LEFT1]] = true;
								enem->keys[set.keys[RIGHT1]] = false;
							}
							
						}
					}
					
					if(next1->cube->origin.z+500+enem->self.f_speed*700 > obstacles[j]->cube->origin.z)
					{
						
						enem->keys[set.keys[BACKWARD1]] = true;
						enem->keys[set.keys[FORWARD1]] = true;
					}
				}
				
			}
				
		}
		
	}
	
	RamPlayer(Player,enem,floors);
	
	
	
}





//update AI similar to Update-Player fct
void UpdateAI(PlayerPawn *enem,int enemytype, PlayerPawn *Player, Tile* floors, int num_tiles, PongBall* pBall)
{
	int old_s_speed = 0;
	int old_on_sth = 0;
	
	//if enemy falls too fast -> die
	if(enem->self.v_speed < -440)
	{
		play(-1,S_EXPLODE);
		if(!enemytype)
			enemy = false;
		else
			life = false;
		return;	
	}	
				
	old_on_sth = enem->self.on_floor;
	if((enem->self.on_floor = OnFloor(enem,floors,num_tiles,pBall))< 0)
		enem->self.v_speed -= 8; //8 = gravity
	else
	{
		if(enem->self.v_speed < 0 
		&& enem->self.on_floor != SIDE_BLOCK_L
		&& enem->self.on_floor != SIDE_BLOCK_R)
		{
			play(2,S_BUMP);
			enem->self.v_speed = 0;
			if(old_on_sth < 0 &&  old_on_sth != enem->self.on_floor) //slow down right/left speed
				enem->self.s_speed = enem->self.s_speed/2;
		}
	
		
		old_s_speed = enem->self.s_speed;
		
		enem->self.s_speed = 0;
			
		
		if(enem->keys[set.keys[FORWARD1]] 	//forward
		&& enem->self.f_speed < MAX_FSPEED) 
		{
			play(2,S_SPEEDUP);
			enem->self.f_speed +=4;
		}
		if(enem->keys[set.keys[BACKWARD1]]	//backward
		 && enem->self.f_speed > -MAX_BSPEED)
		{
			play(2,S_SLOWDOWN);
			enem->self.f_speed -=3;	
		}
		if(enem->keys[set.keys[LEFT1]]		//left
		&& enem->self.s_speed < MAX_SSPEED
		&& enem->self.on_floor != SIDE_BLOCK_L)
		{
			enem->self.s_speed +=95;
		}	
		if(enem->keys[set.keys[RIGHT1]] 	//right
		&& enem->self.s_speed > -MAX_SSPEED
		&& enem->self.on_floor != SIDE_BLOCK_R)
		{
			enem->self.s_speed -=95;			
		}
			
		if(enem->keys[set.keys[JUMP1]])
		{
			if(enem->self.on_floor == SIDE_BLOCK_L)
				enem->self.s_speed = -95; 
			if(enem->self.on_floor == SIDE_BLOCK_R)
				enem->self.s_speed = 95; 
			if(enem->self.v_speed < 100 && enem->self.on_floor != SAND)
			{
				enem->self.v_speed = 125;
				if(!enemytype)
					play(-1,S_JUMP);
				enem->smoke[9].lifetime = SDL_GetTicks()+randomi(300,0,0,900);
				enem->smoke[9].origin = enem->self.pos;
				enem->smoke[9].origin.y+=100;
				enem->smoke[9].origin.x+=200;
				enem->smoke[9].type = 0;
				enem->smoke[9].r = enem->smoke[9].g = 0.75f;
				enem->smoke[9].b = 1.0f;
				enem->smoke[9].xs = 2.5f+((float)randomi(0,0,0,7))/10.0f;
				enem->smoke[9].ys = 2.5f+((float)randomi(0,0,0,7))/10.0f;
				enem->keys[set.keys[JUMP1]] = false;	
			}
		}
			
		switch(enem->self.on_floor)
		{
			case NORMAL:
			break;
			case GOAL:
				if(!enemytype)
					enemy = false;
				else
					life = false;
				return;
			break;
			case SPEED_UP:
				play(2,S_SPEEDUP);
				enem->self.f_speed +=5;	
			break;
			case SLOW_DOWN:
				play(2,S_SLOWDOWN);
				enem->self.f_speed -=4;	
			break;
			case RIGHT_PUSH:
				if(enem->self.on_floor == SIDE_BLOCK_R)
					break;
				
				enem->self.s_speed -=60;
			
			break;
			case LEFT_PUSH:
				if(enem->self.on_floor == SIDE_BLOCK_L)
					break;
				enem->self.s_speed +=60;
				
			break;
			case JUMP:
				if(!enemytype)
					play(-1,S_JUMP);
				enem->self.v_speed = (int)((float)abs(enem->self.f_speed)/1.9f);
			break;
			case HURT:
			{
				
				play(1,S_HURT);
					
				if(!enemytype)
					enem->health -= 3;
	
				if(enem->health <= 0)
				{
					if(!enemytype)
						enemy = false;
					return;
				}
			}
			break;
			case HEAL:
				if(enem->health < 100)
				{
					enem->health += 2;
					play(1,S_HEAL);
				}
			break;
			case ICE:
				
				enem->self.s_speed =old_s_speed;
					
				if(enem->keys[set.keys[FORWARD1]] && enem->self.f_speed < MAX_FSPEED)//forward
					enem->self.f_speed -=3;
				if(enem->keys[set.keys[BACKWARD1]] && enem->self.f_speed > -MAX_BSPEED)//backward
					enem->self.f_speed +=2;
				if(enem->keys[set.keys[LEFT1]] && enem->self.s_speed > -MAX_SSPEED)
				{
					if(enem->self.s_speed < 100)
					{
						if(enem->self.s_speed > 0)
							enem->self.s_speed += 15;
						else
							enem->self.s_speed += 8;
					}

				}
				if(enem->keys[set.keys[RIGHT1]] && enem->self.s_speed < MAX_SSPEED)
				{
					if(enem->self.s_speed > -100)
					{
						if(enem->self.s_speed < 0)
							enem->self.s_speed -= 15;
						else
							enem->self.s_speed -= 8;
					}
					
				}
			break;
			case CRAZY:
				//HMMMMMM
				
			break;		
			
		};
		enem->last_floor = enem->self.on_floor;
	}

	enem->self.oldpos = enem->self.pos;
	enem->self.pos.z	+= enem->self.f_speed;
	enem->self.pos.y	+= enem->self.v_speed;
	enem->self.pos.x	+= enem->self.s_speed;
	
}


//pong AI
void UpdatePong(PlayerPawn *Player, Tile * floors, int num_tiles)
{
	if(!pBall)
		return;
	
	Tile *k = &floors[0];	
	Tile *max = k+num_tiles;	
		
	while(k<max)
	{	
		Bounce(k);
		k++;
	}	
	
	bool left = false;
	bool right = false;
	
	for(int i= 4;i>0;i--)
		pBall->oldpos[i] = pBall->oldpos[i-1];
	pBall->oldpos[0] = pBall->pos;
	
	pBall->pos.x += pBall->speed.x;
	pBall->pos.y += pBall->speed.y;
	pBall->pos.z += pBall->speed.z;
	
	
	if(pBall->speed.x == pBall->speed.y 
	&& pBall->speed.x == pBall->speed.z 
	&& !pBall->speed.x)
	{
		//get startet -> move ball
		pBall->speed.z = -200;
		if(SDL_GetTicks()%2)
			pBall->speed.x = -20;
		else
			pBall->speed.x = 20;	
	}
	
	if(!pBall->floor->cube->flag3)//EASY
	{
		//just move left/right if ball is left/right of us
		if(pBall->pos.x+182 < pongx)
		{
			pongx -= 81;
			left = true;
		}
		else if(pBall->pos.x-182 > pongx)
		{
			pongx += 81;
			right = true;
		}
	}
	else if(pBall->floor->cube->flag3 == 1)//MEDIUM
	{
		Vertex next1pos = pBall->pos;
		
	
		float dist = abs(pBall->floor->cube->flag2-pBall->pos.z);
		if(dist > 3000 && !pBall->drall) //MEDIUM skilled pong-enemies don't know how to calculate the effects of a "drall"
		{
			//calculate next position of ball by looking at its speed and obstacles
			next1pos.x += pBall->speed.x*10;
			next1pos.z += pBall->speed.z*10;
			
			//obstacles, where ball might bounce off
			Tile *k = pBall->floor+1;	
		  	Tile *max = &floors[num_tiles];
			while(k<max)	
			{	
				int x1 = k->cube->origin.x;
				int x3 = k->cube->origin.z;
				//moving floor
				if(k->cube->movetime && k->cube->movecount)
				{
					float part = (float)k->cube->movecount/(float)(k->cube->movetime*50);
					
					if((int)part%2) //on the way back
						part = 1.0f-(part-(int)part);
					else	
						part -=(int)part;	
					
					x1 += (int)(part*(k->cube->flag1-k->cube->origin.x));
					x3 += (int)(part*(k->cube->flag3-k->cube->origin.z));
				}
				
				if(next1pos.x-200 < x1
				&& pBall->pos.x-200 > x1
				&& next1pos.z-200 <= x3+k->cube->ext.z
				&& next1pos.z+200 >= x3)
				{
					next1pos.x = x1+(x1-(next1pos.x-200));
				}	
				if(next1pos.x+200 > x1-k->cube->ext.x
				&& pBall->pos.x+200 < x1-k->cube->ext.x
				&& next1pos.z-200 <= x3+k->cube->ext.z
				&& next1pos.z+200 >= x3)
				{
					next1pos.x = x1-k->cube->ext.x-(next1pos.x+200-(x1-k->cube->ext.x));
				}
				
				if(next1pos.z+200 > x3
				&& pBall->pos.z+200 < x3
				&& next1pos.x-200 <= x1
				&& next1pos.x+200 >= x1-k->cube->ext.x)
				{
					next1pos.z = x3-(next1pos.z+200-k->cube->origin.z);		
				}
				
				if(next1pos.z+200 < x3+k->cube->ext.z
				&& pBall->pos.z+200 > x3+k->cube->ext.z
				&& next1pos.x-200 <= x1
				&& next1pos.x+200 >= x1-k->cube->ext.x)
				{
					next1pos.z = x3+k->cube->ext.z+(x3+k->cube->ext.z-(next1pos.z+200));
					
				}
				
				k++;
			}
			
			if(next1pos.x+200 > pBall->floor->cube->origin.x)
			{
				next1pos.x = pBall->floor->cube->origin.x-(next1pos.x+200-pBall->floor->cube->origin.x);
			}
			else if(next1pos.x-200 < pBall->floor->cube->origin.x-pBall->floor->cube->ext.x)
			{
				next1pos.x = pBall->floor->cube->origin.x-pBall->floor->cube->ext.x+(pBall->floor->cube->origin.x-pBall->floor->cube->ext.x-(next1pos.x-200));
			}
		}
		
		//move, if ball is going to be right/left of us soon
		if(next1pos.x+120 < pongx)
		{
			pongx -= 95;
			left = true;
		}
		else if(next1pos.x-120 > pongx)
		{
			pongx += 95;
			right = true;
		}
	}
	else//HARD
	{
		Vertex next1pos = pBall->pos;
		float dist = abs(pBall->floor->cube->flag2-pBall->pos.z);
		if(dist > 2000)
		{
			//just like MEDIUM, but we know the effects of a ball having "drall"
			next1pos.x += pBall->speed.x*20;
			next1pos.z += pBall->speed.z*20;
			
			
			Tile *k = pBall->floor+1;	
		  	Tile *max = &floors[num_tiles];
			while(k<max)	
			{	
				int x1 = k->cube->origin.x;
				int x3 = k->cube->origin.z;
				//moving floor
				if(k->cube->movetime && k->cube->movecount)
				{
					float part = (float)k->cube->movecount/(float)(k->cube->movetime*50);
					
					if((int)part%2) //on the way back
						part = 1.0f-(part-(int)part);
					else	
						part -=(int)part;	
					
					x1 += (int)(part*(k->cube->flag1-k->cube->origin.x));
					x3 += (int)(part*(k->cube->flag3-k->cube->origin.z));
				}
				
				if(next1pos.x-200 < x1
				&& pBall->pos.x-200 > x1
				&& next1pos.z-200 <= x3+k->cube->ext.z
				&& next1pos.z+200 >= x3)
				{
					next1pos.x = x1+(x1-(next1pos.x-200));
				}	
				if(next1pos.x+200 > x1-k->cube->ext.x
				&& pBall->pos.x+200 < x1-k->cube->ext.x
				&& next1pos.z-200 <= x3+k->cube->ext.z
				&& next1pos.z+200 >= x3)
				{
					next1pos.x = x1-k->cube->ext.x-(next1pos.x+200-(x1-k->cube->ext.x));
				}
				
				if(next1pos.z+200 > x3
				&& pBall->pos.z+200 < x3
				&& next1pos.x-200 <= x1
				&& next1pos.x+200 >= x1-k->cube->ext.x)
				{
					next1pos.z = x3-(next1pos.z+200-x3);		
				}
				
				if(next1pos.z+200 < x3+k->cube->ext.z
				&& pBall->pos.z+200 > x3+k->cube->ext.z
				&& next1pos.x-200 <= x1
				&& next1pos.x+200 >= x1-k->cube->ext.x)
				{
					next1pos.z = x3+k->cube->ext.z+(x3+k->cube->ext.z-(next1pos.z+200));
					
				}
				
				k++;
			}
			
			if(next1pos.x+200 > pBall->floor->cube->origin.x)
			{
				next1pos.x = pBall->floor->cube->origin.x-(next1pos.x+200-pBall->floor->cube->origin.x);
			}
			else if(next1pos.x-200 < pBall->floor->cube->origin.x-pBall->floor->cube->ext.x)
			{
				next1pos.x = pBall->floor->cube->origin.x-pBall->floor->cube->ext.x+(pBall->floor->cube->origin.x-pBall->floor->cube->ext.x-(next1pos.x-200));
			}
		}
		
		if(next1pos.x+110+dist/350 < pongx)
		{
			pongx -= 97;
			left = true;	
		}
		else if(next1pos.x-110-dist/350 > pongx)
		{
			pongx += 97;
			right = true;
		}
	
	}
			
	
			
	
	//let ball bounce off of the right/ left edge of current floor
	if(pBall->pos.x+200 > pBall->floor->cube->origin.x)
	{
		pBall->speed.x = -pBall->speed.x;
		play(-1,S_PONG);
		if(pBall->pos.x > pBall->floor->cube->origin.x)
		{
			if(abs(pBall->speed.x) < 10)
				pBall->speed.x = -10;
		}
		pBall->pos.x = pBall->floor->cube->origin.x-200;
	}	
	if(pBall->pos.x-200 < pBall->floor->cube->origin.x-pBall->floor->cube->ext.x)
	{
		pBall->speed.x = -pBall->speed.x;
		play(-1,S_PONG);
		if(pBall->pos.x < pBall->floor->cube->origin.x-pBall->floor->cube->ext.x)
		{
			if(abs(pBall->speed.x) < 10)
				pBall->speed.x = 10;
		}
		pBall->pos.x = pBall->floor->cube->origin.x-pBall->floor->cube->ext.x+200;
	}
	
	
	//let ball bounce off of enemy
	if((pBall->pos.z+200 > pBall->floor->cube->flag2-250
	|| pBall->pos.z+200+pBall->speed.z > pBall->floor->cube->flag2-250)
	&& pBall->pos.x+200 > pongx-MAX_FSPEED
	&&  pBall->pos.x-200 < pongx+MAX_FSPEED)
	{
		pBall->pos.z = pBall->floor->cube->flag2-MAX_FSPEED;
		pBall->speed.z = -pBall->speed.z;
		play(-1,S_PONG);
		
		if(pBall->floor->cube->flag3 == 0)//EASY
		{
			if(pBall->drall < 0 && abs(pBall->speed.x) > 20)
				pBall->speed.x = (int)(pBall->speed.x/1.7);
			else if(pBall->drall > 0 && abs(pBall->speed.x) > 20)
				pBall->speed.x = (int)(pBall->speed.x/1.7);
				
			if(SDL_GetTicks()%3)
				pBall->drall = 0;	//nullify drall most of the time
		}
		else if(pBall->floor->cube->flag3 == 1)//MEDIUM
		{
			if(abs(pBall->drall) < 2)
			{
				if(right)
					pBall->drall += 1;
				else if(left)
					pBall->drall -= 1;
			}
			else if(abs(pBall->drall) > 2)
			{
				if(pBall->drall > 0)
					pBall->drall -= 1;
				else
					pBall->drall += 1;
			} 
			
		}
		else if(pBall->floor->cube->flag3 == 2)//HARD
		{
			//get some real nice drall once out of three times
			if(SDL_GetTicks()%3)
			{
				if(right && pBall->drall < 5)
					pBall->drall += 1;
				else if(left && pBall->drall > -5)
					pBall->drall -= 1;
			}
			
		}
	}
	
	//let ball bounce off of player
	if((pBall->pos.z-200 < Player->self.pos.z+250
	|| pBall->pos.z-200+pBall->speed.z < Player->self.pos.z+250)
	&& pBall->pos.x+220 > Player->self.pos.x-MAX_FSPEED
	&&  pBall->pos.x-220 < Player->self.pos.x+MAX_FSPEED)
	{
		pBall->pos.z = Player->self.pos.z+MAX_FSPEED;
		pBall->speed.z = -pBall->speed.z;
		play(-1,S_PONG);
		if(Player->keys[set.keys[LEFT1+7]] && pBall->drall > -5)
			pBall->drall -= 1;
		else if(Player->keys[set.keys[RIGHT1+7]] && pBall->drall < 5)
			pBall->drall += 1;
			
	}
	
	if(pBall->drall)	
		pBall->speed.x+=pBall->drall;		
	
	
	if(pBall->pos.z+200 < Player->self.pos.z+250)
	{
		
		Player->bumpedtime = -10;
		
		if(pBall)
		{
			delete pBall;
			pBall = NULL;
		}
		
		pongx = -1000;
		if(Player->protectiontime == 1)
			((Single*)gamestates[GS_SINGLE])->NewRun();	//TODO: CHANGE THIS - it just sucks, if the AI is state-dependend
		else
			Player->protectiontime--;
	}
	else if(pBall->pos.z-200 > pBall->floor->cube->flag2-250)
	{
		if(pBall)
		{
			delete pBall;
			pBall = NULL;
		}
		pongx = -1000;
		Player->bumpedtime = -10;
	}

}



void UpdateLife(PlayerPawn *Player, Tile* floors, int num_tiles)
{
	int old_on_index = Life.cam.pos.x;
	if(Life.pushtime > 0)
		Life.pushtime--;
	else if(!(SDL_GetTicks()%(200-Life.started*10)))
		Life.pushtime = 100+Life.started*40;
		
	UpdateAI(&Life,1,Player,floors,num_tiles,pBall);
	NavigateAI(&Life,old_on_index,Player,floors,num_tiles);
	
	if(Length((float)Player->self.pos.x,(float)Player->self.pos.y,(float)Player->self.pos.z,(float)Life.self.pos.x,(float)Life.self.pos.y,(float)Life.self.pos.z)<500
	&& Life.pushtime <= 0)
	{
		play(-1,S_LIFE);
		Player->protectiontime++; //player receives life
		life = false;
	}		
}



void UpdateEnemy(PlayerPawn *Player, Tile* floors, int num_tiles)
{
	int old_on_index = Enemy.cam.pos.x;
	if(Enemy.powerupused >0)
		Enemy.powerupused--;
	UpdateAI(&Enemy,0,Player,floors,num_tiles,pBall);
	NavigateAI(&Enemy,old_on_index,Player,floors,num_tiles);
	Crash(Player);	
	
}
