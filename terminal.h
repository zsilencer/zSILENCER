#ifndef TERMINAL_H
#define TERMINAL_H

#include "shared.h"
#include "object.h"

class Terminal : public Object
{
public:
	Terminal();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(World & world);
	bool Hack(Uint16 hackerid);
	void HackerGone(void);
	Uint8 GetPercent(void);
	void SetSize(bool big);
	enum {INACTIVE, READY, HACKING, HACKERGONE, BEAMING, SECRETREADY, SECRETBEAMING};
	Uint8 state;
	Uint8 state_i;
	Uint16 hackerid;
	Uint16 files;
	Uint8 secretinfo;
	Uint8 beamingseconds;
	bool isbig;
	Uint8 beamingtime;
	Uint8 tracetime;
	
protected:
	Uint8 juice;
	Uint8 inactiveframes;
	Uint8 beamingframes;
	Uint8 readyframes;
	Uint8 beamingcount;
	bool sizeset;
	bool readysoundplayed;
};

#endif