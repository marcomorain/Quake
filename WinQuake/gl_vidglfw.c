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
    Con_Print("Shutting down GLFW");
    glfwTerminate();
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
    vid.maxwarpwidth = 320; // WARP_WIDTH;
	vid.maxwarpheight = 200; // WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
    vid.width = 320;
    vid.height = 200;
    
    
    /* Initialize the library */
    if (!glfwInit()) {
        Sys_Error("Error initialising GLFW");
    }
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(vid.width, vid.height, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        Sys_Error("Error creating GLFW window");
    }
    
    Con_Printf("GLFW initialised %s", glfwGetVersionString());
    
    glfwMakeContextCurrent(window);
    
    // Copied GL code from Linux
    glClearColor (1,0,0,0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);
    
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);
    
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_FLAT);
    
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
