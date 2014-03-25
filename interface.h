#ifndef INTERFACE_H
#define INTERFACE_H

#include "shared.h"
#include "object.h"

class Interface : public Object
{
public:
	Interface();
	void Tick(World & world);
	void AddObject(Uint16 id);
	void RemoveObject(Uint16 id);
	void AddTabObject(Uint16 id);
	void ProcessKeyPress(World & world, char ascii);
	void ProcessMousePress(World & world, bool pressed, Uint16 x, Uint16 y);
	void ProcessMouseMove(World & world, Uint16 x, Uint16 y);
	void ProcessMouseWheelUp(World & world);
	void ProcessMouseWheelDown(World & world);
	void ActiveChanged(World & world, Interface * callinginterface, bool mouse);
	Object * GetObjectWithUid(World & world, Uint8 uid);
	void DestroyInterface(World & world, Interface * parentinterface);
	static char * WordWrap(const char * string, unsigned int maxlength, const char * breakchar = "\n");
	std::vector<Uint16> objects;
	std::vector<Uint16> tabobjects;
	Uint16 activeobject;
	Uint16 oldactiveobject;
	Uint16 buttonescape;
	Uint16 buttonenter;
	Uint16 mousex;
	Uint16 mousey;
	Uint16 width;
	Uint16 height;
	Uint16 x;
	Uint16 y;
	Uint16 scrollbar;
	Uint16 objectupscroll;
	Uint16 objectdownscroll;
	bool disabled;
	SDL_Scancode lastsym;
	bool modal;
	
private:
	void TabPressed(World & world);
	void EnterPressed(World & world);
	void EscapePressed(World & world);
	void LeftPressed(World & world);
	void RightPressed(World & world);
	void UpPressed(World & world);
	void DownPressed(World & world);
	void Prev(World & world);
	void Next(World & world);
	bool mousedown;
	bool mousewheelup;
	bool mousewheeldown;
};

#endif