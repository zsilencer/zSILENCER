#include "baseexit.h"

BaseExit::BaseExit() : Object(ObjectTypes::BASEEXIT){
	requiresauthority = true;
	teamid = 0;
	soundchannel = -1;
	draw = false;
}

void BaseExit::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, teamid, old);
}

void BaseExit::Tick(World & world){
	if(soundchannel == -1){
		soundchannel = Audio::GetInstance().EmitSound(id, world.resources.soundbank["wndloopc.wav"], 32, true);
	}
}