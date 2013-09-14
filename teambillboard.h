#ifndef TEAMBILLBOARD_H
#define TEAMBILLBOARD_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class TeamBillboard : public Object
{
public:
	TeamBillboard();
	Uint8 agency;
};

#endif