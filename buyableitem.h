#ifndef BUYABLEITEM_H
#define BUYABLEITEM_H

#include "shared.h"

class BuyableItem
{
public:
	BuyableItem(Uint8 id, const char * name, int price, int repairprice, Uint8 res_bank, Uint8 res_index, Uint32 techchoice, Uint8 techslots, const char * description, int agencyspecific = -1);
	Uint8 id;
	int price;
	int repairprice;
	char name[64];
	Uint8 res_bank;
	Uint8 res_index;
	Uint32 techchoice;
	Uint8 techslots;
	int agencyspecific;
	char description[1024];
};

#endif