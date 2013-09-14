#include "grenade.h"
#include "player.h"
#include "plasmaprojectile.h"
#include "flareprojectile.h"
#include "plume.h"
#include <math.h>

Grenade::Grenade() : Object(ObjectTypes::GRENADE){
	// 79:0-15 is grenade
	requiresauthority = true;
	renderpass = 2;
	res_bank = 79;
	res_index = 0;
	type = PLASMA;
	ownerid = 0;
	xv = 30;
	yv = -10;
	state_i = 0;
	color = 0;
	radius = 5;
	isphysical = true;
	snapshotinterval = 6;
}

void Grenade::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	//Sprite::Serialize(write, data, old);
	//Physical::Serialize(write, data, old);
	data.Serialize(write, type, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, ownerid, old);
	data.Serialize(write, color, old);
}

void Grenade::Tick(World & world){
	if(state_i <= 4){
		if(state_i == 4){
			Audio::GetInstance().EmitSound(id, world.resources.soundbank["grenthro.wav"], 64);
		}
		/*Player * player = (Player *)world->GetObjectFromId(ownerid);
		if(player){
			x = player->x;
			y = player->y - 60;
		}*/
	}else{
		if(state_i < 30 || type == FLARE){
			if(state_i < 30){
				res_index = state_i % 16;
			}
			Move(world);
		}
		if(state_i >= 30 && type == FLARE && state_i % 3 == 0){
			for(int i = 0; i < 3; i++){
				FlareProjectile * flareprojectile = static_cast<FlareProjectile *>(world.CreateObject(ObjectTypes::FLAREPROJECTILE));
				if(flareprojectile){
					flareprojectile->ownerid = ownerid;
					flareprojectile->x = x;
					flareprojectile->y = y;
					flareprojectile->originalx = flareprojectile->x;
					flareprojectile->originaly = flareprojectile->y;
					switch(i){
						case 0:
							flareprojectile->yv = -3;
							flareprojectile->xv = -3;
						break;
						case 1:
							flareprojectile->yv = -4;
							flareprojectile->xv = 0;
						break;
						case 2:
							flareprojectile->yv = -3;
							flareprojectile->xv = 3;
						break;
					}
				}
			}
		}
		if(state_i == 30){
			// initial explosion
			draw = false;
			switch(type){
				case EMP:{
					Audio::GetInstance().EmitSound(id, world.resources.soundbank["q_expl02.wav"], 128);
					for(int i = 0; i < 8; i++){
						Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
						if(plume){
							plume->type = 5;
							plume->SetPosition(x + (rand() % 33) - 16, y + (rand() % 33) - 16);
						}
					}
					for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
						Object * object = *it;
						//int radius = 500;
						if(object && object->ishittable && object->id != ownerid){
							//int distance = sqrt((float)((x - object->x) * (x - object->x)) + ((y - object->y) * (y - object->y)));
							//if(distance < radius){
								Object empprojectile(ObjectTypes::PLASMAPROJECTILE);
								empprojectile.healthdamage = 0;
								empprojectile.shielddamage = 0xFFFF;
								empprojectile.ownerid = ownerid;
								object->HandleHit(world, 50, 50, empprojectile);
								object->HandleHit(world, 50, 50, empprojectile);
							//}
						}
					}
				}break;
				case SHAPED:{
					Audio::GetInstance().EmitSound(id, world.resources.soundbank["seekexp1.wav"], 128);
					Sint8 xvs[] = {-10, -5, 0, 5, 10};
					Sint8 yvs[] = {-33, -34, -35, -34, -33};
					Sint8 ys[] = {0, 0, 0, 0, 0};
					for(int i = 0; i < 5; i++){
						PlasmaProjectile * plasmaprojectile = (PlasmaProjectile *)world.CreateObject(ObjectTypes::PLASMAPROJECTILE);
						if(plasmaprojectile){
							plasmaprojectile->large = false;
							plasmaprojectile->x = x;
							plasmaprojectile->y = y + ys[i];
							plasmaprojectile->ownerid = ownerid;
							plasmaprojectile->xv = xvs[i];
							plasmaprojectile->yv = yvs[i];
						}
					}
				}break;
				case PLASMA:{
					Audio::GetInstance().EmitSound(id, world.resources.soundbank["seekexp1.wav"], 128);
					Sint8 xvs[] = {-14, 14, -10, 10, -10, 10};
					Sint8 yvs[] = {-25, -25, -10, -10, -5, -5};
					Sint8 ys[] = {0, 0, 0, 0, 0, 0, 0, 0};
					for(int i = 0; i < 6; i++){
						PlasmaProjectile * plasmaprojectile = (PlasmaProjectile *)world.CreateObject(ObjectTypes::PLASMAPROJECTILE);
						if(plasmaprojectile){
							plasmaprojectile->large = false;
							plasmaprojectile->x = x;
							plasmaprojectile->y = y + ys[i];
							plasmaprojectile->ownerid = ownerid;
							plasmaprojectile->xv = xvs[i];
							plasmaprojectile->yv = yvs[i];
						}
					}
				}break;
				case NEUTRON:{
					Audio::GetInstance().Play(world.resources.soundbank["grenade1.wav"], 128);
					Audio::GetInstance().EmitSound(id, world.resources.soundbank["q_expl02.wav"], 128);
					for(int i = 0; i < 8; i++){
						Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
						if(plume){
							plume->type = 5;
							plume->SetPosition(x + (rand() % 33) - 16, y + (rand() % 33) - 16);
						}
					}
				}break;
				case FLARE:{
					draw = true;
					Audio::GetInstance().EmitSound(id, world.resources.soundbank["rocket1.wav"], 128);
				}break;
			}
		}else
		if(state_i == 33){
			// secondary explosion
			switch(type){
				case SHAPED:{
					Sint8 xvs[] = {-10, -5, 5, 10};
					Sint8 yvs[] = {-29, -30, -30, -29};
					Sint8 ys[] = {0, 0, 0, 0};
					for(int i = 0; i < 4; i++){
						PlasmaProjectile * plasmaprojectile = (PlasmaProjectile *)world.CreateObject(ObjectTypes::PLASMAPROJECTILE);
						if(plasmaprojectile){
							plasmaprojectile->large = true;
							plasmaprojectile->x = x;
							plasmaprojectile->y = y + ys[i];
							plasmaprojectile->oldx = plasmaprojectile->x;
							plasmaprojectile->oldy = plasmaprojectile->y;
							plasmaprojectile->ownerid = ownerid;
							plasmaprojectile->xv = xvs[i];
							plasmaprojectile->yv = yvs[i];
						}
					}
				}break;
				case PLASMA:{
					Sint8 xvs[] = {-14, 0, 14, -5, 0, 5};
					Sint8 yvs[] = {-20, -10, -20, -15, -15, -15};
					Sint8 ys[] = {0, 0, 0, 0, 0, 0};
					for(int i = 0; i < 6; i++){
						PlasmaProjectile * plasmaprojectile = (PlasmaProjectile *)world.CreateObject(ObjectTypes::PLASMAPROJECTILE);
						if(plasmaprojectile){
							plasmaprojectile->large = true;
							plasmaprojectile->x = x;
							plasmaprojectile->y = y + ys[i];
							plasmaprojectile->ownerid = ownerid;
							plasmaprojectile->xv = xvs[i];
							plasmaprojectile->yv = yvs[i];
						}
					}
				}break;
			}
		}else
		if(state_i == 40 && type != NEUTRON && type != FLARE){
			world.MarkDestroyObject(id);
		}else
		if(state_i == 120 && type == NEUTRON){
			world.MarkDestroyObject(id);
			for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
				Object * object = *it;
				if(object && object->ishittable && object->y <= world.map.height * 64){
					Object neutronprojectile(ObjectTypes::PLASMAPROJECTILE);
					neutronprojectile.healthdamage = 0xFFFF;
					neutronprojectile.shielddamage = 0xFFFF;
					neutronprojectile.ownerid = ownerid;
					object->HandleHit(world, 50, 50, neutronprojectile);
					object->HandleHit(world, 50, 50, neutronprojectile);
				}
			}
		}else
		if(state_i == 250){
			world.MarkDestroyObject(id);
		}
	}
	state_i++;
}

