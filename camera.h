#ifndef CAMERA_H
#define CAMERA_H

#include "shared.h"
#include "sprite.h"
#include "world.h"

class Camera
{
public:
	Camera();
    Camera(unsigned int w, unsigned int h);
	void SetPosition(Sint16 x, Sint16 y);
	void GetPosition(Sint16 * x, Sint16 * y);
	void Tick(void);
    int GetXOffset(void);
    int GetYOffset(void);
	bool IsVisible(World & world, Sprite & sprite);
	void Follow(World & world, Sint16 x, Sint16 y, int w, int h, int xoffset, int yoffset);
	void Smooth(float frametime = 0);
	unsigned int w;
	unsigned int h;
	
//private:
	int x;
    int y;
	int oldx;
	int oldy;
	int newx;
	int newy;
};

#endif