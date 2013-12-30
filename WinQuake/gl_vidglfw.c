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

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

unsigned		d_8to24table[256];
unsigned char	d_15to8table[65536];

static GLFWwindow* window = NULL;

struct {
    int width;
    int height;
} window_settings;

void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect (int x, int y, int width, int height)
{
}

void VID_Shutdown(void)
{
    Con_Print("Shutting down GLFW\n");
    glfwTerminate();
}

void VID_ShiftPalette(unsigned char *p)
{
//	VID_SetPalette(p);
}

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned r,g,b;
	unsigned v;
	int     r1,g1,b1;
	int		j,k,l,m;
	unsigned short i;
	unsigned	*table;
	FILE *f;
	char s[255];
	int dist, bestdist;
    
    //
    // 8 8 8 encoding
    //
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] &= 0xffffff;	// 255 is transparent
    
	for (i=0; i < (1<<15); i++) {
		/* Maps
         000000000000000
         000000000011111 = Red  = 0x1F
         000001111100000 = Blue = 0x03E0
         111110000000000 = Grn  = 0x7C00
         */
		r = ((i & 0x1F) << 3)+4;
		g = ((i & 0x03E0) >> 2)+4;
		b = ((i & 0x7C00) >> 7)+4;
		pal = (unsigned char *)d_8to24table;
		for (v=0,k=0,bestdist=10000*10000; v<256; v++,pal+=4) {
			r1 = (int)r - (int)pal[0];
			g1 = (int)g - (int)pal[1];
			b1 = (int)b - (int)pal[2];
			dist = (r1*r1)+(g1*g1)+(b1*b1);
			if (dist < bestdist) {
				k=v;
				bestdist = dist;
			}
		}
		d_15to8table[i]=k;
	}
}

void GL_Init (void)
{
    gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);
    
	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);
    
    //	Con_Printf ("%s %s\n", gl_renderer, gl_version);
    
    gl_mtexable = false; //CheckMultiTextureExtensions ();
    
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

void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
    *x = *y = 0;
	*width  = window_settings.width;
	*height = window_settings.height;
}

void GL_EndRendering (void)
{
    glFlush();
    glfwSwapBuffers(window);
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

void error_callback(int error, const char* description)
{
    Con_Printf("GLFW Error 0x%08x: %s\n", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_REPEAT) return;
    
#define KEY(g,q) case (g): key = (q); break;
    
    switch(key) {
        // normal keys should be passed as lowercased ascii
        KEY(GLFW_KEY_TAB, K_TAB);
        KEY(GLFW_KEY_ENTER, K_ENTER);
        KEY(GLFW_KEY_ESCAPE, K_ESCAPE);
        KEY(GLFW_KEY_SPACE, K_SPACE);
        KEY(GLFW_KEY_BACKSPACE, K_BACKSPACE);
        
        KEY(GLFW_KEY_UP, K_UPARROW);
        KEY(GLFW_KEY_DOWN, K_DOWNARROW);
        KEY(GLFW_KEY_LEFT, K_LEFTARROW);
        KEY(GLFW_KEY_RIGHT, K_RIGHTARROW);
            
        default:
            Con_Printf("key %d\n", key);
            break;
/*
#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152
            
#define K_PAUSE			255
 
 */
    }
    
    Key_Event(key, action == GLFW_PRESS);
}

static void size_callback(GLFWwindow* window, int w, int h)
{
    window_settings.width = w;
    window_settings.height = h;
    // force a surface cache flush
    // TODO: is this needed?
    vid.recalc_refdef = 1;
}

void VID_Init(unsigned char *palette)
{
    
    window_settings.height = 480;
    window_settings.width  = 640;

    char	gldir[MAX_OSPATH];
    
    Con_Print("VID_Init\n");
    
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
	vid.width = vid.conwidth;
	vid.height = vid.conheight;
    
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
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, size_callback);
    
	GL_Init();
    
    sprintf (gldir, "%s/glquake", com_gamedir);
	Sys_mkdir (gldir);
    
    
//	Check_Gamma(palette);
	VID_SetPalette(palette);
    
	// Check for 3DFX Extensions and initialize them.
	VID_Init8bitPalette();
    
	Con_SafePrintf ("Video mode %dx%d initialized.\n", vid.width, vid.height);
    
	vid.recalc_refdef = 1;				// force a surface cache flush
}

void IN_Init (void)
{
    // see vid init
}

void IN_Shutdown (void)
{
    // vid shutdown
}

void IN_Commands (void)
{
}

void Sys_SendKeyEvents (void)
{
    glfwPollEvents();
    
    if (glfwWindowShouldClose(window))
    {
        Con_Print("Closing GLFW window\n");
        glfwTerminate();
        Sys_Quit();
    }
}

void IN_Move (usercmd_t *cmd)
{
}