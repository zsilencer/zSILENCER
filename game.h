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
	void LoadProgressCallback(int progress, int totalprogressitems);
	
	friend class Audio;

private:
	bool Tick(void);
	void Present(void);
	bool SetupOpenGL(void);
	void CreateRenderer(void);
	void CreateStreamingTexture(void);
	static Uint32 TimerCallback(Uint32 interval, void * param);
	void SetColors(SDL_Color * colors);
	void UpdateInputState(Input & input);
	bool LoadMap(const char * name);
	void UnloadGame(void);
	bool CheckForQuit(void);
	bool CheckForEndOfGame(void);
	bool CheckForConnectionLost(void);
	void ProcessInGameInterfaces(void);
	void ShowDeployMessage(void);
	void GiveDefaultItems(Player & player);
	void JoinGame(LobbyGame & lobbygame, char * password = 0);
	void GoToState(Uint8 newstate);
	Interface * CreateMainMenuInterface(void);
	Interface * CreateOptionsInterface(void);
	Interface * CreateOptionsControlsInterface(void);
	Interface * CreateOptionsDisplayInterface(void);
	Interface * CreateOptionsAudioInterface(void);
	Interface * CreateLobbyConnectInterface(void);
	Interface * CreateLobbyInterface(void);
	Interface * CreateCharacterInterface(void);
	Interface * CreateGameSelectInterface(void);
	Interface * CreateChatInterface(void);
	Interface * CreateGameCreateInterface(void);
	Interface * CreateGameJoinInterface(void);
	Interface * CreateGameTechInterface(void);
	Interface * CreateGameSummaryInterface(Stats & stats, Uint8 agency);
	Interface * CreateModalDialog(const char * message, bool ok = true);
	Interface * CreateMapPreview(const char * filename);
	void PlayMusic(Mix_Music * music);
	void DestroyModalDialog(void);
	Interface * CreatePasswordDialog(void);
	bool GoBack(void);
	Uint16 lobbyinterface;
	Uint16 characterinterface;
	Uint16 chatinterface;
	Uint16 gameselectinterface;
	Uint16 gamecreateinterface;
	Uint16 gamejoininterface;
	Uint16 gametechinterface;
	Uint16 gamesummaryinterface;
	Uint16 modalinterface;
	Uint16 passwordinterface;
	Uint16 mappreviewinterface;
	Overlay * keynameoverlay[6];
	Button * c1button[6];
	Button * cobutton[6];
	Button * c2button[6];
	bool ProcessMainMenuInterface(Interface * iface);
	void ProcessLobbyConnectInterface(Interface * iface);
	bool ProcessLobbyInterface(Interface * iface);
	void ProcessGameSummaryInterface(Interface * iface);
	void UpdateLobbyMapName(const char * name);
	void UpdateTechInterface(void);
	void UpdateGameSummaryInterface(void);
	void AddSummaryLine(TextBox & textbox, const char * name, Uint32 value, bool percentage = false);
	void ShowTeamOverlays(bool show);
	Uint8 GetSelectedAgency(void);
	void IndexToConfigKey(int index, SDL_Scancode ** key1, SDL_Scancode ** key2, bool ** keyop);
	const char * GetKeyName(SDL_Scancode sym);
	void GetGameChannelName(LobbyGame & lobbygame, char * name);
	void CreateAmbienceChannels(void);
	void UpdateAmbienceChannels(void);
	bool FadedIn(void);
	std::vector<std::string> ListFiles(const char * directory);
	void LoadRandomGameMusic(void);
	std::string FindMap(const char * name, unsigned char (*hash)[20] = 0, const char * directory = 0);
	std::string SaveMap(const char * name, unsigned char * data, int size);
	void CalculateMapHash(const char * filename, unsigned char (*hash)[20]);
	std::string StringFromHash(unsigned char (*hash)[20]);
	void LoadMapData(const char * filename);
	void ProcessMapDownload(void);
	static const int numkeys = 20;
	const char * keynames[numkeys];
	Uint8 keystate[SDL_NUM_SCANCODES];
	enum {NONE, FADEOUT, MAINMENU, LOBBYCONNECT, LOBBY, INGAME, MISSIONSUMMARY, SINGLEPLAYERGAME, OPTIONS, OPTIONSCONTROLS, OPTIONSDISPLAY, OPTIONSAUDIO, HOSTGAME, JOINGAME, REPLAYGAME, TESTGAME};
	Uint8 state;
	Uint8 nextstate;
	Uint8 fade_i;
	bool stateisnew;
	bool nextstateprocessed;
	class World world;
	Renderer renderer;
	SDL_Window * window;
	SDL_GLContext glcontext;
	GLuint gltextures[2];
	GLuint fbo;
	bool usingopengl;
	SDL_Renderer * windowrenderer;
	Surface screenbuffer;
	SDL_Surface * sdlscreenbuffer;
	SDL_Texture * streamingtexture;
	SDL_PixelFormat * streamingtexturepixelformat;
	Uint32 streamingtexturepalette[256];
	int frames;
	int fps;
	Uint32 lasttick;
	Uint16 currentinterface;
	Uint16 aftermodalinterface;
	bool motdprinted;
	Uint32 chatlinesprinted;
	char localusername[16 + 1];
	Uint16 sharedstate;
	int bgchannel[3];
	enum {BG_AMBIENT = 0, BG_BASE, BG_OUTSIDE};
	int oldselecteditem;
	Uint8 singleplayermessage;
	bool updatetitle;
	Uint32 currentlobbygameid;
	char lastchannel[64];
	Uint8 oldselectedagency;
	Uint8 oldambiencelevel;
	bool agencychanged;
	bool gamesummaryinfoloaded;
	bool minimized;
	bool creategameclicked;
	bool modaldialoghasok;
	bool joininggame;
	bool deploymessageshown;
	Uint32 optionscontrolstick;
	int quitscancode;
	bool interfaceenterfix;
	Uint32 lastmapchunkrequest;
	bool mapexistchecked;
	int selectedmap;
	Uint32 lastmusicplaytime;
	char currentmusictrack[256];
	bool fullscreentoggled;
};

#endif