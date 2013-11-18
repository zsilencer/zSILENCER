#ifndef LASERPROJECTILE_H
#define LASERPROJECTILE_H

#include "shared.h"
#include "object.h"

class LaserProjectile : public Object
{
public:
	LaserProjectile();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	
private:
	Uint8 state_i;
};

#endif