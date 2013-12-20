#include "civilian.h"
#include "projectile.h"
#include "bodypart.h"
#include "player.h"
#include "plasmaprojectile.h"

Civilian::Civilian() : Object(ObjectTypes::CIVILIAN){
	requiresauthority = true;
	state = NEW;
	state_i = 0;
	speed = 4;
	res_bank = 121;
	res_index = 0;
	suitcolor = (7 << 4) + 11;
	renderpass = 2;
	ishittable = true;
	isbipedal = true;
	isphysical = true;
	snapshotinterval = 72;
	tractteamid = 0;
}

void Civilian::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, tractteamid, old);
}

void Civilian::Tick(World & world){
	Hittable::Tick(*this, world);
	Bipedal::Tick(*this, world);
	switch(state){
		case NEW:{
			if(FindCurrentPlatform(*this, world)){
				state = WALKING;
				state_i = -1;
				break;
			}
			/*res_bank = 121;
			res_index = 0;
			yv += world->gravity;
			if(yv > world->maxyvelocity){
				yv = world->maxyvelocity;
			}
			Uint32 xe = x + xv;
			Uint32 ye = y + yv;
			Platform * platform = world->map.TestLine(x, y, xe, ye, &xe, &ye, Platform::RECTANGLE | Platform::STAIRSDOWN | Platform::STAIRSDOWN);
			if(platform){
				currentplatformid = platform->id;
				state = WALKING;
				state_i = 0;
			}
			x = xe;
			y = ye;*/
		}break;
		case STANDING:{
			if(CheckTractVictim(world)){
				break;
			}
			if(state_i >= 10){
				state_i = 0;
			}
			res_bank = 121;
			res_index = state_i;
		}break;
		case WALKING:{
			if(CheckTractVictim(world)){
				break;
			}
			if(state_i >= 20){
				state_i = 0;
			}
			res_bank = 122;
			res_index = state_i;
			if(res_index == 5){
				EmitSound(world, world.resources.soundbank["stostep1.wav"], 16);
			}
			if(res_index == 15){
				EmitSound(world, world.resources.soundbank["stostepr.wav"], 16);
			}
			if(DistanceToEnd(*this, world) <= world.minwalldistance){
				mirrored = mirrored ? false : true;
			}
			xv = mirrored ? -speed : speed;
			FollowGround(*this, world, xv);
			if(state_i % 5 == 0){
				Look(world);
			}
		}break;
		case RUNNING:{
			if(CheckTractVictim(world)){
				break;
			}
			if(state_i >= 150){
				state = WALKING;
				state_i = -1;
				break;
			}
			xv = (mirrored ? -1 : 1) * (5 + speed);
			res_bank = 123;
			res_index = state_i % 15;
			if(res_index == 6){
				EmitSound(world, world.resources.soundbank["futstonl.wav"], 16);
			}
			if(res_index == 14){
				EmitSound(world, world.resources.soundbank["futstonr.wav"], 16);
			}
			if(DistanceToEnd(*this, world) <= world.minwalldistance){
				mirrored = mirrored ? false : true;
			}
			FollowGround(*this, world, xv);
			if(state_i % 10 == 9){
				Look(world);
			}
		}break;
		case DYINGFORWARD:{
			tractteamid = 0;
			if(state_i == 0){
				switch(rand() % 3){
					case 0:
						EmitSound(world, world.resources.soundbank["groan2.wav"], 128);
					break;
					case 1:
						EmitSound(world, world.resources.soundbank["groan2a.wav"], 128);
					break;
					case 2:
						EmitSound(world, world.resources.soundbank["grunt2a.wav"], 128);
					break;
				}
			}
			collidable = false;
			if(state_i >= 14){
				state = DEAD;
				state_i = -1;
				break;
			}
			FollowGround(*this, world, xv);
			res_bank = 126;
			res_index = state_i;
		}break;
		case DYINGBACKWARD:{
			tractteamid = 0;
			if(state_i == 0){
				switch(rand() % 3){
					case 0:
						EmitSound(world, world.resources.soundbank["groan2.wav"], 128);
					break;
					case 1:
						EmitSound(world, world.resources.soundbank["groan2a.wav"], 128);
					break;
					case 2:
						EmitSound(world, world.resources.soundbank["grunt2a.wav"], 128);
					break;
				}
			}
			collidable = false;
			if(state_i >= 14){
				state = DEAD;
				state_i = -1;
				break;
			}
			FollowGround(*this, world, xv);
			res_bank = 125;
			res_index = state_i;
		}break;
		case DYINGEXPLODE:{
			tractteamid = 0;
			draw = false;
			state = DEAD;
			state_i = -1;
			break;
		}break;
		case DEAD:{
			collidable = false;
			if(state_i >= 100){
				draw = true;
				collidable = true;
				state = WALKING;
				state_warp = 12;
				state_i = -1;
				break;
			}
		}break;
	}
	state_i++;
}

