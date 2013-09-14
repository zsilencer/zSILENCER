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
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void SetSize(Uint8 size);

	Camera camera;
	Sint8 renderxoffset;
	Sint8 renderyoffset;
	Uint16 objectfollowing;
	int surveillancecamera;
	Uint16 teamid;
};

#endif