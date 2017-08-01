#ifndef RESOURCES_H
#define RESOURCES_H

#include "shared.h"
#include "palette.h"
#include "surface.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

class Resources
{
public:
	Resources();
	bool Load(class Game & game, bool dedicatedserver = false);
	bool LoadSprites(class Game & game, bool dedicatedserver = false);
	bool LoadTiles(class Game & game, bool dedicatedserver = false);
	bool LoadSounds(class Game & game, bool dedicatedserver = false);
	void UnloadSounds(void);
	std::vector<std::vector<std::shared_ptr<Surface> > > spritebank;
	std::vector<std::vector<std::shared_ptr<Surface> > > tilebank;
	std::vector<std::vector<std::shared_ptr<Surface> > > tileflippedbank;
	std::vector<std::vector<int> > spriteoffsetx;
	std::vector<std::vector<int> > spriteoffsety;
	std::vector<std::vector<unsigned int> > spritewidth;
	std::vector<std::vector<unsigned int> > spriteheight;
	std::map<std::string, Mix_Chunk *> soundbank;
	Mix_Music * menumusic;
	Mix_Music * gamemusic;

private:
	void MirrorY(Surface * surface);
	void RLESurface(Surface * surface);
	int progress;
	int totalprogressitems;
};

#endif