#include <ctype.h>
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
cvar_t gl_ztrick        = {"gl_ztrick","1"};
static cvar_t m_filter  = {"m_filter", "0"};

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

unsigned		d_8to24table[256];
unsigned char	d_15to8table[65536];

static unsigned char glfw_to_quake_keys[GLFW_KEY_LAST+1] = {};

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
    for (int i=0; i <= GLFW_KEY_LAST; i++)
    {
        if (isprint(i))
        {
            // the engine expects normal keys to be lowercase
            // swap and ASCII characters to lowercase (or leave unchanged).
            glfw_to_quake_keys[i] = tolower(i);
        }
    }
    
    glfw_to_quake_keys[GLFW_KEY_TAB] =  K_TAB;
    glfw_to_quake_keys[GLFW_KEY_ENTER] =  K_ENTER;
    glfw_to_quake_keys[GLFW_KEY_ESCAPE] =  K_ESCAPE;
    glfw_to_quake_keys[GLFW_KEY_SPACE] =  K_SPACE;
    glfw_to_quake_keys[GLFW_KEY_BACKSPACE] =  K_BACKSPACE;
    glfw_to_quake_keys[GLFW_KEY_UP] =             K_UPARROW;
    glfw_to_quake_keys[GLFW_KEY_DOWN] =           K_DOWNARROW;
    glfw_to_quake_keys[GLFW_KEY_LEFT] =           K_LEFTARROW;
    glfw_to_quake_keys[GLFW_KEY_RIGHT] =          K_RIGHTARROW;
    glfw_to_quake_keys[GLFW_KEY_RIGHT_ALT] =      K_ALT;
    glfw_to_quake_keys[GLFW_KEY_LEFT_ALT] =       K_ALT;
    glfw_to_quake_keys[GLFW_KEY_RIGHT_CONTROL] =  K_CTRL;
    glfw_to_quake_keys[GLFW_KEY_LEFT_CONTROL] =   K_CTRL;
    glfw_to_quake_keys[GLFW_KEY_RIGHT_SHIFT] =    K_SHIFT;
    glfw_to_quake_keys[GLFW_KEY_LEFT_SHIFT] =     K_SHIFT;
    glfw_to_quake_keys[GLFW_KEY_F1] =   K_F1;
    glfw_to_quake_keys[GLFW_KEY_F2] =   K_F2;
    glfw_to_quake_keys[GLFW_KEY_F3] =   K_F3;
    glfw_to_quake_keys[GLFW_KEY_F4] =   K_F4;
    glfw_to_quake_keys[GLFW_KEY_F5] =   K_F5;
    glfw_to_quake_keys[GLFW_KEY_F6] =   K_F6;
    glfw_to_quake_keys[GLFW_KEY_F7] =   K_F7;
    glfw_to_quake_keys[GLFW_KEY_F8] =   K_F8;
    glfw_to_quake_keys[GLFW_KEY_F9] =   K_F9;
    glfw_to_quake_keys[GLFW_KEY_F10] =  K_F10;
    glfw_to_quake_keys[GLFW_KEY_F11] =  K_F11;
    glfw_to_quake_keys[GLFW_KEY_F12] =  K_F12;
    glfw_to_quake_keys[GLFW_KEY_INSERT]     = K_INS;
    glfw_to_quake_keys[GLFW_KEY_DELETE]     = K_DEL;
    glfw_to_quake_keys[GLFW_KEY_PAGE_DOWN]  = K_PGDN;
    glfw_to_quake_keys[GLFW_KEY_PAGE_UP]    = K_PGUP;
    glfw_to_quake_keys[GLFW_KEY_HOME]       = K_HOME;
    glfw_to_quake_keys[GLFW_KEY_END]        = K_END;
    glfw_to_quake_keys[GLFW_KEY_PAUSE]      = K_PAUSE;
    
    // special
    glfw_to_quake_keys[GLFW_KEY_LEFT_SUPER]  = K_CTRL;
    glfw_to_quake_keys[GLFW_KEY_RIGHT_SUPER] = K_CTRL;
    
    // mouse
    glfw_to_quake_keys[GLFW_MOUSE_BUTTON_1] = K_MOUSE1;
    glfw_to_quake_keys[GLFW_MOUSE_BUTTON_2] = K_MOUSE2;
    glfw_to_quake_keys[GLFW_MOUSE_BUTTON_3] = K_MOUSE3;
    glfw_to_quake_keys[161] =  126; // ~
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
    Key_Event(glfw_to_quake_keys[key], action == GLFW_PRESS);
}

static void size_callback(GLFWwindow* window, int w, int h)
{
    window_settings.width = w;
    window_settings.height = h;
    // force a surface cache flush
    // TODO: is this needed?
    vid.recalc_refdef = 1;
}

static void cursor_callback(GLFWwindow* window, double x, double y)
{
    window_settings.mouse_delta.x += (x - window_settings.mouse.x);
    window_settings.mouse_delta.y += (y - window_settings.mouse.y);
    window_settings.mouse.x = x;
    window_settings.mouse.y = y;
}

static void click_callback(GLFWwindow* window, int button, int action, int mods)
{
    Key_Event(glfw_to_quake_keys[button], action == GLFW_PRESS);
}


void VID_Init(unsigned char *palette)
{
    Cvar_RegisterVariable (&m_filter);
	Cvar_RegisterVariable (&gl_ztrick);
    
    Init_KBD();
    
    
    memset(&window_settings, 0, sizeof(window_settings));
    window_settings.height    = 480;
    window_settings.width     = 640;

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
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetMouseButtonCallback(window, click_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
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

/*
 ===========
 IN_Move
 ===========
 */
void IN_MouseMove (usercmd_t *cmd)
{
    double x = window_settings.mouse_delta.x;
    double y = window_settings.mouse_delta.y;
    
	if (m_filter.value)
	{
        x *= 0.5;
        y *= 0.5;
	}
    
    window_settings.mouse_delta.x = 0;
    window_settings.mouse_delta.y = 0;

    if (1) {
        window_settings.mouse.x = window_settings.width  / 2.0;
        window_settings.mouse.y = window_settings.height / 2.0;
        glfwSetCursorPos(window, window_settings.mouse.x, window_settings.mouse.y);\
    }
    
    x *= sensitivity.value;
	y *= sensitivity.value;
    
    // add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * x;
	else
		cl.viewangles[YAW] -= m_yaw.value * x;
	
	if (in_mlook.state & 1)
		V_StopPitchDrift ();
    
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * y;
		else
			cmd->forwardmove -= m_forward.value * y;
	}
}

void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove(cmd);
}
