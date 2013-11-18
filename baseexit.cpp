#include "baseexit.h"

BaseExit::BaseExit() : Object(ObjectTypes::BASEEXIT){
	requiresauthority = false;
	teamid = 0;
	soundchannel = -1;
	draw = false;
}

void BaseExit::Tick(World & world){
	if(soundchannel == -1){
		soundchannel = EmitSound(world, world.resources.soundbank["wndloopc.wav"], 32, true);
	}
}