#ifndef SELECTBOX_H
#define SELECTBOX_H

#include "shared.h"
#include "object.h"
#include "sprite.h"
#include <deque>

class SelectBox : public Object
{
public:
	SelectBox();
	~SelectBox();
	bool AddItem(const char * text, Uint32 id = 0);
	bool DeleteItem(unsigned int index);
	Uint32 IndexToId(unsigned int index);
	int IdToIndex(Uint32 id);
	char * GetItemName(unsigned int index);
	int MouseInside(World & world, Uint16 mousex, Uint16 mousey);
	void ListFiles(const char * directory);
	std::deque<char *> items;
	std::deque<Uint32> itemids;
	Uint16 width;
	Uint16 height;
	Uint8 lineheight;
	Uint16 scrolled;
	Uint16 maxlines;
	int selecteditem;
	Uint8 uid;
	bool enterpressed;
};

#endif