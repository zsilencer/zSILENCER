#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "shared.h"
#include "object.h"
#include "sprite.h"
#include <deque>
#include <vector>

class TextBox : public Object
{
public:
	TextBox();
	void Tick(World & world);
	void AddLine(const char * string, Uint8 color = 0, Uint8 brightness = 128, bool scroll = true);
	void AddText(const char * string, Uint8 color = 0, Uint8 brightness = 128, int indent = 0, bool scroll = true);
	Uint8 lineheight;
	Uint8 fontwidth;
	Uint16 width;
	Uint16 height;
	std::deque<std::vector<char>> text;
	bool bottomtotop;
	Uint16 scrolled;
	Uint16 maxlines;
};

#endif