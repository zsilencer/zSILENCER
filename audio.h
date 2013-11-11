#ifndef AUDIO_H
#define AUDIO_H

#include "shared.h"
#include <map>
#include <string>

class Audio
{
public:
	Audio();
	~Audio();
	static Audio & GetInstance(void);
	bool Init(void);
	void Close(void);
	int Play(Mix_Chunk * chunk, Uint8 volume = 128, bool loop = false);
	void Stop(int channel, int fadeoutms = 0);
	void StopAll(int fadeoutms = 0);
	void Pause(int channel);
	void Resume(int channel);
	bool Paused(int channel);
	int EmitSound(Uint16 objectid, Mix_Chunk * chunk, Uint8 volume = 128, bool loop = false);
	void UpdateVolume(class World & world, int channel, Sint16 x, Sint16 y, int radius);
	void UpdateAllVolumes(class World & world, Sint16 x, Sint16 y, int radius);
	void SetVolume(int channel, Uint8 volume);
	void Mute(Uint8 volume);
	void Unmute(void);
	void PlayMusic(Mix_Music * music);
	void StopMusic(void);
	
	bool enabled;
	bool musicenabled;
	
private:
	static void ChannelFinished(int channel);
	static void MixingFunction(void * udata, Uint8 * stream, int len);
	
	static const int maxchannels = 1024;
	int channelobject[maxchannels];
	int channelvolume[maxchannels];
	float effectvolume;
};

#endif