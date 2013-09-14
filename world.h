#ifndef WORLD_H
#define WORLD_H

#include "shared.h"
#include <list>
#include <map>
#include <deque>
#include "peer.h"
#include "object.h"
#include "sprite.h"
#include "input.h"
#include "objecttypes.h"
#include "map.h"
#include "resources.h"
#include "audio.h"
#include "lobby.h"
#include "lobbygame.h"
#include "lagsimulator.h"
#include "dedicatedserver.h"
#include "buyableitem.h"

class World
{
public:
	World(bool mode = AUTHORITY);
	~World();
	class Object * CreateObject(Uint8 type, Uint16 id = 0);
	Object * GetObjectFromId(Uint16 id);
	void MarkDestroyObject(Uint16 id);
	void DestroyMarkedObjects(void);
	void DestroyObject(Uint16 id);
	void DestroyAllObjects(void);
	void DoNetwork(void);
	void Tick(void);
	void TickObjects(void);
	void SetVersion(const char * version);
	bool Listen(unsigned short port = 0);
	unsigned short Bind(unsigned short port = 0);
	//void NATHolePunch(Peer & peer);
	//void RequestPublicPort(char * serveraddress, unsigned short port);
	void Connect(Uint8 agency, Uint32 accountid, const char * password = 0);
	void Disconnect(void);
	Peer * GetAuthorityPeer(void);
	class Player * GetPeerPlayer(Uint8 peerid);
	Team * GetPeerTeam(Uint8 peerid);
	bool FindTeamForPeer(Peer & peer, Uint8 agency, int start = 0);
	void SendInput(void);
	void SwitchToLocalAuthorityMode(void);
	bool IsAuthority(void);
	void Illuminate(void);
	void ShowMessage(const char * message, Uint8 time = 255, Uint8 type = 0);
	void ShowStatus(const char * status, Uint8 color = 0, bool networked = false, Peer * peer = 0);
	void SendChat(bool toteam, char * message);
	void ChangeTeam(void);
	void SetTech(Uint32 techchoices);
	int TechSlotsUsed(Peer & peer);
	void SendPing(void);
	int GetPingTime(void);
	void SetSystemCamera(bool system, Uint16 objectfollow, Sint16 x, Sint16 y);
	bool TestAABB(int x1, int y1, int x2, int y2, Object * object, std::vector<Uint8> & types);
	std::vector<Object *> TestAABB(int x1, int y1, int x2, int y2, std::vector<Uint8> & types, Uint16 except = 0);
	Object * TestIncr(int x1, int y1, int x2, int y2, int * xv, int * yv, std::vector<Uint8> & types, Uint16 except = 0);
	enum modes {AUTHORITY, REPLICA};
	enum {BUY_NONE, BUY_LASER, BUY_ROCKET, BUY_FLAMER, BUY_HEALTH, BUY_EMPB, BUY_SHAPEDB, BUY_PLASMAB, BUY_NEUTRONB, BUY_DET, BUY_FIXEDC, BUY_FLARE, BUY_DOOR, BUY_DEFENSE, BUY_INFO, BUY_GIVE0, BUY_GIVE1, BUY_GIVE2, BUY_GIVE3};
	Input localinput;
	class Map map;
	Resources resources;
	class Audio & audio;
	Lobby lobby;
	unsigned int totalbytesread;
	unsigned int totalbytessent;
	unsigned int totalsnapshots;
	unsigned int totalinputpackets;
	Uint8 gravity;
	Uint8 maxyvelocity;
	bool replaying;
	Uint8 quitstate;
	std::deque<std::string> chatlines;
	Uint8 showchat_i;
	std::vector<BuyableItem *> buyableitems;
	std::deque<char *> statusmessages;
	static const int maxstatusmessages = 4;
	Uint32 tickcount;
	char password[32];
	bool choosingtech;
	
