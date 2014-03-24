#ifndef GRENADE_H
#define GRENADE_H

#include "shared.h"
#include "object.h"

class Grenade : public Object
{
public:
	Grenade();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	bool WasThrown(void);
	bool UpdatePosition(World & world, Player & player);
	void SetType(Uint8 type);
	static void Move(Object & object, World & world, int v = 0);
	enum {EMP, SHAPED, PLASMA, NEUTRON, FLARE, POISONFLARE};
	Uint8 type;
	Uint16 ownerid;
	Uint8 color;
	
private:
	Uint8 state_i;
	Uint8 radius;
};

#endif