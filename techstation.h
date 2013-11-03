#ifndef TECHSTATION_H
#define TECHSTATION_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class TechStation : public Object
{
public:
	TechStation();
	void Tick(World & world);
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile);
	void Repair(void);
	Uint8 type;
	Uint16 teamid;
	Uint32 techdisabled;
};

#endif