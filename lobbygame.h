#ifndef LOBBYGAME_H
#define LOBBYGAME_H

#include "shared.h"
#include "serializer.h"

class LobbyGame
{
public:
	LobbyGame();
	void Serialize(bool write, Serializer & data);
	void CalculateMapHash(void);
	char name[64];
	char mapname[64];
	Uint8 maphash[20];
	char hostname[64];
	char password[64];
	unsigned short port;
	Uint32 accountid;
	Uint8 players;
	Uint8 state;
	bool loaded;
	Uint32 createdtime;
};

#endif