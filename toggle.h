#ifndef TOGGLE_H
#define TOGGLE_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class Toggle : public Object
{
public:
	Toggle();
	void Tick(World & world);
	bool MouseInside(World & world, Uint16 mousex, Uint16 mousey);
	Uint8 uid;
	Uint8 set;
	bool selected;
	Uint8 width;
	Uint8 height;
	char text[64];
};

#endif