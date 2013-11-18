#ifndef DEDICATEDSERVER_H
#define DEDICATEDSERVER_H

#include "shared.h"

class DedicatedServer
{
public:
	DedicatedServer();
	~DedicatedServer();
	void Start(char * lobbyaddress, unsigned short lobbyport, Uint32 accountid);
	void Tick(class World & world);
	void SendHeartBeat(class World & world, Uint8 state);
	bool active;
	Uint32 accountid;
	int nopeerstime;
	
private:
	char lobbyaddress[256];
	unsigned short lobbyport;
	Uint8 state_i;
	SOCKET sockethandle;
};

#endif