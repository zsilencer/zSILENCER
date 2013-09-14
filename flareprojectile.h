#ifndef FLAREPROJECTILE_H
#define FLAREPROJECTILE_H

#include "shared.h"
#include "object.h"

class FlareProjectile : public Object
{
public:
	FlareProjectile();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	Sint16 originalx;
	Sint16 originaly;
	
private:
	Uint8 state_i;
	static const int plumecount = 4;
	Uint16 plumeids[plumecount];
	int soundplaying;
};

#endif