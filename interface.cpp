#include "interface.h"
#include "button.h"
#include "toggle.h"
#include "textinput.h"
#include "scrollbar.h"
#include "selectbox.h"
#include "overlay.h"
#include <algorithm>

Interface::Interface() : Object(ObjectTypes::INTERFACE){
	activeobject = 0;
	buttonenter = 0;
	buttonescape = 0;
	mousex = 0;
	mousey = 0;
	width = 0;
	height = 0;
	x = 0;
	y = 0;
	scrollbar = 0;
	mousewheelup = false;
	mousewheeldown = false;
	requiresmaptobeloaded = false;
	requiresauthority = false;
	disabled = false;
	lastsym = SDL_SCANCODE_UNKNOWN;
	objectupscroll = 0;
	objectdownscroll = 0;
	mousedown = false;
	issprite = false;
	iscontrollable = true;
	modal = false;
}

void Interface::Tick(World & world){

}

void Interface::AddObject(Uint16 id){
	objects.push_back(id);
}

void Interface::RemoveObject(Uint16 id){
	std::vector<Uint16>::iterator it = std::find(objects.begin(), objects.end(), id);
	if(it != objects.end()){
		if(activeobject == *it){
			activeobject = 0;
		}
		objects.erase(it);
	}
}

void Interface::AddTabObject(Uint16 id){
	if(!activeobject){
		activeobject = id;
		oldactiveobject = activeobject;
	}
	tabobjects.push_back(id);
}

void Interface::ProcessKeyPress(World & world, char ascii){
	if(disabled){
		return;
	}
	char key = ascii;
	switch(key){
		case '\t':
			TabPressed(world);
		break;
		case '\n':
			EnterPressed(world);
		break;
		case 0x1B:
			EscapePressed(world);
		break;
		case 1:
			LeftPressed(world);
		break;
		case 2:
			RightPressed(world);
		break;
		case 3:
			UpPressed(world);
		break;
		case 4:
			DownPressed(world);
		break;
	}
	Object * object = world.GetObjectFromId(activeobject);
	if(object){
		switch(object->type){
			case ObjectTypes::INTERFACE:{
				Interface * iface = static_cast<Interface *>(object);
				if(iface){
					iface->ProcessKeyPress(world, ascii);
				}
			}break;
			case ObjectTypes::TEXTINPUT:{
				TextInput * textinput = static_cast<TextInput *>(object);
				if(textinput){
					textinput->ProcessKeyPress(key);
				}
			}break;
		}
	}
}

void Interface::ProcessMousePress(World & world, bool pressed, Uint16 x, Uint16 y){
	mousedown = pressed;
	mousex = x;
	mousey = y;
	ActiveChanged(world, this, true);
}

void Interface::ProcessMouseMove(World & world, Uint16 x, Uint16 y){
	mousedown = false;
	mousex = x;
	mousey = y;
	ActiveChanged(world, this, true);
}

void Interface::ProcessMouseWheelUp(World & world){
	mousewheelup = true;
	ActiveChanged(world, this, true);
}

void Interface::ProcessMouseWheelDown(World & world){
	mousewheeldown = true;
	ActiveChanged(world, this, true);
}

