#ifndef RESOURCES_H
#define RESOURCES_H

#include "shared.h"
#include "palette.h"
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
	SDL_Surface *** spritebank;
	SDL_Surface *** tilebank;
	SDL_Surface *** tileflippedbank;
	int ** spriteoffsetx;
	int ** spriteoffsety;
	unsigned int ** spritewidth;
	unsigned int ** spriteheight;
	std::map<std::string, Mix_Chunk *> soundbank;
	Mix_Music * music;

private:
	void MirrorY(SDL_Surface * surface);
	void RLESurface(SDL_Surface * surface);
	int progress;
	int totalprogressitems;
};

#endif