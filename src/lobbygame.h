#ifndef LOBBYGAME_H
#define LOBBYGAME_H

#include "shared.h"
#include "serializer.h"

class LobbyGame
{
public:
	LobbyGame();
	void Serialize(bool write, Serializer & data);
	enum {SECNONE, SECLOW, SECMEDIUM, SECHIGH};
	char name[64];
	char mapname[64];
	Uint8 maphash[20];
	char hostname[64];
	char password[64];
	unsigned short port;
	Uint32 id;
	Uint32 accountid;
	Uint8 players;
	Uint8 state;
	Uint8 securitylevel;
	Uint8 minlevel;
	Uint8 maxlevel;
	Uint8 maxplayers;
	Uint8 maxteams;
	Uint8 extra;
	bool loaded;
	bool mapdownloaded;
	Uint32 createdtime;
};

#endif