#include "plume.h"
#include "objecttypes.h"

Plume::Plume() : Object(ObjectTypes::PLUME){
	type = 0;
	res_index = 0;
	state_i = 0;
	requiresauthority = false;
	xv = (rand() % 9) - 4;
	yv = (rand() % 9) - 4;
	drawcheckered = true;
	cycle = false;
	renderpass = 2;
	life = 0;
	quick = false;
	isphysical = true;
}

void Plume::Tick(World & world){
	Uint8 maxindex = 0;
	switch(type){
		case 0:
			res_bank = 77;
			maxindex = 19;
		break;
		case 1:
			res_bank = 76;
			maxindex = 18;
		break;
		case 2:
			res_bank = 75;
			maxindex = 19;
		break;
		case 3:
			res_bank = 74;
			maxindex = 24;
		break;
		case 4:
			res_bank = 73;
			maxindex = 19;
		case 5:
			res_bank = 72;
			maxindex = 24;
		break;
		case 6:
			res_bank = 71;
			maxindex = 18;
		break;
		case 7:
			res_bank = 70;
			maxindex = 19;
		break;
	}
	if(!life){
		life = maxindex;
	}
	x2 += xv;
	y2 += yv;
	x = x2 / 8;
	y = y2 / 8;
	if(cycle){
		Uint8 middle = (maxindex / 2) - 2;
		if(state_i < middle){
			res_index = maxindex - state_i;
		}else
		if(state_i >= middle && state_i < maxindex){
			res_index = maxindex - state_i - ((state_i - middle));
		}else{
			res_index = maxindex + 1;
		}
	}else{
		res_index = state_i;
	}
	if(res_index > life){
		res_index = maxindex;
		world.MarkDestroyObject(id);
	}
	state_i++;
	if(quick){
		state_i++;
	}
}

void Plume::SetPosition(Uint32 x, Uint32 y){
	x2 = x * 8;
	y2 = y * 8;
}