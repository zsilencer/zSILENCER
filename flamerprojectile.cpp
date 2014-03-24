#include "flamerprojectile.h"
#include "plume.h"

FlamerProjectile::FlamerProjectile() : Object(ObjectTypes::FLAMERPROJECTILE){
	requiresauthority = true;
	res_bank = 0xFF;
	res_index = 0;
	state_i = 0;
	bypassshield = true;
	healthdamage = 2;
	shielddamage = 1;
	velocity = 7;
	drawcheckered = true;
	for(int i = 0; i < plumecount; i++){
		plumeids[i] = 0;
	}
	emitoffset = -7;
	moveamount = 6;
	soundplaying = 0;
	renderpass = 2;
	radius = 10;
	stopatobjectcollision = false;
	isprojectile = true;
	isphysical = true;
	snapshotinterval = 6;
	hitonce = false;
}

void FlamerProjectile::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
}

void FlamerProjectile::Tick(World & world){
	for(int i = 0; i < plumecount; i++){
		if(!plumeids[i]){
			Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
			if(plume){
				plume->type = 4;
				plume->xv = (rand() % 17) - 8 + (xv * 12);
				plume->yv = (rand() % 17) - 8 + (yv * 12);
				plume->SetPosition(x - (xv * ((i + 1) * 1)), y - (yv * ((i + 1) * 1)));
				plumeids[i] = plume->id;
				//plume->state_i = i;
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
			xv /= 3;
			yv /= 3;
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