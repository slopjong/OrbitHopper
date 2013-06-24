/*
#				Orbit-Hopper
#				Game.cpp
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

#include "../headers/Game.h"
#include "../headers/Gamestate.h"
#include "../headers/Init.h"
#include "../headers/TimeAttack.h"
#include "../headers/LevelEd.h"
#include "../headers/Menu.h"
#include "../headers/Options.h"
#include "../headers/MapSel.h"
#include "../headers/LevelEnd.h"
#include "../headers/Console.h"
#include "../headers/ModeSelM.h"
#include "../headers/CastleM.h"
#include "../headers/CastleMEnd.h"
#include "../headers/RaceM.h"
#include "../headers/RaceEnd.h"
#include "../headers/SinglePlSel.h"
#include "../headers/Single.h"
#include "../headers/SinglePlCSel.h"
#include "../headers/Utils.h"


GLuint  **anims = NULL;
bool shadersavailable = false;
bool blooming = false;
Gamestate *cur_gamestate = NULL;
Gamestate *gamestates[15];

int origwidth = 1024;
int origheight = 768;

extern float *edgepts;
extern float * cornerpts;

unsigned int frames = 0;

extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC	glDeleteRenderbuffersEXT;


extern PFNGLUNIFORM1IARBPROC glUniform1iARB;
extern PFNGLUNIFORM1FARBPROC glUniform1fARB;
extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB ;
extern PFNGLSHADERSOURCEARBPROC glShaderSourceARB ;
extern PFNGLCOMPILESHADERARBPROC glCompileShaderARB ;
extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB ;
extern PFNGLATTACHOBJECTARBPROC glAttachObjectARB ;
extern PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
extern PFNGLLINKPROGRAMARBPROC glLinkProgramARB ;
extern PFNGLUSEPROGRAMOBJECTARBPROC  glUseProgramObjectARB ;
extern PFNGLGETUNIFORMLOCATIONARBPROC  glGetUniformLocationARB ;


extern GLuint framebuffer;	
extern GLuint depthbuffer;


extern GLhandleARB s_filter;		
extern GLhandleARB p_filter;	
extern GLint texblurred3;


extern GLhandleARB s_vblur;
extern GLhandleARB p_vblur;		
extern GLint texblurred1;
extern GLint vpixsize;

extern GLhandleARB s_hblur;		
extern GLhandleARB p_hblur;		
extern GLint texblurred2;
extern GLint hpixsize;


extern GLuint blur1;
extern GLuint blur2;
extern Settings set;

int animcount = 0;

void SwitchGamestate(int new_state)
{
	cur_gamestate = gamestates[new_state];
	cur_gamestate->Init();
}


void applyBloom()
{
	/*
		use shaders
	*/
	glViewport(0,0,set.bloomw,set.bloomh);

	//filter non-alpha values
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgramObjectARB(p_filter);	

	//set read-targets
	glBindTexture(GL_TEXTURE_2D,blur1);
	glUniform1iARB(texblurred3,0); //texture-unit 0

	glMatrixMode(GL_PROJECTION);						
	glPushMatrix();									
	glLoadIdentity();									
	gluOrtho2D(0,set.bloomw,0,set.bloomh);					
	glMatrixMode(GL_MODELVIEW);						
	glPushMatrix();									
		glLoadIdentity();						
			glBegin(GL_QUADS);
		    		glTexCoord2i(0, 0); 	glVertex2i(0, 0);
		    		glTexCoord2i(1, 0); 	glVertex2i(set.bloomw, 0);
		    		glTexCoord2i(1, 1);		glVertex2i(set.bloomw,set.bloomh);
		    		glTexCoord2i(0, 1); 	glVertex2i(0, set.bloomh);
			glEnd();				
	glMatrixMode(GL_PROJECTION);						
	glPopMatrix();										
	glMatrixMode(GL_MODELVIEW);							
	glPopMatrix();	


	//vertical blur
	glDrawBuffer (GL_COLOR_ATTACHMENT0_EXT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgramObjectARB(p_vblur);	

	//set read-targets
	glBindTexture(GL_TEXTURE_2D,blur2);
	glUniform1iARB(texblurred1,0); //texture-unit 0
	glUniform1fARB(vpixsize,1.0/(float)set.bloomh); 
	
	glMatrixMode(GL_PROJECTION);						
	glPushMatrix();									
	glLoadIdentity();									
	gluOrtho2D(0,set.bloomw,0,set.bloomh);					
	glMatrixMode(GL_MODELVIEW);						
	glPushMatrix();									
		glLoadIdentity();						
			glBegin(GL_QUADS);
		    		glTexCoord2i(0, 0); 	glVertex2i(0, 0);
		    		glTexCoord2i(1, 0); 	glVertex2i(set.bloomw, 0);
		    		glTexCoord2i(1, 1);		glVertex2i(set.bloomw, set.bloomh);
		    		glTexCoord2i(0, 1); 	glVertex2i(0, set.bloomh);
			glEnd();				
	glMatrixMode(GL_PROJECTION);						
	glPopMatrix();										
	glMatrixMode(GL_MODELVIEW);							
	glPopMatrix();	


	//horizontal blur
	
	glDrawBuffer (GL_COLOR_ATTACHMENT1_EXT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgramObjectARB(p_hblur);	

	//set read-targets
	glBindTexture(GL_TEXTURE_2D,blur1);
	glUniform1iARB(texblurred2,0); //texture-unit 0
	glUniform1fARB(hpixsize,1.0/(float)set.bloomw); 

	glMatrixMode(GL_PROJECTION);						
	glPushMatrix();									
	glLoadIdentity();									
	gluOrtho2D(0,set.bloomw,0,set.bloomh);					
	glMatrixMode(GL_MODELVIEW);						
	glPushMatrix();									
		glLoadIdentity();						
			glBegin(GL_QUADS);
		    		glTexCoord2i(0, 0); 	glVertex2i(0, 0);
		    		glTexCoord2i(1, 0); 	glVertex2i(set.bloomw, 0);
		    		glTexCoord2i(1, 1);		glVertex2i(set.bloomw, set.bloomh);
		    		glTexCoord2i(0, 1); 	glVertex2i(0, set.bloomh);
			glEnd();				
	glMatrixMode(GL_PROJECTION);						
	glPopMatrix();										
	glMatrixMode(GL_MODELVIEW);							
	glPopMatrix();	

	glUseProgramObjectARB(0);	
	//cgGLDisableProfile(cgProfile);
}


void doRender()
{
	glBlendFunc(GL_ONE, GL_ONE);
	if(shadersavailable && set.details == 3)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,framebuffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);	//render to blur texture
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glLoadIdentity();
		glColor3f(1.0f,1.0f,1.0f);

		origwidth = set.width;
		origheight = set.height;
		set.width = set.bloomw;
		set.height = set.bloomh;

		glViewport(0,0,set.bloomw,set.bloomh);

		blooming = true;
		cur_gamestate->Render();

		set.width = origwidth;
		set.height = origheight;

		applyBloom();	
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);		
  	glLoadIdentity();
  	glColor3f(1.0f,1.0f,1.0f);

	
	glViewport(0,0,set.width,set.height);
	blooming = false;
	glDisable(GL_BLEND);
	cur_gamestate->Render();	//do normal rendering 


	if(shadersavailable && set.details == 3)
	{
		//add blur to screen
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glMatrixMode(GL_PROJECTION);						
		glPushMatrix();									
		glLoadIdentity();									
		gluOrtho2D(0,set.width,0,set.height);					
		glMatrixMode(GL_MODELVIEW);						
		glPushMatrix();									
			glLoadIdentity();	
			glBindTexture(GL_TEXTURE_2D,blur2);
								
			glBegin(GL_QUADS);
				for(int i = 0;i<set.bloom;++i)
				{
				    	glTexCoord2i(0, 0); 	glVertex2i(0, 0);
				    	glTexCoord2i(1, 0); 	glVertex2i(set.width, 0);
				    	glTexCoord2i(1, 1);		glVertex2i(set.width, set.height);
				    	glTexCoord2i(0, 1); 	glVertex2i(0, set.height);
				}
			glEnd();	
				
						
		glMatrixMode(GL_PROJECTION);						
		glPopMatrix();										
		glMatrixMode(GL_MODELVIEW);							
		glPopMatrix();
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
	}

	SDL_GL_SwapBuffers();

}