	friend class Renderer;
	friend class Game;
	friend class Team;
	friend class Lobby;
	friend class Player;
	friend class Map;
	friend class RocketProjectile;
	friend class DedicatedServer;
	friend class SurveillanceMonitor;
	friend class Warper;
	friend class Grenade;
	friend class BaseDoor;
	friend class Terminal;
	
protected:
	std::list<class Object *> objectlist;
	void SaveSnapshot(Serializer & data, Uint8 peerid);
	void LoadSnapshot(Serializer & data, bool create = true, Serializer * delta = 0, Uint16 objectid = 0);
	Peer * AddPeer(char * address, unsigned short port, Uint8 agency, Uint32 accountid);
	char mapname[256];
	std::vector<Uint32> ingameusers;

private:
	void DoNetwork_Authority(void);
	void DoNetwork_Replica(void);
	Peer * FindPeer(sockaddr_in & sockaddr);
	void ProcessInputQueue(void);
	void ProcessSnapshotQueue(void);
	void ClearSnapshotQueue(void);
	void SendGameInfo(Uint8 peerid);
	void SendReady(void);
	bool AllPeersReady(Uint8 except);
	bool AllPeersLoadedGameInfo(void);
	char * CreateStatusString(const char * status, Uint8 color = 0, Uint8 duration = 100);
	void PushStatusString(char * statusstring);
	void RequestPeerList(void);
	void SendSnapshots(void);
	void SendPeerList(Uint8 peerid = 0);
	void ReadPeerList(Serializer & data);
	void SendPacket(Peer * peer, char * data, unsigned int size);
	void SwitchToMode(bool newmode);
	void DeleteOldSnapshots(Uint8 peerid);
	void HandleDisconnect(Uint8 peerid);
	bool RelevantToPlayer(class Player * player, Object * object);
	void ActivateTerminals(void);
	void LoadBuyableItems(void);
	void BuyItem(Uint8 id);
	void SendStats(Peer & peer);
	static bool CompareTeamByNumber(Team * team1, Team * team2);
	std::map<Uint16, class Object *> objectidlookup;
	std::list<Uint16> objectdestroylist;
	bool mode;
	SOCKET sockethandle;
	unsigned short boundport;
	unsigned int currentid;
	static const unsigned int maxobjects = 32000;
	static const unsigned int maxpeers = 25;
	static const unsigned int maxoldsnapshots = 36;
	static const unsigned int maxlocalinputhistory = maxoldsnapshots + 1;
	static const unsigned int peertimeout = 5000;
	static const unsigned int maxteams = 6;
	Peer * peerlist[maxpeers];
	unsigned int authoritypeer;
	unsigned int peercount;
	char version[16];
	Uint8 localpeerid;
	unsigned short localpublicport;
	enum {IDLE, LISTENING, CONNECTING, CONNECTED} state;
	enum {NONE, INLOBBY, INGAME} gameplaystate;
	enum {MSG_CONNECT, MSG_SNAPSHOT, MSG_INPUT, MSG_PEERLIST, MSG_DISCONNECT, MSG_PING, MSG_PONG, MSG_GAMEINFO, MSG_READY, MSG_CHAT, MSG_BUY, MSG_CHANGETEAM, MSG_STATUS, MSG_TECH, MSG_STATS};
	Serializer * oldsnapshots[maxpeers][maxoldsnapshots];
	ObjectTypes objecttypes;
	Input localinputhistory[maxlocalinputhistory];
	Uint32 localtoremoteticks[maxoldsnapshots];
	LagSimulator lagsimulator;
	std::list<Serializer *> snapshotqueue;
	static const Uint32 snapshotqueueminsize = 1;
	static const Uint32 snapshotqueuemaxsize = 5;
	Uint8 illuminate;
	bool systemcameraactive[2];
	Uint16 systemcamerafollow[2];
	Sint16 systemcamerax[2];
	Sint16 systemcameray[2];
	DedicatedServer dedicatedserver;
	LobbyGame gameinfo;
	Uint16 winningteamid;
	char message[256];
	Uint8 message_i;
	Uint8 messagetype;
	Uint8 messagetime;
	Uint32 lastpingsent;
	Uint32 lastpingid;
	int pingtime;
	bool highlightsecrets;
	bool highlightminimap;
	bool intutorialmode;
};

#endif