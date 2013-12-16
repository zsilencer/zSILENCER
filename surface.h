#ifndef SURFACE_H
#define SURFACE_H

#include "shared.h"

class Surface
{
public:
	Surface();
	Surface(int w, int h, Uint8 clearcolor = 0);
	~Surface();
	void Clear(Uint8 color);
	Uint8 * GetPixels(void);
	int w;
	int h;
	Uint8 * pixels;
	Uint8 * rlepixels;
};

#endif