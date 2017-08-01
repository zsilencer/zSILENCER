#include "overlay.h"

Overlay::Overlay() : Object(ObjectTypes::OVERLAY){
	res_bank = 0xFF;
	res_index = 0;
	state_i = 0;
	textbank = 135;
	textwidth = 8;
	drawalpha = false;
	uid = 0;
	requiresmaptobeloaded = false;
	textcolorramp = false;
	textallownewline = false;
	textlineheight = 10;
	clicked = false;
}

void Overlay::Tick(World & world){
	if(customsprite.size() > 0){
		
	}else{
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
	}
	state_i++;
}

bool Overlay::MouseInside(World & world, Uint16 mousex, Uint16 mousey){
	Sint16 x1, y1, x2, y2;
	if(text.length() > 0){
		x1 = x;
		x2 = x + (text.length() * textwidth);
		y1 = y;
		y2 = y;
		switch(textbank){
			case 133:
				y2 += 11;
			break;
			case 134:
				y2 += 15;
			break;
			case 135:
				y2 += 19;
			break;
			case 136:
				y2 += 23;
			break;
		}
	}else{
		x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
		x2 = x + world.resources.spritewidth[res_bank][res_index] - world.resources.spriteoffsetx[res_bank][res_index];
		y1 = y - world.resources.spriteoffsety[res_bank][res_index];
		y2 = y + world.resources.spriteheight[res_bank][res_index] - world.resources.spriteoffsety[res_bank][res_index];
	}
	if(mousex < x2 && mousex > x1 && mousey < y2 && mousey > y1){
		return true;
	}
	return false;
}