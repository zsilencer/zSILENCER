#include "scrollbar.h"

ScrollBar::ScrollBar() : Object(ObjectTypes::SCROLLBAR){
	res_bank = 7;
	res_index = 9;
	barres_index = 10;
	draw = false;
	scrollmax = 0;
	scrollposition = 0;
	requiresmaptobeloaded = false;
}

bool ScrollBar::MouseInside(World & world, Uint16 mousex, Uint16 mousey){
	Sint16 x1, y1, x2, y2;
	x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
	x2 = x + world.resources.spritewidth[res_bank][res_index] - world.resources.spriteoffsetx[res_bank][res_index];
	y1 = y - world.resources.spriteoffsety[res_bank][res_index];
	y2 = y + world.resources.spriteheight[res_bank][res_index] - world.resources.spriteoffsety[res_bank][res_index];
	if(mousex < x2 && mousex > x1 && mousey < y2 && mousey > y1){
		return true;
	}
	return false;
}

bool ScrollBar::MouseInsideUp(World & world, Uint16 mousex, Uint16 mousey){
	Sint16 x1, y1, x2, y2;
	x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
	x2 = x + world.resources.spritewidth[res_bank][res_index] - world.resources.spriteoffsetx[res_bank][res_index];
	y1 = y - world.resources.spriteoffsety[res_bank][res_index];
	y2 = y + 16 - world.resources.spriteoffsety[res_bank][res_index];
	if(mousex < x2 && mousex > x1 && mousey < y2 && mousey > y1){
		return true;
	}
	return false;
}

bool ScrollBar::MouseInsideDown(World & world, Uint16 mousex, Uint16 mousey){
	Sint16 x1, y1, x2, y2;
	x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
	x2 = x + world.resources.spritewidth[res_bank][res_index] - world.resources.spriteoffsetx[res_bank][res_index];
	y1 = y - world.resources.spriteoffsety[res_bank][res_index] + world.resources.spriteheight[res_bank][res_index] - 16;
	y2 = y + world.resources.spriteheight[res_bank][res_index] - world.resources.spriteoffsety[res_bank][res_index];
	if(mousex < x2 && mousex > x1 && mousey < y2 && mousey > y1){
		return true;
	}
	return false;
}

void ScrollBar::ScrollUp(void){
	if(scrollposition > 0){
		scrollposition--;
	}
}

void ScrollBar::ScrollDown(void){
	if(scrollposition < scrollmax){
		scrollposition++;
	}
}