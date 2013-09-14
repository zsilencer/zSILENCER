#include "sprite.h"

Sprite::Sprite(){
	res_index = 0;
	res_bank = 0;
	x = 0;
	y = 0;
	oldx = 0;
	oldy = 0;
	drawcheckered = false;
	drawalpha = false;
	mirrored = false;
	renderpass = 0;
	effectcolor = 0;
	effectbrightness = 128;
	draw = true;
};

void Sprite::Serialize(bool write, Serializer & data, Serializer * old){
	data.Serialize(write, res_index, old);
	data.Serialize(write, res_bank, old);
	data.Serialize(write, x, old);
	data.Serialize(write, y, old);
	data.Serialize(write, mirrored, old);
	data.Serialize(write, draw, old);
}

void Sprite::GetAABB(Resources & resources, int * x1, int * y1, int * x2, int * y2){
	if(resources.spritebank[res_bank][res_index]){
		if(mirrored){
			*x1 = x - (resources.spritewidth[res_bank][res_index] - resources.spriteoffsetx[res_bank][res_index]);
		}else{
			*x1 = x - resources.spriteoffsetx[res_bank][res_index];
		}
		*y1 = y - resources.spriteoffsety[res_bank][res_index];
		*x2 = *x1 + resources.spritewidth[res_bank][res_index];
		*y2 = *y1 + resources.spriteheight[res_bank][res_index];
	}else{
		*x1 = x;
		*x2 = x;
		*y1 = y;
		*y2 = y;
	}
}