#ifndef VENT_H
#define VENT_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class Vent : public Object
{
public:
	Vent();
	void Tick(World & world);
	void Activate(void);
	
private:
	Uint8 active;
	Uint8 state_i;
};

#endif