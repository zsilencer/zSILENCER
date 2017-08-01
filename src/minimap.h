#ifndef MINIMAP_H
#define MINIMAP_H

#include "shared.h"
#include "surface.h"

class MiniMap
{
public:
	MiniMap();
	void Recolor(Uint8 offset);
	Uint8 pixels[172 * 62];
	Surface surface;
};

#endif