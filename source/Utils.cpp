/*
#				Orbit-Hopper
#				Utils.cpp
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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "../headers/Structs.h"


int randomi(int max1,int min1,int min2,int max2)
{
	
	int random = rand()%((max2-max1)+1) + max1;
	if(random > min1 && random < min2)
		random = randomi(max1,min1,min2,max2);
	return random;

}


void DebugOut(float num)
{
	FILE *out = fopen("debug.txt","a");
	
	//Write debugmessage (float) to debug-file
	fprintf(out,"%f\n",num);
	fclose(out);

}

void DebugOut(unsigned int time)
{
	static unsigned int last = 0;
	
	FILE *out = fopen("debug.txt","a");
	fprintf(out,"ms: %d\n",time-last);
	fclose(out);
	last = time;
	
}

void DebugOut (char *num)
{
	//Write debugmessage (String) to debug-file
	FILE *msgfile = fopen ("debug.txt","a");

	fprintf (msgfile,"%s\n",num); 
	fclose (msgfile);
} 



double Length(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2));	
}


