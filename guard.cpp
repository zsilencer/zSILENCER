#include "guard.h"
#include "projectile.h"
#include "bodypart.h"
#include "player.h"
#include "laserprojectile.h"
#include "rocketprojectile.h"
#include "pickup.h"

Guard::Guard() : Object(ObjectTypes::GUARD){
	requiresauthority = true;
	state = NEW;
	state_i = 0;
	res_bank = 59;
	res_index = 0;
	speed = 3;
	maxhealth = 25;
	health = maxhealth;
	maxshield = 15;
	shield = maxshield;
	chasing = 0;
	weapon = 0;
	renderpass = 2;
	ishittable = true;
	isbipedal = true;
	isphysical = true;
	snapshotinterval = 48;
	respawnseconds = 30;
}

void Guard::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	//Bipedal::Serialize(write, data, old);
	data.Serialize(write, state, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, chasing, old);
	data.Serialize(write, weapon, old);
}

void Guard::Tick(World & world){
	// 62:0-19 climb ladder
	// 63:0-3 hit
	// 154:0-9 shoot up
	// 155:0-8 shoot down
	// 156:0-8 shoot up/right
	// 157:0-8 shoot down/right
	// 158:0-9 crouch
	// 159:0-8 crouch shoot
	// 196:0-8 ladder shoot up
	// 197:0-8 ladder shoot down
	Hittable::Tick(*this, world);
	Bipedal::Tick(*this, world);
	Object * found = 0;
	if(state != DYING && state != DEAD && state != DYINGEXPLODE){
		do{
			if((found = Look(world, 0))){
				if(state == WALKING || state == STANDING || state == LOOKING){
					state = SHOOTSTANDING;
					state_i = 0;
				}else
				if(state == CROUCHED){
					state = UNCROUCHING;
					state_i = 0;
				}
				break;
			}
			if((found = Look(world, 1))){
				if(state == CROUCHED){
					state = SHOOTCROUCHED;
					state_i = 0;
				}else
				if(state == WALKING || state == STANDING || state == LOOKING){
					state = CROUCHING;
					state_i = 0;
				}
				break;
			}
			if((found = Look(world, 2))){
				if(state == WALKING || state == STANDING || state == LOOKING){
					state = SHOOTUP;
					state_i = 0;
				}
				break;
			}
			if((found = Look(world, 3))){
				if(state == WALKING || state == STANDING || state == LOOKING){
					state = SHOOTDOWN;
					state_i = 0;
				}
				break;
			}
			if((found = Look(world, 4))){
				if(state == WALKING || state == STANDING || state == LOOKING){
					state = SHOOTUPANGLE;
					state_i = 0;
				}
				break;
			}
			if((found = Look(world, 5))){
				Player * player = static_cast<Player *>(found);
				if(player){
					if(abs(player->x - x) < 60){
						break;
					}
				}
				if(state == WALKING || state == STANDING || state == LOOKING){
					state = SHOOTDOWNANGLE;
					state_i = 0;
				}
				break;
			}
		}while(0);
		if(found){
			if(!chasing){
				chasing = found->id;
				const char * sounds[5] = {"theres3.wav", "stop4.wav", "freeze3.wav", "freezrt1.wav", "drop4.wav"};
				Audio::GetInstance().EmitSound(id, world.resources.soundbank[sounds[rand() % 5]], 128);
			}
		}else{
			if(state == CROUCHED){
				state = UNCROUCHING;
				state_i = 0;
			}
		}
	}

	if(chasing){
		Player * player = (Player *)world.GetObjectFromId(chasing);
		if(player){
			if(state == STANDING || state == WALKING){
				if(abs(player->x - x) <= 90 && abs(player->x - x) > 80){
					if(player->x > x){
						mirrored = false;
					}else{
						mirrored = true;
					}
				}else
				if(abs(player->x - x) > 90){
					state = WALKING;
					if(player->x > x){
						mirrored = false;
					}else{
						mirrored = true;
					}
				}else{
					state = WALKING;
				}
			}
		}
	}
	switch(state){
		case NEW:{
			if(FindCurrentPlatform(*this, world)){
				state = STANDING;
				state_i = -1;
				break;
			}
		}break;
		case STANDING:{
			res_bank = 59;
			res_index = 0;
			if(state_i >= 48){
				if(rand() % 3 == 0){
					state = WALKING;
				}else{
					state = LOOKING;
				}
				state_i = -1;
			}
		}break;
		case CROUCHING:{
			res_bank = 158;
			res_index = state_i / 2;
			if(state_i / 2 >= 9){
				state = CROUCHED;
				state_i = -1;
				break;
			}
		}break;
		case CROUCHED:{
			res_bank = 158;
			res_index = 9;
		}break;
		case SHOOTCROUCHED:{
			if(state_i == 12){
				Fire(world, 1);
			}
			if((state_i / 2) == 9){
				state_i = 13 * 2;
			}
			if(state_i / 2 >= 16){
				state = CROUCHED;
				state_i = -1;
				break;
			}
			res_bank = 159;
			if(state_i / 2 > 8){
				res_index = 8 - ((state_i / 2) - 8);
			}else{
				res_index = state_i / 2;
			}
		}break;
		case UNCROUCHING:{
			res_bank = 158;
			res_index = 9 - (state_i / 2);
			if(state_i / 2 >= 9){
				state = STANDING;
				state_i = -1;
				break;
			}
		}break;
		case LOOKING:{
			if(!found){
				chasing = 0;
			}
			if(state_i >= 6 * 4){
				state = STANDING;
				state_i = -1;
				break;
			}
			res_bank = 69;
			res_index = state_i / 4;
		}break;
		case WALKING:{
			res_bank = 60;
			res_index = state_i % 19;
			xv = mirrored ? -speed : speed;
			Uint16 oldx = x;
			FollowGround(*this, world, xv);
			if(x == oldx){
				if(!chasing){
					mirrored = !mirrored;
				}else{
					state = STANDING;
				}
			}
			if(state_i == 240){
				state = LOOKING;
				state_i = -1;
				break;
			}
		}break;
		case SHOOTSTANDING:{
			if(state_i == 14){
				Fire(world, 0);
			}
			if((state_i / 2) == 10){
				state_i = 13 * 2;
			}
			if(state_i / 2 >= 18){
				state = STANDING;
				state_i = -1;
				break;
			}
			res_bank = 61;
			if(state_i / 2 > 9){
				res_index = 9 - ((state_i / 2) - 9);
			}else{
				res_index = state_i / 2;
			}
		}break;
		case SHOOTUP:{
			if(state_i == 14){
				Fire(world, 2);
			}
			if((state_i / 2) == 10){
				state_i = 13 * 2;
			}
			if(state_i / 2 >= 18){
				state = STANDING;
				state_i = -1;
				break;
			}
			res_bank = 154;
			if(state_i / 2 > 9){
				res_index = 9 - ((state_i / 2) - 9);
			}else{
				res_index = state_i / 2;
			}
		}break;
		case SHOOTDOWN:{
			if(state_i == 12){
				Fire(world, 3);
			}
			if((state_i / 2) == 9){
				state_i = 13 * 2;
			}
			if(state_i / 2 >= 16){
				state = STANDING;
				state_i = -1;
				break;
			}
			res_bank = 155;
			if(state_i / 2 > 8){
				res_index = 8 - ((state_i / 2) - 8);
			}else{
				res_index = state_i / 2;
			}
		}break;
		case SHOOTUPANGLE:{
			if(state_i == 12){
				Fire(world, 4);
			}
			if((state_i / 2) == 9){
				state_i = 13 * 2;
			}
			if(state_i / 2 >= 16){
				state = STANDING;
				state_i = -1;
				break;
			}
			res_bank = 156;
			if(state_i / 2 > 8){
				res_index = 8 - ((state_i / 2) - 8);
			}else{
				res_index = state_i / 2;
			}
		}break;
		case SHOOTDOWNANGLE:{
			if(state_i == 12){
				Fire(world, 5);
			}
			if((state_i / 2) == 9){
				state_i = 13 * 2;
			}
			if(state_i / 2 >= 16){
				state = STANDING;
				state_i = -1;
				break;
			}
			res_bank = 157;
			if(state_i / 2 > 8){
				res_index = 8 - ((state_i / 2) - 8);
			}else{
				res_index = state_i / 2;
			}
		}break;
		case LADDER:{
			if(state_i >= 20){
				state_i = 0;
			}
			res_bank = 62;
			res_index = state_i;
		}break;
		case DYING:{
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
			if(state_i >= 10){
				state = DEAD;
				state_i = -1;
				break;
			}
			res_bank = 64;
			res_index = state_i;
		}break;
		case DYINGEXPLODE:{
			draw = false;
			res_index = 0xFF;
			state = DEAD;
			state_i = -1;
			break;
		}break;
		case DEAD:{
			/*collidable = false;
			if(state_i >= 24){
				draw = false;
				if(state_i == 240){
					state = STANDING;
					state_warp = 12;
					state_i = -1;
					draw = true;
					//collidable = true;
					health = maxhealth;
					shield = maxshield;
					break;
				}
			}*/
			collidable = false;
			if(state_i > 1){
				draw = false;
			}
			if(state_i >= respawnseconds){
				state = STANDING;
				state_i = -1;
				state_warp = 12;
				draw = true;
				health = maxhealth;
				shield = maxshield;
				break;
			}
			if(world.tickcount % 24 != 0){
				state_i--;
			}
		}break;
	}
	state_i++;
}

