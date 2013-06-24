/*
#				SinglePlSel.h

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
#ifndef _SINGLEPLSEL_H_
#define _SINGLEPLSEL_H_



#include "../headers/Gamestate.h"


class SinglePlSel : Gamestate
{
	
private:

	int mapfirst;
	int num_skipped;
	int num_levels;
	
	void GetSingleProgress();
	void GetMaps();
	
public:
	SinglePlSel();
	~SinglePlSel();
	void Init();
	int Input(SDL_Event event, SDL_keysym key);
	void Update(unsigned int diff);
	void Render();	
	
	void SaveSingleProgress(); //public for Single.cpp
	
	int levelsel; //public for Single.cpp
	int reached; //public for Single.cpp
};

struct SingleData
{
	char levelname[64];
	bool skipped;
	unsigned int msecs;	
	int lives;
	
};




#endif
