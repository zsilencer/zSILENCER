#include "blasterprojectile.h"
#include "shrapnel.h"
#include "overlay.h"

BlasterProjectile::BlasterProjectile() : Object(ObjectTypes::BLASTERPROJECTILE){
	requiresauthority = true;
	res_bank = 0xFF;
	res_index = 0;
	state_i = 0;
	healthdamage = 40;
	shielddamage = 4;
	moveamount = 10;
	renderpass = 2;
	isprojectile = true;
	isphysical = true;
	snapshotinterval = 6;
}

void BlasterProjectile::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
}

void BlasterProjectile::Tick(World & world){
	if(yv < 0 && xv == 0){ // up
		res_bank = 160;
	}
	if(yv < 0 && xv > 0){ // up right
		res_bank = 161;
	}
	if(yv == 0 && xv > 0){ // right
		res_bank = 162;
	}
	if(yv > 0 && xv > 0){ // down right
		res_bank = 163;
	}
	if(yv > 0 && xv == 0){ // down
		res_bank = 164;
	}
	if(yv > 0 && xv < 0){ // down left
		res_bank = 163;
	}
	if(yv == 0 && xv < 0){ // left
		res_bank = 162;
	}
	if(yv < 0 && xv < 0){ // up left
		res_bank = 161;
	}
	Uint8 life = 3;
	if(state_i == 4){
		Audio::GetInstance().EmitSound(id, world.resources.soundbank["!laserme.wav"], 128);
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
				if(rand() % 2 == 0){
					Audio::GetInstance().EmitSound(overlay->id, world.resources.soundbank["rico1.wav"], 32);
				}else{
					Audio::GetInstance().EmitSound(overlay->id, world.resources.soundbank["rico2.wav"], 32);
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