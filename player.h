#ifndef PLAYER_H
#define PLAYER_H

#include "shared.h"
#include "object.h"
#include "team.h"
#include "camera.h"
#include "input.h"
#include "peer.h"
#include "pickup.h"
#include "playerai.h"

class Player : public Object
{
public:
	Player();
	void Serialize(bool write, Serializer & data, Serializer * old = 0);
	void Tick(class World & world);
	void HandleHit(class World & world, Uint8 x, Uint8 y, Object & projectile);
	void HandleInput(Input & input);
	void HandleDisconnect(World & world, Uint8 peerid);
	void OnDestroy(World & world);
	bool InBase(World & world);
	bool InOwnBase(World & world);
	Team * TeamOfCurrentBase(World & world);
	bool IsDisguised(void);
	void UnDisguise(World & world);
	bool Poison(World & world, Uint16 playerid, Uint8 amount);
	void UnPoison(void);
	bool HasSecurityPass(void);
	Team * GetTeam(World & world);
	bool AddInventoryItem(Uint8 id, bool settocurrent = false);
	void RemoveInventoryItem(Uint8 id);
	Uint8 InventoryItemCount(Uint8 id);
	bool BuyItem(World & world, Uint8 id);
	bool RepairItem(World & world, Uint8 id);
	bool VirusItem(World & world, Uint8 id);
	void LoadAbilities(World & world);
	void KillByGovt(World & world);
	void AddCredits(int amount);
	void UnDeploy(void);
	bool CanExhaustInputQueue(World & world);
	Peer * GetPeer(World & world);
	enum {INV_NONE, INV_HEALTHPACK, INV_LAZARUSTRACT, INV_SECURITYPASS, INV_VIRUS,
		INV_POISON, INV_NEUTRONBOMB, INV_EMPBOMB, INV_SHAPEDBOMB, INV_PLASMABOMB, INV_PLASMADET,
		INV_FIXEDCANNON, INV_FLARE, INV_POISONFLARE, INV_BASEDOOR, INV_CAMERA};
	Uint16 maxhealth;
	Uint16 maxshield;
	Uint8 laserammo;
	Uint8 rocketammo;
	Uint8 flamerammo;
	bool fuellow;
	Uint8 fuel;
	Uint8 maxfuel;
	Uint8 currentweapon;
	Uint8 oldweapon;
	Uint16 files;
	Uint16 maxfiles;
	Uint16 credits;
	bool effecthacking;
	int effecthackingcontinue;
	int effectshieldcontinue;
	bool hassecret;
	bool oldhassecret;
	Uint16 secretteamid;
	Uint8 suitcolor;
	Uint16 chatinterfaceid;
	bool chatwithteam;
	Sint8 fallingnudge;
	Uint16 buyinterfaceid;
	Uint16 techinterfaceid;
	bool isbuying;
	bool techstationactive;
	Uint8 inventoryitems[4];
	Uint8 inventoryitemsnum[4];
	Uint8 currentinventoryitem;
	int buyifacelastitem;
	int buyifacelastscrolled;
	Uint8 disguised;
	class PlayerAI * ai;

	friend class Renderer;
	friend class World;
	friend class Terminal;
	friend class Grenade;
	friend class Warper;
	friend class PickUp;
	friend class Game;
	friend class PlayerAI;

private:
	bool CheckForBaseExit(World & world);
	bool CheckForLadder(World & world);
	bool CheckForGround(World & world, Platform & platform);
	bool HandleAgainstWall(World & world);
	bool CanCreateBase(World & world);
	void SetToRespawnPosition(World & world);
	void DropAllItems(World & world);
	PickUp * DropItem(World & world, Uint8 type, Uint16 quantity);
	bool BuyAvailable(World & world, Uint8 id);
	Projectile * Fire(World & world, Uint8 direction);
	bool FireDelayPassed(World & world);
	bool ProcessJetpackState(World & world);
	bool ProcessFallingState(World & world);
	bool ProcessStandingState(World & world);
	bool ProcessLadderState(World & world);
	bool OnGround(void);
	void Warp(World & world, Sint16 x, Sint16 y);
	bool PickUpItem(World & world, PickUp & pickup);
	enum {STANDING, RUNNING, WALKIN, WALKOUT, FALLING, LADDER, CROUCHING, UNCROUCHING, CROUCHED,
		CROUCHEDSHOOT, CROUCHEDTHROWING, ROLLING, JUMPING, CLIMBINGLEDGE, JETPACK, HACKING, STANDINGSHOOT,
		FALLINGSHOOT, LADDERSHOOT, JETPACKSHOOT, DYING, DEAD, RESPAWNING, THROWING, DEPLOYING, UNDEPLOYING,
		RESURRECTING};
	Uint8 state;
	Uint8 state_i;
	Uint16 basedoorentering;
	Input input;
	Input oldinput;
	Uint8 weaponfiredelay[4];
	Uint32 lastfire;
	int hacksoundchannel;
	int jetpacksoundchannel;
	int flamersoundchannel;
	Uint16 oldfiles;
	Uint32 hitsoundplaytick;
	Uint16 currentdetonator;
	Uint16 currentgrenade;
	Sint16 warpx;
	Sint16 warpy;
	int olddetlistsize;
	int oldgrenadelistsize;
	bool justjumpedfromladder;
	bool justlandedfromair;
	Uint16 currentprojectileid;
	int jumpimpulse;
	float hackingbonus;
	float creditsbonus;
	bool abilitiesloaded;
	bool canresurrect;
	Uint32 jetpackbonustime;
	Uint32 hackingbonustime;
	Uint32 radarbonustime;
	Uint8 tracetime;
	int secondcounter;
	Uint16 poisonedby;
	Uint8 poisonedamount;
	Uint32 lastweaponchangesound;
	Uint16 teamid;
};

#endif