#ifndef INVENTORYSTATION_H
#define INVENTORYSTATION_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class InventoryStation : public Object
{
public:
	InventoryStation();
	void Tick(World & world);
	Uint16 teamid;
	
private:
	Uint8 state_i;
};

#endif