#ifndef ROCKETPROJECTILE_H
#define ROCKETPROJECTILE_H

#include "shared.h"
#include "object.h"
#include "projectile.h"

class RocketProjectile : public Object
{
public:
	RocketProjectile();
	void Serialize(bool write, Serializer & data, Serializer * old);
	void Tick(World & world);
	bool JustHit(void);
	void FromSecurity(void);

private:
	Uint8 state_i;
	Sint8 oldxv;
	Sint8 oldyv;
	int soundchannel;
};

#endif