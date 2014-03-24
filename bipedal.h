#ifndef BIPEDAL_H
#define BIPEDAL_H

#include "shared.h"
#include "serializer.h"
#include "projectile.h"

class Bipedal
{
public:
	Bipedal();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(class Object & object, class World & world);

	friend class Renderer;

protected:
	void WarpTick(void);
	bool FollowGround(class Object & object, class World & world, Sint8 velocity);
	bool FindCurrentPlatform(class Object & object, class World & world);
	int DistanceToEnd(class Object & object, class World & world);
	Uint16 currentplatformid;
	Uint8 height;
	Uint8 state_warp;
};

#endif