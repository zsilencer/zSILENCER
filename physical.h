#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "shared.h"
#include "serializer.h"

class Physical
{
public:
	Physical();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	Sint8 xv, yv;
	bool collidable;
};

#endif