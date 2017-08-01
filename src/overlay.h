#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "shared.h"
#include "object.h"
#include "sprite.h"
#include <string>

class Overlay : public Object
{
public:
	Overlay();
	void Tick(World & world);
	bool MouseInside(World & world, Uint16 mousex, Uint16 mousey);
	Uint8 state_i;
	std::string text;
	Uint8 textbank;
	Uint8 textwidth;
	bool textcolorramp;
	bool textallownewline;
	int textlineheight;
	bool drawalpha;
	Uint8 uid;
	bool clicked;
	std::vector<Uint8> customsprite;
	int customspritew;
	int customspriteh;
};

#endif