#ifndef LAGSIMULATOR_H
#define LAGSIMULATOR_H

#include "shared.h"
#include "peer.h"
#include <vector>

class LagSimulator
{
public:
	LagSimulator(SOCKET * sockethandle);
	void Process(class World & world);
	bool Active(void);
	void Activate(Uint32 minlatecy, Uint32 maxlatecy);
	void QueuePacket(Peer * peer, char * data, unsigned int size);
	
private:
	SOCKET * sockethandle;
	bool active;
	Uint32 minlatecy;
	Uint32 maxlatecy;
	std::vector<char *> packetbuffer;
	std::vector<int> packetbuffersize;
	std::vector<int> packetbuffertime;
	std::vector<unsigned long> packetbufferip;
	std::vector<unsigned short> packetbufferport;
};

#endif