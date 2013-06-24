/*
#				Orbit-Hopper
#				CastleMEnd.cpp : CastleAttack statistics
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
#include "../headers/CastleMEnd.h"
#include "../headers/CastleM.h"



extern Settings set;
extern Gamestate *gamestates[15];

void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b);

CastleMEnd::CastleMEnd()
{
	
}

CastleMEnd::~CastleMEnd()
{
	
}

void CastleMEnd::Update(unsigned int diff){}

void CastleMEnd::Init()
{
}

void CastleMEnd::Render()
{
	glDisable(GL_LIGHTING);
	glColor3f(0.7,0.7,0.7);	
  	DrawBackGround(35,0);
  	glColor3f(1.0,1.0,1.0);	
  	
	char temp[128];
	sprintf(temp,"Congratulations Player %d - you win.",((CastleM *)gamestates[GS_CASTLEM])->Player1.min ? 2 : 1);
	printtxt2d((int)(0.17*(float)(set.width)),(int)(0.872*(float)(set.height)),1.3f,1.0f,1.0f,0.0f,temp);
	

	sprintf(temp,"Player 1 died %d times",((CastleM *)gamestates[GS_CASTLEM])->Player2.sec);
	printtxt2d((int)(0.2*(float)(set.width)),(int)(0.65*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,temp);
	sprintf(temp,"and used %d powerups.",((CastleM *)gamestates[GS_CASTLEM])->Player2.tsec);
	printtxt2d((int)(0.2*(float)(set.width)),(int)(0.60*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,temp);
	
	sprintf(temp,"Player 2 died %d times",((CastleM *)gamestates[GS_CASTLEM])->Player1.sec);
	printtxt2d((int)(0.2*(float)(set.width)),(int)(0.40*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,temp);
	sprintf(temp,"and used %d powerups.",((CastleM *)gamestates[GS_CASTLEM])->Player1.tsec);
	printtxt2d((int)(0.2*(float)(set.width)),(int)(0.35*(float)(set.height)),1.0f,1.0f,1.0f,1.0f,temp);
	
	
	DrawRectangle((int)(0.029*(float)(set.width)),(int)(0.82*(float)(set.height)),(int)(0.941*(float)(set.width)),(int)(0.651*(float)(set.height)),1.0f,1.0f,0.0f);
	glEnable(GL_LIGHTING);
	
	
	
}

int CastleMEnd::Input(SDL_Event action,SDL_keysym key)
{
	if(action.type == SDL_KEYDOWN)
	{
		if(key.sym == SDLK_RETURN)
			SwitchGamestate(GS_CASTLEM);	
		else if(key.sym == SDLK_ESCAPE)
			SwitchGamestate(GS_MODESELM);
	}	
	return 0;
}
