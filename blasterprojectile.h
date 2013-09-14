#ifndef BLASTERPROJECTILE_H
#define BLASTERPROJECTILE_H

#include "shared.h"
#include "object.h"
#include "projectile.h"

class BlasterProjectile : public Object
{
public:
	BlasterProjectile();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	
private:
	Uint8 state_i;
};

#endif