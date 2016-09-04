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
#include "assert.h"
#include "fmod.h"

enum {
    MAX_SFX = 512,
};

struct Sound {
    FMOD_SYSTEM* system;
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


static float INCHES_PER_METER = 39.3701f;

static void check_error(FMOD_RESULT result);

static inline FMOD_VECTOR FromVec3(const vec3_t v) {
    
    FMOD_VECTOR result;
    result.x = v[0];
    result.y = v[1];
    result.z = v[2];
    return result;
}

void S_Init (void)
{
    
    Cvar_RegisterVariable(&nosound);
    Cvar_RegisterVariable(&volume);
    Cvar_RegisterVariable(&bgmvolume);
    Cvar_RegisterVariable(&precache);
    Cvar_RegisterVariable(&loadas8bit);
    
    FMOD_RESULT result = FMOD_System_Create(&sound.system);
    
    if (result != FMOD_OK) {
        Con_SafePrintf("Error opening FMOD device.\n");
    }
    
    // TODO: extra driver data
    result = FMOD_System_Init(sound.system, MAX_SFX, FMOD_INIT_NORMAL, NULL);
    check_error(result);

    
    // TODO: test FMOD to inches
    result = FMOD_System_Set3DSettings(sound.system, 1.0, INCHES_PER_METER, 1.0f);
    check_error(result);


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
    FMOD_RESULT result;
    result = FMOD_System_Close(sound.system);
    result = FMOD_System_Release(sound.system);
    
    // TODO: check results
    
    memset(&sound, 0, sizeof(struct Sound));
}

void S_TouchSound (char *sample)
{
}

void S_ClearBuffer (void)
{
}

static void play(const sfx_t* sfx, channel_t* channel, const vec3_t position)
{
    // TODO: play
    
    FMOD_CHANNEL* channelOut = NULL;
    
    FMOD_RESULT result;
    result = FMOD_System_PlaySound(sound.system, sfx->fmod_sound, 0, true, &channelOut);
    
    check_error(result);
    channel->channel = channelOut;
    
    //const int sid = channel->openal_source;
    //alSourcei(sid, AL_BUFFER, channel->sfx->openal_buffer);
    //check_error();
    
    result = FMOD_Channel_SetVolume(channelOut, channel->master_vol/255.0f);
    check_error(result);
                                    

    
    //alSourcef(sid, AL_GAIN, channel->master_vol/255.0f);
    //check_error();
    
    FMOD_VECTOR pos = FromVec3(position);
    FMOD_Channel_Set3DAttributes(channelOut, &pos, 0, 0);
    
    FMOD_Channel_SetVolumeRamp(channelOut, false);
    check_error(result);
    FMOD_Channel_SetPaused(channelOut, false);
    check_error(result);
    
    
    //alSourcei(sid, AL_SAMPLE_OFFSET, channel->pos);
    //alSourcefv(sid, AL_POSITION, position_metric);
    //check_error();
    //alSourcePlay(sid);
    //check_error();
    
}

/*
=================
S_StaticSound
=================
*/
void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
    return; // TODO - don't return

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

    play(sfx, ss, origin);
}


