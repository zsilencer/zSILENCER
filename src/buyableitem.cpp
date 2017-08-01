#include "buyableitem.h"

BuyableItem::BuyableItem(Uint8 id, const char * name, int price, int repairprice, Uint8 res_bank, Uint8 res_index, Uint32 techchoice, Uint8 techslots, const char * description, int agencyspecific){
	BuyableItem::id = id;
	strcpy(BuyableItem::name, name);
	BuyableItem::price = price;
	BuyableItem::repairprice = repairprice;
	BuyableItem::res_bank = res_bank;
	BuyableItem::res_index = res_index;
	BuyableItem::techchoice = techchoice;
	BuyableItem::techslots = techslots;
	strcpy(BuyableItem::description, description);
	BuyableItem::agencyspecific = agencyspecific;
}