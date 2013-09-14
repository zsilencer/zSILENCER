#include "techstation.h"
#include "plume.h"

TechStation::TechStation() : Object(ObjectTypes::TECHSTATION){
	res_bank = 106;
	res_index = 0;
	type = 0;
	health = 240;
	shield = 240;
	ishittable = true;
	isphysical = true;
	requiresauthority = true;
}

void TechStation::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, type, old);
}

void TechStation::Tick(World & world){
	Hittable::Tick(*this, world);
}

void TechStation::HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile){
	Hittable::HandleHit(*this, world, x, y, projectile);
	if(health == 0){
		collidable = false;
		Audio::GetInstance().EmitSound(id, world.resources.soundbank["q_expl02.wav"], 128);
		for(int i = 0; i < 24; i++){
			Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
			if(plume){
				if(rand() % 2 == 0){
					plume->type = 6;
				}else{
					plume->type = 5;
				}
				plume->renderpass = renderpass;
				plume->SetPosition(TechStation::x + (rand() % 101) - 40, TechStation::y - (rand() % 120));
			}
		}
	}
}