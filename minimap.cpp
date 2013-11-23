#include "minimap.h"

MiniMap::MiniMap() : surface(172, 62){

}

void MiniMap::Recolor(Uint8 offset){
	for(int i = 0; i < sizeof(pixels); i++){
		pixels[i] += offset;
	}
}