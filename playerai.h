#ifndef PLAYERAI_H
#define PLAYERAI_H

#include "shared.h"
#include "player.h"
#include "terminal.h"
#include "baseexit.h"
#include "secretreturn.h"
#include "basedoor.h"

class PlayerAI
{
public:
	PlayerAI(Player & player);
	~PlayerAI();
	void Tick(World & world);
	bool CreatePathToPlatformSet(World & world, std::deque<PlatformSet *> & path, PlatformSet & target);
	bool FollowPath(World & world);
	bool SetTarget(World & world, Sint16 x, Sint16 y);
	void ClearTarget(void);
	void GetCurrentNode(int & x, int & y);
	Uint8 GetNodeType(World & world, unsigned int x, unsigned int y);
	Platform * FindClosestLadderToPlatform(World & world, PlatformSet & from, PlatformSet & to, Sint16 x = 0);
	bool FindLink(World & world, int type, PlatformSet & from, PlatformSet & to);
	bool FindAnyLink(World & world, PlatformSet & from, PlatformSet & to);
	std::vector<Terminal *> FindNearestTerminals(World & world);
	BaseExit * GetBaseExit(World & world);
	SecretReturn * GetSecretReturn(World & world);
	BaseDoor * GetBaseDoor(World & world);
	
private:
	void SetState(Uint8 state);
	static bool TerminalSort(Terminal * a, Terminal * b);
	enum {IDLE, HACK, EXITBASE, GETSECRET, GOTOBASE, RETURNSECRET};
	Uint8 state;
	Player & player;
	bool direction;
	Sint16 targetx, targety;
	PlatformSet * targetplatformset;
	bool ladderjumping;
	enum {LINK_NONE, LINK_LADDER, LINK_FALL, LINK_MAXENUM};
	int linktype;
	Platform * linkladder;
	std::deque<PlatformSet *> platformsetpath;
};

#endif