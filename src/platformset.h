#ifndef PLATFORMSET_H
#define PLATFORMSET_H

#include "shared.h"
#include "platform.h"
#include <vector>

class PlatformSet
{
public:
	PlatformSet();
	std::vector<class Platform *> platforms;
	std::vector<class Platform *> ladders;
	int length;
	Platform * falll;
	Platform * fallr;
};

#endif