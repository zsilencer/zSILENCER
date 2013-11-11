#include "lobby.h"
#include "sha1.h"
#include "world.h"

Lobby::Lobby(World * world){
	Lobby::world = world;
	msgsize = 0;
	he = 0;
	resolvehost[0] = 0;
	mutex = SDL_CreateMutex();
	state = WAITING;
	accountid = 0;
	sendbuffersize = 0;
	creategamestatus = 0;
	connectgamestatus = 0;
	sendbuffermax = 4096;
	sendbufferoffset = 0;
	sendbuffer = new char[sendbuffermax];
	gamesprocessed = false;
	channelchanged = false;
	channel[0] = 0;
	serverip[0] = 0;
	sockethandle = 0;
	statupgraded = false;
}

Lobby::~Lobby(){
	LockMutex();
	delete[] sendbuffer;
	Disconnect();
	ClearGames();
	SDL_DestroyMutex(mutex);
}

void Lobby::Connect(const char * host, unsigned short port){
	LockMutex();
	motdreceived = false;
	versionchecked = false;
	motd[0] = 0;
	sockethandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	unsigned long iomode = 1;
    // set to nonblocking
    ioctl(sockethandle, FIONBIO, &iomode);
	failmessage[0] = 0;
	if(strcmp(resolvehost, host) != 0){
		he = 0;
	}
	if(strlen(serverip) > 0){
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(serverip);//*(in_addr *)(he->h_addr_list[0]);
		connect(sockethandle, (sockaddr *)&addr, sizeof(addr));
		//printf("%d %d\n", ret, errno);
		state = CONNECTING;
		lasttime = SDL_GetTicks();
	}else{
		state = RESOLVING;
		ResolveHostname(host);
	}
	UnlockMutex();
}

void Lobby::Disconnect(void){
	if(state == CONNECTING || state == RESOLVED || state == CONNECTIONFAILED){
		state = CONNECTIONFAILED;
	}else{
		state = DISCONNECTED;
	}
	sendbuffersize = 0;
	sendbufferoffset = 0;
    shutdown(sockethandle, SHUT_RDWR);
    closesocket(sockethandle);
}

