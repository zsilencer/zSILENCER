#include "flareprojectile.h"
#include "plume.h"

FlareProjectile::FlareProjectile() : Object(ObjectTypes::FLAREPROJECTILE){
	//requiresauthority = true;
	res_bank = 0xFF;
	res_index = 0;
	state_i = 0;
	healthdamage = 1;
	shielddamage = 1;
	velocity = 5;
	drawcheckered = true;
	for(int i = 0; i < plumecount; i++){
		plumeids[i] = 0;
	}
	moveamount = 1;
	soundplaying = 0;
	renderpass = 2;
	stopatobjectcollision = false;
	isprojectile = true;
	isphysical = true;
	poisonous = false;
	//snapshotinterval = 6;
}

void FlareProjectile::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, originalx, old);
	data.Serialize(write, originaly, old);
	data.Serialize(write, poisonous, old);
}

void FlareProjectile::Tick(World & world){
	for(int i = 0; i < plumecount; i++){
		if(!plumeids[i]){
			Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
			if(plume){
				plume->type = 4;
				plume->xv = (rand() % 17) - 8 + (xv * 8);
				plume->yv = (rand() % 17) - 8 + (yv * 8);
				if(poisonous){
					plume->effectcolor = 224;
				}
				plume->SetPosition(originalx + xv, originaly + yv);
				plumeids[i] = plume->id;
				plume->state_i = 0;
				break;
			}
		}
	}
	Object * object = 0;
	Platform * platform = 0;
	if(TestCollision(*this, world, &platform, &object)){
		float xn = 0, yn = 0;
		if(platform){
			platform->GetNormal(x, y, &xn, &yn);
			for(int i = 0; i < plumecount; i++){
				Plume * plume = (Plume *)world.GetObjectFromId(plumeids[i]);
				if(plume){
					plume->xv /= 3;
					plume->yv /= 3;
					//world->MarkDestroyObject(plumeids[i]);
				}
			}
		}
		//world->MarkDestroyObject(id);
	}
	res_index = state_i;
	if(state_i >= 14){
		world.MarkDestroyObject(id);
	}
	state_i++;
}