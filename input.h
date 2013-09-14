#ifndef INPUT_H
#define INPUT_H

#include "shared.h"
#include "serializer.h"

class Input
{
public:
	Input();
	void Serialize(bool write, Serializer & data);
	bool keymoveup;
	bool keymovedown;
	bool keymoveleft;
	bool keymoveright;
	bool keylookupleft;
	bool keylookupright;
	bool keylookdownleft;
	bool keylookdownright;
	bool keynextinv;
	bool keynextcam;
	bool keyprevcam;
	bool keydetonate;
	bool keyjump;
	bool keyjetpack;
	bool keyactivate;
	bool keyuse;
	bool keyfire;
	bool keydisguise;
	bool keyweapon[4];
	// interface keys
	bool keyup;
	bool keydown;
	bool keyleft;
	bool keyright;
	bool keychat;
	Uint16 mousex;
	Uint16 mousey;
	bool mousedown;
};

#endif