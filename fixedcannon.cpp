#include "fixedcannon.h"
#include "laserprojectile.h"
#include "player.h"
#include "team.h"
#include "plume.h"

FixedCannon::FixedCannon() : Object(ObjectTypes::FIXEDCANNON){
	requiresauthority = true;
	requiresmaptobeloaded = true;
	res_bank = 90;
	res_index = 0;
	state = UP;
	state_i = 0;
	ownerid = 0;
	teamid = 0;
	renderpass = 3;
	state = NEW;
	suitcolor = 0;
	health = 40;
	shield = 16;
	renderpass = 2;
	ishittable = true;
	isphysical = true;
}

void FixedCannon::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, ownerid, old);
	data.Serialize(write, teamid, old);
	data.Serialize(write, suitcolor, old);
}

void FixedCannon::Tick(World & world){
	Hittable::Tick(*this, world);
	switch(state){
		case NEW:{
			EmitSound(world, world.resources.soundbank["shield2.wav"], 96);
			state = UP;
		}break;
		case UP:{
			res_bank = 90;
			res_index = 0;
			if(Look(world, true)){
				state = SHOOTING_UP;
				state_i = -1;
				break;
			}
			if(Look(world, false)){
				state = MOVING_DOWN;
				state_i = -1;
				break;
			}
		}break;
		case DOWN:{
			res_bank = 91;
			res_index = 8;
			if(Look(world, false)){
				state = SHOOTING_DOWN;
				state_i = -1;
				break;
			}
			if(Look(world, true)){
				state = MOVING_UP;
				state_i = -1;
				break;
			}
			if(state_i >= 24){
				state = MOVING_UP;
				state_i = -1;
				break;
			}
		}break;
		case MOVING_UP:{
			if(state_i > 8 * 2){
				state = UP;
				state_i = -1;
				break;
			}
			res_bank = 91;
			res_index = 8 - (state_i / 2);
		}break;
		case MOVING_DOWN:{
			if(state_i > 8 * 2){
				state = DOWN;
				state_i = -1;
				break;
			}
			res_bank = 91;
			res_index = (state_i / 2);
		}break;
		case SHOOTING_UP:{
			if(state_i == 0){
				EmitSound(world, world.resources.soundbank["!laserew.wav"], 64);
				LaserProjectile * laserprojectile = (LaserProjectile *)world.CreateObject(ObjectTypes::LASERPROJECTILE);
				if(laserprojectile){
					laserprojectile->x = x + ((mirrored ? -1 : 1) * (45 + laserprojectile->emitoffset));
					laserprojectile->y = y - 47;
					laserprojectile->ownerid = id;
					laserprojectile->xv = laserprojectile->velocity * (mirrored ? -1 : 1);
					laserprojectile->mirrored = mirrored;
				}
			}
			if(state_i > 8 * 2){
				state = UP;
				state_i = -1;
				break;
			}
			res_bank = 92;
			res_index = (state_i / 2);
		}break;
		case SHOOTING_DOWN:{
			if(state_i == 0){
				EmitSound(world, world.resources.soundbank["!laserew.wav"], 64);
				LaserProjectile * laserprojectile = (LaserProjectile *)world.CreateObject(ObjectTypes::LASERPROJECTILE);
				if(laserprojectile){
					laserprojectile->x = x + ((mirrored ? -1 : 1) * (45 + laserprojectile->emitoffset));
					laserprojectile->y = y - 28;
					laserprojectile->ownerid = id;
					laserprojectile->xv = laserprojectile->velocity * (mirrored ? -1 : 1);
					laserprojectile->mirrored = mirrored;
				}
			}
			if(state_i > 8 * 2){
				state = DOWN;
				state_i = -1;
				break;
			}
			res_bank = 93;
			res_index = (state_i / 2);
		}break;
		case DYING:{
			if(state_i == 0){
				EmitSound(world, world.resources.soundbank["q_expl02.wav"], 128);
				for(int i = 0; i < 6; i++){
					Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
					if(plume){
						if(rand() % 2 == 0){
							plume->type = 6;
						}else{
							plume->type = 5;
						}
						plume->renderpass = renderpass;
						plume->SetPosition(x + (rand() % 37) - 18, y + (rand() % 11) - 5 - (i * 10));
					}
				}
			}
			if(state_i >= 5){
				world.MarkDestroyObject(id);
			}
		}break;
	}
	state_i++;
}

void FixedCannon::HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile){
	Hittable::HandleHit(*this, world, x, y, projectile);
	if(health == 0 && state != DYING){
		collidable = false;
		state = DYING;
		state_i = 0;
		Object * owner = world.GetObjectFromId(projectile.ownerid);
		if(owner && owner->type == ObjectTypes::PLAYER){
			Player * player = static_cast<Player *>(owner);
			Peer * peer = player->GetPeer(world);
			if(peer){
				peer->stats.fixedcannonsdestroyed++;
			}
		}
	}
}

void FixedCannon::SetOwner(World & world, Uint16 id){
	ownerid = id;
	Player * player = static_cast<Player *>(world.GetObjectFromId(id));
	if(player){
		suitcolor = player->suitcolor;
		Team * team = player->GetTeam(world);
		if(team){
			teamid = team->id;
		}
	}
}

bool FixedCannon::ImplantVirus(World & world, Uint16 playerid){
	if(playerid != ownerid){
		SetOwner(world, playerid);
		return true;
	}
	return false;
}

bool FixedCannon::Look(World & world, bool up){
	std::vector<Uint8> types;
	types.push_back(ObjectTypes::PLAYER);
	Sint8 y2 = up ? -47 : -28;
	std::vector<Object *> objects = world.TestAABB(x + (mirrored ? -70 : 70), y + y2, x + (mirrored ? -300 : 300), y + y2, types);
	for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
		Player * player = static_cast<Player *>(*it);
		Player * owner = (Player *)world.GetObjectFromId(ownerid);
		Team * team = 0;
		if(owner){
			team = owner->GetTeam(world);
		}
		if(team && player->teamid != team->id && !player->IsDisguised()){
			return true;
		}
	}
	return false;
}