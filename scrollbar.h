#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "shared.h"
#include "object.h"
#include "sprite.h"
#include "interface.h"

class ScrollBar : public Object
{
public:
	ScrollBar();
	bool MouseInside(World & world, Uint16 mousex, Uint16 mousey);
	bool MouseInsideUp(World & world, Uint16 mousex, Uint16 mousey);
	bool MouseInsideDown(World & world, Uint16 mousex, Uint16 mousey);
	void ScrollUp(void);
	void ScrollDown(void);
	bool draw;
	Uint8 barres_index;
	Uint16 scrollposition;
	Uint16 scrollmax;
	Uint8 scrollpixels;
};

#endif