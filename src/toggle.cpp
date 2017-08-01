#include "toggle.h"

Toggle::Toggle() : Object(ObjectTypes::TOGGLE){
	res_bank = 0xFF;
	res_index = 0;
	selected = false;
	set = 0;
	uid = 0;
	requiresmaptobeloaded = false;
	width = 0;
	height = 0;
	text[0] = 0;
}

void Toggle::Tick(World & world){
	if(selected){
		if(res_bank == 181){
			effectcolor = 112;
			effectbrightness = 128;
		}
		if(res_bank == 7){
			res_index = 18;
		}
	}else{
		if(res_bank == 181){
			effectcolor = 112;
			effectbrightness = 32;
		}
		if(res_bank == 7){
			res_index = 19;
		}
	}
	if(res_bank != 0xFF){
		width = world.resources.spritewidth[res_bank][res_index];
		height = world.resources.spriteheight[res_bank][res_index];
	}
}

bool Toggle::MouseInside(World & world, Uint16 mousex, Uint16 mousey){
	Sint16 x1, y1, x2, y2;
	x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
	x2 = x + width - world.resources.spriteoffsetx[res_bank][res_index];
	y1 = y - world.resources.spriteoffsety[res_bank][res_index];
	y2 = y + height - world.resources.spriteoffsety[res_bank][res_index];
	if(mousex < x2 && mousex > x1 && mousey < y2 && mousey > y1){
		return true;
	}
	return false;
}