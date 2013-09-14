#include "stats.h"

Stats::Stats(){
	for(int i = 0; i < 4; i++){
		weaponfires[i] = 0;
		weaponhits[i] = 0;
		playerkillsweapon[i] = 0;
	}
	civilianskilled = 0;
	guardskilled = 0;
	robotskilled = 0;
	defensekilled = 0;
	secretspickedup = 0;
	secretsreturned = 0;
	secretsstolen = 0;
	secretsdropped = 0;
	powerupspickedup = 0;
	deaths = 0;
	kills = 0;
	grenadesthrown = 0;
	neutronsthrown = 0;
	empsthrown = 0;
	shapedthrown = 0;
	plasmasthrown = 0;
	flaresthrown = 0;
	healthpacksused = 0;
	fixedcannonsplaced = 0;
	fixedcannonsdestroyed = 0;
	detsplanted = 0;
	fileshacked = 0;
	filesreturned = 0;
}

void Stats::Serialize(bool write, Serializer & data, Serializer * old){
	for(int i = 0; i < 4; i++){
		data.Serialize(write, weaponfires[i], old);
		data.Serialize(write, weaponhits[i], old);
		data.Serialize(write, playerkillsweapon[i], old);
	}
	data.Serialize(write, civilianskilled, old);
	data.Serialize(write, guardskilled, old);
	data.Serialize(write, robotskilled, old);
	data.Serialize(write, defensekilled, old);
	data.Serialize(write, secretspickedup, old);
	data.Serialize(write, secretsreturned, old);
	data.Serialize(write, secretsstolen, old);
	data.Serialize(write, secretsdropped, old);
	data.Serialize(write, powerupspickedup, old);
	data.Serialize(write, deaths, old);
	data.Serialize(write, kills, old);
	data.Serialize(write, grenadesthrown, old);
	data.Serialize(write, neutronsthrown, old);
	data.Serialize(write, empsthrown, old);
	data.Serialize(write, shapedthrown, old);
	data.Serialize(write, plasmasthrown, old);
	data.Serialize(write, flaresthrown, old);
	data.Serialize(write, healthpacksused, old);
	data.Serialize(write, fixedcannonsplaced, old);
	data.Serialize(write, fixedcannonsdestroyed, old);
	data.Serialize(write, detsplanted, old);
	data.Serialize(write, fileshacked, old);
	data.Serialize(write, filesreturned, old);
}

Uint32 Stats::CalculateExperience(void){
	int xp = 0;
	xp += kills * 15;
	xp -= deaths * 10;
	xp += secretsreturned * 75;
	xp += secretsstolen * 100;
	xp += secretspickedup * 10;
	xp -= secretsdropped * 15;
	xp += fileshacked / 50;
	xp += filesreturned / 50;
	xp += fixedcannonsdestroyed * 10;
	xp += guardskilled * 5;
	xp += robotskilled * 8;
	xp += defensekilled * 10;
	xp -= civilianskilled * 2;
	xp += (weaponhits[0] / 3) * 1;
	xp += (weaponhits[1] / 3) * 2;
	xp += (weaponhits[2] / 3) * 3;
	if(xp < 0){
		return 0;
	}else{
		return xp;
	}
}