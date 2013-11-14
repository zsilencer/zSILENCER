#include "wallprojectile.h"
#include "overlay.h"
#include "shrapnel.h"

WallProjectile::WallProjectile() : Object(ObjectTypes::WALLPROJECTILE){
	requiresauthority = true;
	res_bank = 0xFF;
	res_index = 0;
	state_i = 0;
	healthdamage = 10;
	shielddamage = 60;
	velocity = 35;
	emitoffset = 0;
	moveamount = 6;
	renderpass = 2;
	isprojectile = true;
	isphysical = true;
	snapshotinterval = 6;
}

void WallProjectile::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
}

void WallProjectile::Tick(World & world){
	Uint8 life = 20;
	if(state_i == 1){
		EmitSound(world, world.resources.soundbank["!laserel.wav"], 128);
	}
	if(state_i < 7){
		res_index = state_i;
	}
	if(state_i >= 7){
		if(state_i > 12 + life){
			world.MarkDestroyObject(id);
			res_index = 12;
			return;
		}
		if(state_i >= 12 + life - 5){
			res_index = state_i - life;
		}else{
			res_index = 7;
		}
	}
	if(state_i >= 7){
		Object * object = 0;
		Platform * platform = 0;
		if(TestCollision(*this, world, &platform, &object)){
			Overlay * overlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
			if(overlay){
				overlay->res_bank = 222;
				overlay->x = x;
				overlay->y = y;
				if(platform){
					if(rand() % 2 == 0){
						overlay->EmitSound(world, world.resources.soundbank["strike03.wav"], 96);
					}else{
						overlay->EmitSound(world, world.resources.soundbank["strike04.wav"], 96);
					}
				}
			}
			float xn = 0, yn = 0;
			if(platform){
				platform->GetNormal(x, y, &xn, &yn);
			}
			for(int i = 0; i < 8; i++){
				Shrapnel * shrapnel = (Shrapnel *)world.CreateObject(ObjectTypes::SHRAPNEL);
				if(shrapnel){
					shrapnel->x = x;
					shrapnel->y = y;
					shrapnel->xv = (rand() % 9) - 4;
					shrapnel->yv = (rand() % 9) - 8;
					shrapnel->xv = (xn * abs(shrapnel->xv)) + (rand() % 9) - 4;
					shrapnel->yv = (yn * abs(shrapnel->yv)) + (rand() % 9) - 8;
				}
			}
			world.MarkDestroyObject(id);
		}
	}
	state_i++;
}