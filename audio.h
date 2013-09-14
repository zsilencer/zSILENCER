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
	int Play(Mix_Chunk * chunk, Uint8 volume = 128, bool loop = false, int startms = 0, int endms = -1);
	void Stop(int channel, int fadeoutms = 0);
	void StopAll(int fadeoutms = 0);
	int EmitSound(Uint16 objectid, Mix_Chunk * chunk, Uint8 volume = 128, bool loop = false, int startms = 0, int endms = -1);
	void UpdateVolume(class World & world, int channel, Uint32 x, Uint32 y, Uint32 radius);
	void UpdateAllVolumes(class World & world, Uint32 x, Uint32 y, Uint32 radius);
	void Mute(Uint8 volume);
	void Unmute(void);
	void PlayMusic(Mix_Music * music);
	void StopMusic(void);
	
	bool enabled;
	bool musicenabled;
	
private:
	static void ChannelFinished(int channel);
	
	static const int maxchannels = 64;
	int channelobject[maxchannels];
	int channelvolume[maxchannels];
	std::map<std::string, Mix_Chunk *> chunkcopies;
	float effectvolume;
};

#endif