void S_StartSound (int entnum, int entchannel, const sfx_t *sfx, vec3_t origin, float fvol,  float attenuation) {

    if (!sound.system) {
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


    //const int openal_source = target_chan->openal_source;

    // spatialize
	memset (target_chan, 0, sizeof(*target_chan));
	VectorCopy(origin, target_chan->origin);
	target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
	target_chan->master_vol = vol;
	target_chan->entnum = entnum;
	target_chan->entchannel = entchannel;
    //target_chan->openal_source = openal_source;

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

    for (int ch_idx=NUM_AMBIENTS ; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; ch_idx++, check++)
    {
		if (check == target_chan)
			continue;
		if (check->sfx == sfx && !check->pos)
		{
            const int speed = 11025;
			int skip = rand () % (int)(0.1*speed);
			if (skip >= target_chan->end)
				skip = target_chan->end - 1;
            // TODO: skip for sounds that collide
			//target_chan->pos += skip;
			//target_chan->end -= skip;
			break;
		}

	}
    
    // TODO: remove channels and use FMOD instead

    play(sfx, target_chan, origin);
}

void S_StopSound(int entnum, int entchannel)
{
	for (int i=0 ; i<MAX_DYNAMIC_CHANNELS ; i++)
	{
		if (channels[i].entnum == entnum
			&& channels[i].entchannel == entchannel)
		{
            FMOD_Channel_SetPaused(channels[i].channel, true);
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
//    Con_Printf("Precache sound [%d] %s\n", i, name);
	s->num_sfx++;
	return result;
}


static const char *FMOD_ErrorString(FMOD_RESULT errcode)
{
    switch (errcode)
    {
            /*$ preserve end $*/
        case FMOD_OK:                            return "No errors.";
        case FMOD_ERR_BADCOMMAND:                return "Tried to call a function on a data type that does not allow this type of functionality (ie calling Sound::lock on a streaming sound).";
        case FMOD_ERR_CHANNEL_ALLOC:             return "Error trying to allocate a channel.";
        case FMOD_ERR_CHANNEL_STOLEN:            return "The specified channel has been reused to play another sound.";
        case FMOD_ERR_DMA:                       return "DMA Failure.  See debug output for more information.";
        case FMOD_ERR_DSP_CONNECTION:            return "DSP connection error.  Connection possibly caused a cyclic dependency or connected dsps with incompatible buffer counts.";
        case FMOD_ERR_DSP_DONTPROCESS:           return "DSP return code from a DSP process query callback.  Tells mixer not to call the process callback and therefore not consume CPU.  Use this to optimize the DSP graph.";
        case FMOD_ERR_DSP_FORMAT:                return "DSP Format error.  A DSP unit may have attempted to connect to this network with the wrong format, or a matrix may have been set with the wrong size if the target unit has a specified channel map.";
        case FMOD_ERR_DSP_INUSE:                 return "DSP is already in the mixer's DSP network. It must be removed before being reinserted or released.";
        case FMOD_ERR_DSP_NOTFOUND:              return "DSP connection error.  Couldn't find the DSP unit specified.";
        case FMOD_ERR_DSP_RESERVED:              return "DSP operation error.  Cannot perform operation on this DSP as it is reserved by the system.";
        case FMOD_ERR_DSP_SILENCE:               return "DSP return code from a DSP process query callback.  Tells mixer silence would be produced from read, so go idle and not consume CPU.  Use this to optimize the DSP graph.";
        case FMOD_ERR_DSP_TYPE:                  return "DSP operation cannot be performed on a DSP of this type.";
        case FMOD_ERR_FILE_BAD:                  return "Error loading file.";
        case FMOD_ERR_FILE_COULDNOTSEEK:         return "Couldn't perform seek operation.  This is a limitation of the medium (ie netstreams) or the file format.";
        case FMOD_ERR_FILE_DISKEJECTED:          return "Media was ejected while reading.";
        case FMOD_ERR_FILE_EOF:                  return "End of file unexpectedly reached while trying to read essential data (truncated?).";
        case FMOD_ERR_FILE_ENDOFDATA:            return "End of current chunk reached while trying to read data.";
        case FMOD_ERR_FILE_NOTFOUND:             return "File not found.";
        case FMOD_ERR_FORMAT:                    return "Unsupported file or audio format.";
        case FMOD_ERR_HEADER_MISMATCH:           return "There is a version mismatch between the FMOD header and either the FMOD Studio library or the FMOD Low Level library.";
        case FMOD_ERR_HTTP:                      return "A HTTP error occurred. This is a catch-all for HTTP errors not listed elsewhere.";
        case FMOD_ERR_HTTP_ACCESS:               return "The specified resource requires authentication or is forbidden.";
        case FMOD_ERR_HTTP_PROXY_AUTH:           return "Proxy authentication is required to access the specified resource.";
        case FMOD_ERR_HTTP_SERVER_ERROR:         return "A HTTP server error occurred.";
        case FMOD_ERR_HTTP_TIMEOUT:              return "The HTTP request timed out.";
        case FMOD_ERR_INITIALIZATION:            return "FMOD was not initialized correctly to support this function.";
        case FMOD_ERR_INITIALIZED:               return "Cannot call this command after System::init.";
        case FMOD_ERR_INTERNAL:                  return "An error occurred that wasn't supposed to.  Contact support.";
        case FMOD_ERR_INVALID_FLOAT:             return "Value passed in was a NaN, Inf or denormalized float.";
        case FMOD_ERR_INVALID_HANDLE:            return "An invalid object handle was used.";
        case FMOD_ERR_INVALID_PARAM:             return "An invalid parameter was passed to this function.";
        case FMOD_ERR_INVALID_POSITION:          return "An invalid seek position was passed to this function.";
        case FMOD_ERR_INVALID_SPEAKER:           return "An invalid speaker was passed to this function based on the current speaker mode.";
        case FMOD_ERR_INVALID_SYNCPOINT:         return "The syncpoint did not come from this sound handle.";
        case FMOD_ERR_INVALID_THREAD:            return "Tried to call a function on a thread that is not supported.";
        case FMOD_ERR_INVALID_VECTOR:            return "The vectors passed in are not unit length, or perpendicular.";
        case FMOD_ERR_MAXAUDIBLE:                return "Reached maximum audible playback count for this sound's soundgroup.";
        case FMOD_ERR_MEMORY:                    return "Not enough memory or resources.";
        case FMOD_ERR_MEMORY_CANTPOINT:          return "Can't use FMOD_OPENMEMORY_POINT on non PCM source data, or non mp3/xma/adpcm data if FMOD_CREATECOMPRESSEDSAMPLE was used.";
        case FMOD_ERR_NEEDS3D:                   return "Tried to call a command on a 2d sound when the command was meant for 3d sound.";
        case FMOD_ERR_NEEDSHARDWARE:             return "Tried to use a feature that requires hardware support.";
        case FMOD_ERR_NET_CONNECT:               return "Couldn't connect to the specified host.";
        case FMOD_ERR_NET_SOCKET_ERROR:          return "A socket error occurred.  This is a catch-all for socket-related errors not listed elsewhere.";
        case FMOD_ERR_NET_URL:                   return "The specified URL couldn't be resolved.";
        case FMOD_ERR_NET_WOULD_BLOCK:           return "Operation on a non-blocking socket could not complete immediately.";
        case FMOD_ERR_NOTREADY:                  return "Operation could not be performed because specified sound/DSP connection is not ready.";
        case FMOD_ERR_OUTPUT_ALLOCATED:          return "Error initializing output device, but more specifically, the output device is already in use and cannot be reused.";
        case FMOD_ERR_OUTPUT_CREATEBUFFER:       return "Error creating hardware sound buffer.";
        case FMOD_ERR_OUTPUT_DRIVERCALL:         return "A call to a standard soundcard driver failed, which could possibly mean a bug in the driver or resources were missing or exhausted.";
        case FMOD_ERR_OUTPUT_FORMAT:             return "Soundcard does not support the specified format.";
        case FMOD_ERR_OUTPUT_INIT:               return "Error initializing output device.";
        case FMOD_ERR_OUTPUT_NODRIVERS:          return "The output device has no drivers installed.  If pre-init, FMOD_OUTPUT_NOSOUND is selected as the output mode.  If post-init, the function just fails.";
        case FMOD_ERR_PLUGIN:                    return "An unspecified error has been returned from a plugin.";
        case FMOD_ERR_PLUGIN_MISSING:            return "A requested output, dsp unit type or codec was not available.";
        case FMOD_ERR_PLUGIN_RESOURCE:           return "A resource that the plugin requires cannot be found. (ie the DLS file for MIDI playback)";
        case FMOD_ERR_PLUGIN_VERSION:            return "A plugin was built with an unsupported SDK version.";
        case FMOD_ERR_RECORD:                    return "An error occurred trying to initialize the recording device.";
        case FMOD_ERR_REVERB_CHANNELGROUP:       return "Reverb properties cannot be set on this channel because a parent channelgroup owns the reverb connection.";
        case FMOD_ERR_REVERB_INSTANCE:           return "Specified instance in FMOD_REVERB_PROPERTIES couldn't be set. Most likely because it is an invalid instance number or the reverb doesn't exist.";
        case FMOD_ERR_SUBSOUNDS:                 return "The error occurred because the sound referenced contains subsounds when it shouldn't have, or it doesn't contain subsounds when it should have.  The operation may also not be able to be performed on a parent sound.";
        case FMOD_ERR_SUBSOUND_ALLOCATED:        return "This subsound is already being used by another sound, you cannot have more than one parent to a sound.  Null out the other parent's entry first.";
        case FMOD_ERR_SUBSOUND_CANTMOVE:         return "Shared subsounds cannot be replaced or moved from their parent stream, such as when the parent stream is an FSB file.";
        case FMOD_ERR_TAGNOTFOUND:               return "The specified tag could not be found or there are no tags.";
        case FMOD_ERR_TOOMANYCHANNELS:           return "The sound created exceeds the allowable input channel count.  This can be increased using the 'maxinputchannels' parameter in System::setSoftwareFormat.";
        case FMOD_ERR_TRUNCATED:                 return "The retrieved string is too long to fit in the supplied buffer and has been truncated.";
        case FMOD_ERR_UNIMPLEMENTED:             return "Something in FMOD hasn't been implemented when it should be! contact support!";
        case FMOD_ERR_UNINITIALIZED:             return "This command failed because System::init or System::setDriver was not called.";
        case FMOD_ERR_UNSUPPORTED:               return "A command issued was not supported by this object.  Possibly a plugin without certain callbacks specified.";
        case FMOD_ERR_VERSION:                   return "The version number of this file format is not supported.";
        case FMOD_ERR_EVENT_ALREADY_LOADED:      return "The specified bank has already been loaded.";
        case FMOD_ERR_EVENT_LIVEUPDATE_BUSY:     return "The live update connection failed due to the game already being connected.";
        case FMOD_ERR_EVENT_LIVEUPDATE_MISMATCH: return "The live update connection failed due to the game data being out of sync with the tool.";
        case FMOD_ERR_EVENT_LIVEUPDATE_TIMEOUT:  return "The live update connection timed out.";
        case FMOD_ERR_EVENT_NOTFOUND:            return "The requested event, bus or vca could not be found.";
        case FMOD_ERR_STUDIO_UNINITIALIZED:      return "The Studio::System object is not yet initialized.";
        case FMOD_ERR_STUDIO_NOT_LOADED:         return "The specified resource is not loaded, so it can't be unloaded.";
        case FMOD_ERR_INVALID_STRING:            return "An invalid string was passed to this function.";
        case FMOD_ERR_ALREADY_LOCKED:            return "The specified resource is already locked.";
        case FMOD_ERR_NOT_LOCKED:                return "The specified resource is not locked, so it can't be unlocked.";
        case FMOD_ERR_RECORD_DISCONNECTED:       return "The specified recording driver has been disconnected.";
        case FMOD_ERR_TOOMANYSAMPLES:            return "The length provided exceeds the allowable limit.";
        default :                                return "Unknown error.";
    };
}



static void check_error(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        Con_Printf("FMOD error %d - %s", result, FMOD_ErrorString(result));
        assert(0);
    }
}

static void load(const char* name, sfx_t* dest)
{
    
    if (!sound.system) {
        return;
    }
    
    S_LoadSound(dest);
    
    sfxcache_t* cached = dest->cache.data;
    
    FMOD_RESULT result;
    FMOD_MODE mode = FMOD_OPENMEMORY_POINT | FMOD_OPENRAW | FMOD_3D;
    
    FMOD_CREATESOUNDEXINFO e = {}; // init to zero.
    
    e.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    e.length = cached->length;
    e.numchannels = 1;
    e.defaultfrequency = cached->speed;
    
    switch (cached->width) {
        case 1:
            e.format = FMOD_SOUND_FORMAT_PCM8;
            break;
            
        case 2:
            e.format = FMOD_SOUND_FORMAT_PCM16;
            break;
            
        default:
            assert(0);
    }
    
    
    
    
    
    
    FMOD_SOUND* newSound = NULL;
    
    result = FMOD_System_CreateSound(sound.system, cached->data, mode, &e, &newSound);
    check_error(result);
    
    result = FMOD_Sound_Set3DMinMaxDistance(newSound, 0.5f * INCHES_PER_METER, 5000.0f * INCHES_PER_METER);
    check_error(result);
    
    dest->fmod_sound = newSound;
    
    /*const int format = AL_FORMAT_MONO16;
    
    ALuint buffer = 0;
    alGenBuffers(1, &buffer);
    check_error();
    dest->openal_buffer = buffer;
    alBufferData(buffer, format, cached->data, cached->length, cached->speed / 2);
    check_error();
     */
    
    // TODO: load sample
    
    printf("loading %s\n", name);
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
    assert(sound.system);

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
    
    FMOD_VECTOR fmpos = FromVec3(origin);
    FMOD_VECTOR fmvel = {};
    FMOD_VECTOR fmforward = FromVec3(forward);
    FMOD_VECTOR fmup = FromVec3(up);
    
    FMOD_RESULT result;
    result = FMOD_System_Set3DListenerAttributes(sound.system, 0, &fmpos, &fmvel, &fmforward, &fmup);
    
    check_error(result);

    float orientation[6];
    for (int i=0; i<3; i++) {
        orientation[i]   = forward[i];
        orientation[i+3] = up[i];
    }
    
    // alListenerfv(AL_POSITION,    origin);
    // alListenerfv(AL_ORIENTATION, orientation);
    
    result = FMOD_System_Update(sound.system);
    check_error(result);
    
    // TODO: FMOD_SYSTEM_CALLBACK
}

void S_StopAllSounds (qboolean clear)
{
    if (!sound.system)
        return;

    total_channels = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;	// no statics

    for (int i=0; i<MAX_CHANNELS; i++) {
        if (channels[i].channel) {
            FMOD_RESULT result = FMOD_OK;
            
            if (channels[i].sfx) { // added by marc
                //result = FMOD_Channel_SetPaused(channels[i].channel, true);
                if (result != FMOD_ERR_CHANNEL_STOLEN) {
                    check_error(result);
                }
                
            }
            
        }
        //alSourceStop(channels[i].openal_source);
        //check_error();
        // TODO: stop all sounds

    }

    // TODO: Stop all sources
    for (int i=0; i<sound.num_sfx; i++) {
        sfx_t* sfx = sound.known_sfx + i;
//        alDeleteBuffers(1, &sfx->openal_buffer);
//        check_error();
        
        
        // TODO: destroy sound
        FMOD_RESULT result = FMOD_Sound_Release(sfx->fmod_sound);
        check_error(result);
        sfx->fmod_sound = NULL;
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

void S_LocalSound (char *name)
{
	if (nosound.value)
		return;

	if (!sound.system)
		return;

	const sfx_t* sfx = S_PrecacheSound (name);
	if (!sfx)
	{
		Con_Printf ("S_LocalSound: can't cache %s\n", sound);
		return;
	}
	S_StartSound (cl.viewentity, -1, sfx, vec3_origin, 1, 1);
}