#ifndef WIN32
int main(int argc, char *argv[])
#else
int APIENTRY WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
#endif
{
	
	
	bool done = false;
	unsigned int last_time = SDL_GetTicks();
	unsigned int framecount = 0;
	unsigned int msecs = 0;
	
	
	//initialize gamestates - beware: explicit casts
	gamestates[GS_MENU] = (Gamestate *)(new Menu());
	gamestates[GS_MAPSEL] = (Gamestate *)(new MapSel());
	gamestates[GS_TIMEATTACK] = (Gamestate *)(new TimeAttack());
	gamestates[GS_SINGLE] = (Gamestate *)(new Single());
	gamestates[GS_SINGLESEL] = (Gamestate *)(new SinglePlSel());
	gamestates[GS_CONSOLE] = (Gamestate *)(new Console());
	gamestates[GS_RACEM] = (Gamestate *)(new RaceM());
	gamestates[GS_RACEEND] = (Gamestate *)(new RaceEnd());
	gamestates[GS_LEVELED] = (Gamestate *)(new LevelEd());
	gamestates[GS_LEVELEND] = (Gamestate *)(new LevelEnd());
	gamestates[GS_MODESELM] = (Gamestate *)(new ModeSelM());
	gamestates[GS_CASTLEM] = (Gamestate *)(new CastleM());
	gamestates[GS_CAEND] = (Gamestate *)(new CastleMEnd());
	gamestates[GS_OPTIONS] = (Gamestate *)(new Options());
	gamestates[GS_SINGLECSEL] = (Gamestate *)(new SinglePlCSel());
	
	gamestates[GS_CONSOLE]->Init();

	if(Init() == -1)
		return -1;

	SwitchGamestate(GS_MENU);
		
	SDL_EnableUNICODE(true);
	
	while(!done)
	{
		framecount++;	//count frames
		unsigned int new_time = SDL_GetTicks();
		unsigned int diff_time = new_time - last_time;
		last_time = new_time;
		
		msecs+= diff_time;
		if(msecs >= 1000) //1 sec has passed
		{
			msecs = msecs%1000;
			frames = framecount; 	//save number of frames per second
			framecount = 0;
		}

		doRender();
	  		
		
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(cur_gamestate->Input(event,event.key.keysym)==-1)	//check for input
				done = true;
		}
			
		cur_gamestate->Update(diff_time);	//update
		
	}
	
	//clean up
	for(int i = 0;i<15;i++)
	{
		delete gamestates[i];
		gamestates[i] = NULL;
	}
		
	cur_gamestate = NULL;
	if(anims)
		delete[] anims;
	anims = NULL;
	
	if(shadersavailable)
	{
		
		glDeleteObjectARB(p_vblur);
		glDeleteObjectARB(p_hblur);
		glDeleteObjectARB(p_filter);

		glDeleteTextures(1,&blur1);
		glDeleteTextures(1,&blur2);

		glDeleteFramebuffersEXT(1,&framebuffer);
		glDeleteRenderbuffersEXT(1,&depthbuffer);
	}
	delete[] cornerpts;
	delete[] edgepts;

	SDL_Quit();
	
	
	return 0;
	
}
