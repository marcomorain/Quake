#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include <GLFW/glfw3.h>
#include "quakedef.h"

float gldepthmin, gldepthmax;
int texture_extension_number = 1;
int		texture_mode = GL_LINEAR;
const char *gl_renderer = "none";
qboolean isPermedia = false;
qboolean gl_mtexable = false;
cvar_t	gl_ztrick = {"gl_ztrick","1"};

unsigned		d_8to24table[256];
unsigned char	d_15to8table[65536];

static GLFWwindow* window = NULL;

void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect (int x, int y, int width, int height)
{
}

void VID_Shutdown(void)
{
}

void InitSig(void)
{
}

void VID_ShiftPalette(unsigned char *p)
{
//	VID_SetPalette(p);
}

void	VID_SetPalette (unsigned char *palette)
{
}

void GL_Init (void)
{
}

void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
}


void GL_EndRendering (void)
{
}

void Init_KBD(void)
{
}

qboolean VID_Is8bit(void)
{
	//return is8bit;
    return 1;
}

void VID_Init8bitPalette(void) 
{
}

void VID_Init(unsigned char *palette)
{
}
