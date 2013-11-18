#ifndef SURVEILLANCEMONITOR_H
#define SURVEILLANCEMONITOR_H

#include "shared.h"
#include "object.h"
#include "sprite.h"
#include "camera.h"

class SurveillanceMonitor : public Object
{
public:
	SurveillanceMonitor();
	void Tick(World & world);
	void SetSize(Uint8 size);

	Camera camera;
	Sint8 renderxoffset;
	Sint8 renderyoffset;
	Uint16 objectfollowing;
	int surveillancecamera;
	Uint16 teamid;
	bool drawscreen;
	int scalefactor;
	Uint8 size;
	Uint8 state_i;
};

#endif