bool Grenade::WasThrown(void){
	if(state_i > 4){
		return true;
	}
	return false;
}

bool Grenade::UpdatePosition(World & world, Player & player){
	if(state_i <= 0){
		if(state_i == 0){
			xv = 20;
			mirrored = player.mirrored;
			if(player.input.keymoveleft){
				mirrored = true;
			}
			if(player.input.keymoveright){
				mirrored = false;
			}
			x = player.x + (5 * (mirrored ? -1 : 1));
			y = player.y - 70;
			if(player.input.keymoveleft || player.input.keymoveright){
				xv = 30;
			}
			if(player.input.keymovedown){
				y = player.y - 30;
				x = player.x;
				xv = 0;
				yv = 5;
			}
			if(player.input.keylookdownleft || player.input.keylookdownright){
				y = player.y - 30;
				xv = 25;
				yv = 10;
			}
			if(player.input.keymoveup){
				x = player.x;
				xv = 5;
				yv = -30;
			}
			if(player.input.keylookupleft || player.input.keylookupright){
				xv = 25;
				yv = -20;
			}
			if(xv < 0){
				if(!mirrored){
					xv = abs(xv);
				}
			}else{
				if(mirrored){
					xv = -xv;
				}
			}
			xv += (player.xv / 4);
			yv += (player.yv / 4);
			if(world.map.TestAABB(x - radius, y - radius, x + radius, y + radius, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true)){
				return false;
			}
		}
	}
	return true;
}

void Grenade::SetType(Uint8 type){
	Grenade::type = type;
	switch(type){
		case EMP:
			color = (8 << 4) + (10 & 0xF);
		break;
		case SHAPED:
			color = (9 << 4) + (13 & 0xF);
		break;
		case PLASMA:
			color = (9 << 4) + (14 & 0xF);
		break;
		case NEUTRON:
			color = (15 << 4) + (13 & 0xF);
		break;
		case FLARE:
			color = (9 << 4) + (12 & 0xF);
		break;
	}
}

void Grenade::Move(World & world){
	int xv2 = xv;
	int yv2 = yv;
	Platform * platform = world.map.TestIncr(x - radius, y - radius, x + radius, y + radius, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
	if(platform){
		Uint32 yt = platform->XtoY(x);
		if(platform->y2 - platform->y1 <= 1 && y >= yt){
			xv2 = xv;
			yv2 = yv;
			platform = world.map.TestIncr(x - radius, y - radius, x + radius, y + radius, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, platform);
		}
	}
	int morex = abs(xv - xv2);
	int morey = abs(yv - yv2);
	if(platform){
		float xn, yn;
		platform->GetNormal(x, y, &xn, &yn);
		if(xn){
			xv = (xn * abs(xv)) * 0.5;
		}else{
			xv *= 0.8;
		}
		if(yn){
			yv = (yn * abs(yv)) * 0.4;
		}else{
			yv *= 0.8;
		}
	}
	x += xv2;
	y += yv2;
	if(morex || morey){
		Move(world);
	}
	yv += world.gravity;
	/*if(world.map.TestAABB(x - 5, y - 5, x + 5, y + 5, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN)){
		int b = 0;
	}*/
}