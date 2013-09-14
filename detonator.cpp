#include "detonator.h"
#include "plasmaprojectile.h"

Detonator::Detonator() : Object(ObjectTypes::DETONATOR){
	requiresauthority = true;
	requiresmaptobeloaded = true;
	state_i = 0;
	ownerid = 0;
	res_bank = 182;
	res_index = 0;
	originaly = y;
	renderpass = 1;
}

void Detonator::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	Sprite::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, ownerid, old);
	data.Serialize(write, originaly, old);
}

void Detonator::Tick(World & world){
	// 182:0-3 det/camera
	if(state_i == 0){
		Audio::GetInstance().EmitSound(id, world.resources.soundbank["shield2.wav"], 96);
	}
	res_index = (state_i / 4) % 4;
	if(state_i == 4 * 4){
		state_i = 0;
	}
	if(state_i > 4 * 4){
		if(state_i >= (4 * 4) + 6){
			if(state_i == (4 * 4) + 6){
				// explode
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["seekexp1.wav"], 128);
				Sint8 xvs[] = {-14, 14, -10, 10, -10, 10};
				Sint8 yvs[] = {-25, -25, 0, 0, 5, 5};
				Sint8 ys[] = {0, 0, 0, 0, 0, 0, 0, 0};
				for(int i = 0; i < 6; i++){
					PlasmaProjectile * plasmaprojectile = (PlasmaProjectile *)world.CreateObject(ObjectTypes::PLASMAPROJECTILE);
					if(plasmaprojectile){
						plasmaprojectile->large = false;
						plasmaprojectile->x = x;
						plasmaprojectile->y = y + ys[i];
						plasmaprojectile->ownerid = ownerid;
						plasmaprojectile->xv = xvs[i];
						plasmaprojectile->yv = yvs[i];
					}
				}
			}
			if(state_i == (4 * 4) + 8){
				// explode
				Sint8 xvs[] = {-14, 0, 14, -5, 0, 5};
				Sint8 yvs[] = {-5, -10, -5, 5, 5, 5};
				Sint8 ys[] = {0, 0, 0, 0, 0, 0};
				for(int i = 0; i < 6; i++){
					PlasmaProjectile * plasmaprojectile = (PlasmaProjectile *)world.CreateObject(ObjectTypes::PLASMAPROJECTILE);
					if(plasmaprojectile){
						plasmaprojectile->large = true;
						plasmaprojectile->x = x;
						plasmaprojectile->y = y + ys[i];
						plasmaprojectile->ownerid = ownerid;
						plasmaprojectile->xv = xvs[i];
						plasmaprojectile->yv = yvs[i];
					}
				}
			}
			draw = false;
			if(state_i >= (4 * 4) + 50){
				world.MarkDestroyObject(id);
			}
		}else{
			int velocity = (((4 * 4) - (state_i)) * 2) + 20;
			if(velocity < 0){
				velocity = 0;
			}
			y -= velocity;
		}
	}
	state_i++;
}

void Detonator::Detonate(void){
	if(state_i <= 4 * 4){
		state_i = (4 * 4) + 1;
	}
}

bool Detonator::HasDetonated(void){
	if(state_i >= (4 * 4) + 1){
		return true;
	}
	return false;
}