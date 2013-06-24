/*
#				Orbit-Hopper
#				Font.cpp
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


#include "../headers/Structs.h"
#include "../headers/Utils.h"


#include <gl.h>	
#include <glu.h>

#include "SDL_image.h"
GLuint font2d;
GLuint font3d;
GLuint font_tex[1];
extern bool wireframemode;
extern Settings set;
extern bool blooming;

void printtxt2d(int x,int y,float scale, float r, float g, float b,char *text)	
{
	if(blooming)
		return;
	if(wireframemode)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
		glEnable(GL_TEXTURE_2D);
	}
	
	
	glEnable(GL_BLEND);	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glColor3f(r,g,b); //sets text color to desired value

	glBindTexture(GL_TEXTURE_2D, font_tex[0]);									
		glMatrixMode(GL_PROJECTION);						
		glPushMatrix();									
		glLoadIdentity();									
		gluOrtho2D(0,set.width,0,set.height);	//we are drawing 2D text -> use 2D viewport						
		glMatrixMode(GL_MODELVIEW);						
		glPushMatrix();									
			glLoadIdentity();						
			glTranslated(x,y,0);	//move to upper left corner of text
			glScaled(scale,scale,scale);	//scale text
			float xf,yf;
			for(int j = 0;j<(int)strlen(text);j++)
			{
				xf=(float)((text[j]-32)%16)/16.0f;	//calculate x-coordinate of character (on texture)				
				yf=(float)((text[j]-32)/16)/8.0f;	//calculate y-coordinate of character (on texture)							
					
				glBegin(GL_QUADS);							
					glTexCoord2f(xf,yf+0.0030);					glVertex2i(0,0);
					glTexCoord2f(xf,yf+0.125-0.0030);			glVertex2i(0,(int)(-0.026*set.height));												
					glTexCoord2f(xf+0.0625,yf+0.125-0.0030);	glVertex2i((int)(0.0195*set.width),(int)(-0.026*set.height));						
					glTexCoord2f(xf+0.0625,yf+0.0030);			glVertex2i((int)(0.0195*set.width),0);					
				glEnd();									
				glTranslated(0.0146*set.width,0,0);	//translate along x-axis to position of next symbol
			}								
		glMatrixMode(GL_PROJECTION);						
		glPopMatrix();										
		glMatrixMode(GL_MODELVIEW);							
		glPopMatrix();	
											
	glEnable(GL_DEPTH_TEST);						
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);

	if(wireframemode)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glDisable(GL_TEXTURE_2D);
	}				
}



int LoadFontBmp()
{
	
	SDL_Surface *TempTex[1]; 
	memset(TempTex,0,sizeof(void *)*1);   
	
	if((TempTex[0] = IMG_Load("gfx/font1.png")))
	{
		
		glGenTextures(1, &font_tex[0]);
			
		glBindTexture(GL_TEXTURE_2D, font_tex[0]);	
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, TempTex[0]->w, TempTex[0]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, TempTex[0]->pixels);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		
		SDL_FreeSurface(TempTex[0]);	
		
		glEnable(GL_TEXTURE_2D);
		glBlendFunc(GL_ONE,GL_ONE);
		return 0;	
	}
	return -1;
	

}


void printtxt3d(int eyey, int eyez, int x,int y,int z, float r, float g, float b,float scale,char *text)	
{
	if(blooming)
		return;
	//glDisable(GL_CULL_FACE);	
	if(wireframemode)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
		glEnable(GL_TEXTURE_2D);	
	}
	
	glColor3f(r,g,b);
	

	glEnable(GL_BLEND);	
	glDisable(GL_LIGHTING);
	glPushMatrix();		
		//note: translate/rotated methods are applied in reversed order				
		glTranslated(x,y,z);
			if(eyez > z)							//no need to do billboarding since game logic is 2D
				glRotated(180.0f,0.0f,1.0f,0.0f);	//rotate text around y-axis
			glBindTexture(GL_TEXTURE_2D, font_tex[0]);	
			glScaled(scale,scale,scale);	
			float xf,yf;
			for(int j = 0;j<(int)strlen(text);j++)
			{
				xf=(float)((text[j]-32)%16)/16.0f;	//calculate x-coordinate of character (on texture)							
				yf=(float)((text[j]-32)/16)/8.0f;	//calculate y-coordinate of character (on texture)							
					
				glBegin(GL_QUADS);
					glTexCoord2f(xf, yf+0.0030);					glVertex3i(0, 0,0);
					glTexCoord2f(xf, yf+0.125-0.0030);				glVertex3i(0, (int)(-0.208*set.height),0);
					glTexCoord2f(xf+0.0625,yf+0.125-0.0030);		glVertex3i((int)(-0.156*set.width), (int)(-0.208*set.height),0);
					glTexCoord2f(xf+0.0625, yf+0.0030);				glVertex3i((int)(-0.156*set.width), 0,0);
				
				glEnd();										
				glTranslated(-0.137*set.width,0,0);		//translate along x-axis to position of next symbol
			}										
	glPopMatrix();	
	glEnable(GL_LIGHTING);	
	glDisable(GL_BLEND);
	
	if(wireframemode)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glDisable(GL_TEXTURE_2D);
	}	
	//glEnable(GL_CULL_FACE);
}
