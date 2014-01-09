#include "techstation.h"
#include "plume.h"
#include "team.h"
#include <algorithm>

TechStation::TechStation() : Object(ObjectTypes::TECHSTATION){
	res_bank = 106;
	res_index = 0;
	type = 0;
	teamid = 0;
	ishittable = true;
	isphysical = true;
	requiresauthority = true;
	Repair();
}

void TechStation::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, type, old);
	data.Serialize(write, teamid, old);
	data.Serialize(write, techdisabled, old);
}

void TechStation::Tick(World & world){
	Hittable::Tick(*this, world);
	if(health == 0){
		int xs[] = {-20, -35, 70, 35};
		int ys[] = {-130, -27, -115, -43};
		for(int i = 0; i < 3; i++){
			Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
			if(plume){
				plume->type = rand() % 2;
				plume->renderpass = renderpass;
				plume->SetPosition(TechStation::x + xs[i] + (rand() % 9) - 4, TechStation::y + ys[i] + (rand() % 9) - 4);
				plume->xv = (rand() % 9) - 4;
				plume->yv = -25;
			}
		}
		Team * team = static_cast<Team *>(world.GetObjectFromId(teamid));
		if(team && !(team->disabledtech & techdisabled)){
			Repair();
		}
	}
}

void TechStation::HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile){
	Hittable::HandleHit(*this, world, x, y, projectile);
	if(health == 0 && projectile.healthdamage > 0){
		collidable = false;
		EmitSound(world, world.resources.soundbank["q_expl02.wav"], 96);
		Team * team = static_cast<Team *>(world.GetObjectFromId(teamid));
		if(team){
			Uint32 tech = team->GetAvailableTech(world);
			std::vector<BuyableItem *> buyablecopy = world.buyableitems;
			//std::random_shuffle(buyablecopy.begin(), buyablecopy.end());
			for(int i = 0; i < buyablecopy.size(); i++){
				int r = world.Random() % buyablecopy.size();
				BuyableItem * temp = buyablecopy[i];
				buyablecopy[i] = buyablecopy[r];
				buyablecopy[r] = temp;
			}
			for(std::vector<BuyableItem *>::iterator it = buyablecopy.begin(); it != buyablecopy.end(); it++){
				BuyableItem * buyableitem = *it;
				if((buyableitem->techchoice & tech) && !(buyableitem->techchoice & team->disabledtech)){
					team->disabledtech |= buyableitem->techchoice;
					techdisabled = buyableitem->techchoice;
					printf("Disabled the %s tech\n", buyableitem->name);
					break;
				}
			}
		}
		for(int i = 0; i < 24; i++){
			Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
			if(plume){
				if(rand() % 2 == 0){
					plume->type = 6;
				}else{
					plume->type = 5;
				}
				plume->renderpass = renderpass;
				plume->SetPosition(TechStation::x + (rand() % 101) - 40, TechStation::y - (rand() % 120));
			}
		}
	}
}

void TechStation::Repair(void){
	health = 240;
	shield = 240;
	techdisabled = 0;
	collidable = true;
}