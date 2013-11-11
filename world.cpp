#include "world.h"
#include "serializer.h"
#include "player.h"
#include "fixedcannon.h"
#include "walldefense.h"
#include "techstation.h"
#include "surveillancemonitor.h"
#include "team.h"
#include "objecttypes.h"
#include "terminal.h"
#include "basedoor.h"
#include "interface.h"
#include <algorithm>

#define DELTAENABLED 1

World::World(bool mode) : lobby(this), lagsimulator(&sockethandle), audio(Audio::GetInstance()){
	this->mode = mode;
	sockethandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	unsigned long iomode = 1;
    ioctl(sockethandle, FIONBIO, &iomode);
	currentid = 1;
	memset(peerlist, 0, sizeof(peerlist));
	peercount = 0;
	authoritypeer = 0;
	localpeerid = 0;
	tickcount = 0;
	memset(oldsnapshots, 0, sizeof(oldsnapshots));
	totalsnapshots = 0;
	totalinputpackets = 0;
	gravity = 3;
	maxyvelocity = 45;
	replaying = false;
	state = IDLE;
	illuminate = 0;
	totalbytesread = 0;
	totalbytessent = 0;
	quitstate = 0;
	winningteamid = 0;
	message[0] = 0;
	message_i = 0;
	showchat_i = 0;
	gameplaystate = NONE;
	LoadBuyableItems();
	lastpingsent = 0;
	pingtime = 0;
	highlightsecrets = false;
	highlightminimap = false;
	intutorialmode = false;
	memset(password, 0, sizeof(password));
	choosingtech = false;
	//lagsimulator.Activate(80, 100);
}

World::~World(){
	Disconnect();
    shutdown(sockethandle, SHUT_RDWR);
    closesocket(sockethandle);
	for(std::vector<BuyableItem *>::iterator it = buyableitems.begin(); it != buyableitems.end(); it++){
		delete (*it);
	}
	buyableitems.clear();
	for(unsigned int i = 0; i < maxpeers; i++){
		DeleteOldSnapshots(i);
	}
}

void World::DoNetwork(void){
	if(mode == AUTHORITY){
		DoNetwork_Authority();
	}else{
		DoNetwork_Replica();
	}
	lobby.DoNetwork();
	lagsimulator.Process(*this);
}

void World::Tick(void){
	if(mode == AUTHORITY){
		//if(rand() % 10 == 0){
		SendSnapshots();
		//}
	}
	if(mode == REPLICA){
		ProcessSnapshotQueue();
	}
	if(dedicatedserver.active){
		dedicatedserver.Tick(*this);
	}
	if(mode == REPLICA){
		/*replaying = true;
		 audio.enabled = false;
		 TickObjects();
		 audio.enabled = true;
		 replaying = false;*/
	}else{
		TickObjects();
	}
	if(tickcount % 25 == 0 && IsAuthority()){
		ActivateTerminals();
	}
	tickcount++;
	if(message_i){
		message_i++;
		if(message_i >= messagetime){
			message_i = 0;
		}
	}
	if(showchat_i){
		showchat_i--;
	}
	for(std::deque<char *>::iterator it = statusmessages.begin(); it != statusmessages.end(); it++){
		char * status = *it;
		char * time = &status[strlen(status) + 1];
		(*time)--;
		if(*time == 0){
			delete[] status;
			statusmessages.erase(it);
			break;
		}
	}
}

void World::TickObjects(void){
	for(std::list<Object *>::reverse_iterator i = objectlist.rbegin(); i != objectlist.rend(); i++){
		Object * object = (*i);
		bool peercontrolled = false;
		bool localpeercontrolled = false;
		if(object->iscontrollable){
			//if(IsAuthority()){
			for(int i = 0; i < maxpeers; i++){
				Peer * peer = peerlist[i];
				if(peer && !peer->isbot){
					for(std::list<Uint16>::iterator it = peer->controlledlist.begin(); it != peer->controlledlist.end(); it++){
						if((*it) == object->id){
							peercontrolled = true;
							if(peer == peerlist[localpeerid]){
								localpeercontrolled = true;
							}
						}
					}
				}
			}
			/*}else{
			 Peer * peer = peerlist[localpeerid];
			 if(peer){
			 for(std::list<Uint16>::iterator it = peer->controlledlist.begin(); it != peer->controlledlist.end(); it++){
			 if((*it) == object->id){
			 peercontrolled = true;
			 }
			 }
			 }
			 }*/
		}
		if(!(object->requiresmaptobeloaded && !map.loaded) && !object->wasdestroyed){
			if(!peercontrolled){
				object->oldx = object->x;
				object->oldy = object->y;
				object->Tick(*this);
			}else
			if(!IsAuthority() && !localpeercontrolled){
				// Tick everything to do the sounds and effects, then go back
				Serializer olddata;
				object->Serialize(Serializer::WRITE, olddata);
				//LoadSnapshot(*data, false);
				//replaying = true;
				//object->oldx = object->x;
				//object->oldy = object->y;
				object->Tick(*this);
				//replaying = false;
				object->Serialize(Serializer::READ, olddata);
				//
			}
		}
	}
	DestroyMarkedObjects();
	Player * localplayer = GetPeerPlayer(localpeerid);
	if(localplayer){
		audio.UpdateAllVolumes(*this, localplayer->x, localplayer->y, 500);
	}
}

void World::SetVersion(const char * version){
	strcpy(World::version, version);
}

