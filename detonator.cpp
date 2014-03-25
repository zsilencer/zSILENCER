#include "detonator.h"
#include "plasmaprojectile.h"
#include "plume.h"
#include "grenade.h"

Detonator::Detonator() : Object(ObjectTypes::DETONATOR){
	requiresauthority = true;
	requiresmaptobeloaded = true;
	state_i = 0;
	ownerid = 0;
	res_bank = 182;
	res_index = 0;
	lowestypos = y;
	renderpass = 1;
	iscamera = false;
}

void Detonator::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, ownerid, old);
	data.Serialize(write, iscamera, old);
}

void Detonator::Tick(World & world){
	WarpTick();
	if(state_i <= (4 * 4) + 6){
		Grenade::Move(*this, world);
	}
	if(y > lowestypos){
		lowestypos = y;
	}
	// 182:0-3 det/camera
	if(state_i == 0){
		EmitSound(world, world.resources.soundbank["shield2.wav"], 96);
		state_warp = 12;
	}
	res_index = (state_i / 4) % 4;
	if(state_i == 4 * 4){
		state_i = 0;
	}
	if(state_i > 4 * 4){
		if(state_i == (4 * 4) + 1){
			yv = -15;
		}
		if(state_i >= (4 * 4) + 6){
			if(state_i == (4 * 4) + 6){
				// explode
				if(iscamera){
					draw = false;
					EmitSound(world, world.resources.soundbank["q_expl02.wav"], 64);
					for(int i = 0; i < 8; i++){
						Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
						if(plume){
							plume->type = 5;
							plume->SetPosition(x + (rand() % 33) - 16, y + (rand() % 33) - 16);
						}
					}
				}else{
					EmitSound(world, world.resources.soundbank["seekexp1.wav"], 128);
					Sint8 xvs[] = {-14, 14, -10, 10, -10, 10};
					Sint8 yvs[] = {-25, -25, 0, 0, 5, 5};
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
				}
			}
			if(state_i == (4 * 4) + 8){
				if(!iscamera){
					// explode
					Sint8 xvs[] = {-14, 0, 14, -5, 0, 5};
					Sint8 yvs[] = {-5, -10, -5, 5, 5, 5};
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
				}
			}
			draw = false;
			if(state_i >= (4 * 4) + 50){
				world.MarkDestroyObject(id);
			}
		}/*else{
			int velocity = (((4 * 4) - (state_i)) * 2) + 20;
			if(velocity < 0){
				velocity = 0;
			}
			y -= velocity;
		}*/
	}
	state_i++;
}

void Detonator::Detonate(void){
	if(state_i <= 4 * 4){
		state_i = (4 * 4) + 1;
	}
}

Uint8 Detonator::HasDetonated(void){
	if(state_i > (4 * 4) + 1){
		return state_i - ((4 * 4) + 1);
	}
	return false;
}