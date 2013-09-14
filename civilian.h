#ifndef CIVILIAN_H
#define CIVILIAN_H

#include "shared.h"
#include "object.h"

class Civilian : public Object
{
public:
	Civilian();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile);
	Uint8 suitcolor;

private:
	bool Look(World & world);
	enum {NEW, STANDING, WALKING, RUNNING, DYINGFORWARD, DYINGBACKWARD, DYINGEXPLODE, DEAD};
	Uint8 state;
	Uint8 state_i;
	Uint8 speed;
};

#endif