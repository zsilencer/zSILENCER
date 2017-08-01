#include "textbox.h"
#include "interface.h"

TextBox::TextBox() : Object(ObjectTypes::TEXTBOX){
	res_bank = 133;
	res_index = 0xFF;
	lineheight = 11;
	fontwidth = 6;
	width = 100;
	height = 100;
	requiresauthority = false;
	bottomtotop = false;
	scrolled = 0;
	maxlines = 256;
	requiresmaptobeloaded = false;
}

void TextBox::Tick(World & world){
	
}

void TextBox::AddLine(const char * string, Uint8 color, Uint8 brightness, bool scroll){
	if(text.size() > maxlines){
		text.pop_front();
	}
	if(scroll){
		if(text.size() > height / lineheight){
			scrolled = text.size() - (height / lineheight);
		}else{
			scrolled = 0;
		}
	}
	unsigned int size = strlen(string);
	if(size * fontwidth > width){
		size = width / fontwidth;
	}
	std::vector<char> newstring(size + 1 + 2);
	newstring[size] = 0;
	strncpy(newstring.data(), string, size);
	newstring[size + 1] = color;
	newstring[size + 2] = brightness;
	text.push_back(newstring);
}

void TextBox::AddText(const char * string, Uint8 color, Uint8 brightness, int indent, bool scroll){
	char breakchar[10];
	strcpy(breakchar, "\n");
	for(int i = 0; i < indent && i < 8; i++){
		strcat(breakchar, " ");
	}
	char * wrapped = Interface::WordWrap(string, width / fontwidth, breakchar);
	char * line = strtok(wrapped, "\n");
	while(line){
		AddLine(line, color, brightness, scroll);
		line = strtok(NULL, "\n");
	}
	delete[] wrapped;
}