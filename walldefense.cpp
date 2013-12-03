#include "walldefense.h"
#include "wallprojectile.h"
#include "plume.h"
#include "player.h"
#include <math.h>

WallDefense::WallDefense() : Object(ObjectTypes::WALLDEFENSE){
	// 112:0-6
	// 43:0
	res_bank = 112;
	res_index = 0;
	shield = 0;
	health = 0;
	teamid = 0;
	ishittable = true;
	isphysical = true;
	state = DEAD;
	state_i = 0;
	requiresauthority = true;
}

void WallDefense::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, teamid, old);
	data.Serialize(write, state, old);
	data.Serialize(write, state_i, old);
}

void WallDefense::Tick(World & world){
	Hittable::Tick(*this, world);
	switch(state){
		case ACTIVATING:{
			collidable = true;
			res_index = state_i / 2;
			if(res_index > 6){
				res_index = 6;
				state = WAITING;
				state_i = -1;
				break;
			}
		}break;
		case WAITING:{
			if(state_i >= 12){
				state_i = 11;
				Object * object = Look(world);
				if(object){
					EmitSound(world, world.resources.soundbank["!laserew.wav"], 64);
					WallProjectile * wallprojectile = (WallProjectile *)world.CreateObject(ObjectTypes::WALLPROJECTILE);
					if(wallprojectile){
						int x1, y1, x2, y2;
						object->GetAABB(world.resources, &x1, &y1, &x2, &y2);
						int objectx = x1 + ((x2 - x1) / 2);
						int objecty = y1 + ((y2 - y1) / 2);
						float ax = objectx - x;
						float ay = objecty - y;
						float length = sqrt((ax * ax) + (ay * ay));
						wallprojectile->x = x;
						wallprojectile->y = y;
						wallprojectile->ownerid = id;
						wallprojectile->xv = wallprojectile->velocity * (ax / length);
						wallprojectile->yv = wallprojectile->velocity * (ay / length);
					}
					state_i = -1;
				}
			}
		}break;
		case DEAD:{
			collidable = false;
			res_index = 0;
			if(state_i >= 60){
				if(teamid == 0){
					state = ACTIVATING;
					state_i = -1;
					break;
				}else{
					state_i = -1;
				}
			}
			if(world.tickcount % 24 != 0){
				state_i--;
			}
		}break;
	}
	state_i++;
}

bool WallDefense::AddDefense(void){
	if(state == DEAD){
		state = ACTIVATING;
		state_i = 0;
	}
	if(!teamid){
		health = 40;
		shield = 0;
		return true;
	}
	if(health < 60){
		health += 15;
		shield += 1;
		if(health > 60){
			health = 60;
			shield = 4;
		}
		return true;
	}
	return false;
}

void WallDefense::HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile){
	Hittable::HandleHit(*this, world, x, y, projectile);
	if(health == 0 && state != DEAD){
		state = DEAD;
		state_i = 0;
		EmitSound(world, world.resources.soundbank["q_expl02.wav"], 96);
		for(int i = 0; i < 6; i++){
			Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
			if(plume){
				if(rand() % 2 == 0){
					plume->type = 6;
				}else{
					plume->type = 5;
				}
				plume->renderpass = renderpass;
				plume->SetPosition(WallDefense::x + (rand() % 37) - 18, WallDefense::y + (rand() % 37) - 18);
			}
		}
		Object * owner = world.GetObjectFromId(projectile.ownerid);
		if(owner && owner->type == ObjectTypes::PLAYER){
			Player * player = static_cast<Player *>(owner);
			Peer * peer = player->GetPeer(world);
			if(peer){
				peer->stats.defensekilled++;
			}
		}
	}
}

Object * WallDefense::Look(World & world){
	std::vector<Uint8> types;
	types.push_back(ObjectTypes::PLAYER);
	std::vector<Object *> objects = world.TestAABB(x - 600, y - 600, x + 600, y + 600, types);
	for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
		Object * object = *it;
		Player * player = static_cast<Player *>(object);
		if(player->IsDisguised() || player->HasSecurityPass()){
			continue;
		}
		Team * team = player->GetTeam(world);
		if(team && (team->id == teamid || (team->agency == Team::BLACKROSE && teamid))){
			continue;
		}
		int x1, y1, x2, y2;
		object->GetAABB(world.resources, &x1, &y1, &x2, &y2);
		int objectx = x1 + ((x2 - x1) / 2);
		int objecty = y1 + ((y2 - y1) / 2);
		int xv2 = objectx - x;
		int yv2 = objecty - y;
		if(!world.map.TestIncr(x, y, x, y, &xv2, &yv2, Platform::STAIRSDOWN | Platform::STAIRSDOWN | Platform::RECTANGLE, 0, true)){
			return object;
		}
	}
	return 0;
}