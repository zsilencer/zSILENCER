#ifndef FLAMERPROJECTILE_H
#define FLAMERPROJECTILE_H

#include "shared.h"
#include "object.h"

class FlamerProjectile : public Object
{
public:
	FlamerProjectile();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	
	friend class Projectile;
	
private:
	Uint8 state_i;
	static const int plumecount = 7;
	Uint16 plumeids[plumecount];
	int soundplaying;
	bool hitonce;
};

#endif