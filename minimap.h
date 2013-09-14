#ifndef MINIMAP_H
#define MINIMAP_H

#include "shared.h"

class MiniMap
{
public:
	MiniMap();
	~MiniMap();
	void CreateSurface(void);
	void Recolor(Uint8 offset);
	Uint8 pixels[172 * 62];
	SDL_Surface * surface;
};

#endif