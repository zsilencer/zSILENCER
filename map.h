#ifndef MAP_H
#define MAP_H

#include "shared.h"
#include <vector>
#include <map>
#include <memory>
#include "platform.h"
#include "platformset.h"
#include "minimap.h"
#include "zlib/zlib.h"

class Map
{
public:
	Map();
	~Map();
	struct Header{
		Header();
		Uint8 firstbyte;
		Uint8 version;
		Uint8 maxplayers;
		Uint8 maxteams;
		Uint16 width;
		Uint16 height;
		Uint8 parallax;
		Sint8 ambience;
		Uint32 flags;
		char description[0x80];
		Uint32 minimapcompressedsize;
		Uint8 minimapcompressed[172 * 62];
		Uint32 levelsize;
	};
	bool Load(const char * filename, class World & world);
	bool LoadBase(class Team & team, class World & world);
	bool LoadFile(const char * filename, class World & world, Team * team = 0);
	static bool LoadHeader(SDL_RWops * file, Map::Header & header);
	static bool UncompressMinimap(Uint8 (*pixels)[172 * 62], const Uint8 * compressed, int compressedsize);
	void Unload(void);
	void MiniMapCoords(int & x, int & y);
	void RandomPlayerStartLocation(World & world, Sint16 & x, Sint16 & y);
	void CalculateRainPuddleLocations(void);
	void CalculateAdjacentPlatforms(void);
	void CalculatePlatformSets(void);
	void CalculatePlatformSetConnections(void);
	void CalculateNodes(void);
	int TeamNumberFromY(Sint16 y);
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
	struct Tile{
		Tile() { fg = 0; bg = 0; bgflags = 0; fgflags = 0; };
		Uint16 fg, bg;
		int fgflags, bgflags;
		enum {
			FLIPPED = 1 << 0,
			LUM = 1 << 1
		};
	};
	std::vector<Tile> tiles[4];
	Sint8 ambience;
	Sint8 baseambience;
	std::map<Uint16, Platform *> platformids;
	struct XY { Sint16 x, y; };
	std::vector<XY> playerstartlocations;
	std::vector<XY> surveillancecameras;
	std::vector<XY> rainpuddlelocations;
	Uint16 currentid;
	MiniMap minimap;
	std::vector<Uint8> nodetypes;
	
//private:
	static bool CompareType(std::shared_ptr<Platform> a, std::shared_ptr<Platform> b);
	std::vector<std::shared_ptr<Platform>> platforms;
	std::vector<std::shared_ptr<PlatformSet>> platformsets;
};

#endif