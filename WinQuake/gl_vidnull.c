/*
 Copyright (C) 1996-1997 Id Software, Inc.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 */

#include "quakedef.h"


unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];
unsigned char d_15to8table[65536];

int		texture_mode = GL_LINEAR;
int		texture_extension_number = 1;
float		gldepthmin, gldepthmax;

cvar_t	gl_ztrick = {"gl_ztrick","1"};

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

qboolean is8bit = false;
qboolean isPermedia = false;
qboolean gl_mtexable = false;

void VID_Shutdown(void)
{
}

void VID_ShiftPalette(unsigned char *p)
{
}

void	VID_SetPalette (unsigned char *palette)
{
}

/*
 ===============
 GL_Init
 ===============
 */
void GL_Init (void)
{
}

/*
 =================
 GL_BeginRendering

 =================
 */
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
}


void GL_EndRendering (void)
{
	glFlush();
}

void Init_KBD(void)
{
}

qboolean VID_Is8bit(void)
{
	return is8bit;
}

void VID_Init8bitPalette(void)
{
}

void VID_Init(unsigned char *palette)
{
}
