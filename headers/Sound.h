/*
#				Sound.h

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
#ifndef _SOUND_H_
#define _SOUND_H_

int InitSound();
void play(int channel,int sound);

#define S_HEAL		0
#define S_HURT		1
#define S_JUMP		2
#define S_BUMP		3
#define S_MENU		4
#define S_SELECT	5
#define S_PLACE		6
#define S_CHECK		7
#define S_SLOWDOWN	8
#define S_SPEEDUP	9
#define S_EXPLODE	10
#define S_POWERUP	11
#define S_PONG		12
#define S_LIFE		13
#define S_PUSH		14

#endif
