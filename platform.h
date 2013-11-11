#ifndef PLATFORM_H
#define PLATFORM_H

#include "shared.h"
#include "platformset.h"

class Platform
{
public:
    Platform(Uint8 type, Uint16 id, int x1, int y1, int x2, int y2);
	void GetTopSegment(int & x1, int & y1, int & x2, int & y2);
	int XtoY(int x);
	void GetNormal(int x, int y, float * xn, float * yn);
	int GetLength(void);

	static const Uint8 RECTANGLE = 1 << 0;
	static const Uint8 STAIRSUP = 1 << 1;
	static const Uint8 STAIRSDOWN = 1 << 2;
	static const Uint8 LADDER = 1 << 3;
	static const Uint8 TRACK = 1 << 4;
	static const Uint8 OUTSIDEROOM = 1 << 5;
	static const Uint8 SPECIFICROOM = 1 << 6;
    Uint8 type;
    int x1, x2, y1, y2;
	Uint16 id;
	Platform * adjacentl;
	Platform * adjacentr;
	class PlatformSet * set;
};

#endif