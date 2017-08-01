#ifndef SURFACE_H
#define SURFACE_H

#include "shared.h"
#include <vector>

class Surface
{
public:
	Surface();
	Surface(int w, int h, Uint8 clearcolor = 0);
	void Clear(Uint8 color);
	Uint8 * GetPixels(void);
	int w;
	int h;
	std::vector<Uint8> pixels;
	std::vector<Uint8> rlepixels;
};

#endif