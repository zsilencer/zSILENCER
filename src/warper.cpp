#include "warper.h"
#include "player.h"

Warper::Warper() : Object(ObjectTypes::WARPER){
	requiresauthority = true;
	res_bank = 85;
	res_index = 0;
	state_i = 0;
	match = 0;
	actormatch = 0;
	renderpass = 1;
}

void Warper::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, match, old);
}

void Warper::Tick(World & world){
	state_i++;
	if(state_i / 24 >= 6){
		state_i = 0;
	}
	if(GetCountdown() == 0 && state_i % 24 == 0){
		std::vector<Uint8> types;
		types.push_back(ObjectTypes::PLAYER);
		Uint16 x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
		Uint16 x2 = x - world.resources.spriteoffsetx[res_bank][res_index] + world.resources.spritewidth[res_bank][res_index];
		Uint16 y1 = y - world.resources.spriteoffsety[res_bank][res_index];
		Uint16 y2 = y - world.resources.spriteoffsety[res_bank][res_index] + world.resources.spriteheight[res_bank][res_index];
		std::vector<Object *> objects = world.TestAABB(x1, y1, x2, y2, types);
		for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
			Object * object = *it;
			switch(object->type){
				case ObjectTypes::PLAYER:{
					Player * player = static_cast<Player *>(object);
					if(player->x >= x1 && player->x <= x2){
						Warper * warpermatch = (Warper *)world.GetObjectFromId(match);
						if(warpermatch){
							player->Warp(world, warpermatch->x, warpermatch->y);
						}
					}
				}break;
			}
		}
	}
}

void Warper::FindMatch(World & world){
	for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
		Object * object = (*it);
		if(object->type == ObjectTypes::WARPER){
			Warper * warper2 = static_cast<Warper *>(object);
			if(warper2->actormatch == actormatch && warper2->id != id){
				warper2->match = id;
				match = warper2->id;
			}
		}
	}
}

Uint8 Warper::GetCountdown(void){
	return 5 - (state_i / 24);
}