#include "rocketprojectile.h"
#include "plume.h"
#include "player.h"
#include <math.h>

RocketProjectile::RocketProjectile() : Object(ObjectTypes::ROCKETPROJECTILE){
	requiresauthority = true;
	res_bank = 0xFF;
	res_index = 0;
	state_i = 0;
	healthdamage = 75;
	shielddamage = 25;
	moveamount = 15;
	velocity = 35;
	renderpass = 2;
	isprojectile = true;
	isphysical = true;
	snapshotinterval = 6;
	soundchannel = 0;
}

void RocketProjectile::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	//Projectile::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, oldxv, old);
	data.Serialize(write, oldyv, old);
}

void RocketProjectile::Tick(World & world){
	if(yv < 0 && xv == 0){ // up
		res_index = 4;
	}
	if(yv < 0 && xv > 0){ // up right
		res_index = 3;
		mirrored = false;
	}
	if(yv == 0 && xv > 0){ // right
		res_index = 0;
		mirrored = false;
	}
	if(yv > 0 && xv > 0){ // down right
		res_index = 1;
		mirrored = false;
	}
	if(yv > 0 && xv == 0){ // down
		res_index = 2;
	}
	if(yv > 0 && xv < 0){ // down left
		res_index = 1;
		mirrored = true;
	}
	if(yv == 0 && xv < 0){ // left
		res_index = 0;
		mirrored = true;
	}
	if(yv < 0 && xv < 0){ // up left
		res_index = 3;
		mirrored = true;
	}
	if(state_i == 0){
		oldxv = xv;
		oldyv = yv;
		xv = ceil(float(xv) * 0.2);
		yv = ceil(float(yv) * 0.2);
		soundchannel = Audio::GetInstance().EmitSound(id, world.resources.soundbank["rocket9.wav"], 128);
	}
	if(state_i == 3){
		res_bank = 87;
	}
	if(state_i == 11){
		if(xv > 0){
			xv++;
		}else{
			xv--;
		}
		if(yv > 0){
			yv++;
		}else{
			yv--;
		}
		if(abs(xv) > abs(oldxv)){
			xv = oldxv;
		}
		if(abs(yv) > abs(oldyv)){
			yv = oldyv;
		}
		Platform * platform = 0;
		Object * object = 0;
		Sint16 oldx = x;
		Sint16 oldy = y;
		if(TestCollision(*this, world, &platform, &object)){
			float xn = 0, yn = 0;
			if(platform){
				platform->GetNormal(x, y, &xn, &yn);
			}
			int numplumes = 6;
			for(int i = 0; i < numplumes; i++){
				Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
				if(plume){
					plume->type = 4;
					/*plume->xv = (rand() % 17) - 8;
					plume->yv = (rand() % 17) - 8;
					plume->xv = (xn * abs(plume->xv)) + (rand() % 33) - 16;
					plume->yv = (yn * abs(plume->yv)) + (rand() % 33) - 16;*/
					float angle = (i / float(numplumes)) * (2 * 3.14);
					plume->xv = (sin(angle)) * 15;
					plume->yv = (cos(angle)) * 15;
					if(xn || yn){
						plume->xv = (xn * abs(plume->xv)) + (rand() % 17) - 8;
						plume->yv = (yn * abs(plume->yv)) + (rand() % 17) - 8;
					}
					plume->SetPosition(x, y);
					
					Plume * plume2 = (Plume *)world.CreateObject(ObjectTypes::PLUME);
					if(plume2){
						plume2->type = 4;
						plume2->xv = plume->xv + (rand() % 7) - 3;
						plume2->yv = plume->yv + (rand() % 7) - 3;
						plume2->SetPosition(x, y);
					}
				}
			}
			xv = 0;
			yv = 0;
			res_bank = 0xFF;
			if(soundchannel){
				Audio::GetInstance().Stop(soundchannel, 100);
			}
			Audio::GetInstance().EmitSound(id, world.resources.soundbank["seekexp1.wav"], 128);
		}
		for(int i = 0; i < 2; i++){
			int xv2 = (signed(oldx) - x) * (1.25 * i);
			int yv2 = (signed(oldy) - y) * (1.25 * i);
			Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
			if(plume){
				plume->type = 3;
				plume->xv = rand() % 7 - 3;
				plume->yv = rand() % 7 - 3;
				plume->SetPosition(x + xv2, y + yv2);
			}
		}
	}
	if(state_i < 11 || (state_i >= 11 && xv == 0 && yv == 0)){
		state_i++;
	}
	if(state_i >= 25){
		world.MarkDestroyObject(id);
	}else{
		Player * localplayer = world.GetPeerPlayer(world.localpeerid);
		if(localplayer && ownerid == localplayer->id){
			if(!world.systemcameraactive[0]){
				world.SetSystemCamera(0, id, 0, 20);
			}
		}
	}
}

bool RocketProjectile::JustHit(void){
	if(state_i == 12){
		return true;
	}
	return false;
}