#ifndef WARPER_H
#define WARPER_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class Warper : public Object
{
public:
	Warper();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void FindMatch(World & world);
	Uint8 GetCountdown(void);
	Uint16 match;
	Uint32 actormatch;
	
private:
	Uint8 state_i;
};

#endif