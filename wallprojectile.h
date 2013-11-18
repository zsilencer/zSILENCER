#ifndef WALLPROJECTILE_H
#define WALLPROJECTILE_H

#include "shared.h"
#include "object.h"

class WallProjectile : public Object
{
public:
	WallProjectile();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	
	friend class Renderer;
	
private:
	Uint8 state_i;
};

#endif