void Interface::ActiveChanged(World & world, Interface * callinginterface, bool mouse){
	if(disabled){
		return;
	}
	for(std::vector<Uint16>::iterator it = objects.begin(); it != objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object){
			switch(object->type){
				case ObjectTypes::SCROLLBAR:{
					ScrollBar * scrollbar = static_cast<ScrollBar *>(object);
					if(scrollbar){
						if(world.GetObjectFromId(callinginterface->activeobject) == this || callinginterface == this){
							if(mousewheelup){
								scrollbar->ScrollUp();
							}
							if(mousewheeldown){
								scrollbar->ScrollDown();
							}
						}
						if(mousedown){
							if(scrollbar->MouseInside(world, mousex, mousey)){
								if(scrollbar->MouseInsideUp(world, mousex, mousey)){
									scrollbar->ScrollUp();
								}else
								if(scrollbar->MouseInsideDown(world, mousex, mousey)){
									scrollbar->ScrollDown();
								}else{
									/*Uint16 scrollarea = world.resources.spriteheight[scrollbar->res_bank][scrollbar->barres_index];
									int scrollbarthickness = scrollarea - (scrollbar->scrollmax);
									if(scrollbarthickness < 32){
										scrollbarthickness = 32;
									}
									float scrolly = mousey + world.resources.spriteoffsety[scrollbar->res_bank][scrollbar->res_index] - 16;
									if(scrolly < 0){
										scrolly = 0;
									}
									if(scrolly > scrollarea){
										scrolly = scrollarea;
									}
									scrolly = ((scrolly * 2) / (scrollarea)) - 1;
									if(scrolly > 1){
										scrolly = 1;
									}
									if(scrolly < -1){
										scrolly = -1;
									}
									scrollbar->scrollposition = abs(scrolly) * scrollbar->scrollmax;*/
 								}
							}
						}
					}
				}break;
				case ObjectTypes::INTERFACE:{
					Interface * iface = static_cast<Interface *>(object);
					if(callinginterface && callinginterface->modal){
						break;
					}
					iface->mousedown = mousedown;
					iface->mousewheelup = mousewheelup;
					iface->mousewheeldown = mousewheeldown;
					iface->mousex = mousex;
					iface->mousey = mousey;
					if(mousedown){
						if(mousex < iface->x + iface->width && mousex > iface->x && mousey < iface->y + iface->height && mousey > iface->y){
							activeobject = iface->id;
							iface->activeobject = iface->oldactiveobject;
							for(std::vector<Uint16>::iterator it = objects.begin(); it != objects.end(); it++){
								Object * object = world.GetObjectFromId(*it);
								if(object){
									if(object->type == ObjectTypes::INTERFACE){
										Interface * iface = static_cast<Interface *>(object);
										if(iface){
											if(iface->id != activeobject){
												if(iface->activeobject){
													iface->oldactiveobject = iface->activeobject;
													iface->activeobject = 0;
												}
												iface->ActiveChanged(world, this, mouse);
											}
										}
									}
								}
							}
						}
					}
					iface->ActiveChanged(world, this, mouse);
				}break;
				case ObjectTypes::SELECTBOX:{
					SelectBox * selectbox = static_cast<SelectBox *>(object);
					if(selectbox){
						if(mouse && mousedown){
							int index = selectbox->MouseInside(world, mousex, mousey);
							if(index >= 0){
								selectbox->selecteditem = index;
							}
						}
					}
				}break;
				case ObjectTypes::OVERLAY:{
					Overlay * overlay = static_cast<Overlay *>(object);
					if(overlay){
						if(mouse && mousedown){
							if(overlay->MouseInside(world, mousex, mousey)){
								overlay->clicked = true;
							}
						}
					}
				}break;
				case ObjectTypes::BUTTON:{
					Button * button = static_cast<Button *>(object);
					if(button){
						if(mouse){
							if(button->MouseInside(world, mousex, mousey)){
								if((button->state == Button::INACTIVE || button->state == Button::DEACTIVATING)){
									button->Activate();
								}
								if(mousedown){
									button->clicked = true;
								}
							}else{
								button->Deactivate();
							}
						}else{
							if(button->id == activeobject && (button->state == Button::INACTIVE || button->state == Button::DEACTIVATING)){
								button->Activate();
							}
							if(button->id != activeobject){
								button->Deactivate();
							}
						}
					}
				}break;
				case ObjectTypes::TEXTINPUT:{
					TextInput * textinput = static_cast<TextInput *>(object);
					if(textinput){
						if(mouse && mousedown){
							int index = textinput->MouseInside(mousex, mousey);
							if(index != -1){
								activeobject = textinput->id;
								//textinput->SetCaretPosition(index);
#ifdef __ANDROID__
								JNIEnv * env;
								jvm->GetEnv((void **)&env, JNI_VERSION_1_6);
								jclass cls = env->FindClass("com/zSILENCER/game/zSILENCER");
								jmethodID show = env->GetStaticMethodID(cls, "showKeyboard", "()V");
								env->CallStaticVoidMethod(cls, show);
#endif
							}
						}
						if(activeobject == textinput->id){
							textinput->showcaret = true;
						}else{
							textinput->showcaret = false;
						}
					}
				}break;
				case ObjectTypes::TOGGLE:{
					Toggle * toggle = static_cast<Toggle *>(object);
					if(toggle){
						if(mouse){
							if(toggle->MouseInside(world, mousex, mousey) && mousedown){
								toggle->selected = true;
								activeobject = toggle->id;
							}
						}else{
							if(toggle->id == activeobject){
								toggle->selected = true;
							}
						}
						if(toggle->selected && toggle->set){
							for(std::vector<Uint16>::iterator it = objects.begin(); it != objects.end(); it++){
								Object * object = world.GetObjectFromId(*it);
								if(object){
									if(object->type == ObjectTypes::TOGGLE){
										Toggle * toggle2 = static_cast<Toggle *>(object);
										if(toggle2){
											if(toggle2->set == toggle->set && toggle2->id != toggle->id){
												toggle2->selected = false;
											}
										}
									}
								}
							}
						}
					}
				}break;
			}
		}
	}
	mousewheeldown = false;
	mousewheelup = false;
}

