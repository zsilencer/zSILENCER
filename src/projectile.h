#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "shared.h"
#include "serializer.h"
#include "platform.h"

class Projectile
{
public:
	Projectile();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	Uint16 ownerid;
	Uint16 shielddamage;
	Uint16 healthdamage;
	Sint8 velocity;
	Sint8 emitoffset;
	bool bypassshield;
	Sint8 moveamount;
	Uint8 radius;
	bool moving;
	bool stopatobjectcollision;
	
protected:
	bool TestCollision(class Object & object, class World & world, Platform ** collidedplatform, class Object ** collidedobject);
};

#endif