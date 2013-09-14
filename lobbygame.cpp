#include "lobbygame.h"
#include "serializer.h"
#include "sha1.h"

LobbyGame::LobbyGame(){
	memset(name, 0, sizeof(name));
	memset(mapname, 0, sizeof(mapname));
	memset(password, 0, sizeof(password));
	memset(hostname, 0, sizeof(hostname));
	memset(maphash, 0, sizeof(maphash));
	accountid = 0;
	port = 0;
	players = 0;
	state = 0;
	loaded = false;
	createdtime = 0;
}

void LobbyGame::Serialize(bool write, Serializer & data){
	data.Serialize(write, accountid);
	Uint8 namesize = strlen(name);
	data.Serialize(write, namesize);
	for(int i = 0; i < namesize; i++){
		data.Serialize(write, name[i]);
	}
	name[namesize] = 0;
	Uint8 passwordsize = strlen(password);
	data.Serialize(write, passwordsize);
	for(int i = 0; i < passwordsize; i++){
		data.Serialize(write, password[i]);
	}
	password[passwordsize] = 0;
	Uint8 hostnamesize = strlen(hostname);
	data.Serialize(write, hostnamesize);
	for(int i = 0; i < hostnamesize; i++){
		data.Serialize(write, hostname[i]);
	}
	hostname[hostnamesize] = 0;
	Uint8 mapnamesize = strlen(mapname);
	data.Serialize(write, mapnamesize);
	for(int i = 0; i < mapnamesize; i++){
		data.Serialize(write, mapname[i]);
	}
	mapname[mapnamesize] = 0;
	for(int i = 0; i < sizeof(maphash); i++){
		data.Serialize(write, maphash[i]);
	}
	data.Serialize(write, players);
	data.Serialize(write, state);
	char * host = strtok(hostname, ",");
	if(host){
		char * portstr = strtok(NULL, ",");
		if(portstr){
			port = atoi(portstr);
		}
	}
	data.Serialize(write, port);
	if(write == Serializer::READ){
		loaded = true;
	}
}

void LobbyGame::CalculateMapHash(void){
	unsigned char * mapdata = new unsigned char[65535];
	char mapfilename[256];
	sprintf(mapfilename, "level/%s", mapname);
	SDL_RWops * file = SDL_RWFromFile(mapfilename, "rb");
	if(file){
		int mapdatasize = SDL_RWread(file, mapdata, 1, 65535);
		SDL_RWclose(file);
		sha1::calc(mapdata, mapdatasize, maphash);
	}
	delete[] mapdata;
}