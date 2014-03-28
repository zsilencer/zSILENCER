#include "peer.h"

Peer::Peer(){
	lasttick = 0;
	theirlasttick = 0;
	id = 0;
	port = 0;
	ip = 0;
	accountid = 0;
	lastpacket = SDL_GetTicks();
	ishost = false;
	gameinfoloaded = false;
	mapdownloaded = false;
	isready = false;
	techchoices = 0;
	wantedtechchoices = 0;
	isbot = false;
	firstinputtime = 0;
	totalinputs = 0;
}

void Peer::Serialize(bool write, Serializer & data){
	data.Serialize(write, id);
	data.Serialize(write, ishost);
	data.Serialize(write, gameinfoloaded);
	data.Serialize(write, mapdownloaded);
	data.Serialize(write, isready);
	data.Serialize(write, techchoices);
	//data.Serialize(write, port);
	//data.Serialize(write, publicport);
	//data.Serialize(write, ip);
	data.Serialize(write, accountid);
	Uint8 count = controlledlist.size();
	data.Serialize(write, count);
	if(write){
		for(std::list<Uint16>::iterator i = controlledlist.begin(); i != controlledlist.end(); i++){
			data.Serialize(write, (*i));
		}
	}else{
		controlledlist.clear();
		for(unsigned int i = 0; i < count; i++){
			Uint16 controlled;
			data.Serialize(write, controlled);
			controlledlist.push_back(controlled);
		}
	}
}