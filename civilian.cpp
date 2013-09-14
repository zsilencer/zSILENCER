#include "civilian.h"
#include "projectile.h"
#include "bodypart.h"
#include "player.h"

Civilian::Civilian() : Object(ObjectTypes::CIVILIAN){
	requiresauthority = true;
	state = NEW;
	state_i = 0;
	speed = (rand() % 2) + 3;
	res_bank = 121;
	res_index = 0;
	suitcolor = (9 << 4) + 11;
	renderpass = 2;
	ishittable = true;
	isbipedal = true;
	isphysical = true;
	snapshotinterval = 72;
}

void Civilian::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	//Bipedal::Serialize(write, data, old);
	data.Serialize(write, state, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, speed, old);
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
			if(state_i >= 10){
				state_i = 0;
			}
			res_bank = 121;
			res_index = state_i;
		}break;
		case WALKING:{
			if(state_i >= 20){
				state_i = 0;
			}
			xv = mirrored ? -speed : speed;
			res_bank = 122;
			res_index = state_i;
			if(res_index == 5){
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["stostep1.wav"], 16);
			}
			if(res_index == 15){
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["stostepr.wav"], 16);
			}
			Uint16 oldx = x;
			FollowGround(*this, world, xv);
			if(x == oldx){
				mirrored = mirrored ? false : true;
			}
			if(state_i % 5 == 0){
				Look(world);
			}
		}break;
		case RUNNING:{
			if(state_i >= 150){
				state = WALKING;
				state_i = -1;
				break;
			}
			xv = (mirrored ? -1 : 1) * (5 + speed);
			res_bank = 123;
			res_index = state_i % 15;
			if(res_index == 6){
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["futstonl.wav"], 16);
			}
			if(res_index == 14){
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["futstonr.wav"], 16);
			}
			Uint16 oldx = x;
			FollowGround(*this, world, xv);
			if(x == oldx){
				mirrored = mirrored ? false : true;
			}
			if(state_i % 10 == 9){
				Look(world);
			}
		}break;
		case DYINGFORWARD:{
			if(state_i == 0){
				switch(rand() % 3){
					case 0:
						Audio::GetInstance().EmitSound(id, world.resources.soundbank["groan2.wav"], 128);
					break;
					case 1:
						Audio::GetInstance().EmitSound(id, world.resources.soundbank["groan2a.wav"], 128);
					break;
					case 2:
						Audio::GetInstance().EmitSound(id, world.resources.soundbank["grunt2a.wav"], 128);
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
			if(state_i == 0){
				switch(rand() % 3){
					case 0:
						Audio::GetInstance().EmitSound(id, world.resources.soundbank["groan2.wav"], 128);
						break;
					case 1:
						Audio::GetInstance().EmitSound(id, world.resources.soundbank["groan2a.wav"], 128);
						break;
					case 2:
						Audio::GetInstance().EmitSound(id, world.resources.soundbank["grunt2a.wav"], 128);
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
	switch(projectile.type){
		case ObjectTypes::BLASTERPROJECTILE:
		case ObjectTypes::LASERPROJECTILE:{
			if(rand() % 2 == 0){
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["strike03.wav"], 96);
			}else{
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["strike04.wav"], 96);
			}
		}break;
		case ObjectTypes::ROCKETPROJECTILE:{
			state = DYINGEXPLODE;
			//draw = false;
			for(int i = 0; i < 6; i++){
				BodyPart * bodypart = (BodyPart *)world.CreateObject(ObjectTypes::BODYPART);
				if(bodypart){
					bodypart->suitcolor = suitcolor;
					bodypart->x = Civilian::x;
					bodypart->y = Civilian::y - 50;
					bodypart->type = i;
					bodypart->xv += (abs(xv) * 2) * xpcnt;
				}
			}
		}break;
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