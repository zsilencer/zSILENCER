#ifndef USER_H
#define USER_H

#include "shared.h"
#include "serializer.h"
#include "stats.h"

class User
{
public:
	User();
	void Serialize(bool write, Serializer & data);
	int TotalUpgradePointsPossible(Uint8 agency);
	bool retrieving;
	Uint32 accountid;
	char name[64];
	struct{
		Uint16 wins;
		Uint16 losses;
		Uint16 xptonextlevel;
		Uint8 level;
		Uint8 endurance;
		Uint8 shield;
		Uint8 jetpack;
		Uint8 techslots;
		Uint8 hacking;
		Uint8 contacts;
		Uint8 defaultbonuses;
		Uint8 maxendurance;
		Uint8 maxshield;
		Uint8 maxjetpack;
		Uint8 maxtechslots;
		Uint8 maxhacking;
		Uint8 maxcontacts;
	} agency[5];
	static const Uint8 maxlevel = 99;
	static const Uint8 maxendurance = 5;
	static const Uint8 maxshield = 5;
	static const Uint8 maxjetpack = 5;
	static const Uint8 maxtechslots = 8;
	static const Uint8 maxhacking = 5;
	static const Uint8 maxcontacts = 5;
	// local only
	Stats statscopy;
	Uint8 statsagency;
	Uint8 teamnumber;
};

#endif