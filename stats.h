#ifndef STATS_H
#define STATS_H

#include "shared.h"
#include "serializer.h"

class Stats
{
public:
	Stats();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	Uint32 CalculateExperience(void);
	Uint32 weaponfires[4];
	Uint32 weaponhits[4];
	Uint32 playerkillsweapon[4];
	Uint32 civilianskilled;
	Uint32 guardskilled;
	Uint32 robotskilled;
	Uint32 defensekilled;
	Uint32 secretspickedup;
	Uint32 secretsreturned;
	Uint32 secretsstolen;
	Uint32 secretsdropped;
	Uint32 powerupspickedup;
	Uint32 deaths;
	Uint32 kills;
	Uint32 suicides;
	Uint32 poisons;
	Uint32 tractsplanted;
	Uint32 grenadesthrown;
	Uint32 neutronsthrown;
	Uint32 empsthrown;
	Uint32 shapedthrown;
	Uint32 plasmasthrown;
	Uint32 flaresthrown;
	Uint32 poisonflaresthrown;
	Uint32 healthpacksused;
	Uint32 fixedcannonsplaced;
	Uint32 fixedcannonsdestroyed;
	Uint32 detsplanted;
	Uint32 camerasplanted;
	Uint32 virusesused;
	Uint32 fileshacked;
	Uint32 filesreturned;
};

#endif