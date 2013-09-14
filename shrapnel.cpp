#include "shrapnel.h"
#include <math.h>

Shrapnel::Shrapnel() : Object(ObjectTypes::SHRAPNEL){
	requiresauthority = false;
	renderpass = 1;
	res_bank = 108;
	res_index = 0;
	state_i = 0;
	drawalpha = true;
	isphysical = true;
}

void Shrapnel::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	//Sprite::Serialize(write, data, old);
	//Physical::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
}

void Shrapnel::Tick(World & world){
	if((yv != 0 && abs(xv / yv) == 1)){
		res_bank = 107;
	}else{
		res_bank = state_i < 5 ? 108 : 109;
	}
	if(xv > 0){
		res_index = 0;
		if(yv > 0){
			res_index = 1;
		}
	}else
	if(xv == 0){
		res_bank = 107;
		res_index = 2;
	}else
	if(xv < 0){
		if(yv > 0){
			res_index = 3;
		}else
		if(yv == 0){
			res_bank = 107;
			res_index = 4;
		}
		if(yv < 0){
			res_index = 5;
		}
	}
	int xe = xv;
	int ye = yv;
	Platform * platform = world.map.TestIncr(x, y, x, y, &xe, &ye, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
	if(platform){
		float xn, yn;
		platform->GetNormal(x, y, &xn, &yn);
		xv = (xn * abs(xv)) / 2;
		yv = (yn * abs(yv)) / 2;
	}
	x += xv;
	y += yv;
	yv += world.gravity;
	if(state_i >= 10){
		world.MarkDestroyObject(id);
		return;
	}
	state_i++;
}

Uint8 Shrapnel::GetBrightness(void){
	return 128 - (state_i * 12);
}