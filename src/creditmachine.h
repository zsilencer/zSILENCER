#ifndef CREDITMACHINE_H
#define CREDITMACHINE_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class CreditMachine : public Object
{
public:
	CreditMachine();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void Activate(void);

	Uint8 state_i;
};

#endif