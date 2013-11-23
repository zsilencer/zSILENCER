#include "surface.h"

Surface::Surface(){
	pixels = 0;
	rlepixels = 0;
}

Surface::Surface(int w, int h){
	Surface::w = w;
	Surface::h = h;
	pixels = new Uint8[w * h];
	rlepixels = 0;
	Clear(0);
}

Surface::~Surface(){
	if(pixels){
		delete[] pixels;
	}
	if(rlepixels){
		delete[] rlepixels;
	}
}

void Surface::Clear(Uint8 color){
	memset(pixels, color, w * h);
}