Object * Interface::GetObjectWithUid(World & world, Uint8 uid){
	for(std::vector<Uint16>::iterator it = objects.begin(); it != objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object){
			if(object->type == ObjectTypes::BUTTON){
				Button * button = static_cast<Button *>(object);
				if(button){
					if(button->uid == uid){
						return object;
					}
				}
			}else
			if(object->type == ObjectTypes::TEXTINPUT){
				TextInput * textinput = static_cast<TextInput *>(object);
				if(textinput){
					if(textinput->uid == uid){
						return object;
					}
				}
			}else
			if(object->type == ObjectTypes::SELECTBOX){
				SelectBox * selectbox = static_cast<SelectBox *>(object);
				if(selectbox){
					if(selectbox->uid == uid){
						return object;
					}
				}
			}else
			if(object->type == ObjectTypes::OVERLAY){
				Overlay * overlay = static_cast<Overlay *>(object);
				if(overlay){
					if(overlay->uid == uid){
						return object;
					}
				}
			}
		}
	}
	return 0;
}

void Interface::DestroyInterface(World & world, Interface * parentinterface){
	for(std::vector<Uint16>::iterator it = parentinterface->objects.begin(); it != parentinterface->objects.end(); it++){
		if(*it == id){
			parentinterface->objects.erase(it);
			break;
		}
	}
	for(std::vector<Uint16>::iterator it = objects.begin(); it != objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object){
			if(object->type == ObjectTypes::INTERFACE){
				Interface * iface = static_cast<Interface *>(object);
				iface->DestroyInterface(world, this);
				it = objects.begin();
			}
			world.MarkDestroyObject(object->id);
		}
	}
	world.MarkDestroyObject(id);
}

