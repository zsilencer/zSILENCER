#include "button.h"
#include "objecttypes.h"

Button::Button() : Object(ObjectTypes::BUTTON){
	x = 0;
	y = 0;
	state = INACTIVE;
	state_i = 0;
	clicked = false;
	SetType(B196x33);
	uid = 0;
	text[0] = 0;
	requiresmaptobeloaded = false;
}

void Button::Tick(World & world){
	switch(type){
		case B112x33:
			res_index = 28;
		break;
		case B220x33:
			res_index = 23;
		break;
		case B196x33:
			res_index = 7;
		break;
		case B236x27:
			res_index = 2;
		break;
		case B156x21:
			res_index = 24;
		break;
		case BCHECKBOX:
			
		break;
		default:
			res_index = 0xFF;
		break;
	}
	switch(state){
		case INACTIVE:{
			if(type == BCHECKBOX){
				res_index = 19;
				break;
			}
		}break;
		case ACTIVATING:{
			if(type == BCHECKBOX){
				break;
			}
			if(state_i == 0){
				Audio::GetInstance().Play(world.resources.soundbank["whoom.wav"]);
			}
			effectbrightness = 128 + (state_i * 2);
			if(type != B156x21){
				res_index += state_i;
			}
			if(state_i >= 4){
				state = ACTIVE;
				state_i = -1;
				break;
			}
		}break;
		case DEACTIVATING:{
			if(type == BCHECKBOX){
				break;
			}
			effectbrightness = 128 + ((4 - state_i) * 2);
			if(type != B156x21){
				res_index -= -4 + state_i;
			}
			if(state_i >= 4){
				state = INACTIVE;
				state_i = -1;
				break;
			}
		}break;
		case ACTIVE:{
			if(type == BCHECKBOX){
				res_index = 18;
				break;
			}
			if(type != B156x21){
				res_index += 4;
			}
			effectbrightness = 128 + (4 * 2);
		}break;
	}
	state_i++;
}

void Button::SetType(Uint8 type){
	Button::type = type;
	switch(type){
		case BNONE:
			res_bank = 0xFF;
		break;
		case B112x33:
			res_bank = 6;
			width = 112;
			height = 33;
			textbank = 135;
			textwidth = 11;
		break;
		case B220x33:
			res_bank = 6;
			width = 112;
			height = 33;
			textbank = 135;
			textwidth = 11;
		break;
		case B196x33:
			res_bank = 6;
			width = 196;
			height = 33;
			textbank = 135;
			textwidth = 11;
		break;
		case B236x27:
			res_bank = 6;
			width = 236;
			height = 27;
			textbank = 135;
			textwidth = 11;
		break;
		case B52x21:
			res_bank = 0xFF;
			width = 52;
			height = 21;
			textbank = 133;
			textwidth = 7;
		break;
		case B156x21:
			res_bank = 7;
			res_index = 24;
			width = 156;
			height = 21;
			textbank = 134;
			textwidth = 8;
		break;
		case BCHECKBOX:
			res_bank = 7;
			res_index = 19;
			width = 13;
			height = 13;
		break;
	}
}

void Button::Activate(void){
	if(type != BCHECKBOX){
		state = ACTIVATING;
		state_i = 0;
	}
}

void Button::Deactivate(void){
	if(type != BCHECKBOX){
		if(state != DEACTIVATING && state != INACTIVE){
			state = DEACTIVATING;
			state_i = 0;
		}
	}
}

bool Button::MouseInside(World & world, Uint16 mousex, Uint16 mousey){
	Sint16 x1, y1, x2, y2;
	x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
	x2 = x + (width) - world.resources.spriteoffsetx[res_bank][res_index];
	y1 = y - world.resources.spriteoffsety[res_bank][res_index];
	y2 = y + (height) - world.resources.spriteoffsety[res_bank][res_index];
	if(mousex < x2 && mousex > x1 && mousey < y2 && mousey > y1){
		return true;
	}
	return false;
}

void Button::GetTextOffset(World & world, Sint16 * x, Sint16 * y){
	int xoff = int((width - (strlen(text) * textwidth)) / 2);
	int yoff = 0;
	switch(type){
		case B112x33:
		case B196x33:
		case B220x33:
		case B236x27:
			yoff = 8;
		break;
		case B52x21:
			yoff = 8;
			xoff += 1;
		break;
		case B156x21:
			yoff = 4;
		break;
	}
	*x = Button::x - world.resources.spriteoffsetx[res_bank][res_index] + xoff;
	*y = Button::y - world.resources.spriteoffsety[res_bank][res_index] + yoff;
}