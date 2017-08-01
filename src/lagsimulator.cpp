#include "lagsimulator.h"
#include "world.h"

LagSimulator::LagSimulator(SOCKET * sockethandle){
	LagSimulator::sockethandle = sockethandle;
	active = false;
}

bool LagSimulator::Active(void){
	return active;
}

void LagSimulator::Activate(Uint32 minlatecy, Uint32 maxlatecy, float packetloss){
	active = true;
	LagSimulator::minlatecy = minlatecy;
	LagSimulator::maxlatecy = maxlatecy;
	LagSimulator::packetloss = packetloss;
}

void LagSimulator::QueuePacket(Peer * peer, char * data, unsigned int size){
	char * newdata = new char[size];
	memcpy(newdata, data, size);
	packetbuffer.push_back(newdata);
	packetbufferip.push_back(peer->ip);
	packetbufferport.push_back(peer->port);
	packetbuffersize.push_back(size);
	Uint32 wait = minlatecy;
	if(maxlatecy - minlatecy > 0){
		wait += rand() % (maxlatecy - minlatecy);
	}
	packetbuffertime.push_back(SDL_GetTicks() + wait);
}

void LagSimulator::Process(World & world){
	std::vector<unsigned long>::iterator i = packetbufferip.begin();
	std::vector<unsigned short>::iterator i5 = packetbufferport.begin();
	std::vector<char *>::iterator i2 = packetbuffer.begin();
	std::vector<int>::iterator i3 = packetbuffersize.begin();
	std::vector<int>::iterator i4 = packetbuffertime.begin();
	bool more;
	do{
		more = false;
		for(; i != packetbufferip.end(); i++, i2++, i3++, i4++, i5++){
			/*Uint32 wait = minlatecy;
			if(maxlatecy - minlatecy > 0){
				wait += rand() % (maxlatecy - minlatecy);
			}*/
			if(SDL_GetTicks() >= (*i4)){
				more = true;
				sockaddr_in recvaddr;
				recvaddr.sin_family = AF_INET;
				recvaddr.sin_port = htons((*i5));
				recvaddr.sin_addr.s_addr = htonl((*i));
				if((float(rand()) / RAND_MAX) * 100 > packetloss){
					int ret = sendto(*sockethandle, (*i2), (*i3), 0, (sockaddr *)&recvaddr, sizeof(recvaddr));
					if(ret > 0){
						world.totalbytessent += ret;
					}
				}
				delete[] (*i2);
				packetbufferip.erase(i);
				packetbufferport.erase(i5);
				packetbuffer.erase(i2);
				packetbuffersize.erase(i3);
				packetbuffertime.erase(i4);
				break;
			}
		}
	}while(more);
}