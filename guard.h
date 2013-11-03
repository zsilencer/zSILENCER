#ifndef GUARD_H
#define GUARD_H

#include "shared.h"
#include "object.h"

class Guard : public Object
{
public:
	Guard();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile);
	Uint8 weapon;
	bool patrol;
	Sint16 originalx;
	Sint16 originaly;
	bool originalmirrored;
	
private:
	Object * Look(World & world, Uint8 direction);
	void Fire(World & world, Uint8 direction);
	enum {NEW, STANDING, CROUCHING, CROUCHED, SHOOTCROUCHED, UNCROUCHING, LOOKING, WALKING, SHOOTSTANDING,
		SHOOTUP, SHOOTDOWN, SHOOTUPANGLE, SHOOTDOWNANGLE, SHOOTLADDERUP, SHOOTLADDERDOWN, LADDER, DYING,
		DYINGEXPLODE, DEAD};
	Uint8 state;
	Uint8 state_i;
	Uint8 speed;
	Uint16 chasing;
	Uint8 maxhealth;
	Uint8 maxshield;
	Uint8 respawnseconds;
	Uint32 lastspoke;
};

#endif
