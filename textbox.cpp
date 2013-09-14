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

TextBox::~TextBox(){
	for(std::deque<char *>::iterator it = text.begin(); it != text.end(); it++){
		delete[] (*it);
	}
}

void TextBox::Tick(World & world){
	
}

void TextBox::AddLine(const char * string, Uint8 color, Uint8 brightness){
	if(text.size() > maxlines){
		char * oldstring = text.front();
		delete[] oldstring;
		text.pop_front();
	}
	if(text.size() > height / lineheight){
		scrolled = text.size() - (height / lineheight);
	}else{
		scrolled = 0;
	}
	unsigned int size = strlen(string);
	if(size * fontwidth > width){
		size = width / fontwidth;
	}
	char * newstring = new char[size + 1 + 2];
	newstring[size] = 0;
	strncpy(newstring, string, size);
	newstring[size + 1] = color;
	newstring[size + 2] = brightness;
	text.push_back(newstring);
}

void TextBox::AddText(const char * string, Uint8 color, Uint8 brightness){
	char * wrapped = Interface::WordWrap(string, width / fontwidth);
	char * line = strtok(wrapped, "\n");
	while(line){
		AddLine(line, color, brightness);
		line = strtok(NULL, "\n");
	}
	delete[] wrapped;
}