char * Interface::WordWrap(const char * text, unsigned int maxlength){
	// This function was taken from php
	const char * breakchar = "\n";
	char * newtext = 0;
	int textlen = strlen(text), breakcharlen = 1, newtextlen, chk;
	size_t alloced;
	long current = 0, laststart = 0, lastspace = 0;
	long linelength = maxlength;
	bool docut = true;

	/* Special case for a single-character break as it needs no
	 additional storage space */
	if (breakcharlen == 1 && !docut) {
		//newtext = estrndup(text, textlen);
		newtext = new char[textlen + 1];
		strcpy(newtext, text);
		
		laststart = lastspace = 0;
		for (current = 0; current < textlen; current++) {
			if (text[current] == breakchar[0]) {
				laststart = lastspace = current + 1;
			} else if (text[current] == ' ') {
				if (current - laststart >= linelength) {
					newtext[current] = breakchar[0];
					laststart = current + 1;
				}
				lastspace = current;
			} else if (current - laststart >= linelength && laststart != lastspace) {
				newtext[lastspace] = breakchar[0];
				laststart = lastspace + 1;
			}
		}
		
		//RETURN_STRINGL(newtext, textlen, 0);
		return newtext;
	} else {
		/* Multiple character line break or forced cut */
		if (linelength > 0) {
			chk = (int)(textlen/linelength + 1);
			//newtext = safe_emalloc(chk, breakcharlen, textlen + 1);
			newtext = new char[textlen + 1];
			alloced = textlen + chk * breakcharlen + 1;
		} else {
			chk = textlen;
			alloced = textlen * (breakcharlen + 1) + 1;
			//newtext = safe_emalloc(textlen, (breakcharlen + 1), 1);
			newtext = new char[1];
		}
		
		/* now keep track of the actual new text length */
		newtextlen = 0;
		
		laststart = lastspace = 0;
		for (current = 0; current < textlen; current++) {
			if (chk <= 0) {
				alloced += (int) (((textlen - current + 1)/linelength + 1) * breakcharlen) + 1;
				//newtext = erealloc(newtext, alloced);
				if(newtext){
					delete[] newtext;
				}
				newtext = new char[alloced];
				chk = (int) ((textlen - current)/linelength) + 1;
			}
			/* when we hit an existing break, copy to new buffer, and
			 * fix up laststart and lastspace */
			if (text[current] == breakchar[0]
				&& current + breakcharlen < textlen
				&& !strncmp(text+current, breakchar, breakcharlen)) {
				memcpy(newtext+newtextlen, text+laststart, current-laststart+breakcharlen);
				newtextlen += current-laststart+breakcharlen;
				current += breakcharlen - 1;
				laststart = lastspace = current + 1;
				chk--;
			}
			/* if it is a space, check if it is at the line boundary,
			 * copy and insert a break, or just keep track of it */
			else if (text[current] == ' ') {
				if (current - laststart >= linelength) {
					memcpy(newtext+newtextlen, text+laststart, current-laststart);
					newtextlen += current - laststart;
					memcpy(newtext+newtextlen, breakchar, breakcharlen);
					newtextlen += breakcharlen;
					laststart = current + 1;
					chk--;
				}
				lastspace = current;
			}
			/* if we are cutting, and we've accumulated enough
			 * characters, and we haven't see a space for this line,
			 * copy and insert a break. */
			else if (current - laststart >= linelength
					 && docut && laststart >= lastspace) {
				memcpy(newtext+newtextlen, text+laststart, current-laststart);
				newtextlen += current - laststart;
				memcpy(newtext+newtextlen, breakchar, breakcharlen);
				newtextlen += breakcharlen;
				laststart = lastspace = current;
				chk--;
			}
			/* if the current word puts us over the linelength, copy
			 * back up until the last space, insert a break, and move
			 * up the laststart */
			else if (current - laststart >= linelength
					 && laststart < lastspace) {
				memcpy(newtext+newtextlen, text+laststart, lastspace-laststart);
				newtextlen += lastspace - laststart;
				memcpy(newtext+newtextlen, breakchar, breakcharlen);
				newtextlen += breakcharlen;
				laststart = lastspace = lastspace + 1;
				chk--;
			}
		}
		
		/* copy over any stragglers */
		if (laststart != current) {
			memcpy(newtext+newtextlen, text+laststart, current-laststart);
			newtextlen += current - laststart;
		}
		
		newtext[newtextlen] = '\0';
		/* free unused memory */
		//newtext = erealloc(newtext, newtextlen+1);
		
		return newtext;
		//RETURN_STRINGL(newtext, newtextlen, 0);
	}
}

void Interface::TabPressed(World & world){
	Next(world);
}

