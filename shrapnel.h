#ifndef SHRAPNEL_H
#define SHRAPNEL_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class Shrapnel : public Object
{
public:
	Shrapnel();
	void Tick(World & world);
	Uint8 GetBrightness(void);
	
private:
	Uint8 state_i;
};

#endif