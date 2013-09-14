#ifndef WALLDEFENSE_H
#define WALLDEFENSE_H

#include "shared.h"
#include "object.h"

class WallDefense : public Object
{
public:
	WallDefense();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile);
	bool AddDefense(void);
	enum {DEAD, ACTIVATING, WAITING, SHOOTING};
	Uint16 teamid;
	Uint8 state;
	Uint8 state_i;
	
private:
	Object * Look(World & world);
};

#endif