#include "surface.h"

Surface::Surface(unsigned int w, unsigned int h){
	pixels = new Uint8[w * h];
}

Surface::~Surface(){
	delete[] pixels;
}