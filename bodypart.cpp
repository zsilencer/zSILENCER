#include "bodypart.h"

BodyPart::BodyPart() : Object(ObjectTypes::BODYPART){
	res_bank = 0xFF;
	requiresauthority = false;
	renderpass = 1;
	state_i = 0;
	type = 0;
	suitcolor = 0;
	xv = (rand() % 33) - 16;
	yv = (rand() % 33) - 16;
	isphysical = true;
}

void BodyPart::Tick(World & world){
	if(type > 5){
		type = 5;
	}
	res_bank = type + 115;
	res_index = state_i / 4;
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
	yv += world.gravity / 2;
	if(res_index > 8){
		world.MarkDestroyObject(id);
	}
	state_i++;
}