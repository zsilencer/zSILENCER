#ifndef BUTTON_H
#define BUTTON_H

#include "shared.h"
#include "object.h"
#include "sprite.h"

class Button : public Object
{
public:
	Button();
	void Tick(World & world);
	void SetType(Uint8 type);
	void Activate(void);
	void Deactivate(void);
	bool MouseInside(World & world, Uint16 mousex, Uint16 mousey);
	void GetTextOffset(World & world, Sint16 * x, Sint16 * y);
	enum {BNONE, B112x33, B220x33, B196x33, B236x27, B52x21, B156x21, BCHECKBOX};
	enum {INACTIVE, ACTIVATING, DEACTIVATING, ACTIVE};
	Uint8 uid;
	Uint8 type;
	Uint8 state;
	Uint8 state_i;
	Uint8 textbank;
	Uint8 textwidth;
	char text[32];
	bool clicked;
	Uint16 width;
	Uint16 height;
};

#endif