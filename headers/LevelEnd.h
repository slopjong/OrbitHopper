/*
#				LevelEnd.h

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
#ifndef _LEVELEND_H_
#define _LEVELEND_H_



#include "../headers/Gamestate.h"


class LevelEnd : Gamestate
{
	
private:
	char playername[64];
public:
	LevelEnd();
	~LevelEnd();
	void Init();
	int Input(SDL_Event event, SDL_keysym key);
	void Update(unsigned int diff);
	void Render();	
};
#endif

