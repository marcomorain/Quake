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
// snd_null.c -- include this instead of all the other snd_* files to have
// no sound code whatsoever

#include "quakedef.h"
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "assert.h"

enum {
    NUM_BUFFERS = MAX_CHANNELS
};
static ALCdevice*  device  = 0;
static ALCcontext* context = 0;

static ALuint buffers[NUM_BUFFERS];
static qboolean	snd_ambient = 1;

cvar_t bgmvolume  = {"bgmvolume",  "1", true};
cvar_t volume     = {"volume",     "0.7", true};
cvar_t nosound    = {"nosound",    "0"};
cvar_t precache   = {"precache",   "1"};
cvar_t loadas8bit = {"loadas8bit", "0"};

static void check_error() {
    int error = alGetError();

    if (error != AL_NO_ERROR) {
        Con_Printf("OpenAL Error: %x %s\n", error, alGetString(error));
        assert(0);
    }
}

void S_Init (void)
{
    
    Cvar_RegisterVariable(&nosound);
    Cvar_RegisterVariable(&volume);
    Cvar_RegisterVariable(&bgmvolume);
    Cvar_RegisterVariable(&precache);
    Cvar_RegisterVariable(&loadas8bit);
    
    device = alcOpenDevice(NULL);


    check_error();
    if (!device) {
        Con_SafePrintf("Error opening OpenAL device.\n");
    }

    Con_Printf("OpenAL initialised: %s\n", alGetString(AL_VERSION));
    Con_Printf("OpenAL Renderer:    %s\n", alGetString(AL_RENDERER));
    Con_Printf("OpenAL Extenstions: %s\n", alGetString(AL_EXTENSIONS));

    context = alcCreateContext(device, NULL);
    check_error();

    alcMakeContextCurrent(context);

    alGenBuffers(NUM_BUFFERS, buffers);
    check_error();
    
    sfx_t* sound = Hunk_AllocName(1 * sizeof(sfx_t), "sfx_tmp");
    Q_strcpy(sound->name, "misc/menu2.wav");
    S_LoadSound(sound);
    
    ALuint source;
    alGenSources(1, &source);
    check_error();
    
    int format = AL_FORMAT_MONO16;
    
    sfxcache_t* s = sound->cache.data;
    
    alBufferData(buffers[0], format, s->data, s->length, 11025);
    check_error();
    
    alSourceQueueBuffers(source, 1, buffers);
    check_error();
    alSourcePlay(source);
    check_error();
}

void S_AmbientOff (void)
{
    snd_ambient = 0;
}

void S_AmbientOn (void)
{
    snd_ambient = 1;
}

void S_Shutdown (void)
{
    alcMakeContextCurrent(NULL);
    alDeleteBuffers(NUM_BUFFERS, buffers);
    alcDestroyContext(context);
    alcCloseDevice(device);
    context = 0;
    device  = 0;
}

void S_TouchSound (char *sample)
{
}

void S_ClearBuffer (void)
{
}

void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
}

void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol,  float attenuation)
{
}

void S_StopSound (int entnum, int entchannel)
{
}

/*
 ==================
 S_PrecacheSound
 
 ==================
 */
sfx_t *S_PrecacheSound (char *name)
{
    return NULL;
}

void S_ClearPrecache (void)
{
}

void S_Update (vec3_t origin, vec3_t v_forward, vec3_t v_right, vec3_t v_up)
{
    float orientation[6];
    for (int i=0; i<3; i++) {
        orientation[i]   = v_forward[i];
        orientation[i+3] = v_up[i];
    }
    alListenerfv(AL_POSITION,    origin);
    alListenerfv(AL_ORIENTATION, orientation);
}

void S_StopAllSounds (qboolean clear)
{
}

void S_BeginPrecaching (void)
{
}

void S_EndPrecaching (void)
{
}

void S_ExtraUpdate (void)
{
}

void S_LocalSound (char *s)
{
}

