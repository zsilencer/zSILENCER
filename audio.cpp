#include "audio.h"
#include "world.h"
#include <math.h>

Audio::Audio(){
	enabled = true;
	effectvolume = 1;
	musicenabled = false;
}

Audio::~Audio(){
	Close();
}

Audio & Audio::GetInstance(void){
	static Audio instance;
	return instance;
}

bool Audio::Init(void){
	if(Mix_OpenAudio(44100, AUDIO_S16, 1, 1024) == -1){
		return false;
	}else{
		Mix_AllocateChannels(maxchannels);
		Mix_ChannelFinished(ChannelFinished);
		return true;
	}
}

void Audio::Close(void){
	enabled = false;
	int freq;
	Uint16 format;
	int channels;
	while(Mix_QuerySpec(&freq, &format, &channels)){
		Mix_CloseAudio();
	}
}

int Audio::Play(Mix_Chunk * chunk, Uint8 volume, bool loop, int startms, int endms){
	if(enabled && chunk){
		int loops = 0;
		if(loop){
			loops = -1;
		}
		if(startms != 0 || endms != -1){
			char chunkuniqstr[256];
			sprintf(chunkuniqstr, "0x%X:%d:%d", (unsigned int)chunk, startms, endms);
			if(!chunkcopies[chunkuniqstr]){
				int start = (startms * ((22050 * 2) / 1000.0));
				if(start > chunk->alen){
					start = chunk->alen;
				}
				int length = (endms * ((22050 * 2) / 1000.0)) - start;
				if(length > chunk->alen - start){
					length = chunk->alen - start;
				}
				if(length < 0){
					length = 0;
				}
				Mix_Chunk * newchunk = Mix_QuickLoad_RAW(&chunk->abuf[start], length);
				chunkcopies[chunkuniqstr] = newchunk;
			}
			chunk = chunkcopies[chunkuniqstr];
		}
		int channel = Mix_PlayChannel(-1, chunk, loops);
		if(channel >= 0){
			Mix_Volume(channel, volume * effectvolume);
			channelvolume[channel] = volume;
		}
		return channel;
	}else{
		return -1;
	}
}

void Audio::Stop(int channel, int fadeoutms){
	if(fadeoutms){
		Mix_FadeOutChannel(channel, fadeoutms);
		Mix_ExpireChannel(channel, fadeoutms);
	}else{
		Mix_HaltChannel(channel);
	}
}

void Audio::StopAll(int fadeoutms){
	Stop(-1, fadeoutms);
}

int Audio::EmitSound(Uint16 objectid, Mix_Chunk * chunk, Uint8 volume, bool loop, int startms, int endms){
	int channel = Play(chunk, volume * effectvolume, loop, startms, endms);
	if(channel != -1){
		Mix_Volume(channel, 0);
		channelobject[channel] = objectid;
	}
	return channel;
}

void Audio::UpdateVolume(World & world, int channel, Uint32 x, Uint32 y, Uint32 radius){
	Uint16 objectid = channelobject[channel];
	if(objectid){
		Object * object = world.GetObjectFromId(objectid);
		if(object){
			int diffx = abs(signed(x) - object->x);
			int diffy = abs(signed(y) - object->y);
			float distance = abs(sqrt(float((diffx * diffx) + (diffy * diffy))));
			float volume = 1 - (distance / radius);
			if(volume < 0){
				volume = 0;
			}
			if(volume > 1){
				volume = 1;
			}
			int oldvolume = channelvolume[channel];
			Mix_Volume(channel, (oldvolume * volume) * effectvolume);
		}
	}
}

void Audio::UpdateAllVolumes(World & world, Uint32 x, Uint32 y, Uint32 radius){
	for(int i = 0; i < maxchannels; i++){
		int channel = i;
		UpdateVolume(world, channel, x, y, radius);
	}
}

void Audio::Mute(Uint8 volume){
	float percent = volume / 128.0;
	effectvolume = percent;
	for(int i = 0; i < maxchannels; i++){
		int oldvolume = channelvolume[i];
		Mix_Volume(i, oldvolume * percent);
	}
	Mix_VolumeMusic(volume);
}

void Audio::Unmute(void){
	effectvolume = 1;
	for(int i = 0; i < maxchannels; i++){
		int oldvolume = channelvolume[i];
		Mix_Volume(i, oldvolume);
	}
	Mix_VolumeMusic(128);
}

void Audio::PlayMusic(Mix_Music * music){
	if(!musicenabled){
		return;
	}
	if(!Mix_PlayingMusic()){
		Mix_PlayMusic(music, -1);
	}
}

void Audio::StopMusic(void){
	if(!musicenabled){
		return;
	}
	Mix_FadeOutMusic(700);
}

void Audio::ChannelFinished(int channel){
	Audio::GetInstance().channelobject[channel] = 0;
}