void Lobby::DoNetwork(void){
	if(state == IDLE || state == WAITING || !sockethandle){
		return;
	}
	LockMutex();
	fd_set readset;
	fd_set writeset;
	int result;
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if(state == CONNECTING){
		Uint32 newtick = SDL_GetTicks();
		if(newtick > lasttime && newtick - lasttime > 5000){
			state = CONNECTIONFAILED;
		}
	}
	if(state == CONNECTED || state == AUTHENTICATING || state == AUTHENTICATED){
		Uint32 newtick = SDL_GetTicks();
		if(newtick > lasttime && newtick - lasttime > 20000){
			Disconnect();
		}
	}
	do{
		result = 0;
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_SET(sockethandle, &readset);
		if(state == CONNECTING || sendbufferoffset > 0){
			FD_SET(sockethandle, &writeset);
		}
		if(state != DISCONNECTED){
			result = select(sockethandle + 1, &readset, &writeset, NULL, &timeout);
			if(FD_ISSET(sockethandle, &writeset)){
				if(sendbuffersize > 0){
					int ret = send(sockethandle, sendbuffer, sendbuffersize, 0);
					if(ret > 0){
						sendbuffersize -= ret;
						memcpy(sendbuffer, &sendbuffer[ret], sendbuffersize);
						sendbufferoffset -= ret;
					}
				}else{
					if(state == CONNECTING){
						state = CONNECTED;
						//unsigned short port = world->Bind();
						//printf("Bound to port %d\n", port);
					}
				}
			}
			if(FD_ISSET(sockethandle, &readset)){
				lasttime = SDL_GetTicks();
				if(!msgsize){
					int ret = recv(sockethandle, (char *)&msgsize, sizeof(msgsize), 0);
					if(ret > 0){
						msgoffset = 0;
					}else
					if(ret == 0){
						Disconnect();
					}else
					if(ret == -1){
						printf("errno: %d\n", errno);
						if(errno == ECONNREFUSED){
							state = CONNECTIONFAILED;
						}
						Disconnect();
					}
				}else{
					int ret = recv(sockethandle, &msg[msgoffset], msgsize - msgoffset, 0);
					if(ret > 0){
						msgoffset += ret;
						if(msgoffset == msgsize){
							Serializer data(msg, msgsize);
							Uint8 code;
							data.Get(code);
							switch(code){
								case MSG_AUTH:{
									Uint8 status;
									data.Get(status);
									if(status){
										state = AUTHENTICATED;
										data.Get(accountid);
										GetUserInfo(accountid);
									}else{
										state = AUTHFAILED;
										strcpy(failmessage, (char *)&msg[2]);
									}
								}break;
								case MSG_MOTD:{
									Uint8 status;
									data.Get(status);
									if(status){
										strcat(motd, (char *)&msg[1]);
									}else{
										motdreceived = true;
									}
								}break;
								case MSG_CHAT:{
									char * channel = (char *)&msg[1];
									char * message = (char *)&msg[1 + strlen(channel) + 1];
									char * newmessage = new char[strlen(message) + 1 + 2];
									newmessage[0] = 0;
									memcpy(newmessage, message, strlen(message) + 1 + 2);
									chatmessages.push_back(newmessage);
								}break;
								case MSG_NEWGAME:{
									Uint8 creategamestatus;
									data.Get(creategamestatus);
									printf("MSG_NEWGAME received %d %d\n", accountid, creategamestatus);
									LobbyGame * lobbygame = new LobbyGame;
									lobbygame->createdtime = SDL_GetTicks();
									lobbygame->Serialize(Serializer::READ, data);
									if(lobbygame->accountid == Lobby::accountid){ // CreateGame response
										Lobby::creategamestatus = creategamestatus;
									}
									if(creategamestatus != 2){ // success
										//char * info = (char *)&msg[1 + sizeof(accountid) + sizeof(creategamestatus)];
										//lobbygame->accountid = accountid;
										//lobbygame->LoadInfo(info);
										games.push_back(lobbygame);
										gamesprocessed = false;
									}else{ // failure
										delete lobbygame;
									}
								}break;
								case MSG_DELGAME:{
									Uint32 id;// = *(Uint32 *)((char *)&msg[1]);
									data.Get(id);
									for(std::list<LobbyGame *>::iterator it = games.begin(); it != games.end(); it++){
										LobbyGame * lobbygame = *it;
										if(lobbygame->accountid == id/* && (SDL_GetTicks() - lobbygame->createdtime > 3000)*/){
											delete lobbygame;
											games.erase(it);
											gamesprocessed = false;
											break;
										}
									}
								}break;
								case MSG_CHANNEL:{
									char * name = (char *)&msg[1];
									strcpy(channel, name);
									channelchanged = true;
								}break;
								case MSG_CONNECT:{
									/*data.Get(connectgamestatus);
									switch(connectgamestatus){
										case 1:{ // game is open
											Uint8 hostsize;
											data.Get(hostsize);
											char hostname[255];
											memset(hostname, 0, sizeof(hostname));
											for(int i = 0; i < hostsize; i++){
												data.Get(hostname[i]);
											}
											char * host = strtok(hostname, ",");
											char * portstr = strtok(NULL, ",");
											unsigned short port = atoi(portstr);
											world->Connect(selectedagency, accountid);
										}break;
										case 2:{ // could not join game
											
										}break;
									}*/
									
									/*Uint8 hostsize;
									data.Get(hostsize);
									char hostname[255];
									memset(hostname, 0, sizeof(hostname));
									for(int i = 0; i < hostsize; i++){
										data.Get(hostname[i]);
									}
									char * host = strtok(hostname, ",");
									char * portstr = strtok(NULL, ",");
									char * publicportstr = strtok(NULL, ",");
									unsigned short port = atoi(portstr);
									unsigned short publicport = atoi(publicportstr);
									Uint32 accountid;
									data.Get(accountid);
									Uint8 agency;
									data.Get(agency);
									printf("received lobby server connect request (%s:%d,%d) accountid: %d, agency: %d\n", host, port, publicport, accountid, agency);
									Peer * peeradded = 0;
									if(world->gameplaystate == World::INLOBBY){
										peeradded = world->AddPeer(hostname, port, publicport, agency, accountid);
									}
									Serializer response;
									Uint8 code = World::MSG_CONNECT;
									response.Put(code);
									if(peeradded){
										response.PutBit(true);
										response.Put(peeradded->id);
									}else{
										response.PutBit(false);
									}
									//int oldoffset = response.offset;
									//response.Put(port);
									sockaddr_in addr;
									addr.sin_family = AF_INET;
									addr.sin_port = htons(port);
									addr.sin_addr.s_addr = inet_addr(host);
									int ret = sendto(world->sockethandle, response.data, response.BitsToBytes(response.offset), 0, (sockaddr *)&addr, sizeof(addr));
									addr.sin_family = AF_INET;
									addr.sin_port = htons(publicport);
									addr.sin_addr.s_addr = inet_addr(host);
									//response.offset = oldoffset;
									//response.Put(publicport);
									int ret2 = sendto(world->sockethandle, response.data, response.BitsToBytes(response.offset), 0, (sockaddr *)&addr, sizeof(addr));
									printf("sent MSG_CONNECTs to client (%d:%d, %d:%d)\n", port, ret, publicport, ret2);*/
								}break;
								case MSG_VERSION:{
									printf("MSG_VERSION received\n");
									Uint8 success;
									data.Get(success);
									versionchecked = true;
									if(success){
										versionok = true;
									}else{
										versionok = false;
									}
								}break;
								case MSG_USERINFO:{
									printf("MSG_USERINFO received\n");
									unsigned int oldreadoffset = data.readoffset;
									Uint32 accountid;
									data.Get(accountid);
									data.readoffset = oldreadoffset;
									if(!userinfos[accountid]){
										userinfos[accountid] = new User;
									}
									User * user = userinfos[accountid];
									user->retrieving = false;
									user->Serialize(Serializer::READ, data);
									printf("account id = %d\n", user->accountid);
									for(int i = 0; i < world->maxpeers; i++){
										Peer * peer = world->peerlist[i];
										if(peer && peer->accountid == accountid){
											world->UserInfoReceived(*peer);
										}
									}
								}break;
								case MSG_PING:{
									//printf("received MSG_PING\n");
									char msg[2];
									msg[0] = MSG_PING;
									msg[1] = 1;
									SendMessage(msg, sizeof(msg));
								}break;
								case MSG_UPGRADESTAT:{
									Uint8 oldstatsagency = GetUserInfo(accountid)->statsagency;
									ForgetUserInfo(accountid);
									GetUserInfo(accountid)->statsagency = oldstatsagency;
									statupgraded = true;
								}break;
							}
							msgsize = 0;
						}
					}else
					if(ret == 0){
						Disconnect();
					}else
					if(ret == -1){
						Disconnect();
					}
				}
			}
		}
	}while(result == 1);
	UnlockMutex();
}

