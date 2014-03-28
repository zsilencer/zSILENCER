#ifndef DEDICATEDSERVER_H
#define DEDICATEDSERVER_H

#include "shared.h"
#include <vector>

class DedicatedServer
{
public:
	DedicatedServer();
	~DedicatedServer();
	void Start(char * lobbyaddress, unsigned short lobbyport, Uint32 gameid, Uint32 accountid);
	void Tick(class World & world);
	void SendHeartBeat(class World & world, Uint8 state);
	void Ban(Uint32 accountid);
	bool IsBanned(Uint32 accountid);
	bool active;
	Uint32 gameid;
	Uint32 accountid;
	int nopeerstime;
	bool checkedhavemap;
	
private:
	char lobbyaddress[256];
	unsigned short lobbyport;
	Uint8 state_i;
	SOCKET sockethandle;
	std::vector<Uint32> banlist;
};

#endif