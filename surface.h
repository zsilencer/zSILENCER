#ifndef SURFACE_H
#define SURFACE_H

#include "shared.h"

class Surface
{
public:
	Surface(unsigned int w, unsigned int h);
	~Surface();
	unsigned int w;
	unsigned int h;
	Uint8 * pixels;
};

#endif