/*
#				LevelEd.h

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
#ifndef _LEVELED_H_
#define _LEVELED_H_



#include "../headers/Structs.h"
#include "../headers/Gamestate.h"


//dialogboxes
#define NON		0
#define SAVE	1
#define LOAD	2
#define LIFE	3	
#define ENEMY	4
#define PONG	5
#define MOVE	6
#define TIMES	7

class LevelEd : Gamestate
{
	
private:
	char filename[32];
	char **files;
	int file_num;
	int file_first;
	int file_sel;
	bool controldown;
	int selectdifficulty;
	int seconds;
	bool view;
	Camera camera2;
	Vertex lookat;
	int cur_tilesel;
	int num_tiles;
	int cur_floor;
	Tile **floors;
	bool lmousebuttondown;
	bool drag;
	Vertex start_resize;
	bool dirs[4];
	int dialog;
	char subdir[32];
	int time1;
	int time2;
	int time3;
	int time_sel;
	int digit;
	//saves properties of trigger-dialog
	int sel[3];
	int triggerline;
	int triggercol;
	int ntype;
	int other;
	bool movement;
	char **types;
	
	void GetFileList();
	void InitCamera();
	void RenderGUI();
	void LoadLevel(char* filen);
	void SaveLevel(char* filen);
	void LeMouseClick(SDL_MouseButtonEvent click);
	void LeMouseRelease(SDL_MouseButtonEvent click);
	void LeMouseMove(SDL_MouseMotionEvent move);
	
public:
	LevelEd();
	~LevelEd();
	void Init();
	int Input(SDL_Event event, SDL_keysym key);
	void Update(unsigned int diff);
	void Render();	
};



#endif

