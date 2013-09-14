#ifndef PLUME_H
#define PLUME_H

#include "shared.h"
#include "object.h"

class Plume : public Object
{
public:
	Plume();
	void Tick(World & world);
	void SetPosition(Uint32 x, Uint32 y);
	Uint8 type;
	bool cycle;
	Uint8 state_i;
	Uint8 life;
	bool quick;
	
private:
	Uint32 x2;
	Uint32 y2;
};

#endif