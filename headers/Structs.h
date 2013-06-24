/*
#				Structs.h

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
#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include "Geometrics.h"

#ifdef WIN32
#include <io.h>
#endif

#include <stdlib.h>

#ifndef WIN32
#include <cstring> //(memset)
#endif

#include <SDL.h>



struct Pawn
{
	int f_speed;
	int s_speed;
	int v_speed;
	int on_floor;
	Vertex pos;
	Vertex oldpos;
};

struct Camera
{
	float fov;
	Vertex pos;
	Vertex *target;	
	Vertex up;
	Vertex offsets;
};

struct Effects
{
	Vertex origin;
	unsigned int lifetime;
	int type;
	float r,g,b;
	float xs,ys,zs;	
	
};


class Tile
{
	public:
		int type;
		int pu_time;
		Cube *cube; //cube
		~Tile();
};

struct PlayerPawn
{
	Pawn self;
	Effects smoke[10];
	int powerup;
	int powerupused;
	int health;
	int dir;
	int last_floor;
	int protectiontime;
	int bumpedtime;
	int pushtime;
	unsigned int run_time;
	int tsec,sec,min;
	bool fastest;
	int started;
	bool keys[323];
	Camera cam;
	Vertex wallpos;
	Tile *laston;
	
	void Draw();
	void CreateShadow(float *lightdir);
	void ShadowVolume(float* lightdir);
	int OnFloor(Tile *i);
	
};


struct PongBall
{
	Vertex pos;
	Vertex oldpos[5];
	Vertex speed;
	int drall;	//hmm...should've called it "spin"...
	Tile * floor;
	
	void Draw();
	
};

struct HighscoreEntry
{
	time_t write_time;
	unsigned int record_time;
	char levelname[64];	
	char playername[32];
	int medal; //gold = 0, silver = 1, bronze = 2, nothing = -1 
	
};


struct Settings
{
	int bloomw;
	int bloomh;
	int keys[14];
	int glow;
	int width;
	int bloom;
	int height;
	int mipmaps;
	int sounds;
	int textures;
	int details;
	int animspeed;
	int drawdist;
	int speed;
	int shadows;
	int shadowdist;
	int windowed;
};

void PlayerModel(int dir,int v_speed,Vertex oldpos, Vertex pos, float r,float slices,int f_speed, bool ghost = false);
void InitEffects();
void DrawEffect(Effects effe);
void DrawBackGround(int texindex,int multi);

#endif

