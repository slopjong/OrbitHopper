/*
#				Orbit-Hopper
#				Sound.cpp
#
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
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


#include "../headers/Sound.h"
#include "../headers/Structs.h"


#include "SDL.h"
#include "SDL_mixer.h"

#define NUM_SOUNDS 15

Mix_Chunk *sounds[NUM_SOUNDS];
extern Settings set;


Mix_Chunk *Load(char * filename)
{
  Mix_Chunk *snd;
  
  if (!(snd = Mix_LoadWAV(filename)))
  {
    SDL_Quit();
    return NULL;
  }
  
  return(snd);
}

void play(int channel,int sound)
{
	if(!set.sounds)
		return;
	if(channel != -1)
	{
		if(Mix_Playing(channel))
			return;
	}
					
	if(sound < NUM_SOUNDS && sound >= 0)
		Mix_PlayChannel(channel,sounds[sound],0);	
}

int InitSound()
{
 	if (Mix_OpenAudio(22050, AUDIO_S16,2, 1024) < 0)
  	{
  		fprintf(stderr, "Could not initialize sound: %s\n", SDL_GetError());
    	SDL_Quit();
    	return -1;
  	}	
	
	//load sounds
  	if(!(sounds[S_EXPLODE] = Load("sounds/explode.wav"))
  	|| !(sounds[S_JUMP] = Load("sounds/jump.wav"))
  	|| !(sounds[S_MENU] = Load("sounds/menu.wav"))
  	|| !(sounds[S_BUMP] = Load("sounds/bump.wav"))
  	|| !(sounds[S_SELECT] = Load("sounds/select.wav"))
  	|| !(sounds[S_HURT] = Load("sounds/hurt.wav"))
  	|| !(sounds[S_HEAL] = Load("sounds/heal.wav"))
  	|| !(sounds[S_SPEEDUP] = Load("sounds/speedup.wav"))
  	|| !(sounds[S_SLOWDOWN] = Load("sounds/slowdown.wav"))
  	|| !(sounds[S_POWERUP] = Load("sounds/powerup.wav"))
  	|| !(sounds[S_PUSH] = Load("sounds/push.wav"))
  	|| !(sounds[S_PLACE] = Load("sounds/place.wav"))
  	|| !(sounds[S_PONG] = Load("sounds/pong1.wav"))
  	|| !(sounds[S_CHECK] = Load("sounds/checkpoint.wav"))
  	|| !(sounds[S_LIFE] = Load("sounds/life.wav")))
  	{
  		fprintf(stderr, "One or more sound files not found.\n");
    	return -1;
  	}	
  		
	return 0;
}