void Interface::EnterPressed(World & world){
	for(std::vector<Uint16>::iterator it = objects.begin(); it != objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object){
			switch(object->type){
				case ObjectTypes::BUTTON:{
					Button * button = static_cast<Button *>(object);
					if(button){
						if(button->id == activeobject || button->id == buttonenter){
							button->clicked = true;
						}
					}
				}break;
				case ObjectTypes::SELECTBOX:{
					SelectBox * selectbox = static_cast<SelectBox *>(object);
					if(selectbox){
						if(selectbox->id == activeobject){
							selectbox->enterpressed = true;
						}
					}
				}
			}
		}
	}
}

void Interface::EscapePressed(World & world){
	for(std::vector<Uint16>::iterator it = objects.begin(); it != objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object){
			if(object->type == ObjectTypes::BUTTON){
				Button * button = static_cast<Button *>(object);
				if(button){
					if(button->id == buttonescape){
						button->clicked = true;
					}
				}
			}
		}
	}
}

void Interface::LeftPressed(World & world){
	Object * object = world.GetObjectFromId(activeobject);
	if(object && object->type == ObjectTypes::SELECTBOX){
		
	}else{
		Prev(world);
	}
}

void Interface::RightPressed(World & world){
	Object * object = world.GetObjectFromId(activeobject);
	if(object && object->type == ObjectTypes::SELECTBOX){
		
	}else{
		Next(world);
	}
}

void Interface::UpPressed(World & world){
	Prev(world);
}

void Interface::DownPressed(World & world){
	Next(world);
}

void Interface::Prev(World & world){
	bool gotonext = true;
	if(activeobject == objectupscroll){
		ScrollBar * scrollbar = (ScrollBar *)world.GetObjectFromId(Interface::scrollbar);
		if(scrollbar){
			scrollbar->ScrollUp();
			if(scrollbar->scrollposition == 0){
				gotonext = true;
			}else{
				Audio::GetInstance().Play(world.resources.soundbank["whoom.wav"]);
				gotonext = false;
			}
		}
	}
	Object * object = world.GetObjectFromId(activeobject);
	if(object && object->type == ObjectTypes::SELECTBOX){
		SelectBox * selectbox = static_cast<SelectBox *>(object);
		if(selectbox){
			if(selectbox->selecteditem > 0){
				selectbox->selecteditem--;
			}
		}
	}
	if(gotonext){
		std::vector<Uint16>::reverse_iterator it = std::find(tabobjects.rbegin(), tabobjects.rend(), activeobject);
		if(it != tabobjects.rend()){
			it++;
			if(it == tabobjects.rend()){
				it = tabobjects.rbegin();
			}
			if(it != tabobjects.rend()){
				activeobject = (*it);
			}
		}else{
			it = tabobjects.rbegin();
			if(it != tabobjects.rend()){
				activeobject = (*it);
			}
		}
		ActiveChanged(world, this, false);
	}
}

void Interface::Next(World & world){
	bool gotonext = true;
	if(activeobject == objectdownscroll){
		ScrollBar * scrollbar = (ScrollBar *)world.GetObjectFromId(Interface::scrollbar);
		if(scrollbar){
			scrollbar->ScrollDown();
			if(scrollbar->scrollposition == scrollbar->scrollmax){
				gotonext = true;
			}else{
				Audio::GetInstance().Play(world.resources.soundbank["whoom.wav"]);
				gotonext = false;
			}
		}
	}
	Object * object = world.GetObjectFromId(activeobject);
	if(object && object->type == ObjectTypes::SELECTBOX){
		SelectBox * selectbox = static_cast<SelectBox *>(object);
		if(selectbox){
			if(selectbox->selecteditem < selectbox->items.size() - 1){
				selectbox->selecteditem++;
			}
		}
	}
	if(gotonext){
		std::vector<Uint16>::iterator it = std::find(tabobjects.begin(), tabobjects.end(), activeobject);
		if(it != tabobjects.end()){
			it++;
			if(it == tabobjects.end()){
				it = tabobjects.begin();
			}
			if(it != tabobjects.end()){
				activeobject = (*it);
			}
		}else{
			it = tabobjects.begin();
			if(it != tabobjects.end()){
				activeobject = (*it);
			}
		}
		ActiveChanged(world, this, false);
	}
}