/*
#				CastleM.h

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
#ifndef _CASTLEM_H_
#define _CASTLEM_H_


#include "../headers/Structs.h"
#include "../headers/Gamestate.h"


//powerups:
#define PU_ICE		1
#define PU_DJUMP	2
#define PU_WALL		3
#define PU_PUSH		4
#define PU_SHIELD	5
#define PU_HURT		6


class CastleM : Gamestate
{
	
private:
	Tile **floors;
	int num_tiles;
	int player1goalminindex;
	int player2goalmaxindex;
	int anglecount;
	int starttile;
	int gravity;
	int lives;
	bool powerups;

	void LoadOptions();
	void AdjustFov(PlayerPawn &Player,float fov, bool force);
	void ResetPlayer(PlayerPawn &Player,int direction);
	int OnFloor(PlayerPawn &player);
	void UpdatePlayer(PlayerPawn &Player, unsigned int diff);
	void WallCrash(PlayerPawn &Player, Vertex wallpos);
	void PlayerCrash();
	void PlayerPush();
	void LoadCastleMap(char filen[]);
	void DisplayDebug();
	
public:
	CastleM();
	~CastleM();
	void Init();
	int Input(SDL_Event event, SDL_keysym key);
	void Update(unsigned int diff);
	void Render();	

	PlayerPawn Player1; //public for CastleMEnd.cpp
	PlayerPawn Player2; //public for CastleMEnd.cpp
};

#endif
