/*
#				MapSel.h

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
#ifndef _MAPSEL_H_
#define _MAPSEL_H_


#include "../headers/Structs.h"
#include "../headers/Gamestate.h"


class MapSel : Gamestate
{
	
private:
	unsigned int total_time;
	unsigned int average_time;
	int num_levels;
public:
	MapSel();
	~MapSel();
	void Init();
	int Input(SDL_Event event, SDL_keysym key);
	void Update(unsigned int diff);
	void Render();	
	
	int cur_level;	//public for LevelEnd.cp
};

#endif
