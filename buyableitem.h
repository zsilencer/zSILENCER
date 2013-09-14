#ifndef BUYABLEITEM_H
#define BUYABLEITEM_H

#include "shared.h"

class BuyableItem
{
public:
	BuyableItem(Uint8 id, const char * name, int price, Uint8 res_bank, Uint8 res_index, Uint32 techchoice, Uint8 techslots);
	Uint8 id;
	int price;
	char name[64];
	Uint8 res_bank;
	Uint8 res_index;
	Uint32 techchoice;
	Uint8 techslots;
};

#endif