void World::DoNetwork_Authority(void){
	Serializer data(1400);
	sockaddr_in senderaddr;
	socklen_t senderaddrsize = sizeof(senderaddr);
	int received;
	while((received = recvfrom(sockethandle, data.data, data.size, 0, (sockaddr *)&senderaddr, &senderaddrsize)) > 0){
		/*if(state == IDLE){
		 continue;
		 }*/
		data.offset = received * 8;
		data.readoffset = 0;
		totalbytesread += received;
		Peer * peer = FindPeer(senderaddr);
		if(peer){
			peer->lastpacket = SDL_GetTicks();
		}
		char code;
		data.Get(code);
		switch(code){
			case MSG_CONNECT:{
				printf("MSG_CONNECT received\n");
				Serializer response;
				Uint8 code = MSG_CONNECT;
				response.Put(code);
				if(gameplaystate == INLOBBY){
					char * host = inet_ntoa(senderaddr.sin_addr);
					unsigned short port = ntohs(senderaddr.sin_port);
					Uint8 agency;
					data.Get(agency);
					Uint32 accountid;
					data.Get(accountid);
					Uint8 passwordsize;
					data.Get(passwordsize);
					char temp[32];
					if(passwordsize > sizeof(temp)){
						passwordsize = sizeof(temp);
					}
					for(int i = 0; i < passwordsize; i++){
						data.Get(temp[i]);
					}
					if(memcmp(temp, password, passwordsize) == 0){
						Peer * newpeer = AddPeer(host, port, agency, accountid);
						if(newpeer){
							if(dedicatedserver.active){
								lobby.GetUserInfo(newpeer->accountid);
								if(newpeer->accountid == dedicatedserver.accountid){
									newpeer->ishost = true;
									newpeer->gameinfoloaded = true;
								}
							}
							response.PutBit(true);
							response.Put(newpeer->id);
						}else{
							response.PutBit(false);
						}
						if(newpeer){
							SendPeerList();
							if(!newpeer->ishost){
								SendGameInfo(newpeer->id);
							}
						}
					}else{
						response.PutBit(false);
					}
				}else{
					response.PutBit(false);
				}
				Peer temppeer;
				temppeer.ip = ntohl(senderaddr.sin_addr.s_addr);
				temppeer.port = ntohs(senderaddr.sin_port);
				SendPacket(&temppeer, response.data, response.BitsToBytes(response.offset));
			}break;
			case MSG_INPUT:{ // client sending input
				if(peer){
					totalinputpackets++;
					Uint32 theirlasttick;
					data.Get(theirlasttick);
					Uint32 lasttick;
					data.Get(lasttick);
					if(lasttick >= peer->lasttick){
						peer->theirlasttick = theirlasttick;
						peer->lasttick = lasttick;
						peer->port = ntohs(senderaddr.sin_port);
						peer->input.Serialize(Serializer::READ, data);
						for(std::list<Uint16>::iterator i = peer->controlledlist.begin(); i != peer->controlledlist.end(); i++){
							Object * object = GetObjectFromId((*i));
							if(object){
								object->oldx = object->x;
								object->oldy = object->y;
								object->HandleInput(peer->input);
								object->Tick(*this);
							}
						}
					}
				}
			}break;
			case MSG_PEERLIST:{ // peerlist requested
				if(peer){
					SendPeerList(peer->id);
				}
			}break;
			case MSG_DISCONNECT:{ // disconnect
				if(peer){
					HandleDisconnect(peer->id);
				}
			}break;
			case MSG_PING:{ // ping
				//printf("received ping from %s:%d\n", inet_ntoa(senderaddr.sin_addr), ntohs(senderaddr.sin_port));
				if(peer){
					Uint32 pingid;
					data.Get(pingid);
					Serializer response;
					Uint8 code = MSG_PONG;
					response.Put(code);
					response.Put(pingid);
					SendPacket(peer, response.data, response.BitsToBytes(response.offset));
					/*Serializer response;
					 Uint8 code = MSG_PONG;
					 response.Put(code);
					 unsigned short port;
					 data.Get(port);
					 response.Put(port);
					 sendto(sockethandle, response.data, response.BitsToBytes(response.offset), 0, (sockaddr *)&senderaddr, sizeof(senderaddr));*/
				}
			}break;
			case MSG_PONG:{ // pong
				//unsigned short port;
				//data.Get(port);
				//printf("received MSG_PONG\n");
				/*if(peer){
				 printf("lockedinport = %d\n", port);
				 peer->lockedinport = port;
				 peer->lastpacket = SDL_GetTicks();
				 SendPeerList();
				 }*/
			}break;
			case MSG_GAMEINFO:{
				printf("Received MSG_GAMEINFO\n");
				if(peer){
					if(peer->ishost){
						printf("loading game info from host\n");
						/*char info[256];
						 int i = 0;
						 while(data.MoreBytesToRead()){
						 data.Get(info[i++]);
						 }
						 gameinfo.LoadInfo(info);*/
						gameinfo.Serialize(Serializer::READ, data);
						Peer * localpeer = peerlist[localpeerid];
						if(localpeer){
							localpeer->gameinfoloaded = true;
						}
						for(int i = 0; i < maxpeers; i++){
							Peer * peer = peerlist[i];
							if(peer && peer->id != localpeerid && !peer->ishost){
								SendGameInfo(peer->id);
							}
						}
					}else{
						peer->gameinfoloaded = true;
						//SendGameInfo(peer->id);
					}
				}
			}break;
			case MSG_READY:{
				if(peer){
					if(peer->isready){
						peer->isready = false;
					}else{
						peer->isready = true;
					}
					SendPeerList();
					if(!peer->gameinfoloaded){
						SendGameInfo(peer->id);
					}
				}
			}break;
			case MSG_CHAT:{
				if(peer){
					Player * player = GetPeerPlayer(peer->id);
					Uint8 to;
					data.Get(to);
					Serializer response;
					Uint8 code = MSG_CHAT;
					response.Put(code);
					response.Put(peer->accountid);
					char * msg = &data.data[data.BitsToBytes(data.readoffset)];
					for(int i = 0; i < strlen(msg); i++){
						response.Put(msg[i]);
					}
					char nullend = 0;
					response.Put(nullend);
					if(to == 1){ // send to team
						if(player){
							Team * team = player->GetTeam(*this);
							if(team){
								for(int i = 0; i < team->numpeers; i++){
									if(peerlist[team->peers[i]] && team->peers[i] != localpeerid){
										SendPacket(peerlist[team->peers[i]], response.data, response.BitsToBytes(response.offset));
									}
								}
							}
						}
					}else{
						for(int i = 0; i < maxpeers; i++){
							if(peerlist[i] && i != localpeerid){
								SendPacket(peerlist[i], response.data, response.BitsToBytes(response.offset));
							}
						}
					}
				}
			}break;
			case MSG_BUY:{
				if(peer){
					Uint8 id;
					data.Get(id);
					Player * player = GetPeerPlayer(peer->id);
					if(player){
						player->BuyItem(*this, id);
					}
				}
			}break;
			case MSG_REPAIR:{
				if(peer){
					Uint8 id;
					data.Get(id);
					Player * player = GetPeerPlayer(peer->id);
					if(player){
						player->RepairItem(*this, id);
					}
				}
			}break;
			case MSG_VIRUS:{
				if(peer){
					Uint8 id;
					data.Get(id);
					Player * player = GetPeerPlayer(peer->id);
					if(player){
						player->VirusItem(*this, id);
					}
				}
			}break;
			case MSG_CHANGETEAM:{
				if(peer && gameplaystate == INLOBBY){
					Team * peerteam = GetPeerTeam(peer->id);
					if(peerteam){
						int start = peerteam->number + 1;
						if(FindTeamForPeer(*peer, peerteam->agency, start)){
							peerteam->RemovePeer(peer->id);
							peer->isready = false;
							SendPeerList();
						}
					}
				}
			}break;
			case MSG_TECH:{
				if(peer && gameplaystate == INLOBBY){
					Uint32 techchoices;
					data.Get(techchoices);
					lobby.GetUserInfo(peer->accountid);
					peer->wantedtechchoices = techchoices;
					ApplyWantedTech(*peer);
				}
			}break;
		}
	}
	Uint32 tickcheck = SDL_GetTicks();
	for(int i = 0; i < maxpeers; i++){
		if(peerlist[i]){
			if(i != localpeerid && !peerlist[i]->isbot && peerlist[i]->lastpacket < tickcheck && tickcheck - peerlist[i]->lastpacket >= peertimeout){
				HandleDisconnect(i);
			}
		}
	}
}

void World::DoNetwork_Replica(void){
	if(!peerlist[authoritypeer]){
		return;
	}
	/*if(!pingsent){
	 printf("no ping sent\n");
	 }*/
	if(SDL_GetTicks() - lastpingsent >= 1000){
		SendPing();
	}
	Serializer data(5000);
	sockaddr_in senderaddr;
	socklen_t senderaddrsize = sizeof(senderaddr);
	Peer * peer = 0;
	int received;
	while((received = recvfrom(sockethandle, data.data, data.size, 0, (sockaddr *)&senderaddr, &senderaddrsize)) > 0){
		//printf("received data from %s:%d\n", inet_ntoa(senderaddr.sin_addr), ntohs(senderaddr.sin_port));
		if(peerlist[authoritypeer]->ip == ntohl(senderaddr.sin_addr.s_addr) && peerlist[authoritypeer]->port == ntohs(senderaddr.sin_port)){
			peer = peerlist[authoritypeer];
			peer->lastpacket = SDL_GetTicks();
		}
		data.offset = received * 8;
		data.readoffset = 0;
		totalbytesread += received;
		char code;
		data.Get(code);
		switch(code){
			case MSG_CONNECT:{ // connect response
				printf("MSG_CONNECT response received from %s:%d\n", inet_ntoa(senderaddr.sin_addr), ntohs(senderaddr.sin_port));
				if(peer){
					if(data.GetBit()){
						data.Get(localpeerid);
						//unsigned short port;
						//data.Get(port);
						//GetAuthorityPeer()->lockedinport = port;
						printf("we are connected, our peer id is %d\n", localpeerid);
						RequestPeerList();
					}else{
						printf("failed to connect to game\n");
						// host not accepting connection, password is wrong, or teams are full, etc
					}
				}
			}break;
			case MSG_SNAPSHOT:{ // snapshot data
				if(peer){
					if(state == CONNECTED){
						totalsnapshots++;
						Serializer * snapshotcopy = new Serializer;
						snapshotcopy->Copy(data);
						snapshotqueue.push_back(snapshotcopy);
					}
				}
			}break;
			case MSG_PEERLIST:{ // peerlist update
				if(peer){
					ReadPeerList(data);
					Peer * localpeer = peerlist[localpeerid];
					if(localpeer && localpeer->ishost && !GetAuthorityPeer()->gameinfoloaded && gameinfo.loaded){
						SendGameInfo(GetAuthorityPeer()->id);
						printf("We are host, sending game info\n");
					}
					if(state == CONNECTING && localpeer){
						state = CONNECTED;
					}
				}
			}break;
			case MSG_DISCONNECT:{ // disconnect
				if(peer){
					HandleDisconnect(authoritypeer);
				}
			}break;
			case MSG_PING:{ // ping
				//printf("received ping from %s:%d\n", inet_ntoa(senderaddr.sin_addr), ntohs(senderaddr.sin_port));
				/*Serializer response;
				 Uint8 code = MSG_PONG;
				 response.Put(code);
				 unsigned short port;
				 data.Get(port);
				 response.Put(port);
				 sendto(sockethandle, response.data, response.BitsToBytes(response.offset), 0, (sockaddr *)&senderaddr, sizeof(senderaddr));*/
			}break;
			case MSG_PONG:{ // pong
				//printf("received MSG_PONG\n");
				Uint32 pingid;
				data.Get(pingid);
				if(pingid == lastpingid){
					pingtime = SDL_GetTicks() - lastpingsent;
				}
			}break;
			case MSG_GAMEINFO:{
				if(peer){
					printf("Received MSG_GAMEINFO\n");
					gameinfo.Serialize(Serializer::READ, data);
					/*char info[256];
					 int i = 0;
					 while(data.MoreBytesToRead()){
					 data.Get(info[i++]);
					 }
					 gameinfo.LoadInfo(info);*/
					Serializer response;
					Uint8 code = MSG_GAMEINFO;
					response.Put(code);
					SendPacket(GetAuthorityPeer(), response.data, response.BitsToBytes(response.offset));
				}
			}break;
			case MSG_CHAT:{
				//printf("Received MSG_CHAT\n");
				Uint32 accountid;
				data.Get(accountid);
				std::string chatmsg(lobby.GetUserInfo(accountid)->name);
				chatmsg.append(":\xA0");
				chatmsg.append(&data.data[1 + 4]);
				
				char * wrapped = Interface::WordWrap(chatmsg.c_str(), 36);
				char * line = strtok(wrapped, "\n");
				while(line){
					chatlines.push_back(line);
					line = strtok(NULL, "\n");
				}
				delete[] wrapped;
				
				//chatlines.push_back(chatmsg);
				showchat_i = 255;
				while(chatlines.size() > 5){
					chatlines.pop_front();
				}
			}break;
			case MSG_STATUS:{
				int size = data.BitsToBytes(data.offset - data.readoffset);
				char * newstatus = new char[size];
				memcpy(newstatus, &data.data[data.BitsToBytes(data.readoffset)], size);
				PushStatusString(newstatus);
			}break;
			case MSG_MESSAGE:{
				char message[1024];
				strcpy(message, &data.data[data.BitsToBytes(data.readoffset)]);
				Uint8 time = data.data[data.BitsToBytes(data.readoffset) + strlen(message) + 1];
				Uint8 type = data.data[data.BitsToBytes(data.readoffset) + strlen(message) + 1 + 1];
				ShowMessage(message, time, type);
			}break;
			case MSG_STATS:{
				if(peer){
					Peer * localpeer = peerlist[localpeerid];
					if(localpeer){
						localpeer->stats.Serialize(Serializer::READ, data);
						printf("MSG_STATS received\n");
						Team * team = GetPeerTeam(localpeer->id);
						if(team){
							User * user = lobby.GetUserInfo(localpeer->accountid);
							char namecopy[64];
							strcpy(namecopy, user->name);
							lobby.ForgetUserInfo(localpeer->accountid);
							user = lobby.GetUserInfo(localpeer->accountid);
							strcpy(user->name, namecopy);
							user->statscopy = localpeer->stats;
							user->statsagency = team->agency;
						}

					}
				}
			}break;
			case MSG_GOVTKILL:{
				if(peer){
					Uint8 peerid = data.data[data.BitsToBytes(data.readoffset)];
					Player * player = GetPeerPlayer(peerid);
					if(player){
						player->KillByGovt(*this);
					}
				}
			}break;
			case MSG_SOUND:{
				if(peer){
					Audio::GetInstance().Play(resources.soundbank[&data.data[data.BitsToBytes(data.readoffset)]]);
				}
			}break;
		}
	}
	Uint32 tickcheck = SDL_GetTicks();
	if(peerlist[authoritypeer]){
		if(peerlist[authoritypeer]->lastpacket < tickcheck && tickcheck - peerlist[authoritypeer]->lastpacket >= peertimeout){
			HandleDisconnect(authoritypeer);
		}
	}
}

