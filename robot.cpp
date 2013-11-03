#include "robot.h"
#include "rocketprojectile.h"
#include "player.h"

Robot::Robot() : Object(ObjectTypes::ROBOT){
	requiresauthority = true;
	state = NEW;
	state_i = 0;
	res_bank = 47;
	res_index = 0;
	maxhealth = 100;
	health = maxhealth;
	maxshield = 50;
	shield = maxshield;
	renderpass = 2;
	ishittable = true;
	isbipedal = true;
	isphysical = true;
	snapshotinterval = 48;
	respawnseconds = 45;
	virusplanter = 0;
	damaging = 0;
	soundchannel = -1;
	patrol = false;
}

void Robot::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, damaging, old);
	data.Serialize(write, virusplanter, old);
	data.Serialize(write, patrol, old);
}

void Robot::Tick(World & world){
	Hittable::Tick(*this, world);
	Bipedal::Tick(*this, world);
	if(rand() % (24 * 15) == 0 && state != DEAD){
		StopAmbience();
		Audio::GetInstance().EmitSound(id, world.resources.soundbank["airlokj.wav"], 64);
	}
	switch(state){
		case NEW:{
			draw = true;
			if(FindCurrentPlatform(*this, world)){
				if(patrol){
					state = WALKING;
				}else{
					state = ASLEEP;
				}
			}
			/*yv += world.gravity;
			if(yv > world.maxyvelocity){
				yv = world.maxyvelocity;
			}
			Uint32 xe = x + xv;
			Uint32 ye = y + yv;
			Platform * platform = world.map.TestLine(x, y, xe, ye, &xe, &ye, Platform::RECTANGLE | Platform::STAIRSDOWN | Platform::STAIRSDOWN);
			if(platform){
				currentplatformid = platform->id;
				state = WALKING;
				state_i = 0;
			}
			x = xe;
			y = ye;*/
		}break;
		case SLEEPING:{
			if(state_i >= 15){
				state = ASLEEP;
				state_i = -1;
				break;
			}
			res_bank = 47;
			res_index = 14 - state_i;
		}break;
		case ASLEEP:{
			if(soundchannel == -1){
				soundchannel = Audio::GetInstance().EmitSound(id, world.resources.soundbank["wndloope.wav"], 32, true);
			}
			res_bank = 47;
			res_index = 0;
			if(Look(world, 1) || Look(world, 2)){
				state = AWAKENING;
				state_i = -1;
				break;
			}
		}break;
		case AWAKENING:{
			if(state_i == 0){
				StopAmbience();
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["robotarm.wav"], 128);
			}
			if(state_i >= 15){
				state = WALKING;
				state_i = -1;
				break;
			}
			res_bank = 47;
			res_index = state_i;
		}break;
		case WALKING:{
			if(state_i > 240){
				state_i = 0;
			}
			if(state_i % 20 == 1){
				StopAmbience();
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["robot3r.wav"], 48);
			}
			if(state_i % 20 == 10){
				StopAmbience();
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["robot3l.wav"], 48);
			}
			if(state_i >= 100 && !patrol){
				state = SLEEPING;
				state_i = -1;
				break;
			}
			if(Look(world, 0)){
				state = SHOOTING;
				state_i = -1;
				break;
			}
			if(Look(world, 2)){
				mirrored = true;
			}
			if(Look(world, 1)){
				mirrored = false;
			}
			if(state_i % 40 == 0){
				int x1, y1, x2, y2;
				GetAABB(world.resources, &x1, &y1, &x2, &y2);
				std::vector<Uint8> types;
				types.push_back(ObjectTypes::PLAYER);
				std::vector<Object *> players = world.TestAABB(x1, y1, x2, y2, types);
				for(std::vector<Object *>::iterator it = players.begin(); it != players.end(); it++){
					Player * player = static_cast<Player *>(*it);
					Team * team = player->GetTeam(world);
					if(!player->IsDisguised() && (team && team->id != virusplanter) && !player->HasSecurityPass()){
						damaging = 1;
						Object damageprojectile(ObjectTypes::PLASMAPROJECTILE);
						damageprojectile.healthdamage = 60;
						damageprojectile.shielddamage = 60;
						damageprojectile.ownerid = id;
						player->HandleHit(world, 50, 50, damageprojectile);
						StopAmbience();
						Audio::GetInstance().EmitSound(id, world.resources.soundbank["!laserew.wav"], 64);
					}
				}
			}
			res_bank = 45;
			res_index = state_i % 20;
			xv = mirrored ? -4 : 4;
			FollowGround(*this, world, xv);
			if(DistanceToEnd(*this, world) <= world.minwalldistrance){
				mirrored = mirrored ? false : true;
			}
		}break;
		case SHOOTING:{
			if(state_i >= 18 * 2){
				state = WALKING;
				state_i = -1;
				break;
			}
			res_bank = 46;
			if(state_i < 18){
				res_index = state_i;
			}else{
				res_index = (18 * 2) - state_i - 1;
			}
			if(state_i == 11){
				RocketProjectile * rocketprojectile = (RocketProjectile *)world.CreateObject(ObjectTypes::ROCKETPROJECTILE);
				if(rocketprojectile){
					rocketprojectile->ownerid = id;
					rocketprojectile->y = y - 60;
					if(mirrored){
						rocketprojectile->x = x - 70;
						rocketprojectile->xv = -25;
					}else{
						rocketprojectile->x = x + 70;
						rocketprojectile->xv = 25;
					}
				}
			}
			/*if(state_i == 18){
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["rocket4.wav"], 128);
			}*/
		}break;
		case DYING:{
			if(state_i == 4 * 4){
				StopAmbience();
				Audio::GetInstance().EmitSound(id, world.resources.soundbank["seekexp1.wav"], 128);
			}
			collidable = false;
			if(state_i >= 16 * 4){
				state = DEAD;
				state_i = -1;
				break;
			}
			res_bank = 48;
			res_index = state_i / 4;
		}break;
		case DEAD:{
			/*collidable = false;
			res_bank = 48;
			res_index = 15;
			if(state_i >= 24){
				draw = false;
				if(state_i == 240){
					state = ASLEEP;
					state_i = -1;
					state_warp = 12;
					draw = true;
					//collidable = true;
					health = maxhealth;
					shield = maxshield;
					break;
				}
			}*/
			StopAmbience();
			collidable = false;
			virusplanter = 0;
			res_bank = 48;
			res_index = 15;
			if(state_i > 1){
				draw = false;
			}
			if(state_i >= respawnseconds){
				state = NEW;
				x = originalx;
				y = originaly;
				state_i = -1;
				state_warp = 12;
				health = maxhealth;
				shield = maxshield;
				break;
			}
			if(world.tickcount % 24 != 0){
				state_i--;
			}
		}break;
	}
	if(damaging){
		damaging++;
		if(damaging > 24){
			damaging = 0;
		}
	}
	state_i++;
}

