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

struct Sound {
    ALCdevice*  device;
    ALCcontext* context;
    int		    num_sfx;
    sfx_t known_sfx[MAX_SFX];
    sfx_t ambient_sfx[NUM_AMBIENTS];
};

// From SND DMA
channel_t   channels[MAX_CHANNELS] = {}; // TODO: heap allocate?
int total_channels;
vec_t		sound_nominal_clip_dist=1000.0;
int   		paintedtime; 	// sample PAIRS
vec3_t		listener_origin;
vec3_t		listener_forward;
vec3_t		listener_right;
vec3_t		listener_up;

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
        Con_Printf("OpenAL Error (%x) %s\n", error, alGetString(error));
        //assert(0);
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


    ALuint sources[MAX_CHANNELS];
    alGenSources(MAX_CHANNELS, sources);
    check_error();

    for (int i=0; i<MAX_CHANNELS; i++) {
        channels[i].openal_source = sources[i];
    }

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

static void play(const channel_t* channel, const vec3_t position)
{
    const int sid = channel->openal_source;

    alSourceStop(sid);
    check_error();

    //alSourceRewind(sid);
    check_error();

    for (;;) {
        ALuint processed;
        alGetSourcei(sid, AL_BUFFERS_PROCESSED, &processed);
        alSourceUnqueueBuffers(<#ALuint sid#>, <#ALsizei numEntries#>, <#ALuint *bids#>)
    }

    alSourceUnqueueBuffers(sid, 0, 0);





    alSourceQueueBuffers(sid, 1, &channel->sfx->openal_buffer);
    check_error();
    alSourcef(sid, AL_GAIN, channel->master_vol/255.0f);

    vec3_t position_metric;
    const double i_to_m = 1; //0.0254;
    for (int i=0; i<3; i++) {
        position_metric[i] = position[i] * i_to_m;
    }

   // alSourcei(sid, AL_SAMPLE_OFFSET, channel->pos);

    alSourcefv(sid, AL_POSITION, position_metric);
    check_error();
    alSourcePlay(sid);

    check_error();

}

/*
=================
S_StaticSound
=================
*/
void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
    return;

	channel_t	*ss;
	sfxcache_t		*sc;

	if (!sfx)
		return;

	if (total_channels == MAX_CHANNELS)
	{
		Con_Printf ("total_channels == MAX_CHANNELS\n");
		return;
	}

	ss = &channels[total_channels];
	total_channels++;

	sc = S_LoadSound (sfx);
	if (!sc)
		return;

	if (sc->loopstart == -1)
	{
		Con_Printf ("Sound %s not looped\n", sfx->name);
		return;
	}

	ss->sfx = sfx;
	VectorCopy (origin, ss->origin);
	ss->master_vol = vol;
	ss->dist_mult = (attenuation/64) / sound_nominal_clip_dist;
    ss->end = paintedtime + sc->length;

    play(ss, origin);
}


void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol,  float attenuation) {

	int		ch_idx;
	int		skip;

    if (!sound.device) {
        return;
    }

	if (!sfx) {
		return;
    }

	if (nosound.value) {
		return;
    }

	const int vol = fvol*255;


    // pick a channel to play on
    channel_t* target_chan = SND_PickChannel(entnum, entchannel);
	if (!target_chan) {
		return;
    }

    printf("playing %s on %d at %d\n", sfx->name, target_chan->openal_source, vol);

    const int openal_source = target_chan->openal_source;

    // spatialize
	memset (target_chan, 0, sizeof(*target_chan));
	VectorCopy(origin, target_chan->origin);
	target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
	target_chan->master_vol = vol;
	target_chan->entnum = entnum;
	target_chan->entchannel = entchannel;
    target_chan->openal_source = openal_source;

    // TODO: put this in
	//if (!target_chan->leftvol && !target_chan->rightvol)
	//	return;		// not audible at all

    // new channel
    sfxcache_t* sc = S_LoadSound (sfx);
	if (!sc)
	{
		target_chan->sfx = NULL;
		return;		// couldn't load the sound's data
	}

	target_chan->sfx = sfx;
	target_chan->pos = 0.0;
    target_chan->end = paintedtime + sc->length;

    // if an identical sound has also been started this frame, offset the pos
    // a bit to keep it from just making the first one louder
    channel_t* check = &channels[NUM_AMBIENTS];
    for (ch_idx=NUM_AMBIENTS ; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; ch_idx++, check++)
    {
		if (check == target_chan)
			continue;
		if (check->sfx == sfx && !check->pos)
		{
            const int speed = 11025;
			skip = rand () % (int)(0.1*speed);
			if (skip >= target_chan->end)
				skip = target_chan->end - 1;
			//target_chan->pos += skip;
			//target_chan->end -= skip;
			break;
		}

	}

    play(target_chan, origin);
}

