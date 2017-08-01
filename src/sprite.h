#ifndef SPRITE_H
#define SPRITE_H

#include "resources.h"
#include "serializer.h"

class Sprite
{
public:
	Sprite();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void GetAABB(Resources & resources, int * x1, int * y1, int * x2, int * y2);
	void UpdateNudge(class World & world, float frametime);
	Sint16 x, y;
	Sint16 oldx, oldy;
	int nudgex, nudgey; // lerp nudges for smooth rendering
	Uint32 lasttick;
	Uint8 res_bank;
	Uint8 res_index;
	bool drawcheckered;
	bool drawalpha;
	bool mirrored;
	Uint8 effectcolor;
	Uint8 effectbrightness;
	Uint8 renderpass;
	bool draw;
};

#endif