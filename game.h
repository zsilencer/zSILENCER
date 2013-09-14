#ifndef GAME_H
#define GAME_H

#include "shared.h"
#include "palette.h"
#include "world.h"
#include "renderer.h"
#include "input.h"
#include "state.h"
#include "interface.h"
#include "button.h"
#include "overlay.h"
#include "textbox.h"

class Game
{
public:
	Game();
	~Game();
	bool Load(char * cmdline);
	bool Loop(void);
	bool HandleSDLEvents(void);

private:
	bool Tick(void);
	static Uint32 TimerCallback(Uint32 interval, void * param);
	void SetColors(SDL_Color * colors);
	void UpdateInputState(Input & input);
	bool LoadMap(const char * name, Uint8 securitylevel = 3);
	void UnloadGame(void);
	bool CheckForQuit(void);
	bool CheckForEndOfGame(void);
	bool CheckForConnectionLost(void);
	void ProcessInGameInterfaces(void);
	void ShowDeployMessage(void);
	void JoinGame(LobbyGame & lobbygame);
	Interface * CreateMainMenuInterface(void);
	Interface * CreateOptionsInterface(void);
	Interface * CreateOptionsControlsInterface(void);
	Interface * CreateLobbyConnectInterface(void);
	Interface * CreateLobbyInterface(void);
	Interface * CreateCharacterInterface(void);
	Interface * CreateGameSelectInterface(void);
	Interface * CreateChatInterface(void);
	Interface * CreateGameCreateInterface(void);
	Interface * CreateGameJoinInterface(void);
	Interface * CreateGameTechInterface(void);
	Interface * CreateGameSummaryInterface(Stats & stats, Uint8 agency);
	Interface * CreateModalDialog(const char * message);
	Interface * CreatePasswordDialog(void);
	Interface * characterinterface;
	Interface * chatinterface;
	Interface * gameselectinterface;
	Interface * gamecreateinterface;
	Interface * gamejoininterface;
	Interface * gametechinterface;
	Interface * gamesummaryinterface;
	Interface * modalinterface;
	Interface * passwordinterface;
	Overlay * keynameoverlay[6];
	Button * c1button[6];
	Button * cobutton[6];
	Button * c2button[6];
	bool ProcessMainMenuInterface(Interface * iface);
	void ProcessLobbyConnectInterface(Interface * iface);
	bool ProcessLobbyInterface(Interface * iface);
	void ProcessGameSummaryInterface(Interface * iface);
	void UpdateTechInterface(void);
	void UpdateGameSummaryInterface(void);
	void AddSummaryLine(TextBox & textbox, const char * name, Uint32 value, bool percentage = false);
	void ShowTeamOverlays(bool show);
	Uint8 GetSelectedAgency(void);
	void IndexToConfigKey(int index, SDL_Scancode ** key1, SDL_Scancode ** key2, bool ** keyop);
	const char * GetKeyName(SDL_Scancode sym);
	void GetGameChannelName(LobbyGame & lobbygame, char * name);
	static const int numkeys = 19;
	const char * keynames[numkeys];
	enum {NONE, FADEOUT, MAINMENU, LOBBYCONNECT, LOBBY, INGAME, MISSIONSUMMARY, SINGLEPLAYERGAME, OPTIONS, OPTIONSCONTROLS, HOSTGAME, JOINGAME};
	Uint8 state;
	Uint8 nextstate;
	Uint8 fade_i;
	bool stateisnew;
	class World world;
	Renderer renderer;
	SDL_Window * window;
	SDL_Renderer * windowrenderer;
	SDL_Surface * screen;
	SDL_Surface * screenbuffer;
	int frames;
	int fps;
	Uint32 lasttick;
	Uint16 currentinterface;
	Uint16 aftermodalinterface;
	bool motdprinted;
	Uint32 chatlinesprinted;
	char localusername[16 + 1];
	State * sharedstate;
	int ambientbgchannel;
	int oldselecteditem;
	Uint8 singleplayermessage;
	bool updatetitle;
	Uint32 currentlobbygameid;
	char lastchannel[64];
	Uint8 oldselectedagency;
	bool agencychanged;
	bool gamesummaryinfoloaded;
	bool minimized;
};

#endif