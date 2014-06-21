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
#include <GLFW/glfw3.h>

static GLFWwindow* window = NULL;

struct pos {
  double x;
  double y;
};

struct {
  int width;
  int height;
  struct pos mouse_delta;
  struct pos mouse;
} window_settings;

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];
unsigned char d_15to8table[65536];

int		texture_mode = GL_LINEAR;
int		texture_extension_number = 1;
float		gldepthmin, gldepthmax;

cvar_t	gl_ztrick = {"gl_ztrick","1"};

const char *gl_vendor = "null_vendor";
const char *gl_renderer = "null_renderer";
const char *gl_version = "null_version";
const char *gl_extensions = "null_extensions";

qboolean is8bit = false;
qboolean isPermedia = false;
qboolean gl_mtexable = false;

void VID_Shutdown(void)
{
    Con_SafePrintf("Shutting down GLFW\n");
    glfwTerminate();
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

static void error_callback(int error, const char* description)
{
    Con_Printf("GLFW Error 0x%08x: %s\n", error, description);
}

void VID_Init(unsigned char *palette)
{
    Init_KBD();

    memset(&window_settings, 0, sizeof(window_settings));
    window_settings.height    = 240; //1024;
    window_settings.width     = 320; // 1280;

    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!glfwInit()) {
        Sys_Error("Error initialising GLFW");
    }
    vid.maxwarpwidth  = window_settings.width;  // WARP_WIDTH;
	vid.maxwarpheight = window_settings.height; // WARP_HEIGHT;
    vid.width  = window_settings.width;
    vid.height = window_settings.height;

	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

    vid.conwidth = 640;

	vid.conwidth &= 0xfff8; // make it a multiple of eight

	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
	vid.conheight = vid.conwidth*3 / 4;

	if (vid.conheight < 200)
		vid.conheight = 200;

	if (vid.conheight > vid.height)
		vid.conheight = vid.height;
	if (vid.conwidth > vid.width)
		vid.conwidth = vid.width;

	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	vid.numpages = 2;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(vid.width, vid.height, "Quake", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        Sys_Error("Error creating GLFW window");
    }

    Con_Printf("GLFW initialised %s\n", glfwGetVersionString());
}
