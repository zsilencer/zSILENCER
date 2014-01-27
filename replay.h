#ifndef REPLAY_H
#define REPLAY_H

#include "shared.h"
#include "serializer.h"
#include "lobbygame.h"

class Replay
{
public:
	Replay();
	~Replay();
	void BeginRecording(const char * filename);
	void EndRecording(void);
	void BeginPlaying(const char * filename);
	void EndPlaying(void);
	void WriteHeader(class World & world);
	bool ReadHeader(class World & world);
	bool ReadToNextTick(class World & world);
	void WriteGameInfo(LobbyGame & gameinfo);
	void WriteNewPeer(Uint8 agency, Uint32 accountid);
	void WriteStart(void);
	void WriteUserInfo(class User & user);
	void WriteChangeTeam(Uint8 peerid);
	void WriteSetTech(Uint8 peerid, Uint32 techchoices);
	void WriteChat(Uint8 peerid, Uint8 to, char * msg);
	void WriteStation(Uint8 peerid, Uint8 action, Uint8 itemid);
	void WriteInputCommand(class World & world, Uint8 peerid, Serializer & data);
	void WriteDisconnect(Uint8 peerid);
	void WriteTick(void);
	bool IsRecording(void);
	bool IsPlaying(void);
	bool GameStarted(void);
	enum {RPL_GAMEINFO, RPL_NEWPEER, RPL_START, RPL_USERINFO, RPL_CHANGETEAM, RPL_TECH, RPL_CHAT, RPL_STATION, RPL_TICK, RPL_INPUT, RPL_DISCONNECT};
	enum {STA_BUY, STA_REPAIR, STA_VIRUS};
	
private:
	int GetInputSize(void);
	SDL_RWops * file;
	bool isrecording;
	bool isplaying;
	bool gamestarted;
	unsigned short uniqueport;
	int inputsize;
};

#endif