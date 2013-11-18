#include "dedicatedserver.h"
#include "world.h"

DedicatedServer::DedicatedServer(){
	active = false;
	state_i = 100;
	sockethandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	unsigned long iomode = 1;
    ioctl(sockethandle, FIONBIO, &iomode);
	nopeerstime = 0;
}

DedicatedServer::~DedicatedServer(){
	shutdown(sockethandle, SHUT_RDWR);
    closesocket(sockethandle);
}

void DedicatedServer::Start(char * lobbyaddress, unsigned short lobbyport, Uint32 accountid){
	strcpy(DedicatedServer::lobbyaddress, lobbyaddress);
	DedicatedServer::lobbyport = lobbyport;
	DedicatedServer::accountid = accountid;
	active = true;
}

void DedicatedServer::Tick(World & world){
	if(world.peercount < 1){
		nopeerstime++;
	}
	if(state_i == 100){
		Uint8 state = 0;
		if(world.gameplaystate == World::INGAME){
			state = 1;
		}
		SendHeartBeat(world, state);
		state_i = 0;
	}
	state_i++;
}

void DedicatedServer::SendHeartBeat(World & world, Uint8 state){
	//printf("dedicated server sent heart beat update to %s:%d\n", lobbyaddress, lobbyport);
	Serializer data;
	char code = 0;
	data.Put(code);
	data.Put(accountid);
	data.Put(world.boundport);
	data.Put(state);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(lobbyport);
	addr.sin_addr.s_addr = inet_addr(lobbyaddress);
	sendto(sockethandle, data.data, data.BitsToBytes(data.offset), 0, (sockaddr *)&addr, sizeof(addr));
}