#ifndef BIPEDAL_H
#define BIPEDAL_H

#include "shared.h"
#include "serializer.h"
#include "projectile.h"

class Bipedal
{
public:
	Bipedal();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(class Object & object, class World & world);
	//void HandleHit(class Object & object, class World * world, Uint8 x, Uint8 y, class Object * projectile);
	
	friend class Renderer;

protected:
	bool FollowGround(class Object & object, class World & world, Sint8 velocity);
	bool FindCurrentPlatform(class Object & object, class World & world);
	Uint16 currentplatformid;
	//Uint8 health;
	//Uint8 shield;
	Uint8 height;
	//Uint8 state_hit;
	//Uint8 hitx;
	//Uint8 hity;
	Uint8 state_warp;
};

#endif