#ifndef PICKUP_H
#define PICKUP_H

#include "shared.h"
#include "object.h"

class PickUp : public Object
{
public:
	PickUp();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	enum {NONE, SECRET, FILES, LASERAMMO, ROCKETAMMO, FLAMERAMMO, EMPBOMB, SHAPEDBOMB, PLASMABOMB,
		NEUTRONBOMB, FIXEDCANNON, FLARE, CAMERA, PLASMADET, HEALTHPACK, SUPERSHIELD, JETPACK, HACKING, RADAR};
	Uint8 type;
	Uint16 quantity;
	Uint8 tracetime;
	bool powerup;
	int poweruprespawntime;
	
private:
	void CheckForPickedUp(World & world);
	Uint8 state_i;
};

#endif