void Civilian::HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile){
	Hittable::HandleHit(*this, world, x, y, projectile);
	if(projectile.healthdamage == 0){
		return;
	}
	if(state == DYINGFORWARD || state == DYINGBACKWARD || state == DEAD || state == DYINGEXPLODE){
		return;
	}
	float xpcnt = -((x - 50) / 50.0) * (mirrored ? -1 : 1);
	if(x < 50){
		state = DYINGFORWARD;
	}else{
		state = DYINGBACKWARD;
	}
	if((xpcnt < 0 && xv < 0) || (xpcnt > 0 && xv > 0)){
		xv = abs(xv) * xpcnt;
	}else{
		xv = speed * xpcnt;
	}
	if(projectile.type == ObjectTypes::ROCKETPROJECTILE){
		state = DYINGEXPLODE;
		world.Explode(*this, suitcolor, xpcnt);
	}
	Object * owner = world.GetObjectFromId(projectile.ownerid);
	if(owner && owner->type == ObjectTypes::PLAYER){
		Player * player = static_cast<Player *>(owner);
		Peer * peer = player->GetPeer(world);
		if(peer){
			peer->stats.civilianskilled++;
		}
	}
	state_i = 0;
}

bool Civilian::Look(World & world){
	std::vector<Uint8> types;
	types.push_back(ObjectTypes::BLASTERPROJECTILE);
	types.push_back(ObjectTypes::LASERPROJECTILE);
	types.push_back(ObjectTypes::ROCKETPROJECTILE);
	types.push_back(ObjectTypes::FLAMERPROJECTILE);
	types.push_back(ObjectTypes::PLASMAPROJECTILE);
	types.push_back(ObjectTypes::WALLPROJECTILE);
	types.push_back(ObjectTypes::FLAREPROJECTILE);
	std::vector<Object *> objects = world.TestAABB(x - 200, y - 100, x + 200, y + 100, types);
	if(objects.size() > 0){
		if(objects[0]->x > x){
			mirrored = true;
		}else{
			mirrored = false;
		}
		if(state != RUNNING){
			state = RUNNING;
			state_i = -1;
		}
		return true;
	}
	return false;
}

bool Civilian::CheckTractVictim(World & world){
	if(!tractteamid){
		return false;
	}
	std::vector<Uint8> types;
	types.push_back(ObjectTypes::PLAYER);
	int x1, y1, x2, y2;
	GetAABB(world.resources, &x1, &y1, &x2, &y2);
	std::vector<Object *> objects = world.TestAABB(x1, y1, x2, y2, types);
	for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
		Player * player = static_cast<Player *>(*it);
		if(player->teamid != tractteamid){
			world.Explode(*this, suitcolor, 1);
			state = DYINGEXPLODE;
			EmitSound(world, world.resources.soundbank["seekexp1.wav"], 128);
			Object tractprojectile(ObjectTypes::PLASMAPROJECTILE);
			tractprojectile.healthdamage = 80;
			tractprojectile.shielddamage = 80;
			tractprojectile.ownerid = id;
			player->HandleHit(world, 50, 50, tractprojectile);
			Sint8 xvs[] = {-14, 14, -10, 10, -10, 10};
			Sint8 yvs[] = {-25, -25, -10, -10, -5, -5};
			for(int i = 0; i < 6; i++){
				PlasmaProjectile * plasmaprojectile = (PlasmaProjectile *)world.CreateObject(ObjectTypes::PLASMAPROJECTILE);
				if(plasmaprojectile){
					plasmaprojectile->large = false;
					plasmaprojectile->x = x;
					plasmaprojectile->y = y - 40;
					plasmaprojectile->ownerid = id;
					plasmaprojectile->xv = xvs[i];
					plasmaprojectile->yv = yvs[i];
				}
			}
			return true;
		}
	}
	return false;
}

bool Civilian::AddTract(Uint16 teamid){
	if(!tractteamid){
		tractteamid = teamid;
		return true;
	}
	return false;
}