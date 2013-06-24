/*
#				Orbit-Hopper
#				RaceEnd.cpp : Race statistics
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
#include "../headers/RaceEnd.h"
#include "../headers/RaceM.h"


extern Gamestate *gamestates[15];
extern Settings set;
void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b);

RaceEnd::RaceEnd()
{
	
}

RaceEnd::~RaceEnd()
{
	
}

void RaceEnd::Init()
{
	
}

void  RaceEnd::Render()
{
	glDisable(GL_LIGHTING);
	glColor3f(0.7,0.7,0.7);	
  	DrawBackGround(35,0);
  	glColor3f(1.0,1.0,1.0);	
  	
	char temp[128];
	int winner = 0;
	//player that scored more points wins
	if(((RaceM *)gamestates[GS_RACEM])->Player1.min > ((RaceM *)gamestates[GS_RACEM])->Player2.min)
		winner = 1;
	else if(((RaceM *)gamestates[GS_RACEM])->Player2.min > ((RaceM *)gamestates[GS_RACEM])->Player1.min)
		winner = 2;
	else //player that drove further wins
	{
		if(((RaceM *)gamestates[GS_RACEM])->Player2.tsec > ((RaceM *)gamestates[GS_RACEM])->Player1.tsec)
			winner = 2;
		else
			winner = 1;	
	}
		
	sprintf(temp,"Congratulations Player %d - you win.",3-winner);
	printtxt2d((int)(0.17*set.width),(int)(0.872*set.height),1.3f,1.0f,0.0f,0.0f,temp);
	

	sprintf(temp,"Player 1 earned %d points",((RaceM *)gamestates[GS_RACEM])->Player2.min);
	printtxt2d((int)(0.2*set.width),(int)(0.65*set.height),1.0f,1.0f,1.0f,1.0f,temp);
	sprintf(temp,"and drove a total distance of %d meters. ",((RaceM *)gamestates[GS_RACEM])->Player2.tsec/500);
	printtxt2d((int)(0.2*set.width),(int)(0.60*set.height),1.0f,1.0f,1.0f,1.0f,temp);
	sprintf(temp,"Longest run was %d meters.", ((RaceM *)gamestates[GS_RACEM])->Player2.powerup/500);
	printtxt2d((int)(0.2*set.width),(int)(0.55*set.height),1.0f,1.0f,1.0f,1.0f,temp);
	
	
	sprintf(temp,"Player 2 earned %d points",((RaceM *)gamestates[GS_RACEM])->Player1.min);
	printtxt2d((int)(0.2*set.width),(int)(0.40*set.height),1.0f,1.0f,1.0f,1.0f,temp);
	sprintf(temp,"and drove a total distance of %d meters.",((RaceM *)gamestates[GS_RACEM])->Player1.tsec/500);
	printtxt2d((int)(0.2*set.width),(int)(0.35*set.height),1.0f,1.0f,1.0f,1.0f,temp);
	sprintf(temp,"Longest run was %d meters.",((RaceM *)gamestates[GS_RACEM])->Player1.powerup/500);
	printtxt2d((int)(0.2*set.width),(int)(0.30*set.height),1.0f,1.0f,1.0f,1.0f,temp);
	
	
	DrawRectangle((int)(0.029*set.width),(int)(0.82*set.height),(int)(0.941*set.width),(int)(0.651*set.height),1.0f,0.0f,0.0f);
	glEnable(GL_LIGHTING);
}

int RaceEnd::Input(SDL_Event action,SDL_keysym key)
{
	if(action.type == SDL_KEYDOWN)
	{
		if(key.sym == SDLK_RETURN)
			SwitchGamestate(GS_RACEM);	
		else if(key.sym == SDLK_ESCAPE)
			SwitchGamestate(GS_MODESELM);
	}	
	return 0;
}

void RaceEnd::Update(unsigned int diff){}