Peer * World::AddPeer(char * address, unsigned short port, Uint8 agency, Uint32 accountid){
	Uint8 newpeerid = 0;
	sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr(address);
	addr.sin_port = htons(port);
	Peer * peer = FindPeer(addr);
	if(!peer){
		bool peeradded = false;
		for(unsigned int i = 1; i < maxpeers; i++){
			if(!peerlist[i]){
				newpeerid = i;
				peeradded = true;
				break;
			}
		}
		Peer * newpeer = new Peer();
		newpeer->id = newpeerid;
		newpeer->ip = ntohl(inet_addr(address));
		newpeer->port = port;
		newpeer->accountid = accountid;
		if(peeradded){
			/*if(newpeer->accountid == dedicatedserver.lobbygame.accountid){
			 newpeer->ishost = true;
			 }*/
			if(!FindTeamForPeer(*newpeer, agency)){
				printf("could not find team for new peer\n");
				delete newpeer;
				return 0;
			}
			
			peerlist[newpeerid] = newpeer;
			peercount++;
			printf("new peer added, peer id: %d (%s:%d) peercount = %d\n", newpeerid, address, port, peercount);
			/*if(gameplaystate == INGAME){
			 Player * newplayer = (Player *)CreateObject(ObjectTypes::PLAYER);
			 newplayer->suitcolor = rand() % 0xFF;
			 newpeer->controlledlist.push_back(newplayer->id);
			 }*/
			return newpeer;
		}else{
			delete newpeer;
			return 0;
		}
	}else{
		printf("existing peer added, peer id: %d\n", peer->id);
		return peer;
	}
	return 0;
}

Peer * World::AddBot(Uint8 agency){
	Uint8 newpeerid = 0;
	bool peeradded = false;
	for(unsigned int i = 1; i < maxpeers; i++){
		if(!peerlist[i]){
			newpeerid = i;
			peeradded = true;
			break;
		}
	}
	Peer * newpeer = new Peer();
	newpeer->id = newpeerid;
	if(peeradded){
		if(!FindTeamForPeer(*newpeer, agency)){
			printf("could not find team for new peer\n");
			delete newpeer;
			return 0;
		}
		newpeer->isbot = true;
		peerlist[newpeerid] = newpeer;
		peercount++;
		printf("added bot, peer id: %d peercount = %d\n", newpeerid, peercount);
		return newpeer;
	}else{
		delete newpeer;
		return 0;
	}
	return 0;
}

Peer * World::FindPeer(sockaddr_in & sockaddr){
	Peer * peer = 0;
	for(int i = 0; i < maxpeers; i++){
		if(peerlist[i]){
			if(peerlist[i]->ip == ntohl(sockaddr.sin_addr.s_addr)){
				if(peerlist[i]->port == ntohs(sockaddr.sin_port)){
					peer = peerlist[i];
				}
			}
		}
	}
	return peer;
}

void World::ProcessSnapshotQueue(void){
	bool shortenqueue = false;
	if(snapshotqueue.size() >= snapshotqueuemaxsize){
		shortenqueue = true;
	}
	if(snapshotqueue.size() >= snapshotqueueminsize){
		do{
			Serializer * data = snapshotqueue.front();
			Uint32 tick;
			data->Get(tick);
			Uint32 ourtick;
			data->Get(ourtick);
			unsigned int readoffset = data->readoffset;
			Uint32 deltatick;
			data->Get(deltatick);
			data->readoffset = readoffset;
			if(peerlist[localpeerid]){
				Serializer ** deltasnapshotptr = &oldsnapshots[localpeerid][deltatick % maxoldsnapshots];
				if(tick > peerlist[localpeerid]->lasttick){
					
					
					
					
					//printf("deltatick: %d, newtick: %d, ourtick: %d, localtick: %d, replaying %d, ticks queue size:%d\n", deltatick, tick, ourtick, peerlist[localpeerid]->lasttick, replayticks, snapshotqueue.size());
					if(DELTAENABLED && *deltasnapshotptr && tick - deltatick < maxoldsnapshots){
						LoadSnapshot(*data, true, *deltasnapshotptr);
					}else{
						LoadSnapshot(*data, true);
					}
					
					Serializer ** newsnapshotptr = &oldsnapshots[localpeerid][tick % maxoldsnapshots];
					if(!*newsnapshotptr){
						*newsnapshotptr = new Serializer;
					}
					(*newsnapshotptr)->offset = 0;
					SaveSnapshot(**newsnapshotptr, localpeerid);
					
					TickObjects();
					
					// Perform client side predication
					int replayticks = tickcount - ourtick - 1;
					if(replayticks < maxlocalinputhistory - 1){
						for(int i = replayticks; i > 0; i--){
							for(std::list<Uint16>::iterator it = peerlist[localpeerid]->controlledlist.begin(); it != peerlist[localpeerid]->controlledlist.end(); it++){
								Object * object = GetObjectFromId((*it));
								if(object){
									// set the oldinput
									if(object->type == ObjectTypes::PLAYER){
										Player * player = (Player *)object;
										player->oldinput = localinputhistory[(tickcount - i - 1) % maxlocalinputhistory];
									}
									//
									object->HandleInput(localinputhistory[(tickcount - i) % maxlocalinputhistory]);
									object->oldx = object->x;
									object->oldy = object->y;
									replaying = true;
									audio.enabled = false;
									object->Tick(*this);
									audio.enabled = true;
									replaying = false;
								}
							}
						}
					}
					//
					peerlist[localpeerid]->lasttick = tick;
				}else
					if(tick < peerlist[localpeerid]->lasttick){
						/*Serializer olddata;
						 SaveSnapshot(olddata, localpeerid);
						 LoadSnapshot(*data, false);
						 replaying = true;
						 TickObjects();
						 replaying = false;
						 LoadSnapshot(olddata, false);
						 printf("old snapshot replayed %d\n", tick);*/
					}else
						if(tick == peerlist[localpeerid]->lasttick){
							// this shouldnt happen
						}
			}
			delete data;
			snapshotqueue.pop_front();
		}while(shortenqueue && snapshotqueue.size() > snapshotqueueminsize);
	}
}

void World::ClearSnapshotQueue(void){
	for(std::list<Serializer *>::iterator it = snapshotqueue.begin(); it != snapshotqueue.end(); it++){
		Serializer * data = *it;
		delete data;
	}
	snapshotqueue.clear();
}

void World::ReadPeerList(Serializer & data){
	//printf("reading peerlist\n");
	for(unsigned int i = 0; i < maxpeers; i++){
		if(i == authoritypeer){
			continue;
		}
		Peer * peer = peerlist[i];
		if(peer){
			delete peer;
			peerlist[i] = 0;
			peercount--;
		}
	}
	while(data.MoreBytesToRead()){
		Peer * peer = new Peer();
		peer->Serialize(Serializer::READ, data);
		if(peer->id == authoritypeer && peerlist[peer->id]){
			peerlist[peer->id]->accountid = peer->accountid;
			peerlist[peer->id]->controlledlist = peer->controlledlist;
			peerlist[peer->id]->gameinfoloaded = peer->gameinfoloaded;
			peerlist[peer->id]->isready = peer->isready;
			peerlist[peer->id]->ishost = peer->ishost;
			delete peer;
			continue;
		}
		peerlist[peer->id] = peer;
		//printf("peerlist[%d]\n", peer->id);
		peercount++;
	}
}

