#include "vent.h"
#include "plume.h"

Vent::Vent() : Object(ObjectTypes::VENT){
	requiresauthority = false;
	res_bank = 179;
	active = 0;
}

void Vent::Tick(World & world){
	if(active >= 1){
		if(active == 1){
			EmitSound(world, world.resources.soundbank["airvent2.wav"], 96);
		}
		if(active <= 18){
			for(int i = 0; i < 4; i++){
				Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
				if(plume){
					plume->type = rand() % 2;
					plume->cycle = true;
					plume->renderpass = 2;
					plume->SetPosition(x + (rand() % 80 - 40), y - (rand() % 8) + 3);
					plume->yv = -(rand() % 20) - 30;
					plume->xv = (rand() % 5) - 2;
				}
			}
		}
		active++;
		if(active >= 20){
			active = 0;
		}
	}
	res_index = (state_i / 4) % 8;
	state_i++;
}

void Vent::Activate(void){
	if(active == 0){
		active = 1;
	}
}