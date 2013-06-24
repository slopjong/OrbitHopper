/*
#				Orbit-Hopper
#				Structs.cpp
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


#include "../headers/Structs.h"
#include "../headers/Game.h"
#include "../headers/Utils.h"
#include "../headers/Sound.h"


#include "math.h"

extern int cur_ghostdir;
extern GLuint **anims;
extern bool wireframemode;
GLuint effects;
extern Settings set;
extern Gamestate *cur_gamestate;
extern Gamestate *gamestates[15];
extern int animcount;
extern bool animated[NUM_TEXTURES];
extern bool blooming;

Tile::~Tile()
{
	if(cube)
		delete cube;
	cube = NULL;
}
#ifndef WIN32	
float min(float a, float b)
{
	return a<b ? a:b;
}
#endif

void DrawCircle(int x, int y, int z, int slices, float rl,float gl, float bl, float scale, float alpha)
{
	double angle = (2.0*M_PI)/slices;
		
	double radiuso = 150*scale;
	double radiusm = 200*scale;
	double radiusu = 150*scale;
	double radiusi = 140*scale;
	radiuso = radiuso > 230 ?  230 : radiuso;
	radiusm = radiusm > 280 ?  280 : radiusm;
	radiusu = radiusu > 230 ?  230 : radiusu;
	radiusi = radiusi > 220 ?  220 : radiusi;
		

	glBegin(GL_QUAD_STRIP);
		//upper outer ring
		glColor4f(rl+0.6f,gl+0.6f,bl+0.6f,alpha);
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiuso),80,(int)(cos(k)*radiuso));
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.0); 	glVertex3i(x+(int)(sin(k)*radiuso),y-80,z+(int)(cos(k)*radiuso));
			glNormal3f((int)(sin(k)*radiusm),100,(int)(cos(k)*radiusm));
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.16); 	glVertex3i(x+(int)(sin(k)*radiusm),y-100,z+(int)(cos(k)*radiusm));
		}
		
		//middle outer ring
		glColor4f(rl+0.5f,gl+0.5f,bl+0.5f,alpha);
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiusm),100,(int)(cos(k)*radiusm));
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.16); 	 glVertex3i(x+(int)(sin(k)*radiusm),y-100,z+(int)(cos(k)*radiusm));
			glNormal3f((int)(sin(k)*radiusm),130,(int)(cos(k)*radiusm));
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.33); 	 glVertex3i(x+(int)(sin(k)*radiusm),y-130,z+(int)(cos(k)*radiusm));
		}
		
		//lower outer ring
		glColor4f(rl+0.4f,gl+0.4f,bl+0.4f,alpha);
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiusm),130,(int)(cos(k)*radiusm));
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.33); 	glVertex3i(x+(int)(sin(k)*radiusm),y-130,z+(int)(cos(k)*radiusm));
			glNormal3f((int)(sin(k)*radiusu),150,(int)(cos(k)*radiusu));
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.5);  	glVertex3i(x+(int)(sin(k)*radiusu),y-150,z+(int)(cos(k)*radiusu));
		}
		
		glColor4f(rl+0.3f,gl+0.3f,bl+0.3f,alpha);
		//upper inner ring
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiusi),115,(int)(cos(k)*radiusi));
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.25); glVertex3i(x+(int)(sin(k)*radiusi),y-115,z+(int)(cos(k)*radiusi));
			glNormal3f((int)(sin(k)*radiuso),80,(int)(cos(k)*radiuso));
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.0);  glVertex3i(x+(int)(sin(k)*radiuso),y-80,z+(int)(cos(k)*radiuso));
		}
		
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiusu),150,(int)(cos(k)*radiusu));
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.5);	glVertex3i(x+(int)(sin(k)*radiusu),y-150,z+(int)(cos(k)*radiusu));
			glNormal3f((int)(sin(k)*radiusi),115,(int)(cos(k)*radiusi));	
			glTexCoord2f(4.0/((slices+1)*angle)*k,0.25);	glVertex3i(x+(int)(sin(k)*radiusi),y-115,z+(int)(cos(k)*radiusi));	
		}

	glEnd();
	
}


void PlayerModel(int dir,int v_speed,Vertex oldpos, Vertex pos, float r,float slices,int f_speed, bool ghost)
{
	//no vbos, since my cr**** notebook can't handle it
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;
	bool arrow = true;
	int anim = 0;
	if(animated[7])
		anim = (animcount/set.animspeed)%animated[7];
		
	if(dir > 1)
	{
		dir = 1;
		arrow = false;
	}
	else if(dir < -1)
	{
		dir = -1;
		arrow = false;	
	}
	if(ghost)
		arrow = false;
	glTranslated(x,y,z);
	if(dir < 0)
		glRotated(180,0,1,0);
		

	

	glBindTexture(GL_TEXTURE_2D, anims[7][anim]);
	double angle = (2.0*M_PI)/slices;
		
	
	int radiuso = 150-v_speed/2;
	int radiusm = 200-v_speed/2;
	int radiusu = 150-v_speed/2;
	int radiusi = 140-v_speed/2;
	radiuso = radiuso > 230 ?  230 : radiuso;
	radiusm = radiusm > 280 ?  280 : radiusm;
	radiusu = radiusu > 230 ?  230 : radiusu;
	radiusi = radiusi > 220 ?  220 : radiusi;
		
	
	glBegin(GL_QUAD_STRIP);
	
		
		//upper outer ring
		glColor4f(r+0.6f,0.6f,0.6f,1.0-ghost);
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiuso),-80,(int)(cos(k)*radiuso));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.01); 	glVertex3i((int)(sin(k)*radiuso),-80,(int)(cos(k)*radiuso));
			glNormal3f((int)(sin(k)*radiusm),-80,(int)(cos(k)*radiusm));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.20); 	glVertex3i((int)(sin(k)*radiusm),-100,(int)(cos(k)*radiusm));
		}
		
		
		//middle outer ring
		glColor4f(r+0.5f,0.5f,0.5f,1.0-ghost);
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiusm),-100,(int)(cos(k)*radiusm));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.20); 	 glVertex3i((int)(sin(k)*radiusm),-100,(int)(cos(k)*radiusm));
			glNormal3f((int)(sin(k)*radiusm),-130,(int)(cos(k)*radiusm));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.3); 	 glVertex3i((int)(sin(k)*radiusm),-130,(int)(cos(k)*radiusm));
		}
		
		
		//lower outer ring
		glColor4f(r+0.4f,0.4f,0.4f,1.0-ghost);
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiusm),-130,(int)(cos(k)*radiusm));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.33); 	 glVertex3i((int)(sin(k)*radiusm),-130,(int)(cos(k)*radiusm));
			glNormal3f((int)(sin(k)*radiusu),-150,(int)(cos(k)*radiusu));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.5);   glVertex3i((int)(sin(k)*radiusu),-150,(int)(cos(k)*radiusu));
		}
		
		glColor4f(r+0.3f,0.3f,0.3f,1.0-ghost);
		//upper inner ring
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiusi),-115,(int)(cos(k)*radiusi));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.25); glVertex3i((int)(sin(k)*radiusi),-115,(int)(cos(k)*radiusi));
			glNormal3f((int)(sin(k)*radiuso),-80,(int)(cos(k)*radiuso));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.0);  glVertex3i((int)(sin(k)*radiuso),-80,(int)(cos(k)*radiuso));
		}
		
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glNormal3f((int)(sin(k)*radiusu),-150,(int)(cos(k)*radiusu));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.5);	glVertex3i((int)(sin(k)*radiusu),-150,(int)(cos(k)*radiusu));
			glNormal3f((int)(sin(k)*radiusi),-115,(int)(cos(k)*radiusi));
			glTexCoord2f(2.0/((slices+1)*angle)*k,0.25);	glVertex3i((int)(sin(k)*radiusi),-115,(int)(cos(k)*radiusi));	
		}
	glEnd();
	
	
	glRotated(dir*-(oldpos.x-pos.x)/10.0,0,1,0);
	
	glColor4f(1.0,1.0,1.0,1.0-ghost);	
	
	int top = 0;
	int width = 120;
	int step = 20;
	float c = 0.5;
	float d = 0;
	if(dir>0)
	{
		c = 0;
		d = 0.5;
	}
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glBegin(GL_QUAD_STRIP);
		for(int i = 0;i<16;i++)
		{
			
			glTexCoord2f(c,0.51+i*0.025);
			glVertex3f(width,top+(70.0*sin(0.3*i)),-90+i*step);
			glTexCoord2f(d,0.51+i*0.025);
			
			glVertex3f(-width,top+(70.0*sin(0.3*i)),-90+i*step);	
			width-=6;
		}
	glEnd();
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	
	float a = 0;
	//right turbine
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2f(0.75,0.75);
		glVertex3f(0,-40,15);
		
		for(int i = 0;i<17;i++)
		{
			glNormal3f(-100-50*sin(a),-45+50*cos(a),-170);
			glTexCoord2f(0.5,0.5+i*0.5/16.0);
			glVertex3f(-100-50*sin(a),-45+50*cos(a),-170);
			a+=(2.0*M_PI)/16.0;
		}
		
	glEnd();
	
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2f(0.875,0.75);
		glVertex3f(-100,-45,-165);
		
		for(int i = 0;i<17;i++)
		{
			glNormal3f(-100+50*sin(a),-45+50*cos(a),-165);
			glTexCoord2f(0.875+0.125*sin(a),0.75+0.25*cos(a));
			glVertex3f(-100+50*sin(a),-45+50*cos(a),-165);
			a+=(2.0*M_PI)/16.0;
		}
		
	glEnd();
	
	//left turbine
	a = 0;
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2f(0.75,0.75);
		glVertex3f(0,-40,15);
		
		for(int i = 0;i<17;i++)
		{
			glNormal3f(100-50*sin(a),-45+50*cos(a),-170);
			glTexCoord2f(0.5,0.5+i*0.5/16.0);
			glVertex3f(100-50*sin(a),-45+50*cos(a),-170);
			a+=(2.0*M_PI)/16.0;
		}
		
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2f(0.875,0.75);
		glVertex3f(100,-45,-165);
		
		for(int i = 0;i<17;i++)
		{
			glNormal3f(100+50*sin(a),-45+50*cos(a),-165);
			glTexCoord2f(0.875+0.125*sin(a),0.75+0.25*cos(a));
			glVertex3f(100+50*sin(a),-45+50*cos(a),-165);
			a+=(2.0*M_PI)/16.0;
		}
		
	glEnd();
	
	//inner parts of turbine-blows
	double length = dir;
	if(ghost)
		length *= 0.5;
	a = 0;
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	
	glColor4f(0.1+float(f_speed)/500.0,0.1+float(f_speed)/900.0,0.0,0.0);
	glBegin(GL_TRIANGLE_FAN);
		if(dir*pos.z>dir*oldpos.z)
			glVertex3f(-90,-30,-165-min(length*(pos.z-oldpos.z),500));
		else
			glVertex3f(-90,-30,-165);
			
		for(int i = 0;i<8;i++)
		{
			glVertex3f(-100+33*sin(a),-45+33*cos(a),-170);
			a+=(2.0*M_PI)/7.0;
		}
		
	glEnd();

	a = 0;
	glBegin(GL_TRIANGLE_FAN);
		if(dir*pos.z>dir*oldpos.z)
			glVertex3f(90,-30,-165-min(length*(pos.z-oldpos.z),500));
		else
			glVertex3f(90,-30,-165);
		
		for(int i = 0;i<8;i++)
		{
			glVertex3f(100+33*sin(a),-45+33*cos(a),-170);
			a+=(2.0*M_PI)/7.0;
		}
		
	glEnd();
	
	//outer parts of blow
	if(dir*pos.z>dir*oldpos.z)
	{
		glEnable(GL_BLEND);
		
		glColor4f(0.3+float(f_speed)/600.0,0.2,0.2,0.0);
		glBegin(GL_TRIANGLE_FAN);
			
			glVertex3f(-80,-30,-170-min(1.5*length*(pos.z-oldpos.z),500));
			
			for(int i = 0;i<17;i++)
			{
				glVertex3f(-100+45*sin(a),-45+45*cos(a),-170);
				a+=(2.0*M_PI)/16.0;
			}
			
		glEnd();
	
		a = 0;
		glBegin(GL_TRIANGLE_FAN);
			glVertex3f(80,-30,-170-	min(1.5*length*(pos.z-oldpos.z),500));
			
			for(int i = 0;i<17;i++)
			{
				glVertex3f(100+45*sin(a),-45+45*cos(a),-170);
				a+=(2.0*M_PI)/16.0;
			}
			
		glEnd();
		
		glDisable(GL_BLEND);
	}
	if(arrow)
	{
		glColor4f(1.0,0.0,0.0,0.8);
		//arrow above head
		glBegin(GL_TRIANGLES);
			glVertex3f(0,500,0);
			glVertex3f(300,1400,0);
			glVertex3f(-300,1400,0);
		glEnd();
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	if(dir<0)
		glRotated(-180,0,1,0);

	

	glRotated(dir*(oldpos.x-pos.x)/10.0,0,1,0);
	glTranslated(-x,-y,-z);
	
	
		
	
	
	
	glColor3f(1.0,1.0,1.0);
	if(!wireframemode)
		glEnable(GL_TEXTURE_2D);
	
}

void PlayerPawn::ShadowVolume(float *lightdir)
{
	//draw shadow volume
	int slices = 12;
	double angle = (2.0*M_PI)/slices;
	double radius = 200;
	
	glTranslated(self.pos.x,self.pos.y,self.pos.z);
	glBegin(GL_QUAD_STRIP);
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glVertex3i((int)(sin(k)*radius),-90,(int)(cos(k)*radius));
			glVertex3i((int)(sin(k)*radius+lightdir[0]*10000),(int)(lightdir[1]*10000),(int)(cos(k)*radius+lightdir[2]*10000));
		}
	glEnd();

	glBegin(GL_POLYGON);
		for(double k = 0.0;k<(slices+1)*angle;k+=angle)
		{
			glVertex3i((int)(sin(k)*radius),-90,(int)(cos(k)*radius));
		}
	glEnd();
	glTranslated(-self.pos.x,-self.pos.y,-self.pos.z);

}


void PlayerPawn::CreateShadow(float*lightdir)
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
void PlayerPawn::Draw()
{
	
	double r = (float)(100-this->health)/10.0f;	//fix this
	PlayerModel(this->dir,this->self.v_speed,this->self.oldpos,this->self.pos,r,16.0,this->self.f_speed);
	
	glColor3f(1.0f,1.0f,1.0f);
}


int PlayerPawn::OnFloor(Tile *i)
{
	
	int x1 = i->cube->origin.x;
	int x2 = i->cube->origin.y;
	int x3 = i->cube->origin.z;
	int ox1 = i->cube->origin.x;
	int ox2 = i->cube->origin.y;
	int ox3 = i->cube->origin.z;
	//update position of moving floor
	if(i->cube->movetime && i->cube->movecount)
	{
		float part = (float)i->cube->movecount/(float)(i->cube->movetime*50);
		
		if((int)part%2) //on the way back
			part = 1.0f-(part-(int)part);
		else	
			part -=(int)part;	
		
		
		x1 += (int)(part*(i->cube->flag1-i->cube->origin.x));
		x2 += (int)(part*(i->cube->flag2-i->cube->origin.y));
		x3 += (int)(part*(i->cube->flag3-i->cube->origin.z));
	
		if(x3 > self.pos.z+155 && dir >= 1)  //floor is far away from player
			return OFF;
			
		if(i->cube->triggered)
		{
			part = (float)(i->cube->movecount-1)/(float)(i->cube->movetime*50);
			
			if((int)part%2) //on the way back
				part = 1.0f-(part-(int)part);
			else	
				part -=(int)part;	
			
			ox1 += (int)(part*(i->cube->flag1-i->cube->origin.x));
			ox2 += (int)(part*(i->cube->flag2-i->cube->origin.y));
			ox3 += (int)(part*(i->cube->flag3-i->cube->origin.z));
		}
		else
		{
			ox1 = x1; ox2 = x2; ox3 = x3;
		}
		
		
	}
	else if(x3 > self.pos.z+155 && dir >= 1) //coming floors are too far away -> skip them
		return SKIP;//skip next floors
		
	if(x3 > self.pos.z+155 && dir < 1)
		return OFF;
	
	if(x3+i->cube->ext.z < self.pos.z-155)
		return OFF;
	
	
	//on floor
	if((i->cube->movetime && (ox2 <= self.pos.y-150 && x2 >= self.pos.y-150 || ox2 >= self.pos.y-150 && x2 <= self.pos.y-150))
	|| (self.pos.y-150 <  x2 && self.oldpos.y-150 >=  x2) //or: adjust player y-pos and check for collisions at fast v_speed	
	|| (self.pos.y-150+40 >=  x2 && self.pos.y-150-40 <=  x2)) 
	{
		if(x1 < self.pos.x-155)
			return OFF;
		
		if(x1-i->cube->ext.x > self.pos.x+155)
			return OFF;

		int offset = self.pos.y-150 - x2;
	
		self.pos.y -= offset;
		cam.pos.y -= offset;
	
		
		self.pos.x -= (ox1-x1);
		if(dir == 1 || dir == -1)	//if this pawn is a bot, cam.pos.x is something else
			cam.pos.x -= (ox1-x1);
			
		self.pos.z -= (ox3-x3);
		cam.pos.z -= (ox3-x3);
		
		if(!i->cube->triggered) //floor is triggered on touch
		{
			if(E_TOUCH(i->cube->triggerflag))
			{
				i->cube->triggered = 1;//trigger floor
				i->cube->movecount = 0;
			}
		}
		
		if(self.pos.y >= x2 && !powerup)
		{
			if(i->cube->flag1 && EX_POWERUP(i->cube->triggerflag)
			&& i->pu_time <= 0)//power up
			{
				if(Length((float)self.pos.x,(float)self.pos.y,(float)self.pos.z,(float)i->cube->flag1,(float)i->cube->origin.y+350,(float)i->cube->flag2) < 500)
				{
					play(1,S_POWERUP);
					powerup = rand()%6+1;
					i->pu_time = 2500;
				}
				
					 	
			}
		}
		return ON;

	}
	else if(self.pos.y-160+20 < x2
	&& self.pos.y+150-20 > x2-i->cube->ext.y)
	{
		if(x1 < self.pos.x-197)
			return OFF;
		
		if(x1-i->cube->ext.x > self.pos.x+197)
			return OFF;

		if(self.oldpos.x-195 >= x1 || self.oldpos.x-195 >= ox1) //player "rams" left side
		{
			self.s_speed = 0;
				
			int offset = x1-(self.pos.x-195);
			
			self.pos.x += offset;
			if(dir == 1 || dir == -1)	//if this pawn is a bot, cam.pos.x is something else
				cam.pos.x += offset;
			
			
			self.pos.x -= (ox1-x1);
			if(dir == 1 || dir == -1)	//if this pawn is a bot, cam.pos.x is something else
				cam.pos.x -= (ox1-x1);
				
			self.pos.z -= (ox3-x3);
			cam.pos.z -= (ox3-x3);
			
			return LEFT_ON;
			
			
		}
		else if(self.oldpos.x+195 <= x1-i->cube->ext.x
		|| self.oldpos.x+195 <= ox1-i->cube->ext.x) //player "rams" right side
		{
			self.s_speed = 0;
		
			int offset = self.pos.x+195-(x1-i->cube->ext.x);
	
			self.pos.x -= offset;
			if(dir == 1 || dir == -1)	//if this pawn is a bot, cam.pos.x is something else
				cam.pos.x -= offset;
			
			self.pos.x -= (ox1-x1);
			if(dir == 1 || dir == -1)	//if this pawn is a bot, cam.pos.x is something else
				cam.pos.x -= (ox1-x1);
				
			self.pos.z -= (ox3-x3);
			cam.pos.z -= (ox3-x3);
			
			return RIGHT_ON;
		}
		else
			return CRASH;
		
	}
	if(x1 < self.pos.x-155)
		return OFF;
		
	if(x1-i->cube->ext.x > self.pos.x+155)
		return OFF;

	if(self.pos.y >= x2 && !powerup)
	{
		if(i->cube->flag1 && EX_POWERUP(i->cube->triggerflag)
		&& i->pu_time <= 0)//power up
		{
			if(Length((float)self.pos.x,(float)self.pos.y,(float)self.pos.z,(float)i->cube->flag1,(float)i->cube->origin.y+350,(float)i->cube->flag2) < 500)
			{
				play(1,S_POWERUP);
				powerup = rand()%6+1;
				i->pu_time = 2500;
			}
			
				 	
		}
	}
		
	return ABOVE;
}

void PongBall::Draw()
{
	glBindTexture(GL_TEXTURE_2D, anims[7][0]);
	DrawCircle(pos.x,pos.y,pos.z,16,0.5,-0.1,-0.1,1.0,1.0);
	
	if(drall)
	{
		if(!blooming)
			glEnable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		
		for(int i = 0;i<5;i++)
			DrawCircle(oldpos[i].x,oldpos[i].y,oldpos[i].z,8,-0.45,-0.45,-0.3,1.0/(float)(i+1),0.3);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		if(!blooming)
			glDisable(GL_BLEND);
	}
	
}

void InitEffects()
{
	effects = glGenLists(5);	//5 smoke puffs						
	for (int i=0; i<5; i++)
	{
		float x = ((float)i)/5.0f;						
		
		glNewList(effects+i,GL_COMPILE);					
		glBegin(GL_QUADS);
			glTexCoord2f(x, 0);						glVertex2i(0, 0);
			glTexCoord2f(x, 1.0f);					glVertex2i(0, -190);
			glTexCoord2f((x+1.0f/5.0f),1.0f);		glVertex2i(-190, -190);
			glTexCoord2f((x+1.0f/5.0f), 0);			glVertex2i(-190, 0);
				
		glEnd();																
		glEndList();															
	}
}

void DrawEffect(Effects effe)	
{
	
	glColor3f(effe.r,effe.g,effe.b);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBindTexture(GL_TEXTURE_2D, anims[22][0]);		
										
		glPushMatrix();	
		
			glTranslated(effe.origin.x,effe.origin.y,effe.origin.z);
			glScaled(effe.xs,effe.ys,effe.zs);																						
			glListBase(effects);						
			glCallLists(1,GL_UNSIGNED_BYTE,&effe.type);	
																					
		glPopMatrix();	
												
	glDisable(GL_BLEND);	
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);	
	glEnable(GL_CULL_FACE);			
}


void DrawBackGround(int texindex,int multi)
{
	if(wireframemode)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_CULL_FACE);	
	}
	
	float div = (float)multi*0.25;
	
	glBindTexture(GL_TEXTURE_2D, anims[texindex][0]);	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);							
		glMatrixMode(GL_PROJECTION);						
		glPushMatrix();									
		glLoadIdentity();									
		gluOrtho2D(0,set.width,0,set.height);							
		glMatrixMode(GL_MODELVIEW);						
		glPushMatrix();
		glLoadIdentity();
													
			glBegin(GL_QUADS);
					
				glTexCoord2f(0.0, 1.0-div);		glVertex2i(0, 0);
				glTexCoord2f(0.999,1.0-div);		glVertex2i(set.width, 0);
				glTexCoord2f(0.999,0.0+div);		glVertex2i(set.width, set.height);
				glTexCoord2f(0.0, 0.0+div);		glVertex2i(0, set.height);
				
			glEnd();	
				
		glMatrixMode(GL_PROJECTION);						
		glPopMatrix();										
		glMatrixMode(GL_MODELVIEW);							
		glPopMatrix();	
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	
	if(wireframemode)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
	}	
						
}