void Guard::HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile){
	Hittable::HandleHit(*this, world, x, y, projectile);
	float xpcnt = -((x - 50) / 50.0) * (mirrored ? -1 : 1);
	if(health == 0 && state != DYING && state != DYINGEXPLODE){
		state = DYING;
		state_i = 0;
		if(weapon != 0){
			PickUp * pickup = (PickUp *)world.CreateObject(ObjectTypes::PICKUP);
			if(pickup){
				if(weapon == 2){
					pickup->type = PickUp::ROCKETAMMO;
					pickup->quantity = 3;
				}else
				if(weapon == 1){
					pickup->type = PickUp::LASERAMMO;
					pickup->quantity = 5;
				}
				pickup->x = Guard::x;
				pickup->y = Guard::y - 1;
				pickup->xv = (rand() % 9) - 4;
				pickup->yv = -15;
			}
		}
		Object * owner = world.GetObjectFromId(projectile.ownerid);
		if(owner && owner->type == ObjectTypes::PLAYER){
			Player * player = static_cast<Player *>(owner);
			Peer * peer = player->GetPeer(world);
			if(peer){
				peer->stats.guardskilled++;
			}
		}
	}
	xv = speed * xpcnt;
	FollowGround(*this, world, xv);
	/*if(x < 50){
		xv = abs(xv) * (mirrored ? -1 : 1);
	}else{
		xv = -abs(speed) * (mirrored ? -1 : 1);
	}*/
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
			if(health == 0 && state != DYINGEXPLODE){
				draw = false;
				state = DYINGEXPLODE;
				state_i = 0;
				res_bank = 0xFF;
				for(int i = 0; i < 6; i++){
					BodyPart * bodypart = (BodyPart *)world.CreateObject(ObjectTypes::BODYPART);
					if(bodypart){
						bodypart->x = Guard::x;
						bodypart->y = Guard::y - 50;
						bodypart->type = i;
						bodypart->xv += (abs(xv) * 2) * xpcnt;
					}
				}
			}
		}break;
	}
}


