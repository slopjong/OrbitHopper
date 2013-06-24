/*
#				Geometrics.h

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
#ifndef _GEOMETRICS_H_
#define _GEOMETRICS_H_

//floor types
#define NONE			-1
#define NORMAL			0
#define SPEED_UP		1
#define SLOW_DOWN		2
#define	LEFT_PUSH		3
#define	RIGHT_PUSH		4
#define HURT			5
#define	ICE				6
#define CRAZY			7
#define HEAL			8
#define	JUMP			9
#define GOAL			10
#define SAND			11

#define LE_VIEW			12
#define LE_SAVE			13
#define LE_LOAD			14
#define SIDE_BLOCK_L	15 //blocks s_speed
#define SIDE_BLOCK_R	16



struct Vertex
{
	int x,y,z;	
};


//bitmasks
#define CA_MOVEMENT	0x00000001
#define CA_MOVE		0x00000004
#define CE_TOUCH	0x00000010
#define CE_START	0x00000020
#define CA_TYPE		0x00000002
#define CT_OTHER	0x00000008

//Macros to extract floor-actions
#define A_MOVEMENT(X)	(bool)((X) &	0x00000001)
#define A_TYPE(X)	(bool)((X) &	0x00000002)
#define A_MOVE(X)	(bool)((X) &	0x00000004)

//Macros to extract floor-extras
#define EX_POWERUP(X)		(bool)((X) &	0x80000000)
#define EX_LIFE(X)			(bool)((X) &	0x40000000)
#define EX_ENEMY(X)			(bool)((X) &	0x20000000)
#define EX_CHECKPOINT(X)	(bool)((X) &	0x10000000)
#define EX_PONG(X)			(bool)((X) &	0x08000000)


////Macros to extract whether another floor is triggered by this one
#define T_OTHER(X)	(bool)((X) & 0x00000008)

//Macros to extract floor trigger-events
#define E_TOUCH(X)	(bool)((X) & 0x00000010)
#define E_START(X)	(bool)((X) & 0x00000020)

//Macros to extract movetime
#define MOVETIME(X)		(int)((X>>6) & 0x0000001f)
#define OTHERINDEX(X)	(int)((X>>11) & 0x00001fff)
#define NEWTYPE(X)		(int)((X>>24) & 0x00000007)

class Cube
{
private:
	void ShadowVolume(float*lightdir);
public:
	Vertex origin;
	Vertex ext;
	Vertex tiled;
	int tex[2];

	int flag1;
	int flag2;
	int flag3;
	
	int triggerflag;	//32 bits: [powerup][life][enemy][checkpoint][pong][3 bits: new type<<24][13 bits: triggers<<11][5 bits: movetime<<6][start][touch][other][move][change type][none]
	
	//will not be saved in *.slv:
	int new_type;			//used in multiplayer as well
	int new_type_time;		//used in multiplayer as well
	int movetime;
	int triggered; //-1: triggered by start   0: not yet triggered by touch , 1:was triggered by touch
	float red;
	int movecount;
	
	Cube(Vertex origin, Vertex ext,Vertex tile,int tex_set);
	Cube(const Cube &c) 
	{ 
		origin = c.origin; 
		ext = c.ext; 
		tiled = c.tiled;
		
		tex[0] = c.tex[0]; 
		tex[1] = c.tex[1];
		
		flag1 = c.flag1;
		flag2 = c.flag2;
		flag3 = c.flag3;
		triggerflag = c.triggerflag;
	}
	void CreateShadow(float*lightdir);
	void Draw(bool blend,int dist = 0);

	
};



#endif

