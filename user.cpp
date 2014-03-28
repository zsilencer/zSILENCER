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
		agency[i].techslots = 3;
		agency[i].hacking = 0;
		agency[i].contacts = 0;
		agency[i].defaultbonuses = 3;
		agency[i].maxcontacts = maxcontacts;
		agency[i].maxendurance = maxendurance;
		agency[i].maxhacking = maxhacking;
		agency[i].maxjetpack = maxjetpack;
		agency[i].maxshield = maxshield;
		agency[i].maxtechslots = maxtechslots;
	}
	agency[Team::NOXIS].endurance = 3;
	agency[Team::NOXIS].maxendurance = maxendurance + 3;
	agency[Team::NOXIS].defaultbonuses += 3;
	agency[Team::CALIBER].contacts = 3;
	agency[Team::CALIBER].maxcontacts = maxcontacts + 3;
	agency[Team::CALIBER].defaultbonuses += 3;
	agency[Team::STATIC].hacking = 3;
	agency[Team::STATIC].maxhacking = maxhacking + 3;
	agency[Team::STATIC].defaultbonuses += 3;
	agency[Team::BLACKROSE].shield = 2;
	agency[Team::BLACKROSE].maxshield = maxshield + 2;
	agency[Team::BLACKROSE].defaultbonuses += 2;
	strcpy(name, "");
	statsagency = 0;
	teamnumber = 0;
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

int User::TotalUpgradePointsPossible(Uint8 agencynum){
	Uint8 total = 0;
	total += agency[agencynum].maxcontacts;
	total += agency[agencynum].maxendurance;
	total += agency[agencynum].maxhacking;
	total += agency[agencynum].maxjetpack;
	total += agency[agencynum].maxshield;
	total += agency[agencynum].maxtechslots;
	return total;
}