Object * Guard::Look(World & world, Uint8 direction){
	// directions:
	// 0: standing and forward
	// 1: crouched and forward
	// 2: up
	// 3: down
	// 4: up angled
	// 5: down angled
	// 6: on ladder and down
	// 7: on ladder and up
	// 8: on ladder and left
	// 9: on ladder and right
	std::vector<Uint8> types;
	types.push_back(ObjectTypes::PLAYER);
	Sint16 y1 = 0;
	Sint16 y2 = 0;
	Sint16 x1 = 0;
	Sint16 x2 = 0;
	switch(direction){
		case 0:
			y1 = -55;
			y2 = y1;
			x1 = 70;
			x2 = 200;
		break;
		case 1:
			y1 = -37;
			y2 = y1;
			x1 = 70;
			x2 = 200;
		break;
		case 2:
			x1 = 2;
			x2 = 2;
			y1 = -150;
			y2 = -300;
		break;
		case 3:
			x1 = 12;
			x2 = 12;
			y1 = 50;
			y2 = 200;
		break;
		case 4:
			x1 = 20;
			y1 = -82;
			x2 = x1 + 200;
			y2 = y1 - 200;
		break;
		case 5:
			x1 = 28;
			y1 = -30;
			x2 = x1 + 200;
			y2 = y1 + 200;
		break;
	}
	x1 *= (mirrored ? -1 : 1);
	x2 *= (mirrored ? -1 : 1);
	/*if(signed(x) + x1 < 0){
		x1 = -x;
	}
	if(signed(x) + x2 < 0){
		x2 = -x;
	}
	if(signed(y) + y1 < 0){
		y2 = -y;
	}
	if(signed(y) + y2 < 0){
		y2 = -y;
	}*/
	if(y1 == y2 || x1 == x2){
		std::vector<Object *> objects = world.TestAABB(x + x1, y + y1, x + x2, y + y2, types);
		for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
			Player * player = static_cast<Player *>(*it);
			if((!chasing && !player->IsDisguised()) || chasing){
				int xv2 = x2 - x1;
				int yv2 = y2 - y1;
				Object * object = world.TestIncr(x + x1, y + y1 - 1, x + x1, y + y1, &xv2, &yv2, types);
				if(object){
					if(!world.map.TestIncr(x + x1, y + y1 - 1, x + x1, y + y1, &xv2, &yv2, Platform::STAIRSDOWN | Platform::STAIRSDOWN | Platform::RECTANGLE, 0, true)){
						return object;
					}
				}
			}
		}
	}else{
		int xv2 = x2 - x1;
		int yv2 = y2 - y1;
		Object * object = world.TestIncr(x + x1, y + y1 - 1, x + x1, y + y1, &xv2, &yv2, types);
		if(object){
			Player * player = static_cast<Player *>(object);
			if((!chasing && !player->IsDisguised()) || chasing){
				if(!world.map.TestIncr(x + x1, y + y1 - 1, x + x1, y + y1, &xv2, &yv2, Platform::STAIRSDOWN | Platform::STAIRSDOWN | Platform::RECTANGLE, 0, true)){
					return object;
				}
			}
		}
	}
	return 0;
}

