#ifndef SECRETRETURN_H
#define SECRETRETURN_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class SecretReturn : public Object
{
public:
	SecretReturn();
	void Tick(World & world);

	Uint16 teamid;
	Uint8 frame[3];
	
private:
	Uint8 framespeed[3];
	Uint8 state_i;
};

#endif