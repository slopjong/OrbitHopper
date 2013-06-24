/*
#				ModeSelM.h

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
#ifndef _MODESELM_H_
#define _MODESELM_H_


#include "../headers/Structs.h"
#include "../headers/Gamestate.h"


#define SM_CASTLE	0
#define SM_RACE		1
#define SM_OPTIONS	2


class ModeSelM : Gamestate
{
	
private:
	int modesel;
	int option;
	int start_index;
	int num_camaps;
	int num_ramaps;
	int ragravity,cagravity;
	int ralives,calives;
	bool powerups;
	GLuint *pics;
	
	void GetRAMaps();
	void GetCAMaps();
	void getPics();
	void drawPic(int i);
	void GetMultSettings();
	void WriteMultSettings();
public:
	ModeSelM();
	~ModeSelM();
	void Init();
	int Input(SDL_Event event, SDL_keysym key);
	void Update(unsigned int diff);
	void Render();	
	
	char **camaps;	//public for CastleM.cpp -> Init
	char **ramaps;	//public for RaceM.cpp -> Init
	int mapsel;				//Public for *End.cpp
	
};



#endif