void World::SendPacket(Peer * peer, char * data, unsigned int size){
	if(peer){
		if(lagsimulator.Active()){
			lagsimulator.QueuePacket(peer, data, size);
		}else{
			sockaddr_in recvaddr;
			recvaddr.sin_family = AF_INET;
			recvaddr.sin_port = htons(peer->port);
			recvaddr.sin_addr.s_addr = htonl(peer->ip);
			int ret = sendto(sockethandle, data, size, 0, (sockaddr *)&recvaddr, sizeof(recvaddr));
			if(ret > 0){
				totalbytessent += ret;
			}
		}
	}
}

void World::SwitchToMode(bool newmode){
	if(newmode == REPLICA && mode == AUTHORITY){
		mode = REPLICA;
		for(unsigned int i = 0; i < maxpeers; i++){
			if(peerlist[i]){
				authoritypeer = i;
				break;
			}
		}
	}else
		if(newmode == AUTHORITY && mode == REPLICA){
			mode = AUTHORITY;
			authoritypeer = localpeerid;
			for(unsigned int i = 0; i < maxpeers; i++){
				if(peerlist[i]){
					peerlist[i]->lastpacket = SDL_GetTicks();
				}
			}
			Listen(GetAuthorityPeer()->port);
			SendPeerList();
		}
}

void World::DeleteOldSnapshots(Uint8 peerid){
	for(unsigned int i = 0; i < maxoldsnapshots; i++){
		if(oldsnapshots[peerid][i]){
			delete oldsnapshots[peerid][i];
			oldsnapshots[peerid][i] = 0;
		}
	}
}

void World::HandleDisconnect(Uint8 peerid){
	printf("peer %d disconnected\n", peerid);
	for(std::list<Uint16>::iterator i = peerlist[peerid]->controlledlist.begin(); i != peerlist[peerid]->controlledlist.end(); i++){
		Object * object = GetObjectFromId((*i));
		if(object){
			object->HandleDisconnect(*this, peerid);
		}
	}
	ClearSnapshotQueue();
	for(std::list<Object *>::iterator it = objectlist.begin(); it != objectlist.end(); it++){
		Object * object = *it;
		if(object->type == ObjectTypes::TEAM){
			Team * team = static_cast<Team *>(object);
			if(team){
				team->RemovePeer(peerid);
			}
		}
	}
	if(mode == REPLICA){
		/*for(std::list<Uint16>::iterator i = peerlist[peerid]->controlledlist.begin(); i != peerlist[peerid]->controlledlist.end(); i++){
			Object * object = GetObjectFromId((*i));
			if(object){
				object->HandleDisconnect(*this, peerid);
			}
		}*/
		delete peerlist[peerid];
		peerlist[peerid] = 0;
		peercount--;
		if(peerid == authoritypeer){
			ShowMessage("CONNECTION LOST", 128, 20);
			state = IDLE;
		}
	}else
	if(mode == AUTHORITY){
		/*for(std::list<Uint16>::iterator i = peerlist[peerid]->controlledlist.begin(); i != peerlist[peerid]->controlledlist.end(); i++){
			Object * object = GetObjectFromId((*i));
			if(object){
				object->HandleDisconnect(*this, peerid);
			}
		}*/
		delete peerlist[peerid];
		peerlist[peerid] = 0;
		peercount--;
		for(int i = 0; i < maxoldsnapshots; i++){
			if(oldsnapshots[peerid][i]){
				delete oldsnapshots[peerid][i];
				oldsnapshots[peerid][i] = 0;
			}
		}
		SendPeerList();
	}
}

bool World::RelevantToPlayer(Player * player, Object * object){
	switch(object->type){
		case ObjectTypes::TEAM:
		case ObjectTypes::STATE:
			return true;
		break;
		case ObjectTypes::PLAYER:{
			Player * objplayer = static_cast<Player *>(object);
			if(objplayer->hassecret || objplayer->state == Player::UNDEPLOYING){
				return true;
			}
		}break;
		case ObjectTypes::PICKUP:{
			PickUp * pickup = static_cast<PickUp *>(object);
			if(pickup->type == PickUp::SECRET){
				return true;
			}
		}break;
		case ObjectTypes::SURVEILLANCEMONITOR:{
			
		}break;
	}
	if(!player){
		return false;
	}
	if(rand() % objectlist.size() == 0){
		return true;
	}
	if(object->snapshotinterval >= 0 && tickcount % (object->snapshotinterval + 1) == 0){
		return true;
	}
	if(object->issprite){
		if(abs(player->x - object->x) <= 500 && abs(player->y - object->y) <= 450){
			return true;
		}
		/*if(player->InBase(*this)){
			BaseDoor * basedoor = static_cast<BaseDoor *>(GetObjectFromId(player->basedoorentering));
			if(basedoor){
				if(abs(basedoor->x - object->x) <= 300 && abs(basedoor->y - object->y) <= 300){
					return true;
				}
			}
		}*/
		for(std::list<Object *>::iterator it = objectlist.begin(); it != objectlist.end(); it++){
			if((*it)->type == ObjectTypes::SURVEILLANCEMONITOR){
				if(abs(player->x - (*it)->x) <= 500 && abs(player->y - (*it)->y) <= 500){
					SurveillanceMonitor * surveillancemonitor = static_cast<SurveillanceMonitor *>(*it);
					if(surveillancemonitor->camera.IsVisible(*this, *object)){
						return true;
					}
				}
			}
		}
		Object * grenade = GetObjectFromId(player->currentgrenade);
		if(grenade){
			if(abs(grenade->x - object->x) <= 300 && abs(grenade->y - object->y) <= 300){
				return true;
			}
		}
		Object * detonator = GetObjectFromId(player->currentdetonator);
		if(detonator){
			if(abs(detonator->x - object->x) <= 300 && abs(detonator->y - object->y) <= 300){
				return true;
			}
		}
	}
	return false;
}

bool World::BelongsToTeam(Object & object, Uint16 teamid){
	switch(object.type){
		case ObjectTypes::PLAYER:{
			Player * player = static_cast<Player *>(&object);
			if(player->teamid == teamid){
				return true;
			}
		}break;
		case ObjectTypes::FIXEDCANNON:{
			FixedCannon * fixedcannon = static_cast<FixedCannon *>(&object);
			Player * player = static_cast<Player *>(GetObjectFromId(fixedcannon->ownerid));
			if(player && player->teamid == teamid){
				return true;
			}
		}break;
		case ObjectTypes::WALLDEFENSE:{
			WallDefense * walldefense = static_cast<WallDefense *>(&object);
			if(walldefense->teamid == teamid){
				return true;
			}
		}break;
		case ObjectTypes::TECHSTATION:{
			TechStation * techstation = static_cast<TechStation *>(&object);
			if(techstation->teamid == teamid){
				return true;
			}
		}break;
	}
	return false;
}

void World::ActivateTerminals(void){
	std::vector<Terminal *> terminallist;
	for(std::list<Object *>::iterator it = objectlist.begin(); it != objectlist.end(); it++){
		Object * object = *it;
		if(object->type == ObjectTypes::TERMINAL){
			Terminal * terminal = static_cast<Terminal *>(object);
			if(terminal){
				terminallist.push_back(terminal);
			}
		}
	}
	std::random_shuffle(terminallist.begin(), terminallist.end());
	int numused = 0;
	int numsecret = 0;
	for(int i = 0; i < terminallist.size(); i++){
		switch(terminallist[i]->state){
			case Terminal::HACKING:
			case Terminal::BEAMING:
			case Terminal::READY:{
				numused++;
			}break;
			case Terminal::SECRETBEAMING:
			case Terminal::SECRETREADY:{
				numsecret++;
			}
		}
	}
	int numtoactivate = terminallist.size() * 0.35;
	numtoactivate -= numused;
	numtoactivate += numsecret;
	int numactivated = 0;
	if(numtoactivate > 0){
		for(int i = 0; i < terminallist.size(); i++){
			Terminal * terminal = terminallist[i];
			if(terminal->state == Terminal::INACTIVE){
				terminal->state = Terminal::BEAMING;
				if(terminal->isbig){
					terminal->beamingseconds = (rand() % 26) + 10;
				}else{
					terminal->beamingseconds = (rand() % 10) + 1;
				}
				numactivated++;
				if(numactivated >= numtoactivate){
					break;
				}
			}
		}
	}
}

