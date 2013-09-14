#ifndef HEALMACHINE_H
#define HEALMACHINE_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class HealMachine : public Object
{
public:
	HealMachine();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	bool Activate(void);
	Uint8 state_i;
	
private:
	Uint8 cooldown;
};

#endif