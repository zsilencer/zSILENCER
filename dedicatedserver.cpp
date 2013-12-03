#include "dedicatedserver.h"
#include "world.h"
#include "team.h"
#include <algorithm>

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

void DedicatedServer::Start(char * lobbyaddress, unsigned short lobbyport, Uint32 gameid, Uint32 accountid){
	strcpy(DedicatedServer::lobbyaddress, lobbyaddress);
	DedicatedServer::lobbyport = lobbyport;
	DedicatedServer::gameid = gameid;
	DedicatedServer::accountid = accountid;
	active = true;
}

void DedicatedServer::Tick(World & world){
	if(world.gameplaystate == World::INLOBBY){
		for(int i = 0; i < world.maxpeers; i++){
			Peer * peer = world.peerlist[i];
			if(peer){
				User * user = world.lobby.GetUserInfo(peer->accountid);
				if(!user->retrieving){
					Team * team = world.GetPeerTeam(peer->id);
					if(team){
						if(user->agency[team->agency].level < world.gameinfo.minlevel || user->agency[team->agency].level > world.gameinfo.maxlevel){
							world.HandleDisconnect(peer->id);
						}
					}
				}
			}
		}
	}
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
	data.Put(gameid);
	data.Put(world.boundport);
	data.Put(state);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(lobbyport);
	addr.sin_addr.s_addr = inet_addr(lobbyaddress);
	sendto(sockethandle, data.data, data.BitsToBytes(data.offset), 0, (sockaddr *)&addr, sizeof(addr));
}

void DedicatedServer::Ban(Uint32 accountid){
	if(!IsBanned(accountid)){
		banlist.push_back(accountid);
	}
}

bool DedicatedServer::IsBanned(Uint32 accountid){
	if(std::find(banlist.begin(), banlist.end(), accountid) != banlist.end()){
		return true;
	}
	return false;
}