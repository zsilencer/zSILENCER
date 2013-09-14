#include "minimap.h"

MiniMap::MiniMap(){
	surface = 0;
}

MiniMap::~MiniMap(){
	if(surface){
		SDL_FreeSurface(surface);
	}
}

void MiniMap::CreateSurface(void){
	if(surface){
		SDL_FreeSurface(surface);
	}
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 172, 62, 8, 0, 0, 0, 0);
}

void MiniMap::Recolor(Uint8 offset){
	for(int i = 0; i < sizeof(pixels); i++){
		pixels[i] += offset;
	}
}