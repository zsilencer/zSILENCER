#include "bipedal.h"
#include "object.h"
#include "projectile.h"

Bipedal::Bipedal(){
	height = 30;
	state_warp = 0;
	currentplatformid = 0;
}

void Bipedal::Serialize(bool write, Serializer & data, Serializer * old){
	data.Serialize(write, currentplatformid, old);
	data.Serialize(write, state_warp, old);
}

void Bipedal::Tick(Object & object, World & world){
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

int Bipedal::DistanceToEnd(Object & object, World & world){
	if(currentplatformid){
		Platform * platform = world.map.platformids[currentplatformid];
		if(object.mirrored){
			Platform & leftmost = world.map.GetLeftmostPlatform(*platform);
			return object.x - leftmost.x1;
		}else{
			Platform & rightmost = world.map.GetRightmostPlatform(*platform);
			return rightmost.x2 - object.x;
		}
	}
	return -1;
}
