/*
#				Orbit-Hopper
#				Init.cpp
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

#include <SDL.h>
#include <gl.h>	
#include <glu.h>
#include <stdlib.h>


#include "../headers/Structs.h"
#include "../headers/Font.h"
#include "../headers/Options.h"
#include "../headers/Sound.h"
#include "../headers/Game.h"
#include "../headers/Utils.h"

#ifndef WIN32
#include <dirent.h>
#endif

#include "SDL_image.h"


PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = NULL;
PFNGLGENRENDERBUFFERSEXTPROC	glGenRenderbuffersEXT	= NULL;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT	= NULL;
PFNGLBINDRENDERBUFFEREXTPROC	glBindRenderbufferEXT	= NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC	glRenderbufferStorageEXT	= NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT	= NULL;

PFNGLUNIFORM1IARBPROC glUniform1iARB = NULL;
PFNGLUNIFORM1FARBPROC glUniform1fARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = NULL;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB = NULL;
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = NULL;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC  glUseProgramObjectARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC  glGetUniformLocationARB = NULL;

PFNGLGETPROGRAMIVPROC glGetProgramiv  = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
GLuint framebuffer;	
GLuint depthbuffer;


GLhandleARB s_vblur;		//vertical blur shader
GLhandleARB p_vblur;		//vertical blur shader
GLint texblurred1;
GLint vpixsize;

GLhandleARB s_hblur;		//horizontal blur shader
GLhandleARB p_hblur;		
GLint texblurred2;
GLint hpixsize;

GLhandleARB s_filter;		
GLhandleARB p_filter;
GLint texblurred3;


GLuint blur1;
GLuint blur2;



extern bool shadersavailable;
extern GLuint  **anims;


int 	animated[NUM_TEXTURES];
GLuint titles[1];

void ComputePoints();
void InitEffects();

extern Settings set;
extern Gamestate *gamestates[15];
int num_dir = 0;

#define GL_TEXTURE_FILTER_CONTROL         0x8500
#define GL_TEXTURE_LOD_BIAS               0x8501


bool mcheckFramebufferStatus()
{
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status) 
	{
		case GL_NO_ERROR:
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return true;
		break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			fprintf(stdout,"Unsupported framebuffer format.\n");
		break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			fprintf(stdout,"Framebuffer incomplete attachment.\n");
		break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			fprintf(stdout,"Framebuffer incomplete, missing attachment.\n");
		break;

		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			fprintf(stdout,"Framebuffer incomplete, attached images must have the same dimensions.\n");
		break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			fprintf(stdout,"Framebuffer incomplete, attached images must have the same format.\n");
		break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			fprintf(stdout,"Framebuffer incomplete, missing draw buffer.\n");
		break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			fprintf(stdout,"Framebuffer incomplete, missing read buffer.\n");
		break;
		default:
			fprintf(stdout,"Undefined error.\n ");
		break;
	}
    return false;
}

char* readSource(char * file)
{
	FILE* srcfile = fopen(file,"rt");
	if(!srcfile)
		return NULL;
	
	fseek(srcfile, 0, SEEK_END);
	int signs = ftell(srcfile);
	char *source = new char[signs+1];
	rewind(srcfile);

	signs = fread(source,sizeof(char),signs,srcfile);
	source[signs] = '\0';

	fclose(srcfile);
		
	return source;
	
}


void initShaders()
{

	char* shadersource = NULL;
	
	shadersource = readSource("shaders/blurh.frag");
	p_hblur = glCreateProgramObjectARB();
	s_hblur = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glAttachObjectARB(p_hblur, s_hblur);
	glShaderSourceARB(s_hblur, 1,(const char**)&shadersource, NULL);
	glCompileShaderARB(s_hblur);
	glLinkProgramARB(p_hblur);
	texblurred2= glGetUniformLocationARB(p_hblur, "blurred");
	hpixsize= glGetUniformLocationARB(p_hblur, "pixsize");
	glDeleteObjectARB(s_hblur);
	delete[] shadersource;
	
	
	int length = 0;
	glGetProgramiv(p_hblur,GL_INFO_LOG_LENGTH,&length);
    	if (length > 0) 
	{
		char *log = new char[length];
		int num_chars = 0;
		glGetProgramInfoLog(p_hblur, length, &num_chars, log);
		fprintf(stdout,log);
       	fprintf(stdout,"\n");
        	delete[] log;
   	}

	
	shadersource = readSource("shaders/blurv.frag");
	

	p_vblur = glCreateProgramObjectARB();
	s_vblur = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glAttachObjectARB(p_vblur, s_vblur);
	glShaderSourceARB(s_vblur, 1,(const char**)&shadersource, NULL);
	glCompileShaderARB(s_vblur);
	glLinkProgramARB(p_vblur);
	texblurred1= glGetUniformLocationARB(p_vblur, "blurred");
	vpixsize= glGetUniformLocationARB(p_vblur, "pixsize");
	glDeleteObjectARB(s_vblur);
	delete[] shadersource;


	length = 0;
	glGetProgramiv(p_vblur,GL_INFO_LOG_LENGTH,&length);
    	if (length > 0) 
	{
		char *log = new char[length];
		int num_chars = 0;
		glGetProgramInfoLog(p_vblur, length, &num_chars, log);
		fprintf(stdout,log);
       	fprintf(stdout,"\n");
       
        	delete[] log;
   	}
	
	
	shadersource = readSource("shaders/filter.frag");
	

	p_filter = glCreateProgramObjectARB();
	s_filter = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glAttachObjectARB(p_filter, s_filter);
	glShaderSourceARB(s_filter, 1,(const char**)&shadersource, NULL);
	glCompileShaderARB(s_filter);
	glLinkProgramARB(p_filter);
	texblurred3= glGetUniformLocationARB(p_filter, "tex");
	glDeleteObjectARB(s_filter);
	delete[] shadersource;
	

	length = 0;
	glGetProgramiv(p_filter,GL_INFO_LOG_LENGTH,&length);
    	if (length > 0) 
	{
		char *log = new char[length];
		int num_chars = 0;
		glGetProgramInfoLog(p_filter, length, &num_chars, log);
		fprintf(stdout,log);
       	fprintf(stdout,"\n");
       
        	delete[] log;
   	}
	

}


int initRendertargets()
{
	glGenFramebuffersEXT(1,&framebuffer);			 	//generiere Frambuffer
    	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,framebuffer);	//binde Frambuffer

	
	glGenRenderbuffersEXT(1, &depthbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,  set.bloomw, set.bloomh);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);




	glGenTextures(1, &blur1);
	glBindTexture(GL_TEXTURE_2D, blur1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, set.bloomw,set.bloomh, 0, GL_RGBA, GL_FLOAT, 0);

	glGenTextures(1, &blur2);
	glBindTexture(GL_TEXTURE_2D, blur2);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,  set.bloomw,set.bloomh, 0, GL_RGBA, GL_FLOAT, 0);

	
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, blur1, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, blur2, 0);

	if(!mcheckFramebufferStatus())
		return -1;

	
	return 0;

}

int initExt()
{
	const char *a = (const char*)glGetString(GL_EXTENSIONS);
	char *exts[6] = {"GL_ARB_shader_objects","GL_ARB_draw_buffers","GL_ARB_fragment_program","GL_ARB_fragment_shader",
			"GL_EXT_framebuffer_object","GL_ARB_texture_float"};
	
	for(int i = 0;i<6;i++)
	{	
		if(!strstr(a,exts[i]))
		{
			shadersavailable = false;
			return -1;
		}
	}
	int num_attachments;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT,&num_attachments);
	if(num_attachments < 2)
	{
		shadersavailable = false;
		return -1;
	}

	#ifdef WIN32
	glGenRenderbuffersEXT		= (PFNGLGENRENDERBUFFERSEXTPROC) 			wglGetProcAddress("glGenRenderbuffersEXT");
	glBindRenderbufferEXT		= (PFNGLBINDRENDERBUFFEREXTPROC) 			wglGetProcAddress("glBindRenderbufferEXT");
	glDeleteRenderbuffersEXT		= (PFNGLDELETERENDERBUFFERSEXTPROC) 		wglGetProcAddress("glDeleteRenderbuffersEXT");
	glRenderbufferStorageEXT		= (PFNGLRENDERBUFFERSTORAGEEXTPROC) 		wglGetProcAddress("glRenderbufferStorageEXT");
	glFramebufferRenderbufferEXT	= (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) wglGetProcAddress("glFramebufferRenderbufferEXT");

	glBindFramebufferEXT 		= (PFNGLBINDFRAMEBUFFEREXTPROC) 		wglGetProcAddress("glBindFramebufferEXT");
	glGenFramebuffersEXT		= (PFNGLGENFRAMEBUFFERSEXTPROC)		wglGetProcAddress("glGenFramebuffersEXT");
	glDeleteFramebuffersEXT		= (PFNGLDELETEFRAMEBUFFERSEXTPROC)		wglGetProcAddress("glDeleteFramebuffersEXT");
	glFramebufferTexture2DEXT	= (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)	wglGetProcAddress("glFramebufferTexture2DEXT");

	glUniform1iARB				= (PFNGLUNIFORM1IARBPROC)			wglGetProcAddress("glUniform1iARB");
	glUniform1fARB				= (PFNGLUNIFORM1FARBPROC)			wglGetProcAddress("glUniform1fARB");
	glCreateShaderObjectARB		= (PFNGLCREATESHADEROBJECTARBPROC)		wglGetProcAddress("glCreateShaderObjectARB");
	glShaderSourceARB			= (PFNGLSHADERSOURCEARBPROC)			wglGetProcAddress("glShaderSourceARB");
	glCompileShaderARB			= (PFNGLCOMPILESHADERARBPROC)			wglGetProcAddress("glCompileShaderARB");
	glCreateProgramObjectARB		= (PFNGLCREATEPROGRAMOBJECTARBPROC)	wglGetProcAddress("glCreateProgramObjectARB");
	glAttachObjectARB			= (PFNGLATTACHOBJECTARBPROC)			wglGetProcAddress("glAttachObjectARB");
	glLinkProgramARB			= (PFNGLLINKPROGRAMARBPROC)			wglGetProcAddress("glLinkProgramARB");
	glUseProgramObjectARB		= (PFNGLUSEPROGRAMOBJECTARBPROC)		wglGetProcAddress("glUseProgramObjectARB");
	glGetUniformLocationARB 		= (PFNGLGETUNIFORMLOCATIONARBPROC)		wglGetProcAddress("glGetUniformLocationARB");
	glDeleteObjectARB 			= (PFNGLDELETEOBJECTARBPROC) 			wglGetProcAddress("glDeleteObjectARB");

	glGetProgramiv  			= (PFNGLGETPROGRAMIVPROC)		wglGetProcAddress("glGetProgramiv");
	glGetProgramInfoLog 		= (PFNGLGETPROGRAMINFOLOGPROC)	wglGetProcAddress("glGetProgramInfoLog");
	glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");
	#else

	glGenRenderbuffersEXT		= (PFNGLGENRENDERBUFFERSEXTPROC) 			glXGetProcAddressARB((const GLubyte*)"glGenRenderbuffersEXT");
	glBindRenderbufferEXT		= (PFNGLBINDRENDERBUFFEREXTPROC) 			glXGetProcAddressARB((const GLubyte*)"glBindRenderbufferEXT");
	glDeleteRenderbuffersEXT		= (PFNGLDELETERENDERBUFFERSEXTPROC) 		glXGetProcAddressARB((const GLubyte*)"glDeleteRenderbuffersEXT");
	glRenderbufferStorageEXT		= (PFNGLRENDERBUFFERSTORAGEEXTPROC) 		glXGetProcAddressARB((const GLubyte*)"glRenderbufferStorageEXT");
	glFramebufferRenderbufferEXT	= (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) glXGetProcAddressARB((const GLubyte*)"glFramebufferRenderbufferEXT");

	glBindFramebufferEXT 		= (PFNGLBINDFRAMEBUFFEREXTPROC) 		glXGetProcAddressARB((const GLubyte*)"glBindFramebufferEXT");
	glGenFramebuffersEXT		= (PFNGLGENFRAMEBUFFERSEXTPROC)		glXGetProcAddressARB((const GLubyte*)"glGenFramebuffersEXT");
	glDeleteFramebuffersEXT		= (PFNGLDELETEFRAMEBUFFERSEXTPROC)		glXGetProcAddressARB((const GLubyte*)"glDeleteFramebuffersEXT");
	glFramebufferTexture2DEXT	= (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)	glXGetProcAddressARB((const GLubyte*)"glFramebufferTexture2DEXT");

	

	glUniform1iARB				= (PFNGLUNIFORM1IARBPROC)			glXGetProcAddressARB((const GLubyte*)"glUniform1iARB");
	glUniform1fARB				= (PFNGLUNIFORM1FARBPROC)			glXGetProcAddressARB((const GLubyte*)"glUniform1fARB");
	glCreateShaderObjectARB		= (PFNGLCREATESHADEROBJECTARBPROC)		glXGetProcAddressARB((const GLubyte*)"glCreateShaderObjectARB");
	glShaderSourceARB			= (PFNGLSHADERSOURCEARBPROC)			glXGetProcAddressARB((const GLubyte*)"glShaderSourceARB");
	glCompileShaderARB			= (PFNGLCOMPILESHADERARBPROC)			glXGetProcAddressARB((const GLubyte*)"glCompileShaderARB");
	glCreateProgramObjectARB		= (PFNGLCREATEPROGRAMOBJECTARBPROC)	glXGetProcAddressARB((const GLubyte*)"glCreateProgramObjectARB");
	glAttachObjectARB			= (PFNGLATTACHOBJECTARBPROC)			glXGetProcAddressARB((const GLubyte*)"glAttachObjectARB");
	glLinkProgramARB			= (PFNGLLINKPROGRAMARBPROC)			glXGetProcAddressARB((const GLubyte*)"glLinkProgramARB");
	glUseProgramObjectARB		= (PFNGLUSEPROGRAMOBJECTARBPROC)		glXGetProcAddressARB((const GLubyte*)"glUseProgramObjectARB");
	glGetUniformLocationARB 		= (PFNGLGETUNIFORMLOCATIONARBPROC)		glXGetProcAddressARB((const GLubyte*)"glGetUniformLocationARB");
	glDeleteObjectARB 			= (PFNGLDELETEOBJECTARBPROC) 			glXGetProcAddressARB((const GLubyte*)"glDeleteObjectARB");

	glGetProgramiv  			= (PFNGLGETPROGRAMIVPROC)		glXGetProcAddressARB((const GLubyte*)"glGetProgramiv");
	glGetProgramInfoLog 		= (PFNGLGETPROGRAMINFOLOGPROC)	glXGetProcAddressARB((const GLubyte*)"glGetProgramInfoLog");
	glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) glXGetProcAddressARB((const GLubyte*)"glCheckFramebufferStatusEXT");
	#endif
	shadersavailable = true;
	return 0;
}


void InitGL(int width,int height)	 
{
	glViewport(0,0,width,height);
	glClearColor(0.0f,0.0f,0.0f,0.0f);		
	glClearDepth(1.0f);	
	
	
	glDepthFunc(GL_LEQUAL);	
	glEnable(GL_DEPTH_TEST);			
	glShadeModel(GL_SMOOTH); //we don't need smooth shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glHint(GL_POINT_SMOOTH_HINT,GL_FASTEST);
		
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				

	gluPerspective(40,(GLfloat)width/(GLfloat)height,150.5f,120000.0f);

	glMatrixMode(GL_MODELVIEW);	
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);

	{
		GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
		GLfloat position[] = { -1.5f, 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		GLfloat ambient[] = { 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	}
	glEnable(GL_LIGHT0);

	{
		GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
		glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
		GLfloat position[] = { 1.5f, 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT1, GL_POSITION, position);
		GLfloat ambient[] = { 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	}
	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);
	
}

int LoadTitles()
{
	
	SDL_Surface *TempTex[1]; 
	memset(TempTex,0,sizeof(void *)*1);   
	
	if((TempTex[0] = IMG_Load("gfx/titles.png")))
	{
		
		glGenTextures(1, &titles[0]);
			
		glBindTexture(GL_TEXTURE_2D, titles[0]);	
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


#ifdef WIN32
	char **GetGfxDirs(int &dir_num) //Windows
	{
		char **result = NULL;
		_finddata_t fileblock;
		dir_num = 0;
		
		int dir = _findfirst("gfx/*",&fileblock);
		if(dir==-1)
		{
			result = new char*[1];
			result[0] = new char[1];
			strcpy(result[0],"");
			return result;
		}
		if(!strstr(fileblock.name,"."))	//fileblock is a driectory
			dir_num++;
		
		while(_findnext(dir,&fileblock) != -1)
		{
			if(!strstr(fileblock.name,"."))	//fileblock is a directory
				dir_num++;
			
		}
		
		if(!dir_num)//no directories found
		{
			result = new char*[1];
			result[0] = new char[1];
			strcpy(result[0],"");
			return result;
		}
		
		_findclose(dir);
		
		result = new char*[dir_num];
		for(int i = 0;i<dir_num;i++)
			result[i] = new char[16];
			
		dir_num = 0;
		
		dir = _findfirst("gfx/*",&fileblock);
		
		if(!strstr(fileblock.name,"."))	//fileblock is a driectory
		{
			dir_num++;
			strncpy(result[0],fileblock.name,16);
		}
		
		while(_findnext(dir,&fileblock) != -1)
		{
			if(!strstr(fileblock.name,"."))	//fileblock is a directory
			{
				strncpy(result[dir_num],fileblock.name,16);
				dir_num++;
			}
			
		}
		
		_findclose(dir);
		return result;
			
	}
#else

	char ** GetGfxDirs(int &dir_num) //linux
	{
		DIR *dir = opendir("gfx/");
		char ** result = NULL;
		dir_num = 0;
	
		dirent *entry = readdir(dir);
	
		while(entry != NULL)
		{
			if(!strstr(entry->d_name,"."))
				dir_num++;
			entry = readdir(dir);
		}
	
		closedir(dir);
		
		result = new char*[dir_num];
		for(int i = 0;i<dir_num;i++)
			result[i] = new char[16];
	
		
		dir_num = 0;
		dir = opendir("gfx/");
		entry = readdir(dir);
	
		while(entry != NULL)
		{
			if(!strstr(entry->d_name,"."))
			{
				strncpy(result[dir_num],entry->d_name,16);
				dir_num++;
			}
			entry = readdir(dir);
		}
	
		closedir(dir);
		
		return result;
	}
#endif

int InitTextures()
{
	//texture names seperated by ","
	static char *texts[] = {
		"tex0","tex1","tex2","tex3","tex4","tex5","tex6","tex7","tex8",
		"menu00","menu01","menu02","menu03","menu04","menu05","tex9",
		"lev03","lev04","lev01","lev02","tex10","tex11","efx01","space1",
		"menu06","spacemenu","tex12","gui","space2","powerups","wall","shield",
		"single","checkp","space3","space4","lev05","tex13"};
	
	SDL_Surface *TempTex[NUM_TEXTURES]; 
	memset(TempTex,0,sizeof(void *)*NUM_TEXTURES);   
	
	char filename[64];
	memset(filename,0,sizeof(char)*64);
	char style[16];
	
	//find out which directory the textures are to be loaded from
	char **a = GetGfxDirs(num_dir);
	if(set.textures < num_dir)
		strcpy(style,a[set.textures]);
	else	//load textures from default directory
		strcpy(style,a[0]);
	for(int i = 0;i<num_dir;i++)
		delete[] a[i];
	delete[] a;
	
	for(int i = 0;i< NUM_TEXTURES;i++)	//by default no texture is animated
		animated[i] = 0;
		
	sprintf(filename,"gfx/%s/%s.png",style,texts[0]);
	if(!(TempTex[0] = IMG_Load(filename)))	//texture not found
	{
		sprintf(filename,"gfx/%s/%s00.png",style,texts[0]);//check, if this texture is animated
		if(!(TempTex[0] = IMG_Load(filename)))
		{
			return -1;
		}
		else
			animated[0] = 1;
	}
	
		
	
		
	for(int i = 1;i<NUM_TEXTURES;i++)
	{
		
		if((i>=0 && i<=9) || i == 15 || i == 20 || i == 21 
		|| i == 26 || i == 30 || i == 32 || i == 33 || i == 37)
			sprintf(filename,"gfx/%s/%s.png",style,texts[i]);
		else
			sprintf(filename,"gfx/%s.png",texts[i]);
			
		if(!(TempTex[i] = IMG_Load(filename)))//texture not found -> check, whether animated texture can be found
		{
			if((i>=0 && i<=8) || i == 15 || i == 20 || i == 21 
			|| i == 26 || i == 30 || i == 33 || i == 37)
				sprintf(filename,"gfx/%s/%s00.png",style,texts[i]);
				
			if(!(TempTex[i] = IMG_Load(filename)))
			{
				return -1;
			}
			else
				animated[i] = 1;
		}
		
	}
	
	if(anims)
		delete[] anims;
	anims = NULL;
	anims = new GLuint*[NUM_TEXTURES];
			

	for(int i = 0; i< NUM_TEXTURES;i++)
	{	
				
		if(animated[i])//get animated textures as well
		{
		
			int one = 0, ten = 0;
			do
			{
				sprintf(filename,"gfx/%s/%s%d%d.png",style,texts[i],ten,one);
				
				one++;
				if(one > 9)
				{
					one = 0;
					ten++;
				}
			}
			while(IMG_Load(filename));
			
			animated[i] = (ten*10+one)-1;
			
			anims[i] = new GLuint[animated[i]];
			glGenTextures(animated[i], anims[i]);
			one = 0;
			ten = 0;
			SDL_Surface *a = NULL;
			for(int p = 0;p < animated[i];p++)
			{
				glBindTexture(GL_TEXTURE_2D, anims[i][p]);
				
				
				sprintf(filename,"gfx/%s/%s%d%d.png",style,texts[i],ten,one);
				
				
				a = IMG_Load(filename);
				
				if(!set.mipmaps)
					glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, a->w, a->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, a->pixels);
			
				if(set.mipmaps) //let OpenGl generate some mipmaps for us
				{
					gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, a->w, a->h, GL_RGBA, GL_UNSIGNED_BYTE, a->pixels);
			 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	
			 		glTexEnvf(GL_TEXTURE_FILTER_CONTROL,GL_TEXTURE_LOD_BIAS,-1.5);
				}
			
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	
				if(!set.mipmaps)
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
					
			
					
				SDL_FreeSurface(a);	
				one++;
				if(one > 9)
				{
					one = 0;
					ten++;
				}
				
			}
			
		}
		else
		{
			anims[i] = new GLuint[1];
			glGenTextures(1, anims[i]);
			glBindTexture(GL_TEXTURE_2D, anims[i][0]);

			if(!set.mipmaps)
				glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, TempTex[i]->w, TempTex[i]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, TempTex[i]->pixels);
		
			if(set.mipmaps) //let OpenGL generate some mipmaps for us
			{
				gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, TempTex[i]->w, TempTex[i]->h, GL_RGBA, GL_UNSIGNED_BYTE, TempTex[i]->pixels);
		 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	
		 		glTexEnvf(GL_TEXTURE_FILTER_CONTROL,GL_TEXTURE_LOD_BIAS,-1.5);
			}
			
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
		
			if(!set.mipmaps)
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			
			SDL_FreeSurface(TempTex[i]);
		}
		
		
	}
	
	glEnable(GL_TEXTURE_2D);
	
	
	return 0;	
	

}


int Init()
{

	//initialize SDL - start
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)==-1)
	{
		fprintf(stdout, "Could not initialize SDL: %s\n", SDL_GetError());
		return -1;	
	}
	
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE ,8); 
	SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE ,8); 
	SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,8); 
	SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE ,8); 
	
	
	
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(false);
	
	char **a = GetGfxDirs(num_dir);
	for(int i = 0;i<num_dir;i++)
		delete[] a[i];
	delete[] a;
	//load settings
	((Options *)(gamestates[GS_OPTIONS]))->LoadSettings();
	
	
	unsigned int flags = SDL_OPENGL | SDL_SWSURFACE;
	
	if(!set.windowed)
		flags |= SDL_FULLSCREEN;
	
	if(SDL_SetVideoMode(set.width,set.height,16,flags)==NULL)
	{
		fprintf(stdout, "Could not set OpenGL Video Mode: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;	
	}
	SDL_WM_SetCaption("Orbit-Hopper 1.16c",NULL);
	//initialize SDL - end	

	srand(SDL_GetTicks());
	
	//Initialize Sounds
	if(set.sounds)
	{
		if(InitSound() == -1)
		{
			SDL_Quit();
			return -1;
		}
	}
	
	//Initialize OpenGL
	InitGL(set.width,set.height);
	
	//Get textures
	if(InitTextures() == -1)
	{
		fprintf(stdout, "Textures not found.\n");
		SDL_Quit();
		return -1;	
	}
	
	InitEffects();
	if(LoadTitles() == -1)
	{
		fprintf(stdout, "Titles-texture not found.\n");
		SDL_Quit();
		return -1;	
	}

	//Get bitmap-font
	if(LoadFontBmp() == -1)
	{
		fprintf(stdout, "Font-texture not found.\n");
		SDL_Quit();
		return -1;	
	}
	ComputePoints();
	
	if(initExt() == 0)
	{
		if(initRendertargets() == 0)
		{
			initShaders();
				
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
			glDrawBuffer(GL_FRAMEBUFFER_EXT);
		}
		else
		{
			fprintf(stdout, "Could not create rendertargets. Shaders are deactivated.\n");
			shadersavailable = false;
		}
	}
	else
		fprintf(stdout, "Could not load extensions. Shaders are deactivated.\n");

	return 0;
	
}



