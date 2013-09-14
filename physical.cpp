#include "physical.h"
#include "world.h"

Physical::Physical(){
	xv = 0;
	yv = 0;
	collidable = true;
}

void Physical::Serialize(bool write, Serializer & data, Serializer * old){
	data.Serialize(write, xv, old);
	data.Serialize(write, yv, old);
	data.Serialize(write, collidable, old);
}