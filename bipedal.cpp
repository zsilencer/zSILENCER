#include "bipedal.h"
#include "object.h"
#include "projectile.h"

Bipedal::Bipedal(){
	height = 30;
	/*state_hit = 0;
	hitx = 0;
	hity = 0;
	health = 0;
	shield = 0;*/
	state_warp = 0;
	//renderpass = 2;
	currentplatformid = 0;
}

void Bipedal::Serialize(bool write, Serializer & data, Serializer * old){
	//Sprite::Serialize(write, data, old);
	//Physical::Serialize(write, data, old);
	data.Serialize(write, currentplatformid, old);
	/*data.Serialize(write, state_hit, old);
	data.Serialize(write, health, old);
	data.Serialize(write, shield, old);
	data.Serialize(write, hitx, old);
	data.Serialize(write, hity, old);*/
	data.Serialize(write, state_warp, old);
}

void Bipedal::Tick(Object & object, World & world){
	/*if(state_hit){
		state_hit++;
	}
	if(state_hit % 32 == 31){
		state_hit = 0;
	}*/
	if(state_warp){
		if(state_warp <= 24){
			object.collidable = false;
		}else{
			object.collidable = true;
		}
		if(state_warp < 40){
			state_warp++;
		}
		if(state_warp == 40){
			state_warp = 0;
		}
	}
}

/*void Bipedal::HandleHit(Object & object, World * world, Uint8 x, Uint8 y, Object * projectile){
	if(projectile){
		if(!shield || projectile->bypassshield){
			if(health - projectile->healthdamage < 0){
				health = 0;
			}else{
				health -= projectile->healthdamage;
			}
			state_hit = 1 + (0 * 32);
		}else{
			if(shield - projectile->shielddamage <= 0){
				int more = abs(shield - projectile->shielddamage);
				more = (float(more) / projectile->shielddamage) * projectile->healthdamage;
				shield = 0;
				if(health - more < 0){
					health = 0;
				}else{
					health -= more;
				}
				state_hit = 1 + (1 * 32);
			}else{
				shield -= projectile->shielddamage;
				state_hit = 1 + (2 * 32);
			}
		}
		if(projectile){
			switch(projectile->type){
				case ObjectTypes::BLASTERPROJECTILE:{
					
				}break;
				case ObjectTypes::ROCKETPROJECTILE:{
					
				}break;
				case ObjectTypes::FLAMERPROJECTILE:{
					if(world->tickcount % 4 == 0){
						Audio::GetInstance().EmitSound(object.id, world->resources.soundbank["s_flmc01.wav"], 128);
					}
				}break;
			}
		}
	}
	hitx = x;
	hity = y;
}*/

bool Bipedal::FollowGround(Object & object, World & world, Sint8 velocity){
	if(velocity == 0 || currentplatformid == 0){
		return true;
	}
	object.x += velocity;
	Platform * currentplatform = world.map.platformids[currentplatformid];
	Platform * oldplatform = currentplatform;
	object.y = currentplatform->XtoY(object.x);
	if(object.x > currentplatform->x2){
		velocity = object.x - currentplatform->x2;
		object.x = currentplatform->x2;
		currentplatform = currentplatform->adjacentr;
	}else
	if(object.x < currentplatform->x1){
		velocity = -(Sint8)(currentplatform->x1 - object.x);
		object.x = currentplatform->x1;
		currentplatform = currentplatform->adjacentl;
	}else{
		return true;
	}
	if(currentplatform){
		currentplatformid = currentplatform->id;
		return FollowGround(object, world, velocity);
	}else{
		Platform * collided = world.map.TestAABB(object.x, object.y - height, object.x, object.y, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, oldplatform);
		if(collided){
			return true;
		}
		return false;
	}
	return true;
}

bool Bipedal::FindCurrentPlatform(Object & object, World & world){
	if(currentplatformid){
		return true;
	}
	int xv2 = 0;
	int yv2 = 1;
	Platform * platform = world.map.TestIncr(object.x, object.y - height, object.x, object.y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
	if(platform){
		currentplatformid = platform->id;
		object.y = platform->XtoY(object.x);
		return true;
	}
	return false;
}
