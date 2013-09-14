#ifndef FIXEDCANNON_H
#define FIXEDCANNON_H

#include "shared.h"
#include "object.h"

class FixedCannon : public Object
{
public:
	FixedCannon();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile);
	void SetOwner(Uint16 id);
	Uint8 suitcolor;
	
private:
	bool Look(World & world, bool up);
	enum {NEW, UP, DOWN, SHOOTING_UP, SHOOTING_DOWN, MOVING_UP, MOVING_DOWN, DYING};
	Uint8 state;
	Uint8 state_i;
	Uint16 ownerid;
};

#endif