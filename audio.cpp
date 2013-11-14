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
		//Mix_SetPostMix(MixingFunction, 0);
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

int Audio::Play(Mix_Chunk * chunk, Uint8 volume, bool loop){
	if(enabled && chunk){
		int loops = 0;
		if(loop){
			loops = -1;
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

void Audio::Pause(int channel){
	Mix_Pause(channel);
}

void Audio::Resume(int channel){
	Mix_Resume(channel);
}

bool Audio::Paused(int channel){
	return Mix_Paused(channel);
}

int Audio::EmitSound(World & world, Uint16 objectid, Mix_Chunk * chunk, Uint8 volume, bool loop){
	int channel = Play(chunk, volume * effectvolume, loop);
	if(channel != -1){
		//Mix_Volume(channel, 0);
		channelobject[channel] = objectid;
		UpdateVolume(world, channel, lastx, lasty, 500);
	}
	return channel;
}

void Audio::UpdateVolume(World & world, int channel, Sint16 x, Sint16 y, int radius){
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
			lastx = x;
			lasty = y;
		}
	}
}

void Audio::UpdateAllVolumes(World & world, Sint16 x, Sint16 y, int radius){
	for(int i = 0; i < maxchannels; i++){
		int channel = i;
		UpdateVolume(world, channel, x, y, radius);
	}
}

void Audio::SetVolume(int channel, Uint8 volume){
	Mix_Volume(channel, volume * effectvolume);
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

void Audio::MixingFunction(void * udata, Uint8 * stream, int len){
	
}