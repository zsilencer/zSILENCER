#ifndef LOBBY_H
#define LOBBY_H

#include "shared.h"
#include "lobbygame.h"
#include "peer.h"
#include "user.h"
#include <list>
#include <map>

class Lobby
{
public:
	Lobby(class World * world);
	~Lobby();
	void Connect(const char * host, unsigned short port);
	void Disconnect(void);
	void DoNetwork(void);
	void LockMutex(void);
	void UnlockMutex(void);
	enum {IDLE, WAITING, CONNECTING, RESOLVING, WAITINGFORRESOLVER, RESOLVED, RESOLVEFAILED, CONNECTIONFAILED, CONNECTED, CHECKINGVERSION, AUTHENTICATING, AUTHSENT, AUTHENTICATED, AUTHFAILED, DISCONNECTED} state;
	enum {MSG_AUTH, MSG_MOTD, MSG_CHAT, MSG_NEWGAME, MSG_DELGAME, MSG_CHANNEL, MSG_CONNECT, MSG_VERSION, MSG_USERINFO, MSG_PING, MSG_UPGRADESTAT, MSG_REGISTERSTATS};
	void SendVersion(void);
	void SendCredentials(const char * username, const char * password);
	void SendChat(const char * channel, const char * message);
	void JoinChannel(const char * channel);
	void CreateGame(const char * name, const char * map, const unsigned char maphash[20], const char * password = 0, Uint8 securitylevel = LobbyGame::SECMEDIUM, Uint8 minlevel = 0, Uint8 maxlevel = 99, Uint8 maxplayers = 24, Uint8 maxteams = 6);
	//void ConnectToGame(LobbyGame & lobbygame, Uint8 agency);
	void ClearGames(void);
	LobbyGame * GetGameById(Uint32 id);
	User * GetUserInfo(Uint32 accountid);
	void ForgetUserInfo(Uint32 accountid);
	void ForgetAllUserInfo(void);
	void UpgradeStat(Uint8 agency, Uint8 stat);
	void RegisterStats(User & user, Uint8 won, Uint32 gameid);
	char failmessage[256];
	Uint32 accountid;
	char motd[2048];
	bool motdreceived;
	Uint8 creategamestatus;
	Uint32 createdgameid;
	Uint8 connectgamestatus;
	std::list<char *> chatmessages;
	std::list<LobbyGame *> games;
	bool gamesprocessed;
	char channel[64];
	bool channelchanged;
	char serverip[256];
	bool versionchecked;
	bool versionok;
	Uint8 selectedagency;
	bool statupgraded;

private:
	bool Send(const char * data, unsigned int size);
	void SendMessage(const char msg[0xFF], Uint8 size);
	void ResolveHostname(const char * host);
	static int ResolveThreadFunc(void * param);
	SOCKET sockethandle;
	Uint8 msgsize;
	Uint8 msgoffset;
	char msg[0xFF];
	Uint32 lasttime;
	SDL_Thread * resolvethread;
	SDL_mutex * mutex;
	hostent * he;
	bool resolvethreadrunning;
	char resolvehost[256];
	char sendbuffer[4096];
	unsigned int sendbuffersize;
	unsigned int sendbufferoffset;
	static const unsigned int maxusername = 16;
	World * world;
	std::map<Uint32, User *> userinfos;
};

#endif