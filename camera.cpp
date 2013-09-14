#include "camera.h"
#include "object.h"
#include <math.h>

Camera::Camera(){
	w = 0;
	h = 0;
	x = 0;
	y = 0;
	oldx = 0;
	oldy = 0;
	newx = 0;
	newy = 0;
}

Camera::Camera(unsigned int w, unsigned int h){
    Camera::w = w;
    Camera::h = h;
}

void Camera::SetPosition(Sint16 x, Sint16 y){
	Camera::x = x;
	Camera::y = y;
	newx = x;
	newy = y;
	oldx = x;
	oldy = y;
}

void Camera::GetPosition(Sint16 * x, Sint16 * y){
	if(x){
		*x = Camera::x;
	}
	if(y){
		*y = Camera::y;
	}
}

void Camera::Tick(void){
	oldx += ceil((newx - oldx) * 0.5);
	//oldx = x;
	oldy += ceil((newy - oldy) * 0.5);
	//oldy = y;
	if(oldx <= w / 2){
		oldx = w / 2;
	}
	if(oldy <= h / 2){
		oldy = h / 2;
	}
}

int Camera::GetXOffset(void){
    return (w/2) - x;
}
 
int Camera::GetYOffset(void){
    return (h/2) - y;
}

bool Camera::IsVisible(World & world, Sprite & sprite){
	unsigned int width = world.resources.spritewidth[sprite.res_bank][sprite.res_index];
	unsigned int height = world.resources.spriteheight[sprite.res_bank][sprite.res_index];
	int offsetx = world.resources.spriteoffsetx[sprite.res_bank][sprite.res_index];
	int offsety = world.resources.spriteoffsety[sprite.res_bank][sprite.res_index];
	int spritex = sprite.x;
	int spritey = sprite.y;
	if(sprite.mirrored){
		offsetx = -offsetx;
	}
	if(spritex - offsetx + width >= x - (w / 2) && spritex - offsetx <= x + width + (w / 2)){
		if(spritey - offsety + height >= y - (h / 2) && spritey - offsety <= y + height + (h / 2)){
			return true;
		}
	}
	return false;
}

void Camera::Follow(World & world, Sint16 x, Sint16 y, int w, int h, int xoffset, int yoffset){
	//int x1 = Camera::x;
	//int y1 = Camera::y;
	if(signed(x - Camera::x) <= -((w / 2) - xoffset)){
		Camera::x = x + ((w / 2) - xoffset);
	}
	if(signed(x - Camera::x) >= ((w / 2) + xoffset)){
		Camera::x = x - ((w / 2) + xoffset);
	}
	if(signed(y - Camera::y) <= -((h / 2) - yoffset)){
		Camera::y = y + ((h / 2) - yoffset);
	}
	if(signed(y - Camera::y) >= ((h / 2) + yoffset)){
		Camera::y = y - ((h / 2) + yoffset);
	}
	if(Camera::x <= Camera::w / 2){
		Camera::x = Camera::w / 2;
	}
	if(Camera::y <= Camera::h / 2){
		Camera::y = Camera::h / 2;
	}
	if(y < world.map.height * 64){
		if(Camera::x >= (world.map.width * 64) - (Camera::w / 2)){
			Camera::x = (world.map.width * 64) - (Camera::w / 2);
		}
		if(Camera::y >= (world.map.height * 64) - (Camera::h / 2)){
			Camera::y = (world.map.height * 64) - (Camera::h / 2);
		}
	}
	newx = Camera::x;
	newy = Camera::y;
	//Camera::x = x1;
	//Camera::y = y1;
}

void Camera::Smooth(float frametime){
	return;
	x = (oldx + ((oldx - newx) * (frametime)));
	//lastx = x;
	y = (oldy + ((oldy - newy) * (frametime)));
	//lasty = y;
	printf("x = %d newx = %d oldx = %d\n", x, newx, oldx);
}