/*
#				Orbit-Hopper
#				Geometrics.cpp : Cubes, Enemies and Checkpoints
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

#include <gl.h>	
#include <glu.h>
#include <math.h>

#include "../headers/Structs.h"
#include "../headers/Game.h"



#define SECTIONS 4

#define DETAIL_OFFSET 	50

//indices of textures of different floor-types
int tex_sets[12][2] = {{8,0},{8,1},{8,2},{8,3},{8,4},{8,5},{6,6},{8,15},{8,20},{8,21},{26,26},{8,37}};


extern GLuint **anims;
extern Settings set;
extern int animated[NUM_TEXTURES];
extern int animcount;
float M_PIH = M_PI/2.0;
const float sq2 = sqrt(2.0);
const float sq3 = sqrt(3.0);
extern bool blooming;
float *edgepts = NULL;
float *cornerpts = NULL;

void ComputePoints()
{
	edgepts = new float[(SECTIONS+1)*2];
	//edge points
	float angle = M_PIH/(float)SECTIONS;
	for(int i = 0;i<= SECTIONS;i++)
	{
		edgepts[i*2] = DETAIL_OFFSET*sin(i*angle);
		edgepts[i*2+1] = -DETAIL_OFFSET*cos(i*angle);
	}


	//corner points
	cornerpts = new float[(SECTIONS+1)*3+(SECTIONS+1)*(SECTIONS-1)*6];
	float s = DETAIL_OFFSET*sin(angle);
	float c = DETAIL_OFFSET*cos(angle);

	for(int i = 0;i<=SECTIONS;i++)
	{
		cornerpts[i*3] = -s*cos(M_PIH-i*angle);
		cornerpts[i*3+1] = c;
		cornerpts[i*3+2] = -s*sin(M_PIH-i*angle);
	}
	int count = (SECTIONS+1)*3;

	for(int u = 0;u<SECTIONS-1;u++)
	{
		float s1 = DETAIL_OFFSET*sin((u+1)*angle);
		float c1 = DETAIL_OFFSET*cos((u+1)*angle);
		float s2 = DETAIL_OFFSET*sin((u+2)*angle);
		float c2 = DETAIL_OFFSET*cos((u+2)*angle);
	 	
		for(int i = 0;i<=SECTIONS;i++)
		{
			cornerpts[count] = -s1*cos(M_PIH-i*angle);
			cornerpts[count+1] = c1;
			cornerpts[count+2] = -s1*sin(M_PIH-i*angle);
			
			cornerpts[count+3] = -s2*cos(M_PIH-i*angle);
			cornerpts[count+4] = c2;
			cornerpts[count+5] = -s2*sin(M_PIH-i*angle);

			count +=6;	
		}
		
	}
}

//superquadrics might be a better solution to do this...still computationally expensive, though
void DrawEdge(int dir1, int dir2,int dir3, Vertex start, float length,int tiles)
{
	glPushMatrix();

	
	glTranslated(start.x,start.y,start.z);
	glRotated(90.0*dir3,1,0,0);
	glRotated(90.0*dir2,0,0,1);
	glRotated(90.0*dir1,0,1,0);

	glBegin(GL_QUAD_STRIP);
		for(int i = 0;i<=SECTIONS;i++)
		{
			glNormal3f(0,edgepts[i*2],edgepts[i*2+1]);
			glTexCoord2f(0,0);
			glVertex3f(0,edgepts[i*2],edgepts[i*2+1]);

			glNormal3f(0,edgepts[i*2],edgepts[i*2+1]);
			glTexCoord2f(tiles,0);
			glVertex3f(-length,edgepts[i*2],edgepts[i*2+1]);
		}
	
	glEnd();
	glPopMatrix();
}

//another inefficient piece of code
void DrawCorner(int dir1, int dir2,int dir3, Vertex middle)
{
	glPushMatrix();
	
	glTranslated(middle.x,middle.y,middle.z);
	glRotated(90.0*dir3,1,0,0);
	glRotated(90.0*dir2,0,0,1);
	glRotated(90.0*dir1,0,1,0);

	

	glBegin(GL_TRIANGLE_FAN);
	
		glNormal3f(0,1,0);
		glTexCoord2f( 0,0);
		glVertex3f(0,DETAIL_OFFSET,0);
		for(int i = 0;i<=SECTIONS;i++)
		{
			
			glNormal3f(cornerpts[i*3],cornerpts[i*3+1],cornerpts[i*3+2]);
			glTexCoord2f(0,0);
			glVertex3f(cornerpts[i*3],cornerpts[i*3+1],cornerpts[i*3+2]);	
		}
	glEnd();
	
	int count = (SECTIONS+1)*3;
	for(int u = 0;u<SECTIONS-1;u++)
	{
	 	glBegin(GL_QUAD_STRIP);
		 	for(int i = 0;i<=SECTIONS;i++)
			{
				glNormal3f(cornerpts[count],cornerpts[count+1],cornerpts[count+2]);
				glTexCoord2f(0,0);
				glVertex3f(cornerpts[count],cornerpts[count+1],cornerpts[count+2]);	
			
				glNormal3f(cornerpts[count+3],cornerpts[count+4],cornerpts[count+5]);
				glTexCoord2f(0,0);
				glVertex3f(cornerpts[count+3],cornerpts[count+4],cornerpts[count+5]);	
				count +=6;
			}
		glEnd();
	}
	glPopMatrix();
	
}
								
Cube::Cube(Vertex origin,Vertex ext,Vertex tile,int tex_set)
{
	this->origin = origin;
	this->ext = ext;
	this->tiled = tile;
	new_type_time = 0;
	
	tex[0] = tex_sets[tex_set][0];
	tex[1] = tex_sets[tex_set][1];
	
	new_type = 0;
	flag1 = 0;
	flag2 = 0;
	flag3 = 0;
	triggerflag = 0;
	red = 0;
	movecount = 0;
	movetime = 0;
	triggered = -1;	
}



void Cube::ShadowVolume(float* lightdir)
{
	int x1 = this->origin.x;
	int x2 = this->origin.y;
	int x3 = this->origin.z;

	if(this->movetime && this->movecount)
	{
		float part = (float)this->movecount/(float)(this->movetime*50);
		
		if((int)part%2) //on its way back
			part = 1.0f-(part-(int)part);
		else	
			part -=(int)part;	
		
		x1 += (int)(part*(this->flag1-this->origin.x));
		x2 += (int)(part*(this->flag2-this->origin.y));
		x3 += (int)(part*(this->flag3-this->origin.z));
		
	}
	

	glBegin(GL_QUAD_STRIP);
		glVertex3i(x1, x2,x3+ext.z);
		glVertex3i((int)(x1+lightdir[0]*100000), (int)(x2+lightdir[1]*100000),(int)(x3+ext.z+lightdir[2]*100000));

		glVertex3i(x1, x2,x3);
		glVertex3i((int)(x1+lightdir[0]*100000), (int)(x2+lightdir[1]*100000),(int)(x3+lightdir[2]*100000));

		glVertex3i(x1-ext.x, x2, x3);
		glVertex3i((int)(x1-ext.x+lightdir[0]*100000), (int)(x2+lightdir[1]*100000), (int)(x3+lightdir[2]*100000));

		glVertex3i(x1-ext.x, x2, x3+ext.z);
		glVertex3i((int)(x1-ext.x+lightdir[0]*100000), (int)(x2+lightdir[1]*100000), (int)(x3+ext.z+lightdir[2]*100000));

		glVertex3i(x1, x2,x3+ext.z);
		glVertex3i((int)(x1+lightdir[0]*100000), (int)(x2+lightdir[1]*100000),(int)(x3+ext.z+lightdir[2]*100000));
	glEnd();
	glBegin(GL_QUADS);
		glVertex3i(x1, x2,x3+ext.z);
		glVertex3i(x1, x2,x3);
		glVertex3i(x1-ext.x, x2, x3);
		glVertex3i(x1-ext.x, x2, x3+ext.z);

		glVertex3i((int)(x1+lightdir[0]*100000), (int)(x2+lightdir[1]*100000),(int)(x3+ext.z+lightdir[2]*100000));
		glVertex3i((int)(x1+lightdir[0]*100000), (int)(x2+lightdir[1]*100000),(int)(x3+lightdir[2]*100000));
		glVertex3i((int)(x1-ext.x+lightdir[0]*100000), (int)(x2+lightdir[1]*100000), (int)(x3+lightdir[2]*100000));
		glVertex3i((int)(x1-ext.x+lightdir[0]*100000), (int)(x2+lightdir[1]*100000), (int)(x3+ext.z+lightdir[2]*100000));
	glEnd();

}

void Cube::CreateShadow(float *lightdir)
{
			
	//just render back-side of shadow-volume
	glCullFace(GL_FRONT);
	glStencilFunc(GL_ALWAYS, 0x0, 0xff);
	//increase value of stencil buffer at positions, where z-buffer fails
	glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);	
		
	ShadowVolume(lightdir);	
		
	//front
	glCullFace(GL_BACK);
	//decrease value of stencil buffer at positions, where z-buffer fails
	glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
			
	ShadowVolume(lightdir);
		
	//=> shadows have stencil-value != 0

}


//note: this might be dynamic, but it's simply slooooow
//alternative: static - vbos....should do some research about hardware-requirements..
void Cube::Draw(bool blend, int dist)
{


	int x1 = this->origin.x;
	int x2 = this->origin.y;
	int x3 = this->origin.z;
	
	int anim1 = 0;
	int anim2 = 0;
	
	//has type been changed by powerup ?
	if(this->new_type_time > 0)
	{
		if(animated[tex_sets[this->new_type][0]])
			anim1 = (animcount/set.animspeed)%animated[tex_sets[this->new_type][0]];
		if(animated[tex_sets[this->new_type][1]])
			anim2 = (animcount/set.animspeed)%animated[tex_sets[this->new_type][1]];

		glBindTexture(GL_TEXTURE_2D, anims[tex_sets[this->new_type][0]][anim1]);
	}
	else
	{
		if(animated[this->tex[0]])
			anim1 = (animcount/set.animspeed)%animated[this->tex[0]];
	
		if(animated[this->tex[1]])
			anim2 = (animcount/set.animspeed)%animated[this->tex[1]];

		glBindTexture(GL_TEXTURE_2D, anims[this->tex[0]][anim1]);	
	}
	
	//update moving floor
	if(this->movetime && this->movecount)
	{
		float part = (float)this->movecount/(float)(this->movetime*50);
		
		if((int)part%2) //on its way back
			part = 1.0f-(part-(int)part);
		else	
			part -=(int)part;	
		
		x1 += (int)(part*(this->flag1-this->origin.x));
		x2 += (int)(part*(this->flag2-this->origin.y));
		x3 += (int)(part*(this->flag3-this->origin.z));
		
	}
	
		

	glPushMatrix();
	glTranslated(x1,x2,x3);
	
	if(!set.details || dist > 50000)//draw low detail cubes
	{
		glBegin(GL_QUADS);
			
			//bottom face
			glNormal3f(0.0,-1.0,0.0);
			glTexCoord2f( 0, 0);					glVertex3i(-ext.x, 	-ext.y,  ext.z);
			glTexCoord2f( 0, this->tiled.z);			glVertex3i(-ext.x, 	-ext.y,  0);
			glTexCoord2f( this->tiled.x, this->tiled.z);		glVertex3i(0, 		-ext.y,  0);
			glTexCoord2f( this->tiled.x, 0);			glVertex3i(0, 		-ext.y,  ext.z);
		
			//left face
			glNormal3f(1.0,0.0,0.0);
			glTexCoord2f( 0, 0);					glVertex3i(0, 0,  	ext.z);
			glTexCoord2f( 0, this->tiled.y);			glVertex3i(0, -ext.y,	ext.z);
			glTexCoord2f( this->tiled.z, this->tiled.y);		glVertex3i(0, -ext.y, 	0);
			glTexCoord2f( this->tiled.z, 0);			glVertex3i(0, 0,  	0);
			
			//right face
			glNormal3f(-1.0,0.0,0.0);
			glTexCoord2f( 0, 0);					glVertex3i(-ext.x, 0,  	0);
			glTexCoord2f( 0, this->tiled.y);			glVertex3i(-ext.x, -ext.y,  0);
			glTexCoord2f( this->tiled.z, this->tiled.y);		glVertex3i(-ext.x, -ext.y,  ext.z);
			glTexCoord2f( this->tiled.z,0);				glVertex3i(-ext.x, 0,  	ext.z);
			
			//back face
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f( 0, 0);					glVertex3i(-ext.x, 0,  		ext.z);
			glTexCoord2f( 0, this->tiled.y);			glVertex3i(-ext.x, -ext.y,  	ext.z);
			glTexCoord2f( this->tiled.x,this->tiled.y);		glVertex3i(0, 	-ext.y,  	ext.z);
			glTexCoord2f( this->tiled.x, 0);			glVertex3i(0, 	0,  		ext.z);
		
			//front face
			glNormal3f(0.0,0.0,-1.0);
			glTexCoord2f( 0, 0);					glVertex3i(0, 0	,  		0);
			glTexCoord2f( 0, this->tiled.y);			glVertex3i(0, 	-ext.y,  	0);
			glTexCoord2f( this->tiled.x, this->tiled.y);		glVertex3i(-ext.x, -ext.y,  	0);
			glTexCoord2f( this->tiled.x, 0);			glVertex3i(-ext.x,0 ,  		0);
			
			
	
		glEnd();
	}
	else if(set.details == 1 || (set.details > 1 && (dist > 25000 || blend)))//draw medium detail -cubes
	{
		glBegin(GL_QUADS);
			//bottom face
			glNormal3f(0.0,-1.0,0.0);
			glTexCoord2f( 0, 0);				glVertex3i(-ext.x, 	-ext.y,  ext.z);
			glTexCoord2f( 0, this->tiled.z);		glVertex3i(-ext.x, 	-ext.y,  0);
			glTexCoord2f( this->tiled.x, this->tiled.z);	glVertex3i(0, 		-ext.y,  0);
			glTexCoord2f( this->tiled.x, 0);		glVertex3i(0, 		-ext.y,  ext.z);
			
			//left face
			//upper edge
			glNormal3f(sq2,sq2,0.0);
			glTexCoord2f( 0.1, 0);			glVertex3i(0, 			-1,  	 		ext.z);	//-2 to reduce clipping-errors
			glTexCoord2f( 0.1, 0.01);		glVertex3i(DETAIL_OFFSET, 	-DETAIL_OFFSET, 	ext.z-DETAIL_OFFSET);
			glTexCoord2f( 0.11, 0.01);		glVertex3i(DETAIL_OFFSET, 	-DETAIL_OFFSET,  	DETAIL_OFFSET);
			glTexCoord2f( 0.11, 0);			glVertex3i(0, 			-1,  			0);//-2 to reduce clipping-errors
		
			glNormal3f(1.0,0.0,0.0);
			glTexCoord2f( 0, 0);				glVertex3i(DETAIL_OFFSET, -DETAIL_OFFSET,  		ext.z-DETAIL_OFFSET);
			glTexCoord2f( 0, this->tiled.y);		glVertex3i(DETAIL_OFFSET, -ext.y+DETAIL_OFFSET, 	ext.z-DETAIL_OFFSET);
			glTexCoord2f( this->tiled.z, this->tiled.y);	glVertex3i(DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  	DETAIL_OFFSET);
			glTexCoord2f( this->tiled.z, 0);		glVertex3i(DETAIL_OFFSET, -DETAIL_OFFSET,  		DETAIL_OFFSET);
		
			//lower edge
			glNormal3f(sq2,-sq2,0.0);
			glTexCoord2f( 0.1, 0.99);	glVertex3i(DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  ext.z-DETAIL_OFFSET);
			glTexCoord2f( 0.1, 1);		glVertex3i(0, 		  -ext.y, 		 ext.z);
			glTexCoord2f( 0.11, 1);		glVertex3i(0, 		  -ext.y,  		 0);
			glTexCoord2f( 0.11, 0.99);	glVertex3i(DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  DETAIL_OFFSET);
		
			//right face
			//upper edge
			glNormal3f(-sq2,sq2,0.0);
			glTexCoord2f( 0, 0);		glVertex3i(-ext.x, 		 -1,  			0);//-2 to reduce clipping-errors
			glTexCoord2f( 0, 0.01);		glVertex3i(-ext.x-DETAIL_OFFSET, -DETAIL_OFFSET,  	DETAIL_OFFSET);
			glTexCoord2f( 0.01, 0.01);	glVertex3i(-ext.x-DETAIL_OFFSET, -DETAIL_OFFSET,  	ext.z-DETAIL_OFFSET);
			glTexCoord2f( 0.01,0);		glVertex3i(-ext.x, 		 -1,  			ext.z);//-2 to reduce clipping-errors
		
			glNormal3f(-1.0,0.0,0.0);
			glTexCoord2f( 0, 0);				glVertex3i(-ext.x-DETAIL_OFFSET, -DETAIL_OFFSET,  	DETAIL_OFFSET);
			glTexCoord2f( 0, this->tiled.y);		glVertex3i(-ext.x-DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  DETAIL_OFFSET);
			glTexCoord2f( this->tiled.z, this->tiled.y);	glVertex3i(-ext.x-DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  ext.z-DETAIL_OFFSET);
			glTexCoord2f( this->tiled.z,0);			glVertex3i(-ext.x-DETAIL_OFFSET, -DETAIL_OFFSET,  	ext.z-DETAIL_OFFSET);
			
			//lower edge
			glNormal3f(-sq2,-sq2,0.0);
			glTexCoord2f( 0.99, 0.99);	glVertex3i(-ext.x-DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  DETAIL_OFFSET);
			glTexCoord2f( 0.99, 1);		glVertex3i(-ext.x, 		 -ext.y,  		0);
			glTexCoord2f( 1, 1);		glVertex3i(-ext.x, 		 -ext.y,  		ext.z);
			glTexCoord2f( 1,0.99);		glVertex3i(-ext.x-DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  ext.z-DETAIL_OFFSET);
	
			//back face
			//upper edge
			glNormal3f(0.0,sq2,sq2);
			glTexCoord2f( 0.1, 0);		glVertex3i(-ext.x, 		 0,  			ext.z);
			glTexCoord2f( 0.1, 0.01);	glVertex3i(-ext.x+DETAIL_OFFSET, -DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0.11,0.01);	glVertex3i(-DETAIL_OFFSET, 	 -DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0.11, 0);		glVertex3i(0, 			 0,  			ext.z);
			
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f( this->tiled.x,0 );		glVertex3i(-ext.x+DETAIL_OFFSET, -DETAIL_OFFSET,  		ext.z+DETAIL_OFFSET);
			glTexCoord2f( this->tiled.x, this->tiled.y);	glVertex3i(-ext.x+DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0,this->tiled.y);			glVertex3i(-DETAIL_OFFSET, 	 -ext.y+DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0, 0);				glVertex3i(-DETAIL_OFFSET, 	 -DETAIL_OFFSET,  		ext.z+DETAIL_OFFSET);
			
			//lower edge
			glNormal3f(0.0,-sq2,sq2);
			glTexCoord2f( 0.1, 0.99);	glVertex3i(-ext.x+DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0.1, 1);		glVertex3i(-ext.x, 		 -ext.y,  		ext.z);
			glTexCoord2f( 0.11,1);		glVertex3i(0, 			 -ext.y,  		ext.z);
			glTexCoord2f( 0.11, 0.99);	glVertex3i(-DETAIL_OFFSET, 	 -ext.y+DETAIL_OFFSET,  ext.z+DETAIL_OFFSET);
			
			//right edge
			glNormal3f(-sq2,0.0,sq2);
			glTexCoord2f( 0, 0);		glVertex3i(-DETAIL_OFFSET, 	-DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
			glTexCoord2f( 1, 0);		glVertex3i(-DETAIL_OFFSET, 	-ext.y+DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
			glTexCoord2f( 1,0);		glVertex3i(DETAIL_OFFSET, 	-ext.y+DETAIL_OFFSET,  	ext.z-DETAIL_OFFSET);
			glTexCoord2f( 0, 0);		glVertex3i(DETAIL_OFFSET, 	-DETAIL_OFFSET,  	ext.z-DETAIL_OFFSET);
		
		
			//left edge	
			glNormal3f(sq2,0.0,sq2);
			glTexCoord2f( 0,0);		glVertex3i(-ext.x-DETAIL_OFFSET, 	-DETAIL_OFFSET,  	ext.z-DETAIL_OFFSET);
			glTexCoord2f( 1, 0);		glVertex3i(-ext.x-DETAIL_OFFSET, 	-ext.y+DETAIL_OFFSET,  	ext.z-DETAIL_OFFSET);
			glTexCoord2f( 1,0);		glVertex3i(-ext.x+DETAIL_OFFSET, 	-ext.y+DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0, 0);		glVertex3i(-ext.x+DETAIL_OFFSET, 	-DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
	
	
			//front face
			//upper edge
			glNormal3f(0.0,sq2,-sq2);
			glTexCoord2f( 0.1, 0);				glVertex3i(0, 		 	 0,  		 0);
			glTexCoord2f( 0.1, 0.01);			glVertex3i(-DETAIL_OFFSET, 	 -DETAIL_OFFSET, -DETAIL_OFFSET);
			glTexCoord2f( 0.11, 0.01);			glVertex3i(-ext.x+DETAIL_OFFSET, -DETAIL_OFFSET, -DETAIL_OFFSET);
			glTexCoord2f( 0.11, 0);				glVertex3i(-ext.x, 		 0,  		 0);
			
			glNormal3f(0.0,0.0,-1.0);
			glTexCoord2f( 0, 0);				glVertex3i(-DETAIL_OFFSET, 	 -DETAIL_OFFSET,  	-DETAIL_OFFSET);
			glTexCoord2f( 0, this->tiled.y);		glVertex3i(-DETAIL_OFFSET, 	 -ext.y+DETAIL_OFFSET,  -DETAIL_OFFSET);
			glTexCoord2f( this->tiled.x, this->tiled.y);	glVertex3i(-ext.x+DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  -DETAIL_OFFSET);
			glTexCoord2f( this->tiled.x, 0);		glVertex3i(-ext.x+DETAIL_OFFSET, -DETAIL_OFFSET,  	-DETAIL_OFFSET);
		
			//lower edge
			glNormal3f(0.0,-sq2,-sq2);
			glTexCoord2f( 0.1, 0.99);	glVertex3i(-DETAIL_OFFSET, 	-ext.y+DETAIL_OFFSET,  	-DETAIL_OFFSET);
			glTexCoord2f( 0.1, 1);		glVertex3i(0, 			-ext.y,  		0);
			glTexCoord2f( 0.11, 1);		glVertex3i(-ext.x, 		-ext.y,  		0);
			glTexCoord2f( 0.11, 0.99);	glVertex3i(-ext.x+DETAIL_OFFSET,-ext.y+DETAIL_OFFSET,  	-DETAIL_OFFSET);
		
			//left edge
			glNormal3f(sq2,0.0,-sq2);
			glTexCoord2f( 0, 0);		glVertex3i(DETAIL_OFFSET,  -DETAIL_OFFSET,  	 DETAIL_OFFSET);
			glTexCoord2f( 1, 0);		glVertex3i(DETAIL_OFFSET,  -ext.y+DETAIL_OFFSET, DETAIL_OFFSET);
			glTexCoord2f( 1, 0);		glVertex3i(-DETAIL_OFFSET, -ext.y+DETAIL_OFFSET, -DETAIL_OFFSET);
			glTexCoord2f( 0, 0);		glVertex3i(-DETAIL_OFFSET, -DETAIL_OFFSET,  	 -DETAIL_OFFSET);
		
			//right edge
			glNormal3f(-sq2,0.0,-sq2);
			glTexCoord2f( 0,0);		glVertex3i(-ext.x+DETAIL_OFFSET, 	-DETAIL_OFFSET,  	-DETAIL_OFFSET);
			glTexCoord2f( 1, 0);		glVertex3i(-ext.x+DETAIL_OFFSET, 	-ext.y+DETAIL_OFFSET,   -DETAIL_OFFSET);
			glTexCoord2f( 1, 0);		glVertex3i(-ext.x-DETAIL_OFFSET, 	-ext.y+DETAIL_OFFSET,   DETAIL_OFFSET);
			glTexCoord2f( 0, 0);		glVertex3i(-ext.x-DETAIL_OFFSET, 	-DETAIL_OFFSET,  	DETAIL_OFFSET);
		
		glEnd();
			
		//corners
		glBegin(GL_TRIANGLES);
			//front
			//upper-right corner
			glNormal3f(-sq3,sq3,-sq3);
			glTexCoord2f( 0.11, 0.01);	glVertex3i(-ext.x+DETAIL_OFFSET, -DETAIL_OFFSET, -DETAIL_OFFSET);
			glTexCoord2f( 0.1, 0.01);	glVertex3i(-ext.x-DETAIL_OFFSET, -DETAIL_OFFSET, DETAIL_OFFSET);
			glTexCoord2f( 0.1, 0);		glVertex3i(-ext.x, 		 0,  		 0);
			
			//upper-left corner
			glNormal3f(sq3,sq3,-sq3);
			glTexCoord2f( 0.11, 0.01);	glVertex3i(DETAIL_OFFSET,  -DETAIL_OFFSET,  DETAIL_OFFSET);
			glTexCoord2f( 0.1, 0.01);	glVertex3i(-DETAIL_OFFSET, -DETAIL_OFFSET, -DETAIL_OFFSET);
			glTexCoord2f( 0.1, 0);		glVertex3i(0, 		   0,  		   0);
			

			//lower-right corner
			glNormal3f(-sq3,-sq3,-sq3);
			glTexCoord2f( 0.1, 0.99);	glVertex3i(-ext.x, 		 -ext.y,  		0);
			glTexCoord2f( 0.1, 1);		glVertex3i(-ext.x-DETAIL_OFFSET, -ext.y+DETAIL_OFFSET, 	DETAIL_OFFSET);
			glTexCoord2f( 0.11, 1);		glVertex3i(-ext.x+DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  -DETAIL_OFFSET);
			
			//lower-left corner
			glNormal3f(sq3,sq3,-sq3);
			glTexCoord2f( 0.1, 0.99);	glVertex3i(0, 		  -ext.y,  		0);
			glTexCoord2f( 0.1, 1);		glVertex3i(-DETAIL_OFFSET,-ext.y+DETAIL_OFFSET, -DETAIL_OFFSET);
			glTexCoord2f( 0.11, 1);		glVertex3i(DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  DETAIL_OFFSET);
		
	
	
	
			//back
			//upper left corner
			glNormal3f(sq3,sq3,sq3);
			glTexCoord2f( 0.1, 0);		glVertex3i(-ext.x, 		 0,  		 ext.z);
			glTexCoord2f( 0.1, 0.01);	glVertex3i(-ext.x-DETAIL_OFFSET, -DETAIL_OFFSET, ext.z-DETAIL_OFFSET);
			glTexCoord2f( 0.11, 0.01);	glVertex3i(-ext.x+DETAIL_OFFSET, -DETAIL_OFFSET, ext.z+DETAIL_OFFSET);
		
			//upper-right corner
			glNormal3f(-sq3,sq3,sq3);
			glTexCoord2f( 0.1, 0);		glVertex3i(0, 		   0,  		    ext.z);
			glTexCoord2f( 0.1, 0.01);	glVertex3i(-DETAIL_OFFSET, -DETAIL_OFFSET,  ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0.11, 0.01);	glVertex3i(DETAIL_OFFSET,  -DETAIL_OFFSET,  ext.z-DETAIL_OFFSET);
			


			//lower-left corner
			glNormal3f(sq3,-sq3,sq3);
			glTexCoord2f( 0.11, 1);		glVertex3i(-ext.x+DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0.1, 1);		glVertex3i(-ext.x-DETAIL_OFFSET, -ext.y+DETAIL_OFFSET, 	ext.z-DETAIL_OFFSET);
			glTexCoord2f( 0.1, 0.99);	glVertex3i(-ext.x, 		 -ext.y,  		ext.z);

	
			
			//lower-right corner
			glNormal3f(-sq3,-sq3,sq3);
			glTexCoord2f( 0.1, 0.99);	glVertex3i(0, 		  -ext.y,  		 ext.z);
			glTexCoord2f( 0.11, 1);		glVertex3i(DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  ext.z-DETAIL_OFFSET);
			glTexCoord2f( 0.1, 1);		glVertex3i(-DETAIL_OFFSET,-ext.y+DETAIL_OFFSET,  ext.z+DETAIL_OFFSET);	
		
		glEnd();
				
		
	}
	else//High detail cubes - terribly slow
	{
		glBegin(GL_QUADS);
			//bottom face
			glNormal3f(0.0,-1.0,0.0);
			glTexCoord2f( 0, 0);					glVertex3i(-ext.x, 	-ext.y,  ext.z);
			glTexCoord2f( 0, this->tiled.z);			glVertex3i(-ext.x, 	-ext.y,  0);
			glTexCoord2f( this->tiled.x, this->tiled.z);		glVertex3i(0, 		-ext.y,  0);
			glTexCoord2f( this->tiled.x, 0);			glVertex3i(0, 		-ext.y,  ext.z);
			
			//left
			glNormal3f(1.0,0.0,0.0);
			glTexCoord2f( 0, 0);					glVertex3i(DETAIL_OFFSET, -DETAIL_OFFSET,  	ext.z);
			glTexCoord2f( 0, this->tiled.y);			glVertex3i(DETAIL_OFFSET, -ext.y+DETAIL_OFFSET, ext.z);
			glTexCoord2f( this->tiled.z, this->tiled.y);		glVertex3i(DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  0);
			glTexCoord2f( this->tiled.z, 0);			glVertex3i(DETAIL_OFFSET, -DETAIL_OFFSET,  	0);
		
		
			//right
			glNormal3f(-1.0,0.0,0.0);
			glTexCoord2f( 0, 0);					glVertex3i(-ext.x-DETAIL_OFFSET, -DETAIL_OFFSET,  	0);
			glTexCoord2f( 0, this->tiled.y);			glVertex3i(-ext.x-DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  0);
			glTexCoord2f( this->tiled.z, this->tiled.y);		glVertex3i(-ext.x-DETAIL_OFFSET, -ext.y+DETAIL_OFFSET,  ext.z);
			glTexCoord2f( this->tiled.z,0);				glVertex3i(-ext.x-DETAIL_OFFSET, -DETAIL_OFFSET,  	ext.z);
			
			//back
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f( this->tiled.x,0 );			glVertex3i(-ext.x, 	-DETAIL_OFFSET,  		ext.z+DETAIL_OFFSET);
			glTexCoord2f( this->tiled.x, this->tiled.y);		glVertex3i(-ext.x, 	-ext.y+DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0,this->tiled.y);				glVertex3i(0, 		-ext.y+DETAIL_OFFSET,  	ext.z+DETAIL_OFFSET);
			glTexCoord2f( 0, 0);					glVertex3i(0, 		-DETAIL_OFFSET,  		ext.z+DETAIL_OFFSET);
			
			//front
			glNormal3f(0.0,0.0,-1.0);
			glTexCoord2f( 0, 0);					glVertex3i(0, 	-DETAIL_OFFSET,  		-DETAIL_OFFSET);
			glTexCoord2f( 0, this->tiled.y);			glVertex3i(0, 	-ext.y+DETAIL_OFFSET,  -DETAIL_OFFSET);
			glTexCoord2f( this->tiled.x, this->tiled.y);		glVertex3i(-ext.x, -ext.y+DETAIL_OFFSET,  -DETAIL_OFFSET);
			glTexCoord2f( this->tiled.x, 0);			glVertex3i(-ext.x, -DETAIL_OFFSET,  		-DETAIL_OFFSET);
		glEnd();
		
		{
			//upper front edge
			Vertex left = {0, -DETAIL_OFFSET,  0};
			DrawEdge(0,0,0,left,ext.x,this->tiled.x);
		}
		
		{
			//lower front edge
			Vertex left = {0,  -ext.y+DETAIL_OFFSET,  0};
			DrawEdge(0,0,-1,left,ext.x,this->tiled.x);
		}
		{
			//upper back edge
			Vertex left = {0, -DETAIL_OFFSET,  ext.z};
			DrawEdge(0,0,1,left,ext.x,this->tiled.x);
		}
		{
			//lower back edge
			Vertex left = {0, -ext.y+DETAIL_OFFSET,  ext.z};
			DrawEdge(0,0,2,left,ext.x,this->tiled.x);
		}
		
		{
			//upper right edge
			Vertex left = {-ext.x, -DETAIL_OFFSET-1,  0};
			DrawEdge(1,0,0,left,ext.z,this->tiled.z);
		}
		{
			//lower right edge
			Vertex left = {-ext.x, -ext.y+DETAIL_OFFSET,  0};
			DrawEdge(1,1,0,left,ext.z,this->tiled.z);
		}
		
		{
			//upper left edge
			
			Vertex left = {0, -DETAIL_OFFSET-1,  ext.z};
			DrawEdge(-1,0,0,left,ext.z,this->tiled.z);
		}
		{
			//lower right edge
			
			Vertex left = {0, -ext.y+DETAIL_OFFSET,  ext.z};
			DrawEdge(-1,-1,0,left,ext.z,this->tiled.z);
		}
		
		{
			//front right edge
			Vertex left = {-ext.x, -DETAIL_OFFSET,  0};
			DrawEdge(0,1,0,left,ext.y-2*DETAIL_OFFSET,this->tiled.y);
		}
		
		{
			//front left edge
			Vertex left = {0, -ext.y+DETAIL_OFFSET,   0};
			DrawEdge(0,-1,0,left,ext.y-2*DETAIL_OFFSET,this->tiled.y);
		}
		
		{
			//back left edge
			Vertex left = {0, -DETAIL_OFFSET,  ext.z};
			DrawEdge(2,-1,0,left,ext.y-2*DETAIL_OFFSET,this->tiled.y);
		}
		
		{
			//back right edge
			Vertex left = {-ext.x, -ext.y+DETAIL_OFFSET,  ext.z};
			DrawEdge(2,1,0,left,ext.y-2*DETAIL_OFFSET,this->tiled.y);
		}
		
		{
			//upper right front 
			Vertex middle = {-ext.x, -DETAIL_OFFSET,  0};
			DrawCorner(0,0,0,middle);
		}
		{
			//lower right front
			Vertex middle = {-ext.x, -ext.y+DETAIL_OFFSET,  0};
			DrawCorner(0,1,0,middle);
		}
		
		{
			//upper left front
			Vertex middle = {0, -DETAIL_OFFSET,  0};
			DrawCorner(-1,0,0,middle);
		}
		
		{
			//lower left front
			Vertex middle = {0, -ext.y+DETAIL_OFFSET,  0};
			DrawCorner(0,2,0,middle);
		}
		
		{
			//lower left back
			Vertex middle = {0, -ext.y+DETAIL_OFFSET,  ext.z};
			DrawCorner(1,2,0,middle);
		}
		
		{
			//upper left back
			Vertex middle = {0, -DETAIL_OFFSET,  ext.z};
			DrawCorner(2,0,0,middle);
		}
		
		{
			//upper right back
			Vertex middle = {-ext.x, -DETAIL_OFFSET,  ext.z};
			DrawCorner(1,0,0,middle);
		}
		
		{
			//lower right back
			Vertex middle = {-ext.x, -ext.y+DETAIL_OFFSET,  ext.z};
			DrawCorner(-2,2,0,middle);
		}
		
		
	}
	
	
	
	if(this->new_type_time <= 0)
		glBindTexture(GL_TEXTURE_2D, anims[this->tex[1]][anim2]);
	else
		glBindTexture(GL_TEXTURE_2D, anims[tex_sets[this->new_type][1]][anim2]);
	
	GLdouble params[4];
	glGetDoublev(GL_CURRENT_COLOR,params); //save current color-settings
	
	glBegin(GL_QUADS);
		if(!blend && this->red)
			glColor3f(1.0f,fabs(this->red-1.0)+0.45,fabs(this->red-1.0)+0.45); //change color to achieve pulse-effect
		
		// top face
		glNormal3f(0.0f,1.0f,0.0f);
		glTexCoord2f( 0,0);				glVertex3i(0, 		0, 	ext.z);
		glTexCoord2f( 0, this->tiled.z);		glVertex3i(0, 		0,  	0);
		glTexCoord2f( this->tiled.x, this->tiled.z);	glVertex3i(-ext.x, 	0,  	0);
		glTexCoord2f( this->tiled.x, 0);		glVertex3i(-ext.x, 	0,  	ext.z);
		
		if(!blend)
			glColor3f(params[0],params[1],params[2]);//restore color-settings
	glEnd();

	glPopMatrix();
}

//draws GUI-rectangles
void DrawRectangle(int x, int y, int extx, int exty, float r, float g, float b)
{
	
	glColor3f(r,g,b);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);		
	glDisable(GL_DEPTH_TEST);											
	glBindTexture(GL_TEXTURE_2D,anims[27][0]);	
	
	glMatrixMode(GL_PROJECTION);								
	glPushMatrix();												
	glLoadIdentity();											
	gluOrtho2D( 0, set.width ,0, set.height);							
	glMatrixMode(GL_MODELVIEW);									
	glPushMatrix();												
		glLoadIdentity();	
		
		glBegin(GL_QUADS);
			//upper-left corner
			glTexCoord2f(0.25,1);	glVertex2i(x+16, y-16);
			glTexCoord2f(0.25,0);	glVertex2i(x+16, y);
			glTexCoord2f(0,0);		glVertex2i(x, y);
			glTexCoord2f(0,1);		glVertex2i(x, y-16);
			
			//upper edge
			glTexCoord2f(0.74,1);	glVertex2i(x+extx-16, y-16);
			glTexCoord2f(0.74,0);	glVertex2i(x+extx-16, y);
			glTexCoord2f(0.51,0);	glVertex2i(x+16, y);
			glTexCoord2f(0.51,1);	glVertex2i(x+16, y-16);
			
			//upper-right corner
			glTexCoord2f(-1,1);		glVertex2i(x+extx, y-16);
			glTexCoord2f(-1,0);		glVertex2i(x+extx, y);
			glTexCoord2f(-0.75,0);	glVertex2i(x+extx-16, y);
			glTexCoord2f(-0.75,1);	glVertex2i(x+extx-16, y-16);
			
			
			//right edge
			glTexCoord2f(0.25,1);	glVertex2i(x+extx, y-exty+16);
			glTexCoord2f(0.25,0);	glVertex2i(x+extx, y-16);
			glTexCoord2f(0.34,0);	glVertex2i(x+extx-7, y-16);
			glTexCoord2f(0.34,1);	glVertex2i(x+extx-7, y-exty+16);
			
			//left edge
			glTexCoord2f(0.34,1);	glVertex2i(x+7, y-exty+16);
			glTexCoord2f(0.34,0);	glVertex2i(x+7, y-16);
			glTexCoord2f(0.25,0);	glVertex2i(x, y-16);
			glTexCoord2f(0.25,1);	glVertex2i(x, y-exty+16);
			
			//lower-right corner
			glTexCoord2f(-1,-1);		glVertex2i(x+extx, y-exty);
			glTexCoord2f(-1,0);			glVertex2i(x+extx, y-exty+16);
			glTexCoord2f(-0.75,0);		glVertex2i(x+extx-16, y-exty+16);
			glTexCoord2f(-0.75,-1);		glVertex2i(x+extx-16, y-exty);	
			
			//lowwer edgte
			glTexCoord2f(0.74,-1);	glVertex2i(x+extx-16, y-exty);
			glTexCoord2f(0.74,0);	glVertex2i(x+extx-16, y-exty+16);
			glTexCoord2f(0.51,0);	glVertex2i(x+16, y-exty+16);
			glTexCoord2f(0.51,-1);	glVertex2i(x+16, y-exty);
			
			//lower-left corner
			glTexCoord2f(0.25,-1);	glVertex2i(x+16, y-exty);
			glTexCoord2f(0.25,0);	glVertex2i(x+16, y-exty+16);
			glTexCoord2f(0,0);		glVertex2i(x, y-exty+16);
			glTexCoord2f(0,-1);		glVertex2i(x, y-exty);		
											
		glEnd();	
		
		glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode( GL_PROJECTION );								
	glPopMatrix();												
	glMatrixMode( GL_MODELVIEW );								
	glPopMatrix();	
	
	glEnable(GL_DEPTH_TEST);
		
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glColor3f(1.0f,1.0f,1.0f);
	
}

//yellow powerup-star
void PowerUp(int angle,Vertex pos, float r, float g, float b)
{
	//glEnable(GL_BLEND);
	
	glTranslated(pos.x,pos.y,pos.z);
	glRotated(angle,0,1,0);
	glDisable(GL_LIGHTING);	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLES);
		
		// :-/ ..still faster than computing sin and cos over and over again
		glColor4f(r,g,b,0.5);
		//front
		//top
		glVertex3i(0,0+500,0);
		glVertex3i(0+60,0+360,0);
		glVertex3i(0,0+300,0+40);
		
		
		glVertex3i(0,0+500,0);
		glVertex3i(0-60,0+360,0);
		glVertex3i(0,0+300,0+40);
		
			//right
		glVertex3i(0,0+300,0+40);
		glVertex3i(0+60,0+360,0);
		glVertex3i(0+240,0+420,0);
		
		
		glVertex3i(0,0+300,0+40);
		glVertex3i(0+140,0+300,0);
		glVertex3i(0+230,0+420,0);
		
			//left
		glVertex3i(0,0+300,0+40);
		glVertex3i(0-60,0+360,0);
		glVertex3i(0-230,0+420,0);
		
		
		glVertex3i(0,0+300,0+40);
		glVertex3i(0-140,0+300,0);
		glVertex3i(0-230,0+420,0);
		
		//bottom
			//left
		glVertex3i(0,0+300,0+40);
		glVertex3i(0-60,0+200,0);
		glVertex3i(0-230,0+180,0);
		
		
		glVertex3i(0,0+300,0+40);
		glVertex3i(0-140,0+300,0);
		glVertex3i(0-230,0+180,0);
		
			//right
		glVertex3i(0,0+300,0+40);
		glVertex3i(0+60,0+200,0);
		glVertex3i(0+230,0+180,0);
		
		
		glVertex3i(0,0+300,0+40);
		glVertex3i(0+140,0+300,0);
		glVertex3i(0+230,0+180,0);
		
		//bottom
		glVertex3i(0,0+300,0+40);
		glVertex3i(0+60,0+200,0);
		glVertex3i(0,0+80,0);
		
		
		glVertex3i(0,0+300,0+40);
		glVertex3i(0-60,0+200,0);
		glVertex3i(0,0+80,0);
		
		
		//back
		//top
		glVertex3i(0,0+500,0);
		glVertex3i(0+60,0+360,0);
		glVertex3i(0,0+300,0-40);
		
		
		glVertex3i(0,0+500,0);
		glVertex3i(0-60,0+360,0);
		glVertex3i(0,0+300,0-40);
		
			//right
		glVertex3i(0,0+300,0-40);
		glVertex3i(0+60,0+360,0);
		glVertex3i(0+230,0+420,0);
		
		
		glVertex3i(0,0+300,0-40);
		glVertex3i(0+140,0+300,0);
		glVertex3i(0+230,0+420,0);
		
			//left
		glVertex3i(0,0+300,0-40);
		glVertex3i(0-60,0+360,0);
		glVertex3i(0-230,0+420,0);
		
		
		glVertex3i(0,0+300,0-40);
		glVertex3i(0-140,0+300,0);
		glVertex3i(0-230,0+420,0);
		
		//bottom
			//left
		glVertex3i(0,0+300,0-40);
		glVertex3i(0-60,0+200,0);
		glVertex3i(0-230,0+180,0);
		
		
		glVertex3i(0,0+300,0-40);
		glVertex3i(0-140,0+300,0);
		glVertex3i(0-230,0+180,0);
		
			//right
		glVertex3i(0,0+300,0-40);
		glVertex3i(0+60,0+200,0);
		glVertex3i(0+230,0+180,0);
		
		
		glVertex3i(0,0+300,0-40);
		glVertex3i(0+140,0+300,0);
		glVertex3i(0+230,0+180,0);
		
		//bottom
		glVertex3i(0,0+300,0-40);
		glVertex3i(0+60,0+200,0);
		glVertex3i(0,0+80,0);
		
		
		glVertex3i(0,0+300,0-40);
		glVertex3i(0-60,0+200,0);
		glVertex3i(0,0+80,0);
		glColor3f(1.0f,1.0f,1.0f);
		
	glEnd();
	
	//glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);	
	glRotated(-angle,0,1,0);
	glTranslated(-pos.x,-pos.y,-pos.z);
	glColor4f(1.0,1.0,1.0,1.0);
}


void DrawPowerUp(int x,int y,int type)//GUI
{
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);			
	glDisable(GL_DEPTH_TEST);											
	glBindTexture(GL_TEXTURE_2D,anims[29][0]);
	

	glMatrixMode(GL_PROJECTION);								
	glPushMatrix();												
	glLoadIdentity();											
	gluOrtho2D( 0, set.width ,0, set.height);							
	glMatrixMode(GL_MODELVIEW);									
	glPushMatrix();												
		glLoadIdentity();	
		
		glBegin(GL_QUADS);
	
			glTexCoord2f(0.25*(type%4+1),(type/4)*0.5+0.5);		glVertex2i((GLint)(x+((float)set.width)*0.04), (GLint)(y-((float)set.height)*0.075));
			glTexCoord2f(0.25*(type%4+1),(type/4)*0.5);		glVertex2i((GLint)(x+((float)set.width)*0.04), (GLint)(y));
			glTexCoord2f(0.25*(type%4),(type/4)*0.5);		glVertex2i((GLint)(x), (GLint)(y));
			glTexCoord2f(0.25*(type%4),(type/4)*0.5+0.5);		glVertex2i((GLint)(x), (GLint)(y-((float)set.height)*0.075));	
											
		glEnd();	
		
		glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode( GL_PROJECTION );								
	glPopMatrix();												
	glMatrixMode( GL_MODELVIEW );								
	glPopMatrix();	
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
			
	glDisable(GL_BLEND);
	
	
}

void DrawCheckPoint(int xleft,int xright,int y,int z, int playerz)
{
	glColor3f(1.0,1.0,1.0);
	int anim = 0;
	if(animated[33])
		anim = (animcount/set.animspeed)%animated[33];
	glBindTexture(GL_TEXTURE_2D, anims[33][anim]);
	
	glColor3f(1.0f,1.0f,1.0f);
	int length = (int)(abs(xleft-xright)/2.0);
	
	//calculate angle based on z-distance b/w player and checkpoint
	float angle = 0;
	if(playerz -8000 < z && playerz+4000 > z)
		angle = 90.0 - (90.0*(float)(z-playerz))/4000.0;
	if(angle > 90.0)
		angle = 90.0;
		
	glTranslated(xleft,y,z);		
	glRotated(angle,0.0f,1.0f,0.0f);
	
	glBegin(GL_QUADS);
	
		glNormal3f(1,0,0);
		glTexCoord2f(0,1); 	glVertex3i(0,0,0);
		glTexCoord2f(0,0.5); 	glVertex3i(0,500,0);
		glTexCoord2f(-0.5,0.5); glVertex3i(0,500,600);
		glTexCoord2f(-0.5,1); 	glVertex3i(0,0,600);
		

		glNormal3f(0,0,-1);
		glTexCoord2f(-0.5,1); 			glVertex3i(0,0,600);
		glTexCoord2f(-0.5,0.5); 		glVertex3i(0,500,600);
		glTexCoord2f(-0.5-length/1200,0.5); 	glVertex3i(-length,500,600);
		glTexCoord2f(-0.5-length/1200,1); 	glVertex3i(-length,0,600);
		

		glNormal3f(-1,0,0);
		glTexCoord2f(-0.5-length/1200,1); 	glVertex3i(-length,0,600);
		glTexCoord2f(-0.5-length/1200,0.5); 	glVertex3i(-length,500,600);
		glTexCoord2f(-1-length/1200,0.5); 	glVertex3i(-length,500,0);
		glTexCoord2f(-1-length/1200,1); 	glVertex3i(-length,0,0);
		

		glNormal3f(0,0,-1);
		glTexCoord2f(-1-length/1200,1); 	glVertex3i(-length,0,0);
		glTexCoord2f(-1-length/1200,0.5); 	glVertex3i(-length,500,0);
		glTexCoord2f(-1-2*length/1200,0.5); 	glVertex3i(0,500,0);
		glTexCoord2f(-1-2*length/1200,1); 	glVertex3i(0,0,0);
		glNormal3f(0,1,0);
		glTexCoord2f(length/1200,0); 	glVertex3i(-length,500,0);
		glTexCoord2f(length/1200,0.5); 	glVertex3i(-length,500,600);
		glTexCoord2f(0,0.5); 		glVertex3i(0,500,600);
		glTexCoord2f(0,0); 		glVertex3i(0,500,0);
	
	glEnd();
	
	glRotated(-angle,0.0f,1.0f,0.0f);
	glTranslated(-xleft,-y,-z);
	
	
	glTranslated(xright,y,z);		
	glRotated(-angle,0.0f,1.0f,0.0f);
	
	glBegin(GL_QUADS);
		glNormal3f(0,0,-1);
		glTexCoord2f(1+2*length/1200,1); 	glVertex3i(0,0,0);
		glTexCoord2f(1+2*length/1200,0.5); 	glVertex3i(0,500,0);
		glTexCoord2f(1+length/1200,0.5); 	glVertex3i(length,500,0);
		glTexCoord2f(1+length/1200,1); 		glVertex3i(length,0,0);
		
		glNormal3f(-1,0,0);
		glTexCoord2f(1+length/1200,1); 		glVertex3i(length,0,0);
		glTexCoord2f(1+length/1200,0.5); 	glVertex3i(length,500,0);
		glTexCoord2f(0.5+length/1200,0.5); 	glVertex3i(length,500,600);
		glTexCoord2f(0.5+length/1200,1); 	glVertex3i(length,0,600);
		
		glNormal3f(0,0,1);
		glTexCoord2f(0.5+length/1200,1); 	glVertex3i(length,0,600);
		glTexCoord2f(0.5+length/1200,0.5); 	glVertex3i(length,500,600);
		glTexCoord2f(0.5,0.5); 			glVertex3i(0,500,600);
		glTexCoord2f(0.5,1); 			glVertex3i(0,0,600);
		
		glNormal3f(1,0,0);
		glTexCoord2f(0.5,1); 	glVertex3i(0,0,600);
		glTexCoord2f(0.5,0.5); 	glVertex3i(0,500,600);
		glTexCoord2f(0,0.5); 	glVertex3i(0,500,0);
		glTexCoord2f(0,1); 	glVertex3i(0,0,0);
		
		glNormal3f(0,1,0);
		glTexCoord2f(0,0); 		glVertex3i(0,500,0);
		glTexCoord2f(0,0.5); 		glVertex3i(0,500,600);
		glTexCoord2f(length/1200,0.5); 	glVertex3i(length,500,600);
		glTexCoord2f(length/1200,0); 	glVertex3i(length,500,0);	
	
	glEnd();
	
	glRotated(angle,0.0f,1.0f,0.0f);
	glTranslated(-xright,-y,-z);
	
}


void DrawPongEnemy(int x,int y,int z)
{
	Vertex abc = {x,y+200,z};
	PlayerModel(-2,0,abc, abc, 0.0,16,0);
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glColor4f(0.0f,0.0f,1.0f,0.0);
	glBegin(GL_QUADS);
		//pong-wall
		glVertex3i(x-450,y,z-250);
		glVertex3i(x-450,y+100,z-250);
	
		glVertex3i(x+450,y+100,z-250);
		glVertex3i(x+450,y,z-250);
		
	
	glEnd();
	
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	
}
