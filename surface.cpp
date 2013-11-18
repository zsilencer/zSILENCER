#include "surface.h"

Surface::Surface(int w, int h){
	pixels = new Uint8[w * h];
}

Surface::~Surface(){
	delete[] pixels;
}