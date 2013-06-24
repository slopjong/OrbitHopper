/*
#				Game.h

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
#ifndef _GAME_H_
#define _GAME_H_

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <SDL.h>
	#include <gl.h>	
	#include <glu.h>
#else
	#include <SDL.h>
	#include <gl.h>
	#include <glx.h>	
	#include <glu.h>
#endif



#include "Gamestate.h"

//gamestates
#define GS_MENU 		0
#define GS_OPTIONS 		1
#define GS_MAPSEL		2
#define GS_LEVELED 		3
#define GS_LEVELEND		4
#define GS_TIMEATTACK	5
#define GS_CONSOLE		6
#define GS_MODESELM		7
#define GS_CASTLEM		8
#define GS_CAEND		9
#define GS_RACEM		10
#define GS_RACEEND		11
#define GS_SINGLESEL 	12
#define GS_SINGLE 		13
#define GS_SINGLECSEL	14

//keys
#define FORWARD1	0
#define BACKWARD1	1
#define RIGHT1		2
#define LEFT1		3
#define JUMP1		4
#define RESET1		5
#define USE1		6

//speeds
#define MAX_FSPEED	450
#define MAX_BSPEED	100
#define MAX_SSPEED	180

//types of collision
#define OFF			0
#define ON			1
#define LEFT_ON		2
#define RIGHT_ON	3
#define CRASH		4
#define SKIP		-1
#define ABOVE		-2


#define NUM_TEXTURES 38


void SwitchGamestate(int new_state);


#define GL_FRAGMENT_SHADER_ARB            0x8B30

#define GL_COLOR_ATTACHMENT0_EXT          0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT          0x8CE1
#define GL_DEPTH_ATTACHMENT_EXT           0x8D00

#define GL_FRAMEBUFFER_EXT                0x8D40
#define GL_RENDERBUFFER_EXT               0x8D41
#define GL_MAX_COLOR_ATTACHMENTS_EXT      0x8CDF

#define GL_INFO_LOG_LENGTH                				0x8B84
#define GL_FRAMEBUFFER_COMPLETE_EXT       				0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT 			0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT 	0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT 	0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT 			0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT 			0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT 		0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT 		0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT    				0x8CDD

typedef unsigned int GLhandleARB;
typedef char GLcharARB;
typedef char GLchar;
typedef float GLfloat;

typedef GLenum (APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (APIENTRY* PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRY* PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRY* PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRY* PFNGLGENRENDERBUFFERSEXTPROC)	(GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRY* PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRY* PFNGLBINDRENDERBUFFEREXTPROC)	(GLenum target, GLuint renderbuffer);
typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEEXTPROC)	(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY* PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

typedef void (APIENTRY* PFNGLUNIFORM1IARBPROC) (GLint location, GLint v0);
typedef void (APIENTRY* PFNGLUNIFORM1FARBPROC) (GLint location, GLfloat v0);
typedef GLhandleARB (APIENTRY* PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void (APIENTRY* PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
typedef void (APIENTRY* PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
typedef GLhandleARB (APIENTRY* PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
typedef void (APIENTRY* PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (APIENTRY* PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
typedef void (APIENTRY* PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
typedef void (APIENTRY* PFNGLUSEPROGRAMOBJECTARBPROC)  (GLhandleARB programObj);
typedef GLint (APIENTRY* PFNGLGETUNIFORMLOCATIONARBPROC)  (GLhandleARB programObj, const GLcharARB *name);

typedef void (APIENTRY* PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
#endif