void World::LoadBuyableItems(void){
	// 97:0 base door, 1 health pack, 2 laz tract, 3 security pass, 4 camera, 5 poison, 6-9 bomb, 10 flare, 11 cannon, 12 plasma det, 13 poison flare, 14 virus, 15 base def, 16 laser ammo, 17 rockets, 18 flamer,
	buyableitems.push_back(new BuyableItem(BUY_LASER, "Laser", 150, 300, 97, 16, 1 << 0, 1, "Immediately after shield technology\nwas capable of dampening weapon\neffects, the Frost-Light laser was\nreleased.\n \nTwo hits should remove any standard\nshield, but very little damage is\ndone to an unshielded target."));
	buyableitems.push_back(new BuyableItem(BUY_ROCKET, "Rocket", 250, 400, 97, 17, 1 << 1, 1, "Long range, high yield mini-warheads\nwill devastate any unshielded\nopponent.\n \nIncludes an attached camera for\nin-flight kill tracking."));
	buyableitems.push_back(new BuyableItem(BUY_FLAMER, "Flamer Ammo", 200, 350, 97, 18, 1 << 2, 1, "Kills any unshielded target.\n \nNo shield has been made that the\nCrucible flamer cannot ignore.\n \nDeadly at close range."));
	buyableitems.push_back(new BuyableItem(BUY_HEALTH, "Health Pack", 200, 400, 97, 1, 1 << 3, 1, "A small portable boost for Noxis\nagents in the field.\n \nRestores lost health, but must be\nmanually applied.", Team::NOXIS));
	buyableitems.push_back(new BuyableItem(BUY_TRACT, "Lazarus Tract", 250, 500, 97, 2, 1 << 14, 1, "The definitive and unquestionable\ntruth about Mars' oddest and most\npowerful religious organization.\n \nHelpful in converting disbelieving\ncitizens to your cause.", Team::LAZARUS));
	buyableitems.push_back(new BuyableItem(BUY_SECURITYPASS, "Security Pass", 1000, 1000, 97, 3, 1 << 17, 1, "Unlimited security access.\n \nGovernment agents will ignore you\nin the field.", Team::CALIBER));
	buyableitems.push_back(new BuyableItem(BUY_VIRUS, "Virus", 400, 300, 97, 14, 1 << 19, 1, "A portable virus whipped up by the\nsnotty underage hackers of Static.\n \nGives you immediate control over\ncannons and robots in the field.\n \nMay harm enemy in-base weapon\nstations.", Team::STATIC));
	buyableitems.push_back(new BuyableItem(BUY_POISON, "Poison", 100, 400, 97, 5, 1 << 15, 1, "Hollowhead poisons are pernicious\nbio-rad toxins, administered via\ninjection.\n \nCauses the victim to lose health\nuntil cured, multiple doses\nrecommended.", Team::BLACKROSE));
	buyableitems.push_back(new BuyableItem(BUY_EMPB, "E.M.P. Bomb", 1000, 1000, 97, 6, 1 << 4, 4, "Upon detonation it emits a\nelectromagnetic pulse that drops all\nshields in a several mile radius.\n \nThe user is protected by a frequency\nmodulator that comes with the\ndevice."));
	buyableitems.push_back(new BuyableItem(BUY_SHAPEDB, "Shaped Bomb", 100, 200, 97, 7, 1 << 5, 1, "Derived from the plasma bomb, this\nbomb focus its destructive force \nupwards."));
	buyableitems.push_back(new BuyableItem(BUY_PLASMAB, "Plasma Bomb", 200, 300, 97, 8, 1 << 6, 2, "An indescriminate terrorist device\ncapable of killing even a\nperfectly healthy opponent.\n \nA core explosion with multiple\nshrapnel tendrils, extremely\nlethal."));
	buyableitems.push_back(new BuyableItem(BUY_NEUTRONB, "Neutron Bomb", 4000, 2000, 97, 9, 1 << 7, 8, "A portable neutron bomb that\nswiftly elimates all opposition in\nthe entire region.\n \nThe only defense against it is to\nbe in your base.\n \nUse with extreme caution."));
	buyableitems.push_back(new BuyableItem(BUY_DET, "Plasma Detonator", 200, 400, 97, 12, 1 << 8, 2, "A remote detonation device.\n \nDeploy in strategic locations and\ndetonate from a safe distance."));
	buyableitems.push_back(new BuyableItem(BUY_FIXEDC, "Fixed Cannon", 300, 500, 97, 11, 1 << 9, 1, "A stationary laser turret with\nexcellent offensive and defensive\ncapabilities.\n \nThis mechanical friend in the\nfield can do your work for you."));
	buyableitems.push_back(new BuyableItem(BUY_FLARE, "Flare", 200, 400, 97, 10, 1 << 10, 1, "The Orion Flare is a stationary\nmini-torch looks like a bomb in\nflight but has a more lingering\neffect.\n \nUseful for blocking off tight areas\nor herding enemies."));
	buyableitems.push_back(new BuyableItem(BUY_POISONFLARE, "Poison Flare", 200, 700, 97, 13, 1 << 16, 1, "A more devious strain of the Orion\nFlare, the Poison Flare\nincapacitates its victims in seconds\nand leaves them poisoned as\nwell.\n \nEnvironmentally disastrous, but\nhighly lethal.", Team::BLACKROSE));
	buyableitems.push_back(new BuyableItem(BUY_CAMERA, "Camera", 100, 200, 97, 4, 1 << 18, 1, "A remote viewing device that allows\nan agent to monitor an area on\nhis/her HUD.\n \nCan be discreetly removed in a puff\nof smoke.\n \nWhen secrecy is a way of life."));
	buyableitems.push_back(new BuyableItem(BUY_DOOR, "Base Door", 300, 600, 97, 0, 1 << 11, 1, "The ability to relocate the warp\ndoor to your base."));
	buyableitems.push_back(new BuyableItem(BUY_DEFENSE, "Base Defense", 100, 500, 97, 15, 1 << 12, 1, "Internal security systems to deter\nwould-be ambushers.\n \nIn-base laser turrets.\n \nMultiple purchases increase turret\nstructure durability."));
	buyableitems.push_back(new BuyableItem(BUY_INFO, "Insider Info", 500, 500, 0xFF, 0, 1 << 13, 1, "Your contacts on the inside can\ngive you information about secrets.\n \nFor a price of course."));
	buyableitems.push_back(new BuyableItem(BUY_GIVE0, "Give To ", 100, 100, 0xFF, 0, 0, 0, ""));
	buyableitems.push_back(new BuyableItem(BUY_GIVE1, "Give To ", 100, 100, 0xFF, 0, 0, 0, ""));
	buyableitems.push_back(new BuyableItem(BUY_GIVE2, "Give To ", 100, 100, 0xFF, 0, 0, 0, ""));
	buyableitems.push_back(new BuyableItem(BUY_GIVE3, "Give To ", 100, 100, 0xFF, 0, 0, 0, ""));
}

void World::BuyItem(Uint8 id){
	char msg[2];
	msg[0] = MSG_BUY;
	msg[1] = id;
	SendPacket(GetAuthorityPeer(), msg, sizeof(msg));
}

void World::RepairItem(Uint8 id){
	char msg[2];
	msg[0] = MSG_REPAIR;
	msg[1] = id;
	SendPacket(GetAuthorityPeer(), msg, sizeof(msg));
}

void World::VirusItem(Uint8 id){
	char msg[2];
	msg[0] = MSG_VIRUS;
	msg[1] = id;
	SendPacket(GetAuthorityPeer(), msg, sizeof(msg));
}

void World::SendStats(Peer & peer){
	Serializer msg;
	Uint8 code = MSG_STATS;
	msg.Put(code);
	peer.stats.Serialize(Serializer::WRITE, msg);
	SendPacket(&peer, msg.data, msg.BitsToBytes(msg.offset));
}

void World::UserInfoReceived(Peer & peer){
	ApplyWantedTech(peer);
}

void World::ApplyWantedTech(Peer & peer){
	Team * peerteam = GetPeerTeam(peer.id);
	User * user = lobby.GetUserInfo(peer.accountid);
	Uint32 techchoices = peer.wantedtechchoices;
	if(peerteam){
		Uint32 oldtechchoices = peer.techchoices;
		for(std::vector<BuyableItem *>::iterator it = buyableitems.begin(); it != buyableitems.end(); it++){
			BuyableItem * buyableitem = *it;
			if(buyableitem->agencyspecific != -1 && buyableitem->agencyspecific != peerteam->agency){
				techchoices &= ~(buyableitem->techchoice);
			}
		}
		peer.techchoices = techchoices;
		if(user && user->agency[peerteam->agency].techslots >= TechSlotsUsed(peer)){
			SendPeerList();
		}else{
			peer.techchoices = oldtechchoices;
		}
	}
}

bool World::CompareTeamByNumber(Team * team1, Team * team2){
	return(team1->number < team2->number);
}

bool World::Listen(unsigned short port){
	if(!boundport){
		if(!Bind(port)){
			return false;
		}
	}
	SwitchToMode(AUTHORITY);
	state = LISTENING;
	Peer * authoritypeerptr = GetAuthorityPeer();
	authoritypeerptr->ip = INADDR_ANY;
	authoritypeerptr->port = boundport;
	printf("Listening on port %d\n", boundport);
	return true;
}

unsigned short World::Bind(unsigned short port){
	sockaddr_in recvaddr;
	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(port);
	recvaddr.sin_addr.s_addr = INADDR_ANY;
	int ret = bind(sockethandle, (sockaddr *)&recvaddr, sizeof(recvaddr));
	if(ret == 0){
		sockaddr_in boundaddr;
		socklen_t boundaddrlen = sizeof(boundaddr);
		getsockname(sockethandle, (sockaddr *)&boundaddr, &boundaddrlen);
		boundport = ntohs(boundaddr.sin_port);
		return boundport;
	}
	return false;
}

