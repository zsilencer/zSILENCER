#include "pickup.h"
#include "player.h"

PickUp::PickUp() : Object(ObjectTypes::PICKUP){
	requiresauthority = true;
	state_i = 0;
	quantity = 1;
	res_bank = 0xFF;
	res_index = 0;
	renderpass = 3;
	type = NONE;
	isphysical = true;
	powerup = false;
	poweruprespawntime = 0;
	tracetime = 0;
}

void PickUp::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	//Sprite::Serialize(write, data, old);
	//Physical::Serialize(write, data, old);
	data.Serialize(write, type, old);
	data.Serialize(write, quantity, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, powerup, old);
	data.Serialize(write, tracetime, old);
}

void PickUp::Tick(World & world){
	// 185:0-15 is secret item drop
	// 82:0-15 is files item drop
	// 42 is item drops 0-flamerammo 1-laserammo 2-rocketammo 3-grenades
	if(!powerup){
		CheckForPickedUp(world);
		switch(type){
			case SECRET:{
				if(tracetime > 0 && world.tickcount % 24 == 0){
					tracetime--;
					if(tracetime == 0){
						world.MarkDestroyObject(id);
					}
				}
				res_bank = 185;
				res_index = state_i % 16;
			}break;
			case FILES:{
				res_bank = 82;
				res_index = (state_i / 4) % 16;
			}break;
			case LASERAMMO:{
				res_bank = 42;
				res_index = 1;
			}break;
			case ROCKETAMMO:{
				res_bank = 42;
				res_index = 2;
			}break;
			case FLAMERAMMO:{
				res_bank = 42;
				res_index = 0;
			}break;
			case EMPBOMB:{
				res_bank = 42;
				res_index = 3;
			}break;
			case SHAPEDBOMB:{
				res_bank = 42;
				res_index = 3;
			}break;
			case PLASMABOMB:{
				res_bank = 42;
				res_index = 3;
			}break;
			case NEUTRONBOMB:{
				res_bank = 204;
				res_index = 0;
			}break;
			case FIXEDCANNON:{
				res_bank = 42;
				res_index = 3;
			}break;
			case FLARE:{
				res_bank = 42;
				res_index = 3;
			}break;
			case PLASMADET:{
				res_bank = 42;
				res_index = 3;
			}break;
		}
		yv += world.gravity;
		if(yv > world.maxyvelocity){
			yv = world.maxyvelocity;
		}
		int xe = xv;
		int ye = yv;
		world.map.TestIncr(x, y, x, y, &xe, &ye, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true);
		if(xe == 0){
			xv = 0;
		}
		x += xe;
		y += ye;
	}else{
		if(quantity == 0){
			draw = true;
			CheckForPickedUp(world);
			Uint8 indexes = 1;
			switch(type){
				case SUPERSHIELD:{
					res_bank = 200;
					indexes = 6;
				}break;
				case JETPACK:{
					res_bank = 201;
					indexes = 14;
				}break;
				default:{
					res_bank = 205;
					indexes = 6;
				}break;
			}
			res_index = indexes - 1 - (state_i / 2);
			if(res_index == 0){
				state_i--;
			}
		}else{
			if(state_i % 24 == 0){
				quantity--;
				if(quantity == 0){
					state_i = -1;
				}
			}
		}
	}
	state_i++;
}

void PickUp::CheckForPickedUp(World & world){
	std::vector<Uint8> types;
	types.push_back(ObjectTypes::PLAYER);
	Sint16 x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
	Sint16 x2 = x - world.resources.spriteoffsetx[res_bank][res_index] + world.resources.spritewidth[res_bank][res_index];
	Sint16 y1 = y - world.resources.spriteoffsety[res_bank][res_index];
	Sint16 y2 = y - world.resources.spriteoffsety[res_bank][res_index] + world.resources.spriteheight[res_bank][res_index];
	std::vector<Object *> objects = world.TestAABB(x1, y1, x2, y2, types);
	for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
		Object * object = *it;
		switch(object->type){
			case ObjectTypes::PLAYER:{
				Player * player = static_cast<Player *>(object);
				player->PickUpItem(world, *this);
			}break;
		}
	}
}