void Lobby::ResolveHostname(const char * host){
	LockMutex();
	resolvethreadrunning = false;
	strcpy(resolvehost, host);
	resolvethread = SDL_CreateThread(ResolveThreadFunc, "resolvehost", (void *)this);
	while(!resolvethreadrunning){
		// wait for thread to start and get mutex pointer;
		SDL_Delay(1);
	}
	UnlockMutex();
}

void Lobby::LockMutex(void){
	SDL_mutexP(mutex);
}

void Lobby::UnlockMutex(void){
	SDL_mutexV(mutex);
}

void Lobby::SendMessage(const char msg[256], Uint8 size){
	Send((char *)&size, sizeof(size));
	Send(msg, size);
}

void Lobby::SendVersion(void){
	memset(msg, 0, sizeof(msg));
	msg[0] = MSG_VERSION;
	strcpy((char *)&msg[1], world->version);
	SendMessage(msg, sizeof(msg[0]) + strlen(world->version) + 1);
}

void Lobby::SendCredentials(const char * username, const char * password){
	if(strlen(username) > maxusername){
		return;
		//username[maxusername] = 0;
	}
	char msg[256];
	memset(msg, 0, sizeof(msg));
	msg[0] = MSG_AUTH;
	strcpy((char *)&msg[1], username);
	unsigned char hash[20];
	sha1::calc(password, strlen(password), hash);
	memcpy((char *)&msg[1 + strlen(username) + 1], hash, 20);
	Uint8 size = sizeof(msg[0]) + strlen(username) + 1 + sizeof(hash);
	SendMessage(msg, size);
}

