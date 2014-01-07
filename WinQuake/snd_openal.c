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
    MAX_SFX = 512,
};

struct ALSound {
    ALuint buffer;
    sfx_t data;
};

struct Sound {
    ALCdevice*  device;
    ALCcontext* context;
    int		    num_sfx;
    struct ALSound known_sfx[MAX_SFX];
    struct ALSound ambient_sfx[NUM_AMBIENTS];
};

static qboolean	snd_ambient = 1;
static struct Sound sound = {};

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
    
    sound.device = alcOpenDevice(NULL);


    check_error();
    if (!sound.device) {
        Con_SafePrintf("Error opening OpenAL device.\n");
    }

    Con_Printf("OpenAL initialised: %s\n", alGetString(AL_VERSION));
    Con_Printf("OpenAL Renderer:    %s\n", alGetString(AL_RENDERER));
    Con_Printf("OpenAL Extenstions: %s\n", alGetString(AL_EXTENSIONS));

    sound.context = alcCreateContext(sound.device, NULL);
    check_error();

    alcMakeContextCurrent(sound.context);

	sound.num_sfx = 0;
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
    // TODO:
    // alDeleteBuffers();
    alcDestroyContext(sound.context);
    alcCloseDevice(sound.device);
    memset(&sound, 0, sizeof(struct Sound));
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

    char* buffer_x = sfx;
    buffer_x -= 4;
    ALuint buffer = *buffer_x;


    ALuint source;
    alGenSources(1, &source);
    check_error();
    alSourceQueueBuffers(source, 1, &buffer);
    check_error();
    alSourcePlay(source);
    check_error();
}

void S_StopSound (int entnum, int entchannel)
{
}

struct ALSound* S_FindName (struct Sound* s, char *name) {

	if (!name)
        Sys_Error ("S_FindName: NULL\n");

	if (Q_strlen(name) >= MAX_QPATH)
        Sys_Error ("Sound name too long: %s", name);

    // see if already loaded
    int i;
	for (i=0 ; i < s->num_sfx ; i++)
		if (!Q_strcmp(s->known_sfx[i].data.name, name))
			return &s->known_sfx[i];

	if (s->num_sfx == MAX_SFX)
		Sys_Error ("S_FindName: out of sfx_t");

    // Not found - set the filename and return
    // some memory to load the sound into.
    struct ALSound* result = &s->known_sfx[i];
	strcpy(result->data.name, name);
	s->num_sfx++;
	return result;
}

/*
 ==================
 S_PrecacheSound
 
 ==================
 */
sfx_t *S_PrecacheSound (char *name)
{
    assert(sound.device && sound.context);

    struct ALSound* result = S_FindName(&sound, name);
    if (precache.value) {
        S_LoadSound(&result->data);

        const int format = AL_FORMAT_MONO16;

        sfxcache_t* cached = result->data.cache.data;

        alGenBuffers(1, &result->buffer);
        check_error();

        alBufferData(result->buffer, format, cached->data, cached->length, 11025);
        check_error();
    }
    return &result->data;
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

