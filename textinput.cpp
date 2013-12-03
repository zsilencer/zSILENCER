#include "textinput.h"

TextInput::TextInput() : Object(ObjectTypes::TEXTINPUT){
	res_bank = 135;
	res_index = 0xFF;
	fontwidth = 9;
	maxchars = 256;
	maxwidth = 10;
	Clear();
	uid = 0;
	showcaret = false;
	password = false;
	numbersonly = false;
	inactive = false;
	caretcolor = 140;
	enterpressed = false;
	requiresmaptobeloaded = false;
	renderpass = 3;
	tabpressed = false;
	width = 0;
	height = 0;
	scrolled = 0;
}

void TextInput::ProcessKeyPress(char ascii){
	if(inactive){
		return;
	}
	char key = ascii;
	if(key == '\b'){
		if(caret > 0){
			caret--;
			text[caret] = 0;
			offset--;
		}
	}else
	if(key == '\n'){
		enterpressed = true;
	}else
	if(key == '\t'){
		tabpressed = true;
	}else
	if((!numbersonly && key >= 0x20 && key <= 0x7F) || (numbersonly && key >= 0x30 && key <= 0x39)){
		if(offset >= maxchars){
			return;
		}
		if(offset >= maxwidth + scrolled){
			scrolled++;
		}
		text[caret] = key;
		offset++;
		caret++;
	}
}

void TextInput::Clear(void){
	memset(text, 0, sizeof(text));
	caret = 0;
	offset = 0;
	scrolled = 0;
}

void TextInput::SetText(const char * text){
	strcpy(TextInput::text, text);
	caret = strlen(text);
	offset = caret;
}

int TextInput::MouseInside(Uint16 mousex, Uint16 mousey){
	Sint16 x1, y1, x2, y2;
	x1 = x;
	x2 = x + width;
	y1 = y;
	y2 = y + height;
	if(mousex < x2 && mousex > x1 && mousey < y2 && mousey > y1){
		int index = ((mousex - x1) / fontwidth);
		return index;
	}
	return -1;
}

void TextInput::SetCaretPosition(int offset){
	// this doesnt work
	if(offset <= strlen(text)){
		caret = offset;
		TextInput::offset = offset;
	}
}