#ifndef BASEDOOR_H
#define BASEDOOR_H

#include "shared.h"
#include "object.h"
#include "sprite.h"
#include "team.h"

class BaseDoor : public Object
{
public:
	BaseDoor();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	void Respawn(void);
	void SetTeam(Team & team);
	void CheckForPlayersInView(World & world);
	Uint8 color;
	Uint16 teamid;
	Uint8 teamnumber;
	bool discoveredby[World::maxteams];
	bool enteredby[World::maxteams];
	
private:
	Uint8 state_i;
};

#endif