void Lobby::SendChat(const char * channel, const char * message){
	if(strlen(channel) > 32){
		return;
		//channel[32] = 0;
	}
	if(strlen(message) > 256 - 1 - strlen(channel)){
		return;
		//message[256 - 1 - strlen(channel)] = 0;
	}
	char msg[256];
	memset(msg, 0, sizeof(msg));
	msg[0] = MSG_CHAT;
	strcpy((char *)&msg[1], channel);
	strcpy((char *)&msg[1 + strlen(channel) + 1], message);
	Uint8 size = sizeof(msg[0]) + strlen(channel) + 1 + strlen(message);
	SendMessage(msg, size);
}

void Lobby::JoinChannel(const char * channel){
	char msg[256];
	memset(msg, 0, sizeof(msg));
	msg[0] = MSG_CHAT;
	strcpy((char *)&msg[1], Lobby::channel);
	strcpy((char *)&msg[1 + strlen(Lobby::channel) + 1], "/join ");
	strcpy((char *)&msg[1 + strlen(Lobby::channel) + 1 + strlen("/join ")], channel);
	Uint8 size = 1 + strlen(Lobby::channel) + 1 + strlen("/join ") + strlen(channel) + 1;
	SendMessage(msg, size);
}

void Lobby::CreateGame(const char * name, const char * map, const char * password){
	Serializer data;
	Uint8 code = MSG_NEWGAME;
	data.Put(code);
	LobbyGame lobbygame;
	strncpy(lobbygame.name, name, sizeof(lobbygame.name));
	strncpy(lobbygame.mapname, map, sizeof(lobbygame.mapname));
	if(password){
		strncpy(lobbygame.password, password, sizeof(lobbygame.password));
	}
	lobbygame.CalculateMapHash();
	lobbygame.Serialize(Serializer::WRITE, data);
	SendMessage(data.data, data.BitsToBytes(data.offset));
	printf("sent creategame %s %s %s\n", name, map, password ? password : "");
}

/*void Lobby::ConnectToGame(LobbyGame & lobbygame, Uint8 agency){
	selectedagency = agency;
	Serializer data;
	Uint8 code = MSG_CONNECT;
	data.Put(code);
	data.Put(lobbygame.accountid);
	data.Put(port);
	data.Put(publicport);
	data.Put(agency);
	world->GetAuthorityPeer()->port = lobbygame.port;
	world->GetAuthorityPeer()->publicport = lobbygame.publicport;
	world->state = World::CONNECTING;
	SendMessage(data.data, data.BitsToBytes(data.offset));
	printf("requested thru lobby server to connect to game id %d, (ports %d,%d) agency %d\n", lobbygame.accountid, port, publicport, agency);
}*/

void Lobby::ClearGames(void){
	for(std::list<LobbyGame *>::iterator it = games.begin(); it != games.end(); it++){
		delete (*it);
	}
	games.clear();
}

