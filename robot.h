#ifndef ROBOT_H
#define ROBOT_H

#include "shared.h"
#include "object.h"

class Robot : public Object
{
public:
	Robot();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile);
	
private:
	bool Look(World & world, Uint8 direction = 0);
	enum {NEW, SLEEPING, ASLEEP, AWAKENING, WALKING, SHOOTING, DYING, DEAD};
	Uint8 state;
	Uint8 state_i;
	Uint8 maxhealth;
	Uint8 maxshield;
	Uint8 respawnseconds;
};

#endif