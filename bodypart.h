#ifndef BODYPART_H
#define BODYPART_H

#include "shared.h"
#include "object.h"
#include "sprite.h"
#include "physical.h"

class BodyPart : public Object
{
public:
	BodyPart();
	void Tick(World & world);
	Uint8 type;
	Uint8 suitcolor;
	
private:
	Uint8 state_i;
};

#endif