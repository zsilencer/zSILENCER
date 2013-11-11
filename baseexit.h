#ifndef BASEEXIT_H
#define BASEEXIT_H

#include "shared.h"
#include "object.h"

class BaseExit : public Object
{
public:
	BaseExit();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	Uint16 teamid;
	
private:
	int soundchannel;
};

#endif