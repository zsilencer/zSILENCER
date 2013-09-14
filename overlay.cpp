#include "overlay.h"

Overlay::Overlay() : Object(ObjectTypes::OVERLAY){
	res_bank = 0xFF;
	res_index = 0;
	state_i = 0;
	text = 0;
	textbank = 135;
	textwidth = 8;
	textlength = 0;
	drawalpha = false;
	uid = 0;
	requiresmaptobeloaded = false;
	textcolorramp = false;
}

Overlay::~Overlay(){
	if(text){
		delete[] text;
	}
}

void Overlay::Tick(World & world){
	switch(res_bank){
		case 54:
			res_index = state_i;
			if(state_i >= 9){
				state_i = -1;
			}
		break;
		case 56:
			res_index = 0;
		break;
		case 57:
			res_index = state_i / 4;
			if(state_i / 4 > 16){
				state_i--;
				if(rand() % 100 == 0){
					state_i = -1;
				}
			}
		break;
		case 58:
			res_index = state_i / 4;
			if(state_i / 4 > 16){
				state_i--;
				if(rand() % 100 == 0){
					state_i = -1;
				}
			}
		break;
		case 171:
			res_index = (state_i / 2) % 4;
		break;
		case 208:
			if(state_i < 30 * 2){
				res_index = (state_i / 2) + 29;
			}else
				if(state_i < 60 * 2){
					res_index = 60;
				}else{
					res_index = (120 - (state_i / 2)) + 29;
					if(res_index <= 29){
						state_i = -1;
						break;
					}
				}
			if(res_index >= 60){
				res_index = 60;
			}
		break;
		case 222:
			if(state_i >= 3){
				world.MarkDestroyObject(id);
			}
			res_index = state_i;
		break;
	}
	state_i++;
}