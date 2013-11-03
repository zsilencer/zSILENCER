#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class Overlay : public Object
{
public:
	Overlay();
	~Overlay();
	void Tick(World & world);
	bool MouseInside(World & world, Uint16 mousex, Uint16 mousey);
	Uint8 state_i;
	char * text;
	Uint8 textbank;
	Uint8 textwidth;
	bool textcolorramp;
	bool drawalpha;
	Uint8 uid;
	bool clicked;
};

#endif