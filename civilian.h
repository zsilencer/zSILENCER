#ifndef CIVILIAN_H
#define CIVILIAN_H

#include "shared.h"
#include "object.h"

class Civilian : public Object
{
public:
	Civilian();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile);
	bool AddTract(Uint16 teamid);
	Uint8 suitcolor;
	Uint8 speed;
	Uint16 tractteamid;

private:
	bool Look(World & world);
	bool CheckTractVictim(World & world);
	enum {NEW, STANDING, WALKING, RUNNING, DYINGFORWARD, DYINGBACKWARD, DYINGEXPLODE, DEAD};
	Uint8 state;
	Uint8 state_i;
};

#endif