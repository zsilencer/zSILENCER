#ifndef RESOURCES_H
#define RESOURCES_H

#include "shared.h"
#include "palette.h"
#include "surface.h"
#include <map>
#include <string>

class Resources
{
public:
	Resources();
	~Resources();
	bool Load(class Game & game, bool dedicatedserver = false);
	bool LoadSprites(class Game & game, bool dedicatedserver = false);
	bool LoadTiles(class Game & game, bool dedicatedserver = false);
	bool LoadSounds(class Game & game, bool dedicatedserver = false);
	void UnloadSounds(void);
	Surface *** spritebank;
	Surface *** tilebank;
	Surface *** tileflippedbank;
	int ** spriteoffsetx;
	int ** spriteoffsety;
	unsigned int ** spritewidth;
	unsigned int ** spriteheight;
	std::map<std::string, Mix_Chunk *> soundbank;
	Mix_Music * music;

private:
	void MirrorY(Surface * surface);
	void RLESurface(Surface * surface);
	int progress;
	int totalprogressitems;
};

#endif