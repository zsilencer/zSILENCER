#include "user.h"
#include "team.h"

User::User(){
	retrieving = false;
	accountid = 0;
	memset(name, 0, sizeof(name));
	for(int i = 0; i < 5; i++){
		agency[i].wins = 0;
		agency[i].losses = 0;
		agency[i].xptonextlevel = 0;
		agency[i].level = 0;
		agency[i].endurance = 0;
		agency[i].shield = 0;
		agency[i].jetpack = 0;
		agency[i].techslots = 5;
		agency[i].hacking = 0;
		agency[i].contacts = 0;
		agency[i].defaultbonuses = 5;
	}
	agency[Team::NOXIS].endurance = 3;
	agency[Team::NOXIS].defaultbonuses += 3;
	agency[Team::CALIBER].contacts = 3;
	agency[Team::CALIBER].defaultbonuses += 3;
	agency[Team::STATIC].hacking = 3;
	agency[Team::STATIC].defaultbonuses += 3;
	agency[Team::BLACKROSE].shield = 2;
	agency[Team::BLACKROSE].defaultbonuses += 2;
	strcpy(name, "");
	statsagency = 0;
}

void User::Serialize(bool write, Serializer & data){
	data.Serialize(write, accountid);
	for(int i = 0; i < 5; i++){
		data.Serialize(write, agency[i].wins);
		data.Serialize(write, agency[i].losses);
		data.Serialize(write, agency[i].xptonextlevel);
		data.Serialize(write, agency[i].level);
		data.Serialize(write, agency[i].endurance);
		data.Serialize(write, agency[i].shield);
		data.Serialize(write, agency[i].jetpack);
		data.Serialize(write, agency[i].techslots);
		data.Serialize(write, agency[i].hacking);
		data.Serialize(write, agency[i].contacts);
	}
	Uint8 namesize = strlen(name);
	data.Serialize(write, namesize);
	for(int i = 0; i < namesize; i++){
		data.Serialize(write, name[i]);
	}
	name[namesize] = 0;
}