#include "surface.h"
#include "renderer.h"

Surface::Surface(){
	w = 0;
	h = 0;
}

Surface::Surface(int w, int h, Uint8 clearcolor){
	Surface::w = w;
	Surface::h = h;
	pixels.resize(w * h);
	Clear(clearcolor);
}

void Surface::Clear(Uint8 color){
	memset(pixels.data(), color, w * h);
}

Uint8 * Surface::GetPixels(void){
	if(pixels.size() == 0){
		pixels.resize(w * h);
		Renderer::BlitSurface(this, 0, this, 0);
	}
	return pixels.data();
}