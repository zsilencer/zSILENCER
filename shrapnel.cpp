#include "shrapnel.h"
#include <math.h>

Shrapnel::Shrapnel() : Object(ObjectTypes::SHRAPNEL){
	requiresauthority = false;
	renderpass = 1;
	res_bank = 107;
	res_index = rand() % 5;
	state_i = 0;
	drawalpha = true;
	isphysical = true;
}

void Shrapnel::Tick(World & world){
	if(res_bank == 110){
		res_index++;
		if(res_index > 8){
			res_index = 0;
		}
		x += xv;
		y += yv;
		if(state_i >= 13){
			world.MarkDestroyObject(id);
		}
	}else{
		/*if((yv != 0 && abs(xv / yv) == 1)){
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
		}*/
		res_index++;
		if(res_index > 4){
			res_index = 0;
		}
		int xe = xv;
		int ye = yv;
		Platform * platform = world.map.TestIncr(x, y, x, y, &xe, &ye, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
		if(platform){
			float xn, yn;
			platform->GetNormal(x, y, &xn, &yn);
			xv = (xn * abs(xv)) / 2;
			xv += (rand() % 3) - 1;
			yv = (yn * abs(yv)) / 2;
			yv += (rand() % 3) - 1;
		}
		x += xv;
		y += yv;
		yv += world.gravity / 2;
		if(state_i >= 20){
			world.MarkDestroyObject(id);
			return;
		}
	}
	state_i++;
}

Uint8 Shrapnel::GetBrightness(void){
	if(state_i > 2){
		return 128 - ((state_i - 2) * 6);
	}else{
		return 128;
	}
}