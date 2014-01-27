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
	int Play(Mix_Chunk * chunk, int volume = 128, bool loop = false);
	void Stop(int channel, int fadeoutms = 0);
	void StopAll(int fadeoutms = 0);
	void Pause(int channel);
	void Resume(int channel);
	bool Paused(int channel);
	int EmitSound(class World & world, Uint16 objectid, Mix_Chunk * chunk, int volume = 128, bool loop = false);
	void UpdateVolume(class World & world, int channel, Sint16 x, Sint16 y, int radius);
	void UpdateAllVolumes(class World & world, Sint16 x, Sint16 y, int radius);
	void SetVolume(int channel, int volume);
	void Mute(int volume);
	void Unmute(void);
	void PlayMusic(Mix_Music * music);
	void StopMusic(void);
	void PauseMusic(void);
	void ResumeMusic(void);
	void SetMusicVolume(int volume);
	
	bool enabled;
	
private:
	static void ChannelFinished(int channel);
	static void MixingFunction(void * udata, Uint8 * stream, int len);
	
	static const int maxchannels = 128;
	int channelobject[maxchannels];
	int channelvolume[maxchannels];
	float effectvolume;
	Sint16 lastx, lasty;
	int musicvolume;
};

#endif