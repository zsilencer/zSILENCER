#ifndef SURFACE_H
#define SURFACE_H

#include "shared.h"

class Surface
{
public:
	Surface(int w, int h);
	~Surface();
	int w;
	int h;
	Uint8 * pixels;
};

#endif