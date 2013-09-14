#include "inventorystation.h"

InventoryStation::InventoryStation() : Object(ObjectTypes::INVENTORYSTATION){
	res_bank = 89;
	res_index = 0;
	teamid = 0;
	state_i = 0;
}

void InventoryStation::Tick(World & world){
	state_i++;
	if(state_i > 4 * 3){
		state_i = 0;
	}
	res_index = (state_i / 4) % 3;
}