void World::Connect(Uint8 agency, Uint32 accountid, const char * password){
	sockaddr_in addr;
	addr.sin_addr.s_addr = htonl(GetAuthorityPeer()->ip);
	printf("sending connect request with agency %d, accountid %d to %s:%d\n", agency, accountid, inet_ntoa(addr.sin_addr), GetAuthorityPeer()->port);
	SwitchToMode(REPLICA);
	state = CONNECTING;
	authoritypeer = 0;
	GetAuthorityPeer()->lastpacket = SDL_GetTicks();
	/*authoritypeer = 0;
	 if(!peerlist[authoritypeer]){
	 peerlist[authoritypeer] = new Peer();
	 }
	 peerlist[authoritypeer]->id = authoritypeer;
	 peerlist[authoritypeer]->ip = ntohl(inet_addr(host));
	 peerlist[authoritypeer]->port = port;
	 peerlist[authoritypeer]->lastpacket = SDL_GetTicks();*/
	Serializer data;
	Uint8 code = MSG_CONNECT;
	data.Put(code);
	//data.Put(boundport);
	data.Put(agency);
	data.Put(accountid);
	Uint8 passwordsize = password ? strlen(password) : 0;
	data.Put(passwordsize);
	for(int i = 0; i < passwordsize; i++){
		data.Put(password[i]);
	}
	SendPacket(GetAuthorityPeer(), data.data, data.offset);
}

void World::Disconnect(void){
	ClearSnapshotQueue();
	state = IDLE;
	char data[1];
	data[0] = MSG_DISCONNECT;
	if(mode == AUTHORITY){
		for(int i = 0; i < maxpeers; i++){
			Peer * peer = peerlist[i];
			if(peer){
				SendPacket(peer, data, sizeof(data));
			}
		}
	}else
		if(mode == REPLICA){
			SendPacket(GetAuthorityPeer(), data, sizeof(data));
		}
}

void World::SendInput(void){
	Peer * peer = 0;
	localinputhistory[tickcount % maxlocalinputhistory] = localinput;
	if(mode == REPLICA && state == CONNECTED){
		peer = peerlist[localpeerid];
		if(peer){
			peer->input = localinput;
			peer->input.mousex = 0xFFFF;
			peer->input.mousey = 0xFFFF;
			Serializer data;
			Uint8 code = MSG_INPUT;
			data.Put(code);
			data.Put(tickcount);
			data.Put(peerlist[localpeerid]->lasttick);
			peer->input.Serialize(Serializer::WRITE, data);
			SendPacket(GetAuthorityPeer(), data.data, data.offset);
		}
	}else
	if(mode == AUTHORITY){
		peer = GetAuthorityPeer();
		peer->input = localinput;
	}
	if(peer){
		for(std::list<Uint16>::iterator i = peer->controlledlist.begin(); i != peer->controlledlist.end(); i++){
			Object * object = GetObjectFromId((*i));
			if(object){
				object->oldx = object->x;
				object->oldy = object->y;
				object->HandleInput(peer->input);
				object->Tick(*this);
			}
		}
	}
}

Peer * World::GetAuthorityPeer(void){
	if(!peerlist[authoritypeer]){
		peerlist[authoritypeer] = new Peer();
		peerlist[authoritypeer]->ip = INADDR_ANY;
		peerlist[authoritypeer]->id = authoritypeer;
	}
	return peerlist[authoritypeer];
}

Player * World::GetPeerPlayer(Uint8 peerid){
	Object * object = 0;
	Player * player = 0;
	if(peerlist[peerid]){
		if(peerlist[peerid]->controlledlist.size() > 0){
			object = GetObjectFromId((*peerlist[peerid]->controlledlist.begin()));
			if(object && object->type == ObjectTypes::PLAYER){
				player = static_cast<Player *>(object);
				if(player){
					return player;
				}
			}
		}
	}
	return 0;
}

Team * World::GetPeerTeam(Uint8 peerid){
	if(peerlist[peerid]){
		for(std::list<Object *>::iterator it = objectlist.begin(); it != objectlist.end(); it++){
			Object * object = *it;
			if(object->type == ObjectTypes::TEAM){
				Team * team = static_cast<Team *>(object);
				for(int i = 0; i < team->numpeers; i++){
					if(team->peers[i] == peerid){
						return team;
					}
				}
			}
		}
	}
	return 0;
}

bool World::FindTeamForPeer(Peer & peer, Uint8 agency, int start){
	if(start >= maxteams){
		start = 0;
	}
	int teamnumber = start;
	bool teamfound = false;
	bool slotfound = true;
	std::vector<Team *> teamlist;
	for(std::list<Object *>::iterator it = objectlist.begin(); it != objectlist.end(); it++){
		Object * object = (*it);
		if(object->type == ObjectTypes::TEAM){
			Team * team = static_cast<Team *>(object);
			if(team){
				teamlist.push_back(team);
			}
		}
	}
	std::sort(teamlist.begin(), teamlist.end(), CompareTeamByNumber);
	std::vector<Team *>::iterator it = teamlist.begin();
	while(it != teamlist.end()){
		Team * team = *it;
		if(team->number == start){
			slotfound = false;
		}
		if(team->number == teamnumber){
			teamnumber = team->number + 1;
			it = teamlist.begin();
		}else{
			it++;
		}
	}
	if(!slotfound){
		for(std::vector<Team *>::iterator it = teamlist.begin(); it != teamlist.end(); it++){
			Team * team = *it;
			if(team->number >= start){
				if(team->agency == agency){
					if(team->AddPeer(peer.id)){
						teamfound = true;
						break;
					}
				}
			}
		}
	}
	if(!teamfound){
		if(teamlist.size() >= maxteams){
			return false;
		}
		Team * newteam = (Team *)CreateObject(ObjectTypes::TEAM);
		newteam->agency = agency;
		newteam->number = teamnumber;
		newteam->AddPeer(peer.id);
	}
	return true;
}

void World::SendSnapshots(void){
	for(unsigned int i = 0; i < maxpeers; i++){
		Peer * peer = peerlist[i];
		if(peer && i != localpeerid){
			Serializer data;
			Uint8 code = MSG_SNAPSHOT;
			data.Put(code);
			data.Put(tickcount);
			data.Put(peer->theirlasttick);
			SaveSnapshot(data, i);
			SendPacket(peer, data.data, data.BitsToBytes(data.offset));
		}
	}
}

void World::SendGameInfo(Uint8 peerid){
	Peer * peer = peerlist[peerid];
	if(peer){
		Serializer data;
		Uint8 code = MSG_GAMEINFO;
		data.Put(code);
		gameinfo.Serialize(Serializer::WRITE, data);
		/*char info[256];
		 int size = gameinfo.SaveInfo(info);
		 for(int i = 0; i < size; i++){
		 data.Put(info[i]);
		 }*/
		SendPacket(peer, data.data, data.BitsToBytes(data.offset));
	}
}

void World::SendReady(void){
	Serializer data;
	Uint8 code = MSG_READY;
	data.Put(code);
	SendPacket(GetAuthorityPeer(), data.data, data.BitsToBytes(data.offset));
}

bool World::AllPeersReady(Uint8 except){
	bool allready = true;
	for(int i = 0; i < maxpeers; i++){
		Peer * peer = peerlist[i];
		if(peer){
			if(!peer->isready && peer->id != except){
				allready = false;
				break;
			}
		}
	}
	return allready;
}

bool World::AllPeersLoadedGameInfo(void){
	bool allloaded = true;
	for(int i = 0; i < maxpeers; i++){
		Peer * peer = peerlist[i];
		if(peer){
			if(!peer->gameinfoloaded){
				allloaded = false;
				break;
			}
		}
	}
	return allloaded;
}

void World::RequestPeerList(void){
	if(authoritypeer != localpeerid){
		Serializer data;
		Uint8 code = MSG_PEERLIST;
		data.Put(code);
		SendPacket(GetAuthorityPeer(), data.data, data.BitsToBytes(data.offset));
	}
}

void World::SendPeerList(Uint8 peerid){
	if(mode == AUTHORITY){
		Serializer data;
		Uint8 code = MSG_PEERLIST;
		data.Put(code);
		for(unsigned int i = 0; i < maxpeers; i++){
			Peer * peer = peerlist[i];
			if(peer){
				peer->Serialize(Serializer::WRITE, data);
			}
		}
		for(unsigned int i = 0; i < maxpeers; i++){
			Peer * peer = peerlist[i];
			if(peer && i != localpeerid && (!peerid || peerid == peer->id)){
				SendPacket(peer, data.data, data.BitsToBytes(data.offset));
			}
		}
	}
}

void World::SwitchToLocalAuthorityMode(void){
	mode = AUTHORITY;
	for(int i = 0; i < maxpeers; i++){
		if(peerlist[i]){
			delete peerlist[i];
			peerlist[i] = 0;
		}
	}
	peercount = 0;
	authoritypeer = GetAuthorityPeer()->id;
	localpeerid = authoritypeer;
}

bool World::IsAuthority(void){
	return mode == AUTHORITY;
}

void World::Illuminate(void){
	illuminate = 15;
}

void World::ShowMessage(const char * message, Uint8 time, Uint8 type, bool networked, Peer * peer){
	if(messagetype >= 10){
		return;
	}
	if(networked && IsAuthority()){
		int msgsize = 1 + strlen(message) + 1 + 1 + 1;
		char * msg = new char[msgsize];
		msg[0] = MSG_MESSAGE;
		memcpy(&msg[1], message, strlen(message) + 1);
		msg[1 + strlen(message) + 1] = time;
		msg[1 + strlen(message) + 1 + 1] = type;
		if(!peer){
			for(unsigned int i = 0; i < maxpeers; i++){
				Peer * peer = peerlist[i];
				if(peer && i != localpeerid){
					SendPacket(peer, msg, msgsize);
				}
			}
		}else{
			SendPacket(peer, msg, msgsize);
		}
		delete[] msg;
	}
	if(!networked || IsAuthority()){
		if((peer && peer->id == localpeerid) || !peer){
			strncpy(World::message, message, sizeof(World::message) - 1);
			World::message[sizeof(World::message) - 1] = 0;
			message_i = 1;
			messagetime = time;
			messagetype = type;
		}
	}
}

