#ifndef PLASMAPROJECTILE_H
#define PLASMAPROJECTILE_H

#include "shared.h"
#include "object.h"
#include "projectile.h"

class PlasmaProjectile : public Object
{
public:
	PlasmaProjectile();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	bool large;
	
private:
	Uint8 state_i;
};

#endif