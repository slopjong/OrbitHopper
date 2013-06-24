/*
#				AI.h

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
#ifndef _AI_H_
#define _AI_H_

void InitAI();
void DestroyAI();

void CreateLife(Tile *k);
void CreateEnemy(Tile *k);
void CreatepBall(Tile* i);

void UpdateEnemy(PlayerPawn *Player, Tile* floors, int num_tiles);
void UpdateLife(PlayerPawn *Player, Tile* floors, int num_tiles);
void UpdatePong(PlayerPawn *Player, Tile * floors, int num_tiles);

void DrawAI(Tile *floors);

#endif //_AI_H_