void World::ShowStatus(const char * status, Uint8 color, bool networked, Peer * peer){
	/*char * newstatus = new char[strlen(status) + 1 + 1 + 1];
	strcpy(newstatus, status);
	newstatus[strlen(status) + 1] = 100;
	newstatus[strlen(status) + 2] = color;*/
	char * newstatus = CreateStatusString(status, color, 100);
	if(networked && IsAuthority()){
		int msgsize = 1 + strlen(status) + 1 + 1 + 1;
		char * msg = new char[msgsize];
		msg[0] = MSG_STATUS;
		memcpy(&msg[1], newstatus, strlen(status) + 1 + 1 + 1);
		if(!peer){
			for(unsigned int i = 0; i < maxpeers; i++){
				Peer * peer = peerlist[i];
				if(peer && i != localpeerid){
					SendPacket(peer, msg, msgsize);
				}
			}
		}else{
			SendPacket(peer, msg, msgsize);
		}
		delete[] msg;
	}
	if(!networked || (IsAuthority() && !peer) || (IsAuthority() && peer && peer->id == localpeerid)){
		/*while(statusmessages.size() >= maxstatusmessages){
			delete[] statusmessages.back();
			statusmessages.pop_back();
		}
		statusmessages.push_front(newstatus);*/
		PushStatusString(newstatus);
	}
}

void World::SendChat(bool toteam, char * message){
	char msg[2 + 100 + 1];
	memset(msg, 0, sizeof(msg));
	msg[0] = MSG_CHAT;
	msg[1] = toteam ? 1 : 0;
	strncpy(&msg[2], message, 100);
	msg[2 + 100] = 0;
	SendPacket(GetAuthorityPeer(), msg, sizeof(msg));
}

void World::SendSound(const char * name){
	if(IsAuthority()){
		Audio::GetInstance().Play(resources.soundbank[name]);
		char msg[1 + 255];
		msg[0] = MSG_SOUND;
		strcpy(&msg[1], name);
		msg[1 + strlen(name)] = 0;
		for(unsigned int i = 0; i < maxpeers; i++){
			Peer * peer = peerlist[i];
			if(peer && i != localpeerid){
				SendPacket(peer, msg, 1 + strlen(name) + 1);
			}
		}
	}
}

char * World::CreateStatusString(const char * status, Uint8 color, Uint8 duration){
	char * newstatus = new char[strlen(status) + 1 + 1 + 1];
	strcpy(newstatus, status);
	newstatus[strlen(status) + 1] = duration;
	newstatus[strlen(status) + 2] = color;
	return newstatus;
}

void World::PushStatusString(char * statusstring){
	while(statusmessages.size() >= maxstatusmessages){
		delete[] statusmessages.back();
		statusmessages.pop_back();
	}
	statusmessages.push_front(statusstring);
}

void World::ChangeTeam(void){
	char msg[1];
	msg[0] = MSG_CHANGETEAM;
	SendPacket(GetAuthorityPeer(), msg, sizeof(msg));
}

void World::KillByGovt(Peer & peer){
	if(IsAuthority()){
		char msg[2];
		msg[0] = MSG_GOVTKILL;
		msg[1] = peer.id;
		for(unsigned int i = 0; i < maxpeers; i++){
			Peer * ipeer = peerlist[i];
			if(ipeer && i != localpeerid){
				SendPacket(ipeer, msg, 2);
				if(ipeer->id == peer.id){
					Player * player = GetPeerPlayer(ipeer->id);
					if(player){
						player->KillByGovt(*this);
					}
				}
			}
		}
	}
}

void World::SetTech(Uint32 techchoices){
	Serializer data;
	Uint8 code = MSG_TECH;
	data.Put(code);
	data.Put(techchoices);
	printf("MSG_TECH %d\n", techchoices);
	SendPacket(GetAuthorityPeer(), data.data, data.BitsToBytes(data.offset));
}

int World::TechSlotsUsed(Peer & peer){
	int slotsused = 0;
	for(std::vector<BuyableItem *>::iterator it = buyableitems.begin(); it != buyableitems.end(); it++){
		BuyableItem * buyableitem = *it;
		if(buyableitem->techchoice & peer.techchoices){
			slotsused += buyableitem->techslots;
		}
	}
	return slotsused;
}

void World::SendPing(void){
	Serializer data;
	Uint8 code = MSG_PING;
	data.Put(code);
	lastpingid = rand();
	data.Put(lastpingid);
	SendPacket(GetAuthorityPeer(), data.data, data.BitsToBytes(data.offset));
	lastpingsent = SDL_GetTicks();
}

int World::GetPingTime(void){
	if(state == CONNECTED){
		return pingtime;
	}else{
		return 0;
	}
}

void World::SetSystemCamera(bool system, Uint16 objectfollow, Sint16 x, Sint16 y){
	systemcameraactive[system] = true;
	systemcamerafollow[system] = objectfollow;
	systemcamerax[system] = x;
	systemcameray[system] = y;
}

Object * World::GetObjectFromId(Uint16 id){
	if(objectidlookup.find(id) != objectidlookup.end()){
		return objectidlookup[id];
	}
	return 0;
}

void World::SaveSnapshot(Serializer & data, Uint8 peerid){
	if(mode == AUTHORITY){
		Player * player = GetPeerPlayer(peerid);
		Serializer ** oldsnapshotptr = &oldsnapshots[peerid][tickcount % maxoldsnapshots];
		Serializer ** deltasnapshotptr = &oldsnapshots[peerid][peerlist[peerid]->lasttick % maxoldsnapshots];
		if(tickcount - peerlist[peerid]->lasttick >= maxoldsnapshots){
			*deltasnapshotptr = 0;
		}
		if(!(*oldsnapshotptr)){
			*oldsnapshotptr = new Serializer;
		}
		
		// Find deleted objects
		std::vector<Uint16> deletedobjects;
		if(*deltasnapshotptr){
			(*deltasnapshotptr)->readoffset = 0;
			while((*deltasnapshotptr)->MoreBytesToRead()){
				Uint8 type;
				Uint16 id;
				int oldreadoffset = (*deltasnapshotptr)->readoffset;
				(*deltasnapshotptr)->Get(type);
				(*deltasnapshotptr)->Get(id);
				(*deltasnapshotptr)->readoffset = oldreadoffset;
				Object * object = GetObjectFromId(id);
				if(!object){
					deletedobjects.push_back(id);
				}
				(*deltasnapshotptr)->readoffset += objecttypes.SerializedSize(type);
			}
		}
		Uint16 deletedobjectscount = deletedobjects.size();
		//
		
		data.Put(peerlist[peerid]->lasttick);
		data.Put(deletedobjectscount);
		for(std::vector<Uint16>::iterator it = deletedobjects.begin(); it != deletedobjects.end(); it++){
			data.Put(*it);
		}
		(*oldsnapshotptr)->offset = 0;
		
		if(DELTAENABLED && *deltasnapshotptr){
			// Write delta'ed objects in snapshot
			(*deltasnapshotptr)->readoffset = 0;
			std::map<Uint16, Object *> oldobjects;
			while((*deltasnapshotptr)->MoreBytesToRead()){
				Uint8 type;
				Uint16 id;
				int oldreadoffset = (*deltasnapshotptr)->readoffset;
				(*deltasnapshotptr)->Get(type);
				(*deltasnapshotptr)->Get(id);
				(*deltasnapshotptr)->readoffset = oldreadoffset;
				Object * object = GetObjectFromId(id);
				if(object){
					oldobjects[id] = object;
					data.PutBit(1);
					object->Serialize(Serializer::WRITE, data, *deltasnapshotptr);
				}else{
					(*deltasnapshotptr)->readoffset += objecttypes.SerializedSize(type);
				}
			}
			//
			
			// Write all new objects
			for(std::list<Object *>::iterator i = objectlist.begin(); i != objectlist.end(); i++){
				if((*i)->RequiresAuthority() && RelevantToPlayer(player, (*i))){
					(*i)->Serialize(Serializer::WRITE, **oldsnapshotptr);
					if(oldobjects.find((*i)->id) == oldobjects.end()){
						data.PutBit(0);
						(*i)->Serialize(Serializer::WRITE, data);
					}
				}
			}
			//
		}else{
			// Write all relevant objects, no delta compression
			for(std::list<Object *>::iterator i = objectlist.begin(); i != objectlist.end(); i++){
				if((*i)->RequiresAuthority() && RelevantToPlayer(player, (*i))){
					(*i)->Serialize(Serializer::WRITE, **oldsnapshotptr);
					data.PutBit(0);
					(*i)->Serialize(Serializer::WRITE, data);
				}
			}
			//
		}
	}else
		if(mode == REPLICA){
			Uint32 nulltickcount = 0;
			data.Put(nulltickcount);
			Uint16 nulldeletedobjectscount = 0;
			data.Put(nulldeletedobjectscount);
			for(std::list<Object *>::iterator i = objectlist.begin(); i != objectlist.end(); i++){
				if((*i)->RequiresAuthority()){
					data.PutBit(0);
					(*i)->Serialize(Serializer::WRITE, data);
				}
			}
		}
}