void Guard::Fire(World & world, Uint8 direction){
	Object * projectile = 0;
	switch(weapon){
		case 0:{
			projectile = world.CreateObject(ObjectTypes::BLASTERPROJECTILE);
		}break;
		case 1:{
			projectile = world.CreateObject(ObjectTypes::LASERPROJECTILE);
		}break;
		case 2:{
			projectile = world.CreateObject(ObjectTypes::ROCKETPROJECTILE);
		}break;
		case 3:{
			projectile = world.CreateObject(ObjectTypes::FLAMERPROJECTILE);
		}break;
	}
	if(projectile){
		projectile->ownerid = id;
		projectile->mirrored = mirrored;
		switch(direction){
			case 0:{
				projectile->x = x + ((mirrored ? -1 : 1) * (36 + projectile->emitoffset));
				projectile->y = y - 55;
				projectile->xv = projectile->velocity * (mirrored ? -1 : 1);
			}break;
			case 1:{
				projectile->x = x + ((mirrored ? -1 : 1) * (36 + projectile->emitoffset));
				projectile->y = y - 37;
				projectile->xv = projectile->velocity * (mirrored ? -1 : 1);
			}break;
			case 2:{
				projectile->x = x + ((mirrored ? -1 : 1) * 2);
				projectile->y = y - 95 - projectile->emitoffset;
				projectile->yv = -projectile->velocity;
			}break;
			case 3:{
				projectile->x = x + ((mirrored ? -1 : 1) * 12);
				projectile->y = y - 5 + projectile->emitoffset;
				projectile->yv = projectile->velocity;
			}break;
			case 4:{
				projectile->x = x + ((mirrored ? -1 : 1) * (20 + (projectile->emitoffset * 0.70710678118655)));
				projectile->y = y - 82 - (projectile->emitoffset * 0.70710678118655);
				projectile->xv = (mirrored ? -1 : 1) * projectile->velocity * 0.70710678118655;
				projectile->yv = -projectile->velocity * 0.70710678118655;
			}break;
			case 5:{
				projectile->x = x + ((mirrored ? -1 : 1) * (28 + (projectile->emitoffset * 0.70710678118655)));
				projectile->y = y - 30 + (projectile->emitoffset * 0.70710678118655);
				projectile->xv = (mirrored ? -1 : 1) * projectile->velocity * 0.70710678118655;
				projectile->yv = projectile->velocity * 0.70710678118655;
			}break;
		}
	}
}