LobbyGame * Lobby::GetGameByAccountId(Uint32 accountid){
	for(std::list<LobbyGame *>::iterator it = games.begin(); it != games.end(); it++){
		LobbyGame * lobbygame = (*it);
		if(lobbygame->accountid == accountid){
			return lobbygame;
		}
	}
	return 0;
}

User * Lobby::GetUserInfo(Uint32 accountid){
	User * userinfo = userinfos[accountid];
	if(!userinfo){
		userinfos[accountid] = new User;
		userinfo = userinfos[accountid];
		userinfo->accountid = accountid;
		Serializer data;
		Uint8 code = MSG_USERINFO;
		data.Put(code);
		data.Put(accountid);
		SendMessage(data.data, data.BitsToBytes(data.offset));
		printf("requested user info for account id %u\n", accountid);
		userinfo->retrieving = true;
		if(accountid >= 0xFFFFFFFF - 24){
			// Bot
			static const char * botnames[] = {"Sweet Pea", "Breadloaf", "Cheeseboy", "Damien", "Quicknades", "Giblets",
				"State Machine", "MileyFan5", "J0hnny", "Young Watson", "Flynn", "Wi11i4m",
				"HURRDURR", "0b4m4", "bitcoin", "strlen", "juan valdez", "Rebdomine",
				"cyber criminal", "ID10T", "barnacle", "nodule", "u r bad", "widowmaker"};
			Uint32 index = 0xFFFFFFFF - accountid;
			printf("index = %d\n", index);
			strcpy(userinfo->name, botnames[index]);
		}
	}
	return userinfo;
}

void Lobby::ForgetUserInfo(Uint32 accountid){
	User * userinfo = userinfos[accountid];
	if(userinfo){
		userinfos.erase(accountid);
		delete userinfo;
	}
}

void Lobby::UpgradeStat(Uint8 agency, Uint8 stat){
	char msg[3];
	msg[0] = MSG_UPGRADESTAT;
	msg[1] = agency;
	msg[2] = stat;
	SendMessage(msg, sizeof(msg));
}

void Lobby::RegisterStats(User & user, Uint8 won){
	Serializer msg;
	Uint8 code = MSG_REGISTERSTATS;
	msg.Put(code);
	msg.Put(user.accountid);
	msg.Put(user.statsagency);
	msg.Put(won);
	Uint32 xp = user.statscopy.CalculateExperience();
	msg.Put(xp);
	user.statscopy.Serialize(Serializer::WRITE, msg);
	SendMessage(msg.data, msg.BitsToBytes(msg.offset));
}

bool Lobby::Send(const char * data, unsigned int size){
	int ret = send(sockethandle, data, size, 0);
	if(ret == -1){
		return false;
	}else{
		if(ret < size){
			unsigned int bytesremaining = size - ret;
			if(sendbuffersize + bytesremaining > sendbuffersize){
				Disconnect();
				return false;
			}
			memcpy(&sendbuffer[sendbufferoffset], data, bytesremaining);
			sendbuffersize += bytesremaining;
		}
		return true;
	}
}

int Lobby::ResolveThreadFunc(void * param){
	SDL_mutex * mutex = ((Lobby *)param)->mutex;
	char host[256];
	strcpy(host, ((Lobby *)param)->resolvehost);
	((Lobby *)param)->resolvethreadrunning = true;
	hostent * he = gethostbyname(host);
	if(SDL_mutexP(mutex) == 0){
		((Lobby *)param)->he = he;
		if(he && he->h_addr){
			((Lobby *)param)->state = RESOLVED;
			strcpy(((Lobby *)param)->serverip, inet_ntoa(*(in_addr *)(he->h_addr)));
		}else{
			((Lobby *)param)->state = RESOLVEFAILED;
		}
		((Lobby *)param)->resolvethreadrunning = false;
		SDL_mutexV(mutex);
	}
	return 0;
}