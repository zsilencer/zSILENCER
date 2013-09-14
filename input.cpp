#include "input.h"

Input::Input(){
	keymoveup = false;
	keymovedown = false;
	keymoveleft = false;
	keymoveright = false;
	keylookupleft = false;
	keylookupright = false;
	keylookdownleft = false;
	keylookdownright = false;
	keynextinv = false;
	keynextcam = false;
	keyprevcam = false;
	keydetonate = false;
	keyjump = false;
	keyjetpack = false;
	keyactivate = false;
	keyuse = false;
	keyfire = false;
	keydisguise = false;
	keyweapon[0] = false;
	keyweapon[1] = false;
	keyweapon[2] = false;
	keyweapon[3] = false;
	keyup = false;
	keydown = false;
	keyleft = false;
	keyright = false;
	keychat = false;
	mousedown = false;
	mousex = 0;
	mousey = 0;
}

void Input::Serialize(bool write, Serializer & data){
	data.Serialize(write, keymoveup);
	data.Serialize(write, keymovedown);
	data.Serialize(write, keymoveleft);
	data.Serialize(write, keymoveright);
	data.Serialize(write, keylookupleft);
	data.Serialize(write, keylookupright);
	data.Serialize(write, keylookdownleft);
	data.Serialize(write, keylookdownright);
	data.Serialize(write, keynextinv);
	data.Serialize(write, keynextcam);
	data.Serialize(write, keyprevcam);
	data.Serialize(write, keydetonate);
	data.Serialize(write, keyjump);
	data.Serialize(write, keyjetpack);
	data.Serialize(write, keyactivate);
	data.Serialize(write, keyuse);
	data.Serialize(write, keyfire);
	data.Serialize(write, keydisguise);
	data.Serialize(write, keyweapon[0]);
	data.Serialize(write, keyweapon[1]);
	data.Serialize(write, keyweapon[2]);
	data.Serialize(write, keyweapon[3]);
	data.Serialize(write, keyfire);
	data.Serialize(write, keyup);
	data.Serialize(write, keydown);
	data.Serialize(write, keyleft);
	data.Serialize(write, keyright);
	data.Serialize(write, keychat);
	if(write && mousex == 0xFFFF && mousey == 0xFFFF){
		data.PutBit(false);
	}else{
		if(write){
			data.PutBit(true);
			data.Serialize(write, mousex);
			data.Serialize(write, mousey);
		}else{
			if(data.GetBit()){
				data.Serialize(write, mousex);
				data.Serialize(write, mousey);
			}
		}
	}
	data.Serialize(write, mousedown);
}