#ifndef STATE_H
#define STATE_H

#include "shared.h"
#include "object.h"

class State : public Object
{
public:
	State();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	Uint8 state;
	Uint8 oldstate;
};

#endif