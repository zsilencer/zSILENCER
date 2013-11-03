#ifndef CONFIG_H
#define CONFIG_H

#include "shared.h"

class Config
{
public:
	Config();
	static Config & GetInstance(void);
	void Save(void);
	bool Load(void);
	void LoadDefaults(void);
	bool KeyIsPressed(const Uint8 * keyboardstate, SDL_Scancode keybindings[2], bool keyoperator);
	enum {OR, AND};
	bool fullscreen;
	Uint8 defaultagency;
	char defaultgamename[64];
	Uint32 defaulttechchoices[5];
	SDL_Scancode keymoveupbinding[2]; bool keymoveupoperator;
	SDL_Scancode keymovedownbinding[2]; bool keymovedownoperator;
	SDL_Scancode keymoveleftbinding[2]; bool keymoveleftoperator;
	SDL_Scancode keymoverightbinding[2]; bool keymoverightoperator;
	SDL_Scancode keyupbinding[2]; bool keyupoperator;
	SDL_Scancode keydownbinding[2]; bool keydownoperator;
	SDL_Scancode keyleftbinding[2]; bool keyleftoperator;
	SDL_Scancode keyrightbinding[2]; bool keyrightoperator;
	SDL_Scancode keylookupleftbinding[2]; bool keylookupleftoperator;
	SDL_Scancode keylookuprightbinding[2]; bool keylookuprightoperator;
	SDL_Scancode keylookdownleftbinding[2]; bool keylookdownleftoperator;
	SDL_Scancode keylookdownrightbinding[2]; bool keylookdownrightoperator;
	SDL_Scancode keynextinvbinding[2]; bool keynextinvoperator;
	SDL_Scancode keynextcambinding[2]; bool keynextcamoperator;
	SDL_Scancode keyprevcambinding[2]; bool keyprevcamoperator;
	SDL_Scancode keydetonatebinding[2]; bool keydetonateoperator;
	SDL_Scancode keyjumpbinding[2]; bool keyjumpoperator;
	SDL_Scancode keyjetpackbinding[2]; bool keyjetpackoperator;
	SDL_Scancode keyactivatebinding[2]; bool keyactivateoperator;
	SDL_Scancode keyusebinding[2]; bool keyuseoperator;
	SDL_Scancode keyfirebinding[2]; bool keyfireoperator;
	SDL_Scancode keychatbinding[2]; bool keychatoperator;
	SDL_Scancode keydisguisebinding[2]; bool keydisguiseoperator;
	
private:
	bool CompareString(const char * str1, const char * str2);
	void WriteKey(SDL_RWops * file, const char * variable, SDL_Scancode keybindings[2], bool keyoperator);
	void ReadKey(char * data, SDL_Scancode (*keybindings)[2], bool * keyoperator);
	void WriteString(SDL_RWops * file, const char * variable, const char * string);
	void ReadString(const char * data, char * variable);
	char * RWgets(SDL_RWops * file, char * buffer, int count);
};

#endif