#ifndef HITTABLE_H
#define HITTABLE_H

#include "shared.h"
#include "serializer.h"

class Hittable
{
public:
	Hittable();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(class Object & object, class World & world);
	void HandleHit(class Object & object, class World & world, Uint8 x, Uint8 y, class Object & projectile);
	
	friend class Renderer;
	
protected:
	Uint8 health;
	Uint16 shield;
	Uint8 state_hit;
	Uint8 hitx;
	Uint8 hity;
};

#endif