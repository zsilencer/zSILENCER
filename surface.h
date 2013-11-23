#ifndef SURFACE_H
#define SURFACE_H

#include "shared.h"

class Surface
{
public:
	Surface();
	Surface(int w, int h);
	~Surface();
	void Clear(Uint8 color);
	int w;
	int h;
	Uint8 * pixels;
	Uint8 * rlepixels;
};

#endif