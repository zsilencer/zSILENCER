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
	void SetOwner(World & world, Uint16 id);
	bool ImplantVirus(World & world, Uint16 playerid);
	Uint8 suitcolor;
	Uint16 ownerid;
	Uint16 teamid;
	
private:
	bool Look(World & world, bool up, bool behind = false);
	enum {NEW, UP, DOWN, SHOOTING_UP, SHOOTING_DOWN, MOVING_UP, MOVING_DOWN, DYING};
	Uint8 state;
	Uint8 state_i;
};

#endif