void Robot::HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile){
	Hittable::HandleHit(*this, world, x, y, projectile);
	if(health == 0 && state != DYING && state != DEAD){
		state = DYING;
		xv = 0;
		state_i = 0;
		Object * owner = world.GetObjectFromId(projectile.ownerid);
		if(owner && owner->type == ObjectTypes::PLAYER){
			Player * player = static_cast<Player *>(owner);
			Peer * peer = player->GetPeer(world);
			if(peer){
				peer->stats.robotskilled++;
			}
		}
	}
}

bool Robot::ImplantVirus(Uint16 teamid){
	if(!virusplanter){
		virusplanter = teamid;
		return true;
	}
	return false;
}

bool Robot::Look(World & world, Uint8 direction){
	// 0: forward target
	// 1: forward
	// 2: backward
	std::vector<Uint8> types;
	types.push_back(ObjectTypes::PLAYER);
	int y1 = -60;
	int y2 = y1;
	int minx = 70;
	int maxx = 500;
	minx *= (mirrored ? -1 : 1);
	maxx *= (mirrored ? -1 : 1);
	switch(direction){
		case 0:
		break;
		case 1:
			minx = 70;
			maxx = 200;
			y1 = -10;
			y2 = -100;
		break;
		case 2:
			minx = -70;
			maxx = -200;
			y1 = -10;
			y2 = -100;
		break;
	}
	if(signed(x) + minx < 0){
		minx = -x;
	}
	if(signed(x) + maxx < 0){
		maxx = -x;
	}
	if(signed(y) + y1 < 0){
		y1 = -y;
	}
	if(signed(y) + y2 < 0){
		y2 = -y;
	}
	std::vector<Object *> objects = world.TestAABB(x + minx, y + y1, x + maxx, y + y2, types);
	for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
		Player * player = static_cast<Player *>(*it);
		Team * team = player->GetTeam(world);
		if(!player->IsDisguised() && !player->HasSecurityPass() && (team && team->id != virusplanter)){
			int xv2 = maxx - minx;
			int yv2 = y2 - y1;
			Object * object = world.TestIncr(x + minx, y + y1 - 1, x + minx, y + y1, &xv2, &yv2, types);
			if(object){
				if(!world.map.TestIncr(x + minx, y + y1 - 1, x + minx, y + y1, &xv2, &yv2, Platform::STAIRSDOWN | Platform::STAIRSDOWN | Platform::RECTANGLE, 0, true)){
					if(state == ASLEEP){
						if(player->x < x){
							mirrored = true;
						}else{
							mirrored = false;
						}
					}
					return true;
				}
			}
		}
	}
	return false;
}

void Robot::StopAmbience(void){
	if(soundchannel != -1){
		Audio::GetInstance().Stop(soundchannel, 800);
	}
	soundchannel = -1;
}