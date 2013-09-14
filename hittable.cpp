#include "hittable.h"
#include "object.h"
#include "projectile.h"

Hittable::Hittable(){
	state_hit = 0;
	hitx = 0;
	hity = 0;
	health = 0;
	shield = 0;
}

void Hittable::Serialize(bool write, Serializer & data, Serializer * old){
	data.Serialize(write, state_hit, old);
	data.Serialize(write, health, old);
	data.Serialize(write, shield, old);
	data.Serialize(write, hitx, old);
	data.Serialize(write, hity, old);
}

void Hittable::Tick(Object & object, World & world){
	if(state_hit){
		state_hit++;
	}
	if(state_hit % 32 == 31){
		state_hit = 0;
	}
}

void Hittable::HandleHit(Object & object, World & world, Uint8 x, Uint8 y, Object & projectile){
	if(!shield || projectile.bypassshield){
		if(health - projectile.healthdamage < 0){
			health = 0;
		}else{
			health -= projectile.healthdamage;
		}
		state_hit = 1 + (0 * 32);
	}else{
		if(shield - projectile.shielddamage <= 0){
			int more = abs(shield - projectile.shielddamage);
			more = (float(more) / projectile.shielddamage) * projectile.healthdamage;
			shield = 0;
			if(health - more < 0){
				health = 0;
			}else{
				health -= more;
			}
			state_hit = 1 + (1 * 32);
		}else{
			shield -= projectile.shielddamage;
			state_hit = 1 + (2 * 32);
		}
	}
	switch(projectile.type){
		case ObjectTypes::BLASTERPROJECTILE:{
			
		}break;
		case ObjectTypes::ROCKETPROJECTILE:{
			
		}break;
		case ObjectTypes::FLAMERPROJECTILE:{
			if(world.tickcount % 4 == 0){
				Audio::GetInstance().EmitSound(object.id, world.resources.soundbank["s_flmc01.wav"], 128);
			}
		}break;
	}
	hitx = x;
	hity = y;
}