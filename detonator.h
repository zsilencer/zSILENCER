#ifndef DETONATOR_H
#define DETONATOR_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class Detonator : public Object
{
public:
	Detonator();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void Detonate(void);
	bool HasDetonated(void);
	Uint16 ownerid;
	Uint16 originaly;
	
private:
	Uint8 state_i;
};

#endif