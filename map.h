#ifndef MAP_H
#define MAP_H

#include "shared.h"
#include <vector>
#include <map>
#include "platform.h"
#include "platformset.h"
#include "minimap.h"
#include "zlib/zlib.h"

class Map
{
public:
	Map();
	~Map();
	bool Load(const char * filename, class World & world);
	bool LoadBase(class Team & team, class World & world);
	bool LoadFile(const char * filename, class World & world, Team * team = 0);
	void Unload(void);
	void MiniMapCoords(int & x, int & y);
	void RandomPlayerStartLocation(Sint16 & x, Sint16 & y);
	void CalculateRainPuddleLocations(void);
	void CalculateAdjacentPlatforms(void);
	void CalculatePlatformSets(void);
	void CalculatePlatformSetConnections(void);
	void CalculateNodes(void);
	int AdjacentPlatformsLength(Platform & platform);
	Platform & GetLeftmostPlatform(Platform & platform);
	Platform & GetRightmostPlatform(Platform & platform);
	std::vector<Platform *> LaddersToPlatform(PlatformSet & from, PlatformSet & to);
	Platform * TestAABB(int x1, int y1, int x2, int y2, Uint8 type, Platform * except = 0, bool ignorethin = false);
	bool TestAABB(int x1, int y1, int x2, int y2, Platform * platform, Uint8 type, bool ignorethin = false);
	Platform * TestLine(int x1, int y1, int x2, int y2, int * xe, int * ye, Uint8 type);
	Platform * TestIncr(int x1, int y1, int x2, int y2, int * xv, int * yv, Uint8 type, Platform * except = 0, bool ignorethin = false);
	bool LineSegmentIntersection(float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Dx, float Dy, float * X, float * Y);
	bool loaded;
	Uint16 width;
	Uint16 height;
	unsigned int expandedwidth;
	unsigned int expandedheight;
	char description[0x80];
	Uint8 parallax;
	Uint16 * fg[4];
	Uint16 * bg[4];
	bool * fgflipped[4];
	bool * bgflipped[4];
	bool * fglum[4];
	bool * bglum[4];
	Sint8 ambience;
	Sint8 baseambience;
	std::map<Uint16, Platform *> platformids;
	struct XY { Sint16 x, y; };
	std::vector<XY> playerstartlocations;
	std::vector<XY> surveillancecameras;
	std::vector<XY> rainpuddlelocations;
	Uint16 currentid;
	MiniMap minimap;
	Uint8 * nodetypes;
	
//private:
	static bool CompareType(Platform * a, Platform * b);
	std::vector<Platform *> platforms;
	std::vector<PlatformSet *> platformsets;
};

#endif