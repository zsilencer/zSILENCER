#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "shared.h"
#include "object.h"
#include "shared.h"

class TextInput : public Object
{
public:
	TextInput();
	void ProcessKeyPress(char ascii);
	void Clear(void);
	void SetText(const char * text);
	int MouseInside(Uint16 mousex, Uint16 mousey);
	void SetCaretPosition(int offset);
	char text[256];
	Uint8 fontwidth;
	unsigned int maxchars;
	unsigned int maxwidth;
	Uint8 uid;
	bool showcaret;
	bool password;
	bool inactive;
	Uint8 caretcolor;
	bool enterpressed;
	bool tabpressed;
	Uint16 width;
	Uint16 height;
	unsigned int scrolled;

private:
	unsigned int offset;
	unsigned int caret;
};

#endif