void World::LoadSnapshot(Serializer & data, bool create, Serializer * delta, Uint16 objectid){
	Uint32 deltatick;
	data.Get(deltatick);
	Uint16 deletedobjectscount;
	data.Get(deletedobjectscount);
	for(int i = 0; i < deletedobjectscount; i++){
		Uint16 objectid;
		data.Get(objectid);
		if(GetObjectFromId(objectid)){
			printf("deleted %d\n", objectid);
			MarkDestroyObject(objectid);
		}
	}
	while(data.MoreBytesToRead()){
		bool isdeltacompressed = data.GetBit();
		Uint8 type;
		Uint16 id;
		int readoffset = data.readoffset;
		data.Get(type);
		data.Get(id);
		data.readoffset = readoffset;
		if(objectid && id != objectid){
			data.readoffset += objecttypes.SerializedSize(type);
			continue;
		}
		Object * object = GetObjectFromId(id);
		if(!object && create){
			object = CreateObject(type, id);
		}
		if(object){
			if(object->iscontrollable){
				object->oldx = object->x;
				object->oldy = object->y;
			}
			if(delta){
				delta->readoffset = 0;
				LoadSnapshot(*delta, false, 0, id);
			}
			if(isdeltacompressed){
				object->Serialize(Serializer::READ, data, (Serializer *)true);
			}else{
				object->Serialize(Serializer::READ, data);
			}
		}else{
			data.readoffset += objecttypes.SerializedSize(type);
		}
	}
}

Object * World::CreateObject(Uint8 type, Uint16 id){
	if(replaying){ // Do not create objects when rewinding/replaying game state
		return 0;
	}
	if(objectlist.size() == maxobjects){
		return 0;
	}
	Object * object = objecttypes.CreateFromType(type);
	if(!object){
		return 0;
	}
	if(id == 0 && object->RequiresAuthority() && mode != AUTHORITY){
		delete object;
		return 0;
	}
	Uint16 searchid = currentid;
	if(id == 0){
		if(object->RequiresAuthority()){
			searchid = currentid;
		}else{
			searchid = currentid | 0x8000;
		}
		while(currentid <= maxobjects){
			if(!objectidlookup[searchid]){
				break;
			}
			currentid++;
			searchid++;
			if(currentid >= maxobjects){
				currentid = 1;
				searchid = 1;
			}
		}
		object->id = searchid;
	}else{
		object->id = id;
	}
	objectlist.push_back(object);
	objectidlookup[object->id] = object;
	return object;
}

void World::MarkDestroyObject(Uint16 id){
	if(replaying){
		return;
	}
	Object * object = GetObjectFromId(id);
	if(object){
		object->wasdestroyed = true;
		objectdestroylist.push_back(id);
	}
}

void World::DestroyMarkedObjects(void){
	for(std::list<Uint16>::iterator i = objectdestroylist.begin(); i != objectdestroylist.end(); i++){
		for(std::list<Object *>::iterator j = objectlist.begin(); j != objectlist.end(); j++){
			if((*j)->id == (*i)){
				objectidlookup.erase((*j)->id);
				(*j)->OnDestroy(*this);
				delete (*j);
				objectlist.erase(j);
				break;
			}
		}
	}
	objectdestroylist.clear();
}

void World::DestroyObject(Uint16 id){
	for(std::list<Object *>::iterator i = objectlist.begin(); i != objectlist.end(); i++){
		if((*i)->id == id){
			objectidlookup.erase((*i)->id);
			(*i)->OnDestroy(*this);
			delete (*i);
			objectlist.erase(i);
			break;
		}
	}
}

void World::DestroyAllObjects(void){
	for(std::list<Object *>::iterator j = objectlist.begin(); j != objectlist.end(); j++){
		(*j)->OnDestroy(*this);
		delete (*j);
	}
	objectlist.clear();
	objectidlookup.clear();
	objectdestroylist.clear();
}

bool World::TestAABB(int x1, int y1, int x2, int y2, Object * object, std::vector<Uint8> & types, bool onlycollidable){
	int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0;
	object->GetAABB(resources, &sx1, &sy1, &sx2, &sy2);
	Uint8 type = object->type;
	if(types.size() > 0 && std::find(types.begin(), types.end(), type) == types.end()){
		return false;
	}
	if(((x1 <= sx1 && x2 >= sx1) || (x1 <= sx2 && x2 >= sx2) || (x1 >= sx1 && x2 <= sx2)) &&
	   ((y1 <= sy1 && y2 >= sy1) || (y1 <= sy2 && y2 >= sy2) || (y1 >= sy1 && y2 <= sy2))){
		return true;
	}
	return false;
}

std::vector<Object *> World::TestAABB(int x1, int y1, int x2, int y2, std::vector<Uint8> & types, Uint16 except, Uint16 teamid, bool onlycollidable){
	std::vector<Object *> objects;
	for(std::list<Object *>::iterator i = objectlist.begin(); i != objectlist.end(); i++){
		Object * object = (*i);
		if(object->issprite){
			if(object->id != except){
				if(!object->isphysical || !onlycollidable || (object->isphysical && object->collidable)){
					if(!teamid || (teamid && !BelongsToTeam(*object, teamid))){
						if(TestAABB(x1, y1, x2, y2, object, types)){
							objects.push_back(object);
						}
					}
				}
			}
		}
	}
	return objects;
}

Object * World::TestIncr(int x1, int y1, int x2, int y2, int * xv, int * yv, std::vector<Uint8> & types, Uint16 except, Uint16 teamid){
	int xb1 = x1 + (*xv < 0 ? *xv : 0);
	int yb1 = y1 + (*yv < 0 ? *yv : 0);
	int xb2 = x2 + (*xv > 0 ? *xv : 0);
	int yb2 = y2 + (*yv > 0 ? *yv : 0);
	/*int xb1 = x1 + *xv;
	 int yb1 = y1 + *yv;
	 int xb2 = x2 + *xv;
	 int yb2 = y2 + *yv;*/
	std::vector<Object *> testobjects = TestAABB(xb1, yb1, xb2, yb2, types, except, teamid); // broadphase
	std::vector<Object *> test;
	for(std::vector<Object *>::iterator it = testobjects.begin(); it != testobjects.end(); it++){
		if((*it)->issprite){
			test.push_back(*it);
		}
	}
	if(test.size() == 0){
		return 0;
	}
	int dx = *xv;
	int dy = *yv;
	int step;
	int error;
	int oldxv = *xv;
	int oldyv = *yv;
	int yv0 = 0;
	int xv0 = 0;
	*xv = 0;
	*yv = 0;
	float slope = 0;
	if(dx){
		slope = (float)dy / dx;
	}else{
		if(oldyv > 0){
			while(*yv < oldyv){
				for(std::vector<Object *>::iterator i = test.begin(); i != test.end(); i++){
					Object * object = *i;
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, object, types)){
						*yv = yv0;
						return object;
					}
				}
				yv0 = *yv;
				(*yv)++;
			}
		}else{
			while(*yv > oldyv){
				for(std::vector<Object *>::iterator i = test.begin(); i != test.end(); i++){
					Object * object = *i;
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, object, types)){
						*yv = yv0;
						return object;
					}
				}
				yv0 = *yv;
				(*yv)--;
			}
		}
	}
	if(slope > -1 && slope < 1){
		error = -dx / 2;
		oldyv > 0 ? step = 1 : step = -1;
		if(oldxv > 0){
			while(*xv < oldxv){
				for(std::vector<Object *>::iterator i = test.begin(); i != test.end(); i++){
					Object * object = *i;
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, object, types)){
						*yv = yv0;
						*xv = xv0;
						return object;
					}
				}
				error += dy * step;
				if(error >= 0){
					yv0 = *yv;
					*yv += step;
					error -= dx;
				}
				xv0 = *xv;
				(*xv)++;
			}
		}else{
			while(*xv > oldxv){
				for(std::vector<Object *>::iterator i = test.begin(); i != test.end(); i++){
					Object * object = *i;
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, object, types)){
						*yv = yv0;
						*xv = xv0;
						return object;
					}
				}
				error += dy * -step;
				if(error <= 0){
					yv0 = *yv;
					*yv += step;
					error -= dx;
				}
				xv0 = *xv;
				(*xv)--;
			}
		}
	}else{
		error = -dy / 2;
		oldxv > 0 ? step = 1 : step = -1;
		if(oldyv > 0){
			while(*yv < oldyv){
				for(std::vector<Object *>::iterator i = test.begin(); i != test.end(); i++){
					Object * object = *i;
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, object, types)){
						*xv = xv0;
						*yv = yv0;
						return object;
					}
				}
				error += dx * step;
				if(error >= 0){
					xv0 = *xv;
					*xv += step;
					error -= dy;
				}
				yv0 = *yv;
				(*yv)++;
			}
		}else{
			while(*yv > oldyv){
				for(std::vector<Object *>::iterator i = test.begin(); i != test.end(); i++){
					Object * object = *i;
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, object, types)){
						*xv = xv0;
						*yv = yv0;
						return object;
					}
				}
				error += dx * -step;
				if(error <= 0){
					xv0 = *xv;
					*xv += step;
					error -= dy;
				}
				yv0 = *yv;
				(*yv)--;
			}
		}
	}
	for(std::vector<Object *>::iterator i = test.begin(); i != test.end(); i++){
		Object * object = *i;
		if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, object, types)){
			*xv = xv0;
			*yv = yv0;
			return object;
		}
	}
	return 0;
}