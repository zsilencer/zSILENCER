#include "surface.h"
#include "renderer.h"

Surface::Surface(){
	pixels = 0;
	rlepixels = 0;
}

Surface::Surface(int w, int h, Uint8 clearcolor){
	Surface::w = w;
	Surface::h = h;
	pixels = new Uint8[w * h];
	rlepixels = 0;
	Clear(clearcolor);
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

Uint8 * Surface::GetPixels(void){
	if(!pixels){
		pixels = new Uint8[w * h];
		Renderer::BlitSurface(this, 0, this, 0);
	}
	return pixels;
}