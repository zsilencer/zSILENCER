#ifndef OBJECT_H
#define OBJECT_H

#include "shared.h"
#include "serializer.h"
#include "input.h"
#include "world.h"
#include "sprite.h"
#include "physical.h"
#include "hittable.h"
#include "bipedal.h"
#include "projectile.h"

class Object : public Sprite, public Physical, public Hittable, public Bipedal, public Projectile
{
public:
	Object(Uint8 type);
	bool RequiresAuthority(void);
	virtual void Tick(class World & world);
	virtual void Serialize(bool write, Serializer & data, Serializer * old = 0);
	virtual void OnDestroy(class World & world);
	virtual void HandleHit(class World & world, Uint8 x, Uint8 y, class Object & projectile);
	virtual void HandleInput(Input & input);
	virtual void HandleDisconnect(World & world, Uint8 peerid);
	bool requiresauthority;
	bool requiresmaptobeloaded;
	Uint8 type;
	Uint16 id;
	int snapshotinterval;
	bool wasdestroyed;
	bool issprite;
	bool isphysical;
	bool ishittable;
	bool isbipedal;
	bool isprojectile;
	bool iscontrollable;
};

#endif