void S_StopSound(int entnum, int entchannel)
{
	for (int i=0 ; i<MAX_DYNAMIC_CHANNELS ; i++)
	{
		if (channels[i].entnum == entnum
			&& channels[i].entchannel == entchannel)
		{
			channels[i].end = 0;
			channels[i].sfx = NULL;
			return;
		}
	}
}



sfx_t* S_FindName (struct Sound* s, char *name) {

	if (!name)
        Sys_Error ("S_FindName: NULL\n");

	if (Q_strlen(name) >= MAX_QPATH)
        Sys_Error ("Sound name too long: %s", name);

    // see if already loaded
    int i;
	for (i=0 ; i < s->num_sfx ; i++)
		if (!Q_strcmp(s->known_sfx[i].name, name))
			return &s->known_sfx[i];

	if (s->num_sfx == MAX_SFX)
		Sys_Error ("S_FindName: out of sfx_t");

    // Not found - set the filename and return
    // some memory to load the sound into.
    sfx_t* result = s->known_sfx + i;
	strcpy(result->name, name);
	s->num_sfx++;
	return result;
}


static void load(const char* name, sfx_t* dest)
{
    S_LoadSound(dest);

    const int format = AL_FORMAT_MONO16;
    sfxcache_t* cached = dest->cache.data;

    ALuint buffer = 0;
    alGenBuffers(1, &buffer);

    check_error();
    //Con_Printf("Sound buffer %d assigned to sound %s\n", buffer, name);
    dest->openal_buffer = buffer;
    alBufferData(buffer, format, cached->data, cached->length, 11025/2);
    check_error();
}


/*
 =================
 SND_PickChannel
 =================
 */
channel_t *SND_PickChannel(int entnum, int entchannel)
{
    int ch_idx;
    int first_to_die;
    int life_left;

    // Check for replacement sound, or find the best one to replace
    first_to_die = -1;
    life_left = 0x7fffffff;
    for (ch_idx=NUM_AMBIENTS ; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; ch_idx++)
    {
		if (entchannel != 0		// channel 0 never overrides
            && channels[ch_idx].entnum == entnum
            && (channels[ch_idx].entchannel == entchannel || entchannel == -1) )
		{	// allways override sound from same entity
			first_to_die = ch_idx;
			break;
		}

		// don't let monster sounds override player sounds
		if (channels[ch_idx].entnum == cl.viewentity && entnum != cl.viewentity && channels[ch_idx].sfx)
			continue;

		if (channels[ch_idx].end - paintedtime < life_left)
		{
			life_left = channels[ch_idx].end - paintedtime;
			first_to_die = ch_idx;
		}
    }

	if (first_to_die == -1)
		return NULL;

	if (channels[first_to_die].sfx)
		channels[first_to_die].sfx = NULL;
    
    return &channels[first_to_die];    
}

/*
 ==================
 S_PrecacheSound
 
 ==================
 */
sfx_t *S_PrecacheSound (char *name)
{
    assert(sound.device && sound.context);

    sfx_t* result = S_FindName(&sound, name);
    if (precache.value) {
        load(name, result);
    }
    return result;
}

void S_ClearPrecache (void)
{
}

void S_Update (vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{
    VectorCopy(origin, listener_origin);
	VectorCopy(forward, listener_forward);
	VectorCopy(right, listener_right);
	VectorCopy(up, listener_up);

    float orientation[6];
    for (int i=0; i<3; i++) {
        orientation[i]   = forward[i];
        orientation[i+3] = up[i];
    }
    alListenerfv(AL_POSITION,    origin);
    alListenerfv(AL_ORIENTATION, orientation);
}

void S_StopAllSounds (qboolean clear)
{
    if (!sound.device)
        return;

    total_channels = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;	// no statics

    for (int i=0; i<MAX_CHANNELS; i++) {
        alSourceStop(channels[i].openal_source);
        check_error();

    }

    // TODO: Stop all sources
    for (int i=0; i<sound.num_sfx; i++) {
        sfx_t* sfx = sound.known_sfx + i;
//        alDeleteBuffers(1, &sfx->openal_buffer);
//        check_error();
        // TODO: destroy buffer
        sfx->openal_buffer = 0;
    }
    sound.num_sfx = 0;
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
    // TODO
   // sfx_t* sfx = S_FindName(&sound, s);
  //  if (sfx->openal_buffer == 0) {
   //     load(s, sfx);
 //   }
//    play(sfx, 1);
}

