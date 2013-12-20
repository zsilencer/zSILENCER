#include "player.h"
#include "resources.h"
#include "objecttypes.h"
#include "plume.h"
#include "civilian.h"
#include "robot.h"
#include "terminal.h"
#include "vent.h"
#include "basedoor.h"
#include "projectile.h"
#include "blasterprojectile.h"
#include "laserprojectile.h"
#include "rocketprojectile.h"
#include "flamerprojectile.h"
#include "plasmaprojectile.h"
#include "healmachine.h"
#include "creditmachine.h"
#include "secretreturn.h"
#include "interface.h"
#include "textinput.h"
#include "selectbox.h"
#include "inventorystation.h"
#include "techstation.h"
#include "detonator.h"
#include "fixedcannon.h"
#include "warper.h"
#include "grenade.h"
#include "bodypart.h"
#include "walldefense.h"
#include <math.h>

Player::Player() : Object(ObjectTypes::PLAYER){
	hassecret = false;
	oldhassecret = false;
	secretteamid = 0;
	state = DEPLOYING;
	draw = false;
	state_i = 0;
	res_bank = 9;
	res_index = 0;
	requiresauthority = true;
	renderpass = 2;
	x = 0;
	y = 0;
	currentplatformid = 0;
	height = 50;
	maxhealth = 100;
	health = maxhealth;
	maxshield = 100;
	shield = maxshield;
	laserammo = 5;
	rocketammo = 0;
	flamerammo = 0;
	fuellow = false;
	maxfuel = 80;
	fuel = maxfuel;
	currentweapon = 0;
	oldweapon = 0;
	files = 0;
	maxfiles = 2400;
	credits = 0;
	effecthacking = false;
	suitcolor = (7 << 4) + 13;
	weaponfiredelay[0] = 6;
	weaponfiredelay[1] = 10;
	weaponfiredelay[2] = 20;
	weaponfiredelay[3] = 0;
	teamid = 0;
	hacksoundchannel = -1;
	jetpacksoundchannel = -1;
	flamersoundchannel = -1;
	lastfire = 0;
	chatinterfaceid = 0;
	buyinterfaceid = 0;
	techinterfaceid = 0;
	chatwithteam = false;
	fallingnudge = 0;
	oldfiles = 0;
	effecthackingcontinue = 0;
	effectshieldcontinue = 0;
	for(int i = 0; i < 4; i++){
		inventoryitems[i] = INV_NONE;
		inventoryitemsnum[i] = 0;
	}
	AddInventoryItem(INV_BASEDOOR);
	//for(int i = 0; i < 4; i++)
	//AddInventoryItem(INV_PLASMABOMB);
	isbuying = false;
	techstationactive = false;
	currentinventoryitem = 0;
	currentdetonator = 0;
	currentgrenade = 0;
	hitsoundplaytick = 0;
	warpx = 0;
	warpy = 0;
	olddetlistsize = 0;
	oldgrenadelistsize = 0;
	buyifacelastitem = 0;
	buyifacelastscrolled = 0;
	justjumpedfromladder = false;
	currentprojectileid = 0;
	ishittable = true;
	isbipedal = true;
	isphysical = true;
	iscontrollable = true;
	disguised = false;
	snapshotinterval = 24;
	jumpimpulse = 0;
	hackingbonus = 0;
	creditsbonus = 0;
	abilitiesloaded = false;
	canresurrect = true;
	jetpackbonustime = 0;
	hackingbonustime = 0;
	radarbonustime = 0;
	tracetime = 0;
	secondcounter = 0;
	poisonedby = 0;
	lastweaponchangesound = 0;
	ai = 0;
};

void Player::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	int oldoffset = data.offset;
	input.Serialize(write, data);
	//oldinput.Serialize(write, data);
	if(Serializer::WRITE == write && old){
		old->readoffset += data.offset - oldoffset;
	}
	data.Serialize(write, state, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, currentweapon, old);
	data.Serialize(write, suitcolor, old);
	data.Serialize(write, basedoorentering, old);
	data.Serialize(write, maxhealth, old);
	data.Serialize(write, maxshield, old);
	data.Serialize(write, laserammo, old);
	data.Serialize(write, rocketammo, old);
	data.Serialize(write, flamerammo, old);
	data.Serialize(write, fuellow, old);
	data.Serialize(write, fuel, old);
	data.Serialize(write, maxfuel, old);
	data.Serialize(write, files, old);
	data.Serialize(write, maxfiles, old);
	data.Serialize(write, credits, old);
	data.Serialize(write, effecthacking, old);
	data.Serialize(write, hassecret, old);
	data.Serialize(write, secretteamid, old);
	data.Serialize(write, fallingnudge, old);
	for(int i = 0; i < 4; i++){
		data.Serialize(write, inventoryitems[i], old);
		data.Serialize(write, inventoryitemsnum[i], old);
	}
	data.Serialize(write, isbuying, old);
	data.Serialize(write, techstationactive, old);
	data.Serialize(write, currentinventoryitem, old);
	data.Serialize(write, justjumpedfromladder, old);
	data.Serialize(write, disguised, old);
	data.Serialize(write, tracetime, old);
	data.Serialize(write, canresurrect, old);
}

void Player::Tick(World & world){
	if(!abilitiesloaded){
		LoadAbilities(world);
	}
	Hittable::Tick(*this, world);
	Bipedal::Tick(*this, world);
	if(ai){
		ai->Tick(world);
	}
	if(input.keychat && !chatinterfaceid && !buyinterfaceid && !techinterfaceid && this == world.GetPeerPlayer(world.localpeerid)){
		Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
		if(iface){
			TextInput * textinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
			if(textinput){
				textinput->x = 100;
				textinput->y = 100;
				textinput->res_bank = 133;
				textinput->fontwidth = 6;
				textinput->draw = false;
				textinput->uid = 1;
				textinput->maxchars = 100;
				textinput->maxwidth = 28;
				//strcpy(textinput->text, "sdfsdf");
				iface->AddObject(textinput->id);
				iface->activeobject = textinput->id;
				iface->ActiveChanged(world, iface, false);
			}
			chatinterfaceid = iface->id;
		}
	}
	if(state_warp){
		if(state_warp == 2){
			//draw = false;
		}
		if(state_warp == 12){
			//draw = true;
			x = warpx;
			y = warpy;
			currentplatformid = 0;
			if(!FindCurrentPlatform(*this, world)){
				state = FALLING;
				state_i = 0;
			}else
			if(state == DEAD){
				health = maxhealth;
				shield = maxshield;
				state = STANDING;
				state_i = 0;
			}
		}
	}
	if(!world.replaying){
		secondcounter++;
		if(secondcounter >= 24){
			secondcounter = 0;
		}
	}
	if(poisonedby && world.tickcount % 25 == 0){
		Player * player = static_cast<Player *>(world.GetObjectFromId(poisonedby));
		Object poisonprojectile(ObjectTypes::PLASMAPROJECTILE);
		poisonprojectile.healthdamage = 3;
		poisonprojectile.bypassshield = true;
		poisonprojectile.ownerid = player->id;
		HandleHit(world, 50, 50, poisonprojectile);
	}
	if(tracetime > 0 && secondcounter == 0 && !world.replaying){
		tracetime--;
		if(tracetime == 8){
			world.SendSound("vModDeto.wav");
		}
		if(tracetime == 0){
			hassecret = false;
			Team * team = GetTeam(world);
			if(team){
				team->playerwithsecret = false;
			}
			Peer * peer = GetPeer(world);
			if(peer){
				world.KillByGovt(*peer);
			}
		}
	}
	std::vector<Uint16> detlist;
	std::vector<Uint16> grenadelist;
	for(std::list<Object *>::reverse_iterator it = world.objectlist.rbegin(); it != world.objectlist.rend(); it++){
		Object * object = *it;
		if(object->type == ObjectTypes::DETONATOR){
			Detonator * detonator = static_cast<Detonator *>(object);
			if(detonator->ownerid == id){
				detlist.push_back(detonator->id);
			}
		}else
		if(object->type == ObjectTypes::GRENADE){
			Grenade * grenade = static_cast<Grenade *>(object);
			if(grenade->ownerid == id){
				grenadelist.push_back(grenade->id);
			}
		}
	}
	if((!currentdetonator || olddetlistsize < detlist.size()) && detlist.size() > 0){
		currentdetonator = detlist[0];
		olddetlistsize = detlist.size();
	}
	if((!currentgrenade || oldgrenadelistsize < grenadelist.size()) && grenadelist.size() > 0){
		currentgrenade = grenadelist[0];
		oldgrenadelistsize = grenadelist.size();
	}
	if(currentdetonator){
		Detonator * detonator = (Detonator *)world.GetObjectFromId(currentdetonator);
		if(detonator){
			if(this == world.GetPeerPlayer(world.localpeerid)){
				world.SetSystemCamera(1, 0, detonator->x, detonator->originaly - 35);
			}
		}else{
			currentdetonator = 0;
		}
	}
	if(currentgrenade){
		Grenade * grenade = (Grenade *)world.GetObjectFromId(currentgrenade);
		if(grenade){
			if(grenade->WasThrown()){
				if(this == world.GetPeerPlayer(world.localpeerid)){
					world.SetSystemCamera(0, grenade->id, 0, -30);
				}
			}else{
				grenade->UpdatePosition(world, *this);
			}
		}else{
			currentgrenade = 0;
		}
	}
	Uint8 inventoryitemscount = 0;
	for(int i = 0; i < 4; i++){
		if(inventoryitems[i] == INV_NONE){
			break;
		}
		inventoryitemscount++;
	}
	if(input.keynextinv && !oldinput.keynextinv){
		currentinventoryitem++;
		if(currentinventoryitem > inventoryitemscount - 1){
			currentinventoryitem = 0;
		}
	}
	if(input.keynextcam && !oldinput.keynextcam){
		if(currentdetonator){
			for(int i = 0; i < detlist.size(); i++){
				if(detlist[i] == currentdetonator){
					if(i < detlist.size() - 1){
						currentdetonator = detlist[i + 1];
						break;
					}
				}
			}
		}
	}
	if(input.keyprevcam && !oldinput.keyprevcam){
		if(currentdetonator){
			for(int i = 0; i < detlist.size(); i++){
				if(detlist[i] == currentdetonator){
					if(i > 0){
						currentdetonator = detlist[i - 1];
						break;
					}
				}
			}
		}
	}
	if(input.keymoveup && !oldinput.keymoveup){
		justjumpedfromladder = false;
	}
	if(input.keydisguise && !oldinput.keydisguise){
		if(disguised >= 100){
			UnDisguise(world);
		}else{
			if(state != JETPACK && state != DEPLOYING){
				disguised = 112;
				EmitSound(world, world.resources.soundbank["disguise.wav"], 64);
				if(OnGround()){
					state = RUNNING;
					state_i = 25;
				}
			}
		}
	}
	if(disguised){
		if(disguised > 100){
			disguised--;
		}
		if(disguised > 0 && disguised < 100){
			disguised--;
		}
	}
	if(state != HACKING || state_i >= 17){
		if(!world.replaying && hacksoundchannel != -1){
			EmitSound(world, world.resources.soundbank["jackout.wav"], 20);
			Audio::GetInstance().Stop(hacksoundchannel, 700);
			hacksoundchannel = -1;
		}
	}
	if(jetpacksoundchannel != -1){
		if(state != JETPACK && state != JETPACKSHOOT){
			if(!world.replaying){
				//GetTeam(*world)->secretdelivered = true;
				EmitSound(world, world.resources.soundbank["jetpak2a.wav"], 64);
				Audio::GetInstance().Stop(jetpacksoundchannel, 200);
				jetpacksoundchannel = -1;
			}
		}
	}
	if(flamersoundchannel != -1){
		if((state != STANDINGSHOOT && state != CROUCHEDSHOOT && state != LADDERSHOOT && state != FALLINGSHOOT && state != JETPACKSHOOT) || res_index != 4){
			if(!world.replaying){
				Audio::GetInstance().Stop(flamersoundchannel, 200);
				flamersoundchannel = -1;
			}
		}
	}
	if(!world.replaying){
		if(state != HACKING){
			effecthacking = false;
		}
		if(effecthackingcontinue > 0){
			effecthackingcontinue--;
			effecthacking = true;
		}else{
			effecthacking = false;
		}
		if(effectshieldcontinue > 0){
			effectshieldcontinue--;
		}
	}
	if(input.keymoveleft || input.keymoveright){
		if(isbuying){
			isbuying = false;
		}
		if(techstationactive){
			techstationactive = false;
		}
	}
	if(world.GetPeerPlayer(world.localpeerid) == this){
		if(isbuying && !buyinterfaceid){
			Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
			if(iface){
				SelectBox * selectbox = (SelectBox *)world.CreateObject(ObjectTypes::SELECTBOX);
				selectbox->draw = false;
				selectbox->uid = 1;
				int i = 0;
				for(std::vector<BuyableItem *>::iterator it = world.buyableitems.begin(); it != world.buyableitems.end(); it++, i++){
					if(BuyAvailable(world, (*it)->id)){
						selectbox->AddItem((*it)->name, (*it)->id);
					}
				}
				selectbox->selecteditem = buyifacelastitem;
				selectbox->scrolled = buyifacelastscrolled;
				iface->AddObject(selectbox->id);
				iface->activeobject = selectbox->id;
				Audio::GetInstance().Play(world.resources.soundbank["cliksel2.wav"], 96);
				buyinterfaceid = iface->id;
			}
		}else
		if(!isbuying && buyinterfaceid && !world.replaying){
			Interface * iface = (Interface *)world.GetObjectFromId(buyinterfaceid);
			if(iface){
				iface->DestroyInterface(world, iface);
			}
			buyinterfaceid = 0;
		}
		
		if(techstationactive && !techinterfaceid){
			Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
			if(iface){
				Team * team = GetTeam(world);
				SelectBox * selectbox = (SelectBox *)world.CreateObject(ObjectTypes::SELECTBOX);
				selectbox->draw = false;
				selectbox->uid = 1;
				int i = 0;
				for(std::vector<BuyableItem *>::iterator it = world.buyableitems.begin(); it != world.buyableitems.end(); it++, i++){
					BuyableItem * buyableitem = *it;
					if(InOwnBase(world)){
						if(team && buyableitem->techchoice & team->disabledtech){
							selectbox->AddItem((*it)->name, (*it)->id);
						}
					}else{
						if(team && buyableitem->techchoice & team->GetAvailableTech(world)){
							selectbox->AddItem((*it)->name, (*it)->id);
						}
					}
				}
				selectbox->selecteditem = 0;
				iface->AddObject(selectbox->id);
				iface->activeobject = selectbox->id;
				Audio::GetInstance().Play(world.resources.soundbank["cliksel2.wav"], 96);
				techinterfaceid = iface->id;
			}
		}else
		if(!techstationactive && techinterfaceid && !world.replaying){
			Interface * iface = (Interface *)world.GetObjectFromId(techinterfaceid);
			if(iface){
				iface->DestroyInterface(world, iface);
			}
			techinterfaceid = 0;
		}
	}
	if(fuel < maxfuel){
		if(!fuellow){
			fuel--;
		}else{
			fuel++;
		}
	}
	if(fuel == 0){
		if(jetpackbonustime > world.tickcount){
			fuel = maxfuel;
		}else{
			fuellow = true;
		}
	}
	if(fuel == maxfuel && fuellow){
		Player * localplayer = world.GetPeerPlayer(world.localpeerid);
		if(localplayer && localplayer->id == id){
			Audio::GetInstance().Play(world.resources.soundbank["charged.wav"], 96);
		}
		fuellow = false;
	}
	if((input.keydetonate && !oldinput.keydetonate) || ((state == DEAD || state == DYING || state == RESPAWNING) && state_i % 1 == 0)){
		Detonator * detonator = (Detonator *)world.GetObjectFromId(currentdetonator);
		if(detonator){
			int i = 0;
			while(detonator && detonator->HasDetonated() && i < detlist.size()){
				detonator = (Detonator *)world.GetObjectFromId(detlist[i]);
				i++;
			}
			if(detonator){
				currentdetonator = detonator->id;
				detonator->Detonate();
			}
		}
	}
	if(oldhassecret != hassecret && !world.replaying){
		Peer * peer = GetPeer(world);
		if(hassecret){
			Team * team = GetTeam(world);
			if(team && team->secrets == 2){
				world.SendSound("alwarn.wav");
			}else{
				world.SendSound("alinvest.wav");
			}
			if(peer && !world.intutorialmode){
				User * user = world.lobby.GetUserInfo(peer->accountid);
				char text[128];
				sprintf(text, "Secret picked up by\n%s", user->name);
				world.ShowMessage(text, 128, 0, true);
				peer->stats.secretspickedup++;
			}
		}else{
			if(peer){
				User * user = world.lobby.GetUserInfo(peer->accountid);
				if(state != DEAD && state != DYING){
					peer->stats.secretsreturned++;
					Team * team = GetTeam(world);
					bool stolen = false;
					if(team && secretteamid != team->id){
						peer->stats.secretsstolen++;
						stolen = true;
					}
				}else{
					char text[128];
					sprintf(text, "%s dropped a secret", user->name);
					if(!world.intutorialmode){
						world.ShowMessage(text, 128, 0, true);
					}
					peer->stats.secretsdropped++;
				}
				tracetime = 0;
			}
		}
		oldhassecret = hassecret;
	}
	switch(currentweapon){
		case 1:
			if(laserammo == 0){
				currentweapon = 0;
			}
		break;
		case 2:
			if(rocketammo == 0){
				currentweapon = 0;
			}
		break;
		case 3:
			if(flamerammo == 0){
				currentweapon = 0;
			}
		break;
	}
	if(currentweapon != oldweapon && !world.replaying){
		Player * localplayer = world.GetPeerPlayer(world.localpeerid);
		if(localplayer && localplayer->id == id && world.tickcount - lastweaponchangesound >= 4){
			switch(currentweapon){
				case 0:
					Audio::GetInstance().Play(world.resources.soundbank["ammo01.wav"]);
				break;
				case 1:
					Audio::GetInstance().Play(world.resources.soundbank["ammo02.wav"]);
				break;
				case 2:
					Audio::GetInstance().Play(world.resources.soundbank["ammo03.wav"]);
				break;
				case 3:
					Audio::GetInstance().Play(world.resources.soundbank["ammo05.wav"]);
				break;
			}
			lastweaponchangesound = world.tickcount;
		}
		oldweapon = currentweapon;
	}
	
	do{
		if(OnGround()){
			if(input.keyactivate && !oldinput.keyactivate){
				std::vector<Uint8> types;
				types.push_back(ObjectTypes::TERMINAL);
				types.push_back(ObjectTypes::BASEDOOR);
				types.push_back(ObjectTypes::VENT);
				types.push_back(ObjectTypes::INVENTORYSTATION);
				types.push_back(ObjectTypes::TECHSTATION);
				/*Uint16 x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
				Uint16 x2 = x - world.resources.spriteoffsetx[res_bank][res_index] + world.resources.spritewidth[res_bank][res_index];
				Uint16 y1 = y - world.resources.spriteoffsety[res_bank][res_index];
				Uint16 y2 = y - world.resources.spriteoffsety[res_bank][res_index] + world.resources.spriteheight[res_bank][res_index];*/
				std::vector<Object *> objects = world.TestAABB(x, y - height, x, y, types, 0, 0, false);
				for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
					switch((*it)->type){
						case ObjectTypes::TERMINAL:{
							Terminal * terminal = static_cast<Terminal *>(*it);
							if(terminal->state == Terminal::READY || terminal->state == Terminal::HACKERGONE){
								state = HACKING;
								state_i = 0;
								break;
							}
						}break;
						case ObjectTypes::BASEDOOR:{
							BaseDoor * basedoor = static_cast<BaseDoor *>(*it);
							Team * team = GetTeam(world);
							if(team && basedoor->discoveredby[team->number]){
								basedoorentering = basedoor->id;
								state = WALKIN;
								state_i = 0;
								break;
							}
						}break;
						case ObjectTypes::INVENTORYSTATION:{
							InventoryStation * inventorystation = static_cast<InventoryStation *>(*it);
							Team * team = GetTeam(world);
							if(team && inventorystation->teamid == team->id){
								if(world.IsAuthority()){
									isbuying = !isbuying;
								}
							}
						}break;
						case ObjectTypes::TECHSTATION:{
							if(world.IsAuthority()){
								techstationactive = !techstationactive;
							}
						}break;
						case ObjectTypes::VENT:{
							Vent * vent = static_cast<Vent *>(*it);
							vent->Activate();
						}break;
					}
				}
			}
		}
		if(input.keyuse && !oldinput.keyuse){
			switch(inventoryitems[currentinventoryitem]){
				case INV_HEALTHPACK:{
					if(health < maxhealth){
						if(maxhealth - health < 100){
							health = maxhealth;
						}else{
							health += 100;
						}
						Peer * peer = GetPeer(world);
						if(peer){
							peer->stats.healthpacksused++;
						}
						RemoveInventoryItem(INV_HEALTHPACK);
					}
				}break;
				case INV_LAZARUSTRACT:{
					std::vector<Uint8> types;
					types.push_back(ObjectTypes::CIVILIAN);
					int x1, y1, x2, y2;
					GetAABB(world.resources, &x1, &y1, &x2, &y2);
					std::vector<Object *> objects = world.TestAABB(x1, y1, x2, y2, types);
					for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
						Civilian * civilian = static_cast<Civilian *>(*it);
						if(civilian->AddTract(teamid)){
							Peer * peer = GetPeer(world);
							if(peer){
								peer->stats.tractsplanted++;
							}
							RemoveInventoryItem(INV_LAZARUSTRACT);
							break;
						}
					}
				}break;
				case INV_VIRUS:{
					std::vector<Uint8> types;
					types.push_back(ObjectTypes::ROBOT);
					types.push_back(ObjectTypes::FIXEDCANNON);
					int x1, y1, x2, y2;
					GetAABB(world.resources, &x1, &y1, &x2, &y2);
					std::vector<Object *> objects = world.TestAABB(x1, y1, x2, y2, types, id);
					for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
						switch((*it)->type){
							case ObjectTypes::ROBOT:{
								Robot * robot = static_cast<Robot *>(*it);
								Team * team = GetTeam(world);
								if(team && robot->ImplantVirus(team->id)){
									Peer * peer = GetPeer(world);
									if(peer){
										peer->stats.virusesused++;
									}
									RemoveInventoryItem(INV_VIRUS);
									break;
								}
							}break;
							case ObjectTypes::FIXEDCANNON:{
								FixedCannon * fixedcannon = static_cast<FixedCannon *>(*it);
								if(fixedcannon->ImplantVirus(world, id)){
									Peer * peer = GetPeer(world);
									if(peer){
										peer->stats.virusesused++;
									}
									RemoveInventoryItem(INV_VIRUS);
									break;
								}
							}break;
						}
					}
				}break;
				case INV_POISON:{
					std::vector<Uint8> types;
					types.push_back(ObjectTypes::PLAYER);
					int x1, y1, x2, y2;
					GetAABB(world.resources, &x1, &y1, &x2, &y2);
					std::vector<Object *> objects = world.TestAABB(x1, y1, x2, y2, types, id);
					for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
						Player * player = static_cast<Player *>(*it);
						if(player->Poison(id)){
							Peer * peer = GetPeer(world);
							if(peer){
								peer->stats.poisons++;
							}
							RemoveInventoryItem(INV_POISON);
							break;
						}
					}
				}break;
				case INV_EMPBOMB:{
					if(state == STANDING || state == THROWING){
						state = THROWING;
						state_i = 3;
					}
					if(state == CROUCHED || state == CROUCHEDTHROWING){
						state = CROUCHEDTHROWING;
						state_i = 3;
					}
					Grenade * grenade = (Grenade *)world.CreateObject(ObjectTypes::GRENADE);
					if(grenade){
						grenade->ownerid = id;
						grenade->SetType(Grenade::EMP);
						if(grenade->UpdatePosition(world, *this)){
							RemoveInventoryItem(INV_EMPBOMB);
							Peer * peer = GetPeer(world);
							if(peer){
								peer->stats.grenadesthrown++;
								peer->stats.empsthrown++;
							}
						}else{
							world.DestroyObject(grenade->id);
						}
					}
				}break;
				case INV_SHAPEDBOMB:{
					if(state == STANDING || state == THROWING){
						state = THROWING;
						state_i = 3;
					}
					if(state == CROUCHED || state == CROUCHEDTHROWING){
						state = CROUCHEDTHROWING;
						state_i = 3;
					}
					Grenade * grenade = (Grenade *)world.CreateObject(ObjectTypes::GRENADE);
					if(grenade){
						grenade->ownerid = id;
						grenade->SetType(Grenade::SHAPED);
						if(grenade->UpdatePosition(world, *this)){
							RemoveInventoryItem(INV_SHAPEDBOMB);
							Peer * peer = GetPeer(world);
							if(peer){
								peer->stats.grenadesthrown++;
								peer->stats.shapedthrown++;
							}
						}else{
							world.DestroyObject(grenade->id);
						}
					}
				}break;
				case INV_PLASMABOMB:{
					if(state == STANDING || state == THROWING){
						state = THROWING;
						state_i = 3;
					}
					if(state == CROUCHED || state == CROUCHEDTHROWING){
						state = CROUCHEDTHROWING;
						state_i = 3;
					}
					Grenade * grenade = (Grenade *)world.CreateObject(ObjectTypes::GRENADE);
					if(grenade){
						grenade->ownerid = id;
						grenade->SetType(Grenade::PLASMA);
						if(grenade->UpdatePosition(world, *this)){
							RemoveInventoryItem(INV_PLASMABOMB);
							Peer * peer = GetPeer(world);
							if(peer){
								peer->stats.grenadesthrown++;
								peer->stats.plasmasthrown++;
							}
						}else{
							world.DestroyObject(grenade->id);
						}
					}
				}break;
				case INV_NEUTRONBOMB:{
					if(!InBase(world)){
						if(state == STANDING || state == THROWING){
							state = THROWING;
							state_i = 3;
						}
						if(state == CROUCHED || state == CROUCHEDTHROWING){
							state = CROUCHEDTHROWING;
							state_i = 3;
						}
						Grenade * grenade = (Grenade *)world.CreateObject(ObjectTypes::GRENADE);
						if(grenade){
							grenade->ownerid = id;
							grenade->SetType(Grenade::NEUTRON);
							if(grenade->UpdatePosition(world, *this)){
								RemoveInventoryItem(INV_NEUTRONBOMB);
								Peer * peer = GetPeer(world);
								if(peer){
									peer->stats.grenadesthrown++;
									peer->stats.neutronsthrown++;
								}
							}else{
								world.DestroyObject(grenade->id);
							}
						}
					}
				}break;
				case INV_CAMERA:
				case INV_PLASMADET:{
					if(OnGround()){
						Detonator * detonator = (Detonator *)world.CreateObject(ObjectTypes::DETONATOR);
						if(detonator){
							detonator->x = x;
							detonator->y = y;
							detonator->originaly = detonator->y;
							detonator->ownerid = id;
							Peer * peer = GetPeer(world);
							if(inventoryitems[currentinventoryitem] == INV_CAMERA){
								detonator->iscamera = true;
								RemoveInventoryItem(INV_CAMERA);
								if(peer){
									peer->stats.camerasplanted++;
								}
							}else{
								RemoveInventoryItem(INV_PLASMADET);
								if(peer){
									peer->stats.detsplanted++;
								}
							}
						}
					}
				}break;
				case INV_FIXEDCANNON:{
					if(OnGround()){
						FixedCannon * fixedcannon = (FixedCannon *)world.CreateObject(ObjectTypes::FIXEDCANNON);
						if(fixedcannon){
							fixedcannon->x = x;
							fixedcannon->y = y;
							fixedcannon->SetOwner(world, id);
							fixedcannon->mirrored = mirrored;
							RemoveInventoryItem(INV_FIXEDCANNON);
							Peer * peer = GetPeer(world);
							if(peer){
								peer->stats.fixedcannonsplaced++;
							}
						}
					}
				}break;
				case INV_FLARE:{
					if(state == STANDING || state == THROWING){
						state = THROWING;
						state_i = 3;
					}
					if(state == CROUCHED || state == CROUCHEDTHROWING){
						state = CROUCHEDTHROWING;
						state_i = 3;
					}
					Grenade * grenade = (Grenade *)world.CreateObject(ObjectTypes::GRENADE);
					if(grenade){
						grenade->ownerid = id;
						grenade->SetType(Grenade::FLARE);
						if(grenade->UpdatePosition(world, *this)){
							RemoveInventoryItem(INV_FLARE);
							Peer * peer = GetPeer(world);
							if(peer){
								peer->stats.grenadesthrown++;
								peer->stats.flaresthrown++;
							}
						}else{
							world.DestroyObject(grenade->id);
						}
					}
				}break;
				case INV_POISONFLARE:{
					if(state == STANDING || state == THROWING){
						state = THROWING;
						state_i = 3;
					}
					if(state == CROUCHED || state == CROUCHEDTHROWING){
						state = CROUCHEDTHROWING;
						state_i = 3;
					}
					Grenade * grenade = (Grenade *)world.CreateObject(ObjectTypes::GRENADE);
					if(grenade){
						grenade->ownerid = id;
						grenade->SetType(Grenade::POISONFLARE);
						if(grenade->UpdatePosition(world, *this)){
							RemoveInventoryItem(INV_POISONFLARE);
							Peer * peer = GetPeer(world);
							if(peer){
								peer->stats.grenadesthrown++;
								peer->stats.poisonflaresthrown++;
							}
						}else{
							world.DestroyObject(grenade->id);
						}
					}
				}break;
				case INV_BASEDOOR:{
					if(OnGround() && !world.replaying){
						Team * team = GetTeam(world);
						if(!team->basedoorid){
							if(CanCreateBase(world)){
								BaseDoor * basedoor = (BaseDoor *)world.CreateObject(ObjectTypes::BASEDOOR);
								if(basedoor){
									basedoor->x = x;
									basedoor->y = y;
									basedoor->SetTeam(*team);
									basedoor->discoveredby[team->number] = true;
									team->basedoorid = basedoor->id;
									for(int i = 0; i < team->numpeers; i++){
										Player * player = world.GetPeerPlayer(team->peers[i]);
										if(player){
											player->RemoveInventoryItem(INV_BASEDOOR);
										}
									}
								}
							}else{
								//if(this == world.GetPeerPlayer(world.localpeerid)){
									world.ShowStatus("Can't build a base here", 208, true, GetPeer(world));
								//}
							}
						}else{
							bool teamhassecret = false;
							for(int i = 0; i < team->numpeers; i++){
								Player * player = world.GetPeerPlayer(team->peers[i]);
								if(player){
									if(player->hassecret){
										teamhassecret = true;
									}
								}
							}
							if(!team->beamingterminalid && !teamhassecret){
								if(CanCreateBase(world)){
									BaseDoor * basedoor = (BaseDoor *)world.GetObjectFromId(team->basedoorid);
									if(basedoor){
										basedoor->x = x;
										basedoor->y = y;
										basedoor->Respawn();
										RemoveInventoryItem(INV_BASEDOOR);
									}
								}else{
									//if(this == world.GetPeerPlayer(world.localpeerid)){
										world.ShowStatus("Can't build a base here", 208, true, GetPeer(world));
									//}
								}
							}else{
								//if(this == world.GetPeerPlayer(world.localpeerid)){
									world.ShowStatus("Cannot acquire dimensional grid", 208, true, GetPeer(world));
								//}
							}
						}
					}
				}break;
			}
		}
		if(input.keyfire){
			if(state == STANDING || state == RUNNING || state == THROWING){
				state = STANDINGSHOOT;
				state_i = 0;
				break;
			}else
			if(state == CROUCHED){
				state = CROUCHEDSHOOT;
				state_i = 0;
				break;
			}
		}
	}while(0);
	
	switch(state){
		/*case STANDSTART:{
			xv = 0;
			yv = 0;
			res_bank = 10;
			res_index = state_i;
			if(state_i >= 1){
				state = STANDING;
				state_i = -1;
				break;
			}
		}break;*/
		case DEPLOYING:{
			Uint8 deploywait = 60;
			if(state_i >= deploywait){
				//if(state_i == deploywait){
				//	Audio::GetInstance().EmitSound(id, world.resources.soundbank["transrev.wav"], 96);
				//}
				draw = true;
				res_bank = 68;
				res_index = (state_i - deploywait);
				if(state_i >= deploywait + 8){
					state = STANDING;
					state_i = -1;
					break;
				}
			}else{
				draw = false;
			}
			// 68:0-8 is player beaming in
		}break;
		case UNDEPLOYING:{
			collidable = false;
			if(state_i == 0 && !world.winningteamid){
				EmitSound(world, world.resources.soundbank["transrev.wav"], 96);
			}
			res_bank = 68;
			res_index = (8 - state_i);
			if(state_i > 8){
				draw = false;
				if(!GetPeer(world)){
					world.MarkDestroyObject(id);
				}
			}
		}break;
		case STANDING:{
			xv = 0;
			yv = 0;
			if(state_i == 12 && world.map.TestAABB(x, y, x, y, Platform::OUTSIDEROOM)){
				EmitSound(world, world.resources.soundbank["breath2.wav"], 16);
				Plume * plume = static_cast<Plume *>(world.CreateObject(ObjectTypes::PLUME));
				if(plume){
					plume->type = 8;
					plume->SetPosition(x + (8 * (mirrored ? -1 : 1)), y - 56);
					plume->yv = 2;
					plume->xv = 3 * (mirrored ? -1 : 1);
				}
			}
			if(IsDisguised()){
				if(input.keymovedown){
					// 121:0-9 civilian standing
					if(state_i > 9 * 4){
						state_i = 0;
					}
					res_bank = 121;
					res_index = state_i / 4;
				}else{
					state = RUNNING;
					state_i = -1;
					break;
				}
			}else{
				if(state_i / 4 >= 12){
					state_i = 4;
				}
				if(state_i <= 1){
					res_bank = 10;
					res_index = state_i;
				}else{
					res_bank = 9;
					res_index = state_i / 4;
				}
			}
			if(ProcessStandingState(world)){
				break;
			}
		}break;
		case STANDINGSHOOT:{
			UnDisguise(world);
			Uint8 direction;
			Uint8 delay = weaponfiredelay[currentweapon];
			res_bank = 21;
			if(state_i >= 8 + delay){
				state_i = 4;
			}
			if(xv >= 4){
				xv -= 4;
			}else
			if(xv <= -4){
				xv += 4;
			}else{
				xv = 0;
			}
			if(1/*state_i <= 4 || state_i >= 10*/){
				if(input.keymoveright || !mirrored){
					direction = 2;
					res_bank = 21;
					mirrored = false;
				}
				if(input.keymoveleft || mirrored){
					direction = 6;
					res_bank = 21;
					mirrored = true;
				}
				if(input.keymoveup){
					direction = 0;
					res_bank = 22;
				}
				if(input.keymovedown){
					direction = 4;
					res_bank = 23;
				}
				if(input.keylookupright){
					direction = 1;
					res_bank = 24;
					mirrored = false;
				}
				if(input.keylookdownright){
					direction = 3;
					res_bank = 25;
					mirrored = false;
				}
				if(input.keylookupleft){
					direction = 7;
					res_bank = 24;
					mirrored = true;
				}
				if(input.keylookdownleft){
					direction = 5;
					res_bank = 25;
					mirrored = true;
				}
			}
			if(input.keymovedown && !(input.keylookdownleft || input.keylookdownright)){
				state = CROUCHING;
				state_i = -1;
				break;
			}
			//if(state_i > 4 + delay){
			//	res_index = state_i - delay;
			//}
			if(state_i < 4){
				res_index = state_i;
			}else
			if(state_i == 9){
				res_index = 5;
			}else
			if(state_i == 10){
				res_index = 6;
			}else
			if(state_i == 11){
				res_index = 7;
			}else
			if(state_i == 12){
				res_index = 8;
			}else{
				res_index = 4;
			}
			if(state_i == 4){
				Fire(world, direction);
			}
			if(!input.keyfire){
				if(state_i >= 4){
					state_i = 4;
				}
				if(state_i <= 0){
					state = STANDING;
					state_i = -1;
					break;
				}
				if(state_i >= 1){
					state_i -= 2;
				}
			}
			if(!FollowGround(*this, world, xv)){
				x += (xv - (x - oldx)) + (xv > 0 ? 1 : -1);
				state = FALLING;
				fallingnudge = 0;
				state_i = 4 * 2;
				break;
			}
		}break;
		case RUNNING:{
			// 122:0-19 civilian walking
			// 123:0-14 civilian running
			yv = 0;
			int yt = input.keymovedown ? 1 : -1;
			int center;
			Platform * ladder = 0;
			if(input.keymoveup || input.keymovedown){
				ladder = world.map.TestAABB(x - 20, y + yt, x + 20, y + yt, Platform::LADDER);
				if(ladder){
					center = ((ladder->x2 - ladder->x1) / 2) + ladder->x1;
					if(abs(signed(center) - x) <= abs(ceil(float(xv) / 2))){
						x = center;
						state = LADDER;
						state_i = -1;
						break;
					}
				}
			}
			if(input.keymoveleft || (ladder && x > center)){
				xv -= 3;
				if(xv > 0){
					xv = 0;
				}
				if(IsDisguised()){
					if(state_i >= 25){
						state_i = 12;
					}
				}
				mirrored = true;
			}else
			if(input.keymoveright || (ladder && x < center)){
				xv += 3;
				if(xv < 0){
					xv = 0;
				}
				if(IsDisguised()){
					if(state_i >= 25){
						state_i = 12;
					}
				}
				mirrored = false;
			}else{
				xv *= 0.5;
				if(IsDisguised()){
					if(state_i < 25){
						state_i = 25;
					}
					xv = 4 * (mirrored ? -1 : 1);
				}else{
					if(state_i < 21){
						state_i = 21;
					}
				}
			}
			int xvmax = 14;
			if(hassecret){
				xvmax = 11;
			}
			if(IsDisguised()){
				xvmax = 11;
			}
			if(xv > xvmax){
				xv = xvmax;
			}
			if(xv < -xvmax){
				xv = -xvmax;
			}
			Sint16 oldx = x;
			if(!FollowGround(*this, world, xv)){
				if(IsDisguised() && state_i >= 25){
					//mirrored = !mirrored;
				}else{
					x += (xv - (x - oldx)) + (xv > 0 ? 1 : -1);
					state = FALLING;
					fallingnudge = 0;
					state_i = 4 * 2;
					break;
				}
			}
			if(IsDisguised() && state_i >= 25 && DistanceToEnd(*this, world) <= world.minwalldistance){
				mirrored = !mirrored;
			}
			if(state_i >= 0 && state_i <= 5){
				if(IsDisguised()){
					state_i = 6;
				}else{
					if(state_i <= 1 && abs(xv) >= 4){
						state_i = 1;
					}
					res_bank = 66;
					res_index = state_i;
					if(res_index == 3){
						EmitSound(world, world.resources.soundbank["futstonl.wav"], 24);
					}
				}
			}
			if(state_i > 5 && state_i <= 20){
				res_bank = 11;
				res_index = state_i - 6;
				if(state_i == 20){
					state_i = 6 - 1;
				}
				if(IsDisguised()){
					res_bank = 123;
				}
				if(res_index == 4){
					EmitSound(world, world.resources.soundbank["futstonl.wav"], 24);
				}
				if(res_index == 11){
					EmitSound(world, world.resources.soundbank["futstonr.wav"], 24);
				}
			}
			if(state_i >= 21 && state_i < 25){
				res_bank = 67;
				res_index = state_i - 21;
				if(res_index == 3){
					EmitSound(world, world.resources.soundbank["futstonl.wav"], 24);
				}
				if(input.keymoveleft || input.keymoveright){
					state_i = -1;
					break;
				}
				if(state_i >= 24){
					state = STANDING;
					state_i = -1;
					break;
				}
			}
			if(state_i >= 25){
				if(!IsDisguised()){
					state_i = 5;
				}
				res_bank = 122;
				res_index = state_i - 25;
				if(state_i >= 25 + 19){
					state_i = 25 - 1;
				}
				if(res_index == 5){
					EmitSound(world, world.resources.soundbank["stostep1.wav"], 16);
				}
				if(res_index == 15){
					EmitSound(world, world.resources.soundbank["stostepr.wav"], 16);
				}
			}
			//printf("bank: %d  index: %d\n", res_bank, res_index);
			if(input.keymovedown && !ladder){
				if(IsDisguised()){
					state = STANDING;
					state_i = -1;
					break;
				}else{
					state = CROUCHING;
					state_i = -1;
					break;
				}
			}
		}break;
		case WALKIN:{
			//UnDisguise(world);
			BaseDoor * basedoor = (BaseDoor *)world.GetObjectFromId(basedoorentering);
			if(basedoor){
				if(state_i == 0){
					Team * team = static_cast<Team *>(world.GetObjectFromId(basedoor->teamid));
					Team * localteam = GetTeam(world);
					if(team && localteam && basedoor->teamid != localteam->id){
						bool discovered = false;
						if(!basedoor->enteredby[team->number]){
							basedoor->enteredby[team->number] = true;
							discovered = true;
						}
						for(int i = 0; i < 4; i++){
							Peer * peer = world.peerlist[team->peers[i]];
							if(peer){
								world.SendSound("alarm3a.wav", peer);
								if(discovered){
									world.SendSound("intrude.wav", peer);
								}
							}
						}
					}
					/*Player * localplayer = world.GetPeerPlayer(world.localpeerid);
					if(localplayer && id != localplayer->id && basedoor->teamid != teamid){
						basealarmplaytick = world.tickcount;
						Team * team = localplayer->GetTeam(world);
						if(team && !basedoor->enteredby[team->number]){
							intrudealarmplaytick = world.tickcount + 24;
							basedoor->enteredby[team->number] = true;
						}
					}*/
				}
				basedoor->CheckForPlayersInView(world);
			}
			collidable = false;
			xv = 0;
			res_bank = 19;
			res_index = state_i;
			if(state_i == 1){
				EmitSound(world, world.resources.soundbank["portpas2.wav"], 32);
			}
			if(state_i >= 15){
				collidable = true;
				if(basedoor){
					y = ((basedoor->teamnumber) * 26 * 64) + 640 + (world.map.height * 64) + 718;
					x = 510;
					currentplatformid = 0;
					FindCurrentPlatform(*this, world);
					/*Sint32 xv2 = 0;
					Sint32 yv2 = 1;
					Platform * platform = world.map.TestIncr(x, y - height, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
					if(platform){
						currentplatformid = platform->id;
						y = platform->XtoY(x);
					}*/
				}
				mirrored = false;
				res_bank = 9;
				res_index = 0;
				state = STANDING;
				state_i = 1;
				break;
			}
		}break;
		case WALKOUT:{
			//UnDisguise(world);
			BaseDoor * basedoor = (BaseDoor *)world.GetObjectFromId(basedoorentering);
			if(basedoor){
				basedoor->CheckForPlayersInView(world);
			}
			collidable = false;
			xv = 0;
			if(state_i == 0){
				EmitSound(world, world.resources.soundbank["portpas2.wav"], 32);
			}
			res_bank = 19;
			res_index = 15 - state_i;
			if(state_i >= 15){
				collidable = true;
				currentplatformid = 0;
				FindCurrentPlatform(*this, world);
				state = STANDING;
				state_i = -1;
				break;
			}
		}break;
		case CROUCHING:{
			//xv = 0;
			if(xv >= 3){
				xv -= 3;
			}else
			if(xv <= -3){
				xv += 3;
			}else{
				xv = 0;
			}
			if(!FollowGround(*this, world, xv)){
				x += (xv - (x - oldx)) + (xv > 0 ? 1 : -1);
				state = FALLING;
				fallingnudge = 0;
				state_i = 4 * 2;
				break;
			}
			res_bank = 17;
			res_index = state_i;
			if(state_i >= 4){
				if(input.keymoveleft || input.keymoveright){
					if(input.keymoveleft){
						mirrored = true;
					}else{
						mirrored = false;
					}
					state = ROLLING;
					state_i = -1;
					break;
				}
				state = CROUCHED;
				state_i = -1;
				break;
			}
		}break;
		case UNCROUCHING:{
			xv = 0;
			res_bank = 17;
			res_index = 4 - state_i;
			if(state_i >= 4){
				if(input.keymoveright || input.keymoveleft){
					state = RUNNING;
					state_i = -1;
					break;
				}
				state = STANDING;
				state_i = -1;
				break;
			}
		}break;
		case CROUCHED:{
			if(state_i == 12 && world.map.TestAABB(x, y, x, y, Platform::OUTSIDEROOM)){
				EmitSound(world, world.resources.soundbank["breath2.wav"], 16);
				Plume * plume = static_cast<Plume *>(world.CreateObject(ObjectTypes::PLUME));
				if(plume){
					plume->type = 8;
					plume->SetPosition(x + (16 * (mirrored ? -1 : 1)), y - 28);
					plume->yv = 2;
					plume->xv = 3 * (mirrored ? -1 : 1);
				}
			}
			xv = 0;
			res_bank = 18;
			res_index = state_i / 4;
			if(!input.keymovedown){
				state = UNCROUCHING;
				state_i = -1;
				break;
			}
			if(input.keymoveright || input.keymoveleft){
				if(input.keymoveleft){
					mirrored = true;
				}else{
					mirrored = false;
				}
				state = ROLLING;
				state_i = -1;
				break;
			}
			if(state_i / 4 >= 11){
				state_i = 4 - 1;
				break;
			}
		}break;
		case CROUCHEDSHOOT:{
			UnDisguise(world);
			res_bank = 36;
			Uint8 direction;
			Uint8 delay = weaponfiredelay[currentweapon];
			if(state_i >= 8 + delay){
				state_i = 4;
			}
			if(1/*state_i <= 4 || state_i >= 10*/){
				if(input.keymoveright || !mirrored){
					direction = 8;
					mirrored = false;
				}
				if(input.keymoveleft || mirrored){
					direction = 9;
					mirrored = true;
				}
			}
			//if(state_i > 4 + delay){
			//	res_index = state_i - delay;
			//}
			if(state_i < 4){
				res_index = state_i;
			}else
			if(state_i == 9){
				res_index = 5;
			}else
			if(state_i == 10){
				res_index = 6;
			}else
			if(state_i == 11){
				res_index = 7;
			}else
			if(state_i == 12){
				res_index = 8;
			}else{
				res_index = 4;
			}
			if(state_i == 4){
				Fire(world, direction);
			}
			if(!input.keyfire){
				if(state_i >= 4){
					state_i = 4;
				}
				if(state_i <= 0){
					state = CROUCHED;
					state_i = -1;
					break;
				}
				if(state_i >= 1){
					state_i -= 2;
				}
			}
			if(!input.keymovedown){
				state = UNCROUCHING;
				state_i = -1;
				break;
			}
		}break;
		case CROUCHEDTHROWING:{
			xv = 0;
			res_bank = 114;
			res_index = state_i;
			if(!input.keymovedown){
				state = UNCROUCHING;
				state_i = -1;
				break;
			}
			if(input.keymoveright || input.keymoveleft){
				if(input.keymoveleft){
					mirrored = true;
				}else{
					mirrored = false;
				}
				state = ROLLING;
				state_i = -1;
				break;
			}
			if(state_i >= 16){
				state_i = -1;
				state = CROUCHED;
				break;
			}
		}break;
		case ROLLING:{
			xv = mirrored ? -12 : 12;
			res_bank = 88;
			res_index = state_i;
			if(res_index == 0){
				EmitSound(world, world.resources.soundbank["roll2.wav"], 32);
			}
			if(state_i >= 8){
				if(state_i >= 10){
					if(input.keymoveleft){
						mirrored = true;
					}
					if(input.keymoveright){
						mirrored = false;
					}
				}
				xv = 0;
				res_bank = 18;
				res_index = (state_i - 8) / 2;
				if((input.keymoveright || input.keymoveleft) && !input.keymovedown){
					state = RUNNING;
					state_i = -1;
					break;
				}
				if(!input.keymovedown){
					state = UNCROUCHING;
					state_i = -1;
					break;
				}
			}
			if(state_i >= 12){
				state = CROUCHED;
				state_i = state_i - 6;
			}
			Sint16 oldx = x;
			if(!FollowGround(*this, world, xv)){
				x += (xv - (x - oldx)) + (xv > 0 ? 1 : -1);
				state = FALLING;
				fallingnudge = 0;
				state_i = 4 * 2;
				break;
			}
		}break;
		case JUMPING:{
			//GetTeam(*world)->secretdelivered = true;
			/*if(xv < 0){
				fallingnudge = -8;
			}
			if(xv > 0){
				fallingnudge = 8;
			}*/
			//xv *= 1.2;
			Uint32 impulse = -17 + jumpimpulse;
			if(justjumpedfromladder && ((!input.keymoveleft && !input.keymoveright) || (input.keymoveleft && input.keymoveright))){
				impulse = -29 + jumpimpulse;
			}
			
			if(HandleAgainstWall(world)){
				break;
			}
			
			// Find velocity that will not hit ceiling
			int xv2 = xv;
			int yv2 = impulse;
			int test = impulse;
			while(test <= 0){
				yv2 += test;
				test += world.gravity;
			}
			world.map.TestIncr(x, y - height - 20, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true);
			int yv3;
			int v = impulse;
			while(v <= 0){
				test = v;
				yv3 = v;
				while(test < 0){
					yv3 += test;
					test += world.gravity;
				}
				if(abs(yv3) <= abs(yv2)){
					break;
				}
				v++;
			}
			////
			
			EmitSound(world, world.resources.soundbank["juunewne.wav"], 96);
			yv = v;
			xv2 = xv;
			yv2 = yv;
			world.map.TestIncr(x, y - height, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, world.map.platformids[currentplatformid], true);
			x = x + xv2;
			y = y + yv2;
			if(!IsDisguised()){
				res_bank = 12;
				res_index = state_i;
			}
			if(state_i >= 0){
				state = FALLING;
				fallingnudge = 0;
				state_i = -1;
				break;
			}
		}break;
		case FALLING:{
			if(ProcessFallingState(world)){
				break;
			}
			if(input.keyfire){
				state = FALLINGSHOOT;
				state_i = -1;
				break;
			}
		}break;
		case JETPACKSHOOT:
		case FALLINGSHOOT:{
			UnDisguise(world);
			// 31:0-8 inair shoot right
			// 32:0-8 inair shoot up
			// 33:0-8 inair shoot down
			// 34:0-8 inair shoot up angle
			// 35:0-8 inair shoot down angle
			Uint8 direction = 200;
			Uint8 delay = weaponfiredelay[currentweapon];
			if(state_i >= 8 + delay){
				state_i = 4;
			}
			res_bank = 31;
			if(input.keyfire && 1/*(state_i <= 4 || state_i >= 10)*/){
				if(input.keymoveright || !mirrored){
					direction = 22;
					res_bank = 31;
					mirrored = false;
				}
				if(input.keymoveleft || mirrored){
					direction = 26;
					res_bank = 31;
					mirrored = true;
				}
				if(input.keymoveup){
					direction = 20;
					res_bank = 32;
				}
				if(input.keymovedown){
					direction = 24;
					res_bank = 33;
				}
				if(input.keylookupright){
					direction = 21;
					res_bank = 34;
					mirrored = false;
				}
				if(input.keylookdownright){
					direction = 23;
					res_bank = 35;
					mirrored = false;
				}
				if(input.keylookupleft){
					direction = 27;
					res_bank = 34;
					mirrored = true;
				}
				if(input.keylookdownleft){
					direction = 25;
					res_bank = 35;
					mirrored = true;
				}
			}
			//if(state_i > 4 + delay){
			//	res_index = state_i - delay;
			//}
			if(state_i < 4){
				res_index = state_i;
			}else
			if(state_i == 9){
				res_index = 5;
			}else
			if(state_i == 10){
				res_index = 6;
			}else
			if(state_i == 11){
				res_index = 7;
			}else
			if(state_i == 12){
				res_index = 8;
			}else{
				res_index = 4;
			}
			if(state_i == 4){
				Fire(world, direction);
			}
			if(!input.keyfire){
				if(state_i >= 4){
					state_i = 4;
				}
				if(state_i <= 0){
					if(state == FALLINGSHOOT){
						state = FALLING;
					}else
					if(state == JETPACKSHOOT){
						state = JETPACK;
					}
					state_i = -1;
					break;
				}
				if(state_i >= 1){
					state_i -= 2;
				}
			}
			Uint8 oldstate_i = state_i;
			Uint8 oldres_bank = res_bank;
			Uint8 oldres_index = res_index;
			if(state == FALLINGSHOOT){
				if(ProcessFallingState(world)){
					if(state == STANDING || state == RUNNING){
						state = STANDINGSHOOT;
						state_i = oldstate_i;
						res_bank = oldres_bank;
						res_index = oldres_index;
					}
					break;
				}
			}else
			if(state == JETPACKSHOOT){
				if(ProcessJetpackState(world)){
					if(state == STANDING || state == RUNNING){
						state = STANDINGSHOOT;
						state_i = oldstate_i;
						res_bank = oldres_bank;
						res_index = oldres_index;
					}
					if(state == FALLING){
						state = FALLINGSHOOT;
						state_i = oldstate_i;
						res_bank = oldres_bank;
						res_index = oldres_index;
					}
					break;
				}
			}
			state_i = oldstate_i;
			res_bank = oldres_bank;
			res_index = oldres_index;
		}break;
		case JETPACK:{
			UnDisguise(world);
			if(ProcessJetpackState(world)){
				break;
			}
			if(input.keyfire){
				state = JETPACKSHOOT;
				state_i = -1;
			}
		}break;
		case HACKING:{
			xv = 0;
			yv = 0;
			bool hackable = true;
			Terminal * terminal = 0;
			std::vector<Uint8> types;
			types.push_back(ObjectTypes::TERMINAL);
			std::vector<Object *> objects = world.TestAABB(x, y - height, x, y, types);
			for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
				if((*it)->type == ObjectTypes::TERMINAL){
					terminal = static_cast<Terminal *>(*it);
					if(terminal && terminal->state != Terminal::HACKERGONE && terminal->state != Terminal::HACKING && terminal->state != Terminal::READY && (terminal->hackerid == 0 || terminal->hackerid == id)){
						hackable = false;
					}
				}
			}
			if(hackable && terminal){
				if(state_i == 14){
					EmitSound(world, world.resources.soundbank["jackin.wav"], 30);
				}
				if(state_i >= 14 && state_i <= 16){
					if(hacksoundchannel == -1){
						hacksoundchannel = EmitSound(world, world.resources.soundbank["ambloop5.wav"], 50, true);
					}
				}
			}
			if(state_i == 16){
				if(terminal && hackable){
					float hackingpowerupbonus = 0;
					if(hackingbonustime > world.tickcount){
						hackingpowerupbonus = 1.00;
					}
					Uint16 oldfilesleft = ((100 - terminal->GetPercent()) / 100.0) * terminal->files;
					Uint8 oldsecretinfoleft = ((100 - terminal->GetPercent()) / 100.0) * (terminal->secretinfo * (1 + hackingbonus + hackingpowerupbonus));
					if(terminal->Hack(id)){
						Uint16 filesleft = ((100 - terminal->GetPercent()) / 100.0) * terminal->files;
						Uint8 secretinfoleft = ((100 - terminal->GetPercent()) / 100.0) * (terminal->secretinfo * (1 + hackingbonus + hackingpowerupbonus));
						if(oldfilesleft == 0){
							oldfilesleft = filesleft;
						}
						if(oldsecretinfoleft == 0){
							oldsecretinfoleft = secretinfoleft;
						}
						//printf("old: %d  filesleft:%d\n", oldfilesleft, filesleft);
						files += oldfilesleft - filesleft;
						Peer * peer = GetPeer(world);
						if(peer){
							peer->stats.fileshacked += oldfilesleft - filesleft;
						}
						oldfilesleft = filesleft;
						
						Team * team = GetTeam(world);
						if(team && team->basedoorid && !team->beamingterminalid){
							team->secretprogress += oldsecretinfoleft - secretinfoleft;
						}
						
						/*while(files - oldfiles >= 6){
							Team * team = GetTeam(world);
							if(team && team->basedoorid && !team->beamingterminalid){
								team->secretprogress++;
							}
							oldfiles += 6;
						}*/
						effecthacking = true;
						effecthackingcontinue = 5;
						state_i = 15;
					}
				}
			}
			if(state_i >= 15 || !hackable){
				if((input.keymoveleft && !oldinput.keymoveleft) || (input.keymoveright && !oldinput.keymoveright)){
					state = RUNNING;
					state_i = -1;
					if(terminal){
						terminal->HackerGone();
					}
					break;
				}
				if(input.keyjump && !oldinput.keyjump){
					state = JUMPING;
					state_i = -1;
					if(terminal){
						terminal->HackerGone();
					}
					break;
				}
				if(input.keyactivate && !oldinput.keyactivate){
					state_i = 16;
					if(terminal){
						terminal->HackerGone();
					}
				}
			}
			res_bank = 173;
			res_index = state_i;
			if(IsDisguised()){
				// 127:0-18 civilian hacking
				res_bank = 127;
			}
			if(state_i >= 17){
				effecthacking = false;
				res_index = (17 - state_i) + 16;
				if(state_i - 17 == 16){
					state = STANDING;
					state_i = -1;
					break;
				}
			}
			if(!hackable && state_i <= 15){
				//effecthacking = false;
				if(terminal){
					terminal->HackerGone();
				}
				//state = STOPHACKING;
				state_i = 16; //
				//state_i -= 2;
			}
		}break;
		case CLIMBINGLEDGE:{
			xv = 0;
			y += yv;
			res_bank = 15;
			res_index = state_i;
			if(state_i >= 15){
				res_index = 15;
				state = STANDING;
				state_i = -1;
				break;
			}
		}break;
		case LADDER:{
			if(ProcessLadderState(world)){
				break;
			}
		}break;
		case LADDERSHOOT:{
			// 26:0-8 ladder shoot right
			// 27:0-8 ladder shoot up
			// 28:0-8 ladder shoot down
			// 29:0-8 ladder shoot up angle
			// 30:0-8 ladder shoot down angle
			// 0-3 is moving toward direction, 4 is the resting direction, 5-8 is firing
			UnDisguise(world);
			Uint8 direction = 200;
			Uint8 delay = weaponfiredelay[currentweapon];
			if(state_i >= 8 + delay){
				state_i = 4;
			}
			res_bank = 26;
			if(input.keyfire && 1/*(state_i <= 4 || state_i >= 10)*/){
				if(input.keymoveup || 1){
					direction = 10;
					res_bank = 27;
				}
				if(input.keymoveright){
					direction = 12;
					res_bank = 26;
					mirrored = false;
				}
				if(input.keymoveleft){
					direction = 16;
					res_bank = 26;
					mirrored = true;
				}
				if(input.keymovedown){
					direction = 14;
					res_bank = 28;
				}
				if(input.keylookupright){
					direction = 11;
					res_bank = 29;
					mirrored = false;
				}
				if(input.keylookdownright){
					direction = 13;
					res_bank = 30;
					mirrored = false;
				}
				if(input.keylookupleft){
					direction = 17;
					res_bank = 29;
					mirrored = true;
				}
				if(input.keylookdownleft){
					direction = 15;
					res_bank = 30;
					mirrored = true;
				}
			}
			//if(state_i > 4 + delay){
			//	res_index = state_i - delay;
			//}
			if(state_i < 4){
				res_index = state_i;
			}else
			if(state_i == 9){
				res_index = 5;
			}else
			if(state_i == 10){
				res_index = 6;
			}else
			if(state_i == 11){
				res_index = 7;
			}else
			if(state_i == 12){
				res_index = 8;
			}else{
				res_index = 4;
			}
			if(state_i == 4){
				Fire(world, direction);
			}
			if(!input.keyfire){
				if(state_i >= 4){
					state_i = 4;
				}
				if(state_i <= 0){
					state = LADDER;
					state_i = -1;
					break;
				}
				if(state_i >= 1){
					state_i -= 2;
				}
				Uint8 oldstate_i = state_i;
				Uint8 oldres_bank = res_bank;
				Uint8 oldres_index = res_index;
				if(ProcessLadderState(world)){
					break;
				}
				state_i = oldstate_i;
				res_bank = oldres_bank;
				res_index = oldres_index;
			}
		}break;
		case DYING:{
			//currentplatformid = 0;
			collidable = false;
			if(state_i == 0){
				DropAllItems(world);
			}
			if(state_i >= 15){
				state = DEAD;
				state_i = -1;
				break;
			}
			yv += world.gravity;
			if(yv > world.maxyvelocity){
				yv = world.maxyvelocity;
			}
			int xe = xv;
			int ye = yv;
			world.map.TestIncr(x - 5, y - 10, x + 5, y, &xe, &ye, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true);
			if(xe == 0){
				xv = 0;
			}
			x += xe;
			y += ye;
			res_bank = 20;
			res_index = state_i;
		}break;
		case DEAD:{
			collidable = false;
			poisonedby = 0;
			yv += world.gravity;
			if(yv > world.maxyvelocity){
				yv = world.maxyvelocity;
			}
			int xe = xv;
			int ye = yv;
			world.map.TestIncr(x - 5, y - 10, x + 5, y, &xe, &ye, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true);
			if(xe == 0){
				xv = 0;
			}
			x += xe;
			y += ye;
			if(state_i > 48 && !state_warp && !world.winningteamid){
				Team * team = GetTeam(world);
				if(team && team->agency == Team::LAZARUS && canresurrect && !world.winningteamid){
					state = RESURRECTING;
					state_i = -1;
				}else{
					if(team && team->basedoorid){
						draw = false;
						state = RESPAWNING;
						if(!world.replaying){
							canresurrect = true;
						}
						SetToRespawnPosition(world);
						health = maxhealth;
						shield = maxshield;
						state_i = -1;
						break;
					}else{
						draw = true;
						Sint16 x;
						Sint16 y;
						world.map.RandomPlayerStartLocation(world, x, y);
						Warp(world, x, y);
					}
				}
			}
		}break;
		case RESPAWNING:{
			draw = true;
			res_bank = 198;
			if(state_i == 0){
				EmitSound(world, world.resources.soundbank["transrev.wav"], 64);
			}
			if(state_i / 2 > 27){
				//rocketammo = 10;
				//laserammo = 5;
				collidable = true;
				/*Sint32 xv2 = 0;
				Sint32 yv2 = 1;
				Platform * platform = world.map.TestIncr(x, y - height, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
				if(platform){
					currentplatformid = platform->id;
					y = platform->XtoY(x);
				}*/
				currentplatformid = 0;
				FindCurrentPlatform(*this, world);
				state = STANDING;
				state_i = 0;
				break;
			}
			res_index = state_i / 2;
		}break;
		case RESURRECTING:{
			// 199:0-27 lazarus resurrection
			draw = true;
			canresurrect = false;
			collidable = false;
			res_bank = 199;
			res_index = state_i;
			if(state_i == 2){
				EmitSound(world, world.resources.soundbank["repair.wav"], 128);
			}
			if(state_i >= 27){
				health = maxhealth;
				collidable = true;
				HandleAgainstWall(world);
				currentplatformid = 0;
				FindCurrentPlatform(*this, world);
				state = STANDING;
				state_i = 1;
				break;
			}
		}break;
		case THROWING:{
			// 113:0-16 player throwing grenade
			if(ProcessStandingState(world)){
				break;
			}
			if(state_i > 16){
				state = STANDING;
				state_i = -1;
				break;
			}
			res_bank = 113;
			res_index = state_i;
		}break;
	}
	if(collidable){
		std::vector<Uint8> types;
		types.push_back(ObjectTypes::TERMINAL);
		types.push_back(ObjectTypes::HEALMACHINE);
		types.push_back(ObjectTypes::CREDITMACHINE);
		types.push_back(ObjectTypes::SECRETRETURN);
		/*int x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
		int x2 = x - world.resources.spriteoffsetx[res_bank][res_index] + world.resources.spritewidth[res_bank][res_index];
		int y1 = y - world.resources.spriteoffsety[res_bank][res_index];
		int y2 = y - world.resources.spriteoffsety[res_bank][res_index] + world.resources.spriteheight[res_bank][res_index];
		std::vector<Object *> objects = world.TestAABB(x1, y1, x2, y2, types);*/
		std::vector<Object *> objects = world.TestAABB(x, y - height, x, y, types);
		for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
			Object * object = *it;
			switch(object->type){
				case ObjectTypes::TERMINAL:{
					Terminal * terminal = static_cast<Terminal *>(object);
					if(terminal && terminal->state == Terminal::SECRETREADY){
						Team * team = GetTeam(world);
						if(team && team->beamingterminalid == terminal->id && !hassecret){
							if(world.IsAuthority()){
								terminal->state = Terminal::INACTIVE;
								team->beamingterminalid = 0;
								secretteamid = team->id;
								tracetime = terminal->tracetime;
								terminal->tracetime = 0;
								hassecret = true;
								team->playerwithsecret = id;
							}
						}
					}
				}break;
				case ObjectTypes::HEALMACHINE:{
					HealMachine * healmachine = static_cast<HealMachine *>(object);
					if(healmachine){
						Team * team = GetTeam(world);
						if(team && team->basedoorid == basedoorentering && (health < maxhealth || shield < maxshield)){
							if(world.IsAuthority()){
								if(healmachine->Activate()){
									poisonedby = 0;
									if(health < maxhealth){
										health = maxhealth;
									}
									if(shield < maxshield){
										shield = maxshield;
									}
								}
							}
						}
					}
				}break;
				case ObjectTypes::CREDITMACHINE:{
					CreditMachine * creditmachine = static_cast<CreditMachine *>(object);
					if(creditmachine){
						Team * team = GetTeam(world);
						if(team && team->basedoorid == basedoorentering && files > 0){
							if(world.IsAuthority()){
								creditmachine->Activate();
								if(files > maxfiles){
									files = maxfiles;
								}
								Uint16 creditamount = files * (1 + creditsbonus);
								AddCredits(creditamount);
								Peer * peer = GetPeer(world);
								if(peer){
									peer->stats.filesreturned += files;
								}
								files = 0;
								oldfiles = 0;
							}
						}
					}
				}break;
				case ObjectTypes::SECRETRETURN:{
					SecretReturn * secretreturn = static_cast<SecretReturn *>(object);
					if(secretreturn && abs(x - secretreturn->x) < 30 && abs(y - secretreturn->y) < 50){
						if(hassecret){
							Team * team = GetTeam(world);
							if(team){
								if(team->id == secretreturn->teamid){
									if(world.IsAuthority()){
										team->secretdelivered = id;
										hassecret = false;
										team->playerwithsecret = 0;
									}
								}
							}
						}
					}
				}break;
			}
		}
	}
	if(CheckForBaseExit(world)){
		BaseDoor * basedoor = (BaseDoor *)world.GetObjectFromId(basedoorentering);
		if(basedoor){
			x = basedoor->x;
			y = basedoor->y;
			/*Sint32 xv2 = 0;
			Sint32 yv2 = 1;
			Platform * platform = world.map.TestIncr(x, y - height, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
			if(platform){
				Uint32 yt = platform->XtoY(x);
				if(y <= yt){
					yv = 0;
					currentplatformid = platform->id;
				}
			}*/
			FindCurrentPlatform(*this, world);
			res_bank = 19;
			res_index = 15;
			state = WALKOUT;
			state_i = -1;
		}
	}
	if(OnGround()){
		if(input.keyjump && !oldinput.keyjump){
			state = JUMPING;
			state_i = -1;
		}
	}
	if(OnGround() || state == FALLING || state == FALLINGSHOOT){
		if(input.keyjetpack && !fuellow){
			if(state == FALLINGSHOOT){
				state = JETPACKSHOOT;
			}else{
				state = JETPACK;
				state_i = -1;
			}
		}
	}
	// Uncomment this if you want the projectiles to follow the player before they are actually moving/shot
	/*if(currentprojectileid){
		Projectile * projectile = static_cast<Projectile *>(world.GetObjectFromId(currentprojectileid));
		if(projectile && !projectile->moving){
			projectile->x += (x - oldx);
			projectile->y += (y - oldy);
		}
	}*/
	//oldinput = input;
	state_i++;
}

void Player::HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile){
	//printf("hit at %d, %d\n", x, y);
	Hittable::HandleHit(*this, world, x, y, projectile);
	UnDisguise(world);
	if(world.tickcount - hitsoundplaytick > 10){
		if(rand() % 2 == 0){
			EmitSound(world, world.resources.soundbank["s_hita01.wav"]);
		}else{
			EmitSound(world, world.resources.soundbank["s_hitb01.wav"]);
		}
		hitsoundplaytick = world.tickcount;
	}
	if(state_hit / 32 == 1){
		effectshieldcontinue = 48;
	}
	float xpcnt = -((x - 50) / 50.0) * (mirrored ? -1 : 1);
	float ypcnt = -((y - 50) / 50.0);
	switch(state){
		case STANDING:
		case RUNNING:
		case CROUCHING:
		case CROUCHED:
		case CROUCHEDSHOOT:
		case ROLLING:
		case STANDINGSHOOT:
		case THROWING:
			FollowGround(*this, world, xpcnt * projectile.moveamount);
		break;
		default:
			xv += xpcnt * projectile.moveamount;
			yv += ypcnt * projectile.moveamount;
		break;
	}
	/*if(x < 50){
		xv = (abs(xv) + 5) * (mirrored ? -1 : 1);
	}else{
		xv = -(abs(xv) + 5) * (mirrored ? -1 : 1);
	}*/
	if(health == 0 && state != DYING && state != DEAD){
		Peer * peer = GetPeer(world);
		if(peer){
			peer->stats.deaths++;
			const char * killedby = "?";
			bool killedself = false;
			Object * owner = world.GetObjectFromId(projectile.ownerid);
			if(owner){
				switch(owner->type){
					case ObjectTypes::WALLDEFENSE:{
						killedby = "a Laser Turret";
					}break;
					case ObjectTypes::FIXEDCANNON:{
						killedby = "a Fixed Cannon";
					}break;
					case ObjectTypes::ROBOT:{
						killedby = "a Robot";
					}break;
					case ObjectTypes::GUARD:{
						killedby = "a Guard";
					}break;
					case ObjectTypes::TEAM:{
						killedby = "a Government Satellite";
					}break;
					case ObjectTypes::CIVILIAN:{
						killedby = "a Lazarus Tract";
					}break;
					case ObjectTypes::PLAYER:{
						killedby = "a Player";
						Player * player = static_cast<Player *>(owner);
						Peer * killedbypeer = player->GetPeer(world);
						if(killedbypeer){
							killedby = world.lobby.GetUserInfo(killedbypeer->accountid)->name;
							if(player->id == id){
								killedself = true;
							}else{
								killedbypeer->stats.kills++;
							}
							switch(projectile.type){
								case ObjectTypes::BLASTERPROJECTILE:{
									killedbypeer->stats.playerkillsweapon[0]++;
								}break;
								case ObjectTypes::LASERPROJECTILE:{
									killedbypeer->stats.playerkillsweapon[1]++;
								}break;
								case ObjectTypes::ROCKETPROJECTILE:{
									killedbypeer->stats.playerkillsweapon[2]++;
								}break;
								case ObjectTypes::FLAMERPROJECTILE:{
									killedbypeer->stats.playerkillsweapon[3]++;
								}break;
							}
						}
					}break;
				}
			}
			char temp[256];
			if(killedself){
				sprintf(temp, "%s committed suicide", world.lobby.GetUserInfo(peer->accountid)->name);
			}else{
				sprintf(temp, "%s was killed by %s", world.lobby.GetUserInfo(peer->accountid)->name, killedby);
			}
			world.ShowStatus(temp, 160, true);
		}
		if(projectile.type == ObjectTypes::ROCKETPROJECTILE){
			world.Explode(*this, suitcolor, xpcnt);
		}
		state = DYING;
		state_i = 0;
		EmitSound(world, world.resources.soundbank["grunt2a.wav"]);
	}
}

void Player::HandleInput(Input & input){
	oldinput = Player::input;
	Player::input = input;
	if(input.keyweapon[0]){
		currentweapon = 0;
	}
	if(input.keyweapon[1] && laserammo > 0){
		currentweapon = 1;
	}
	if(input.keyweapon[2] && rocketammo > 0){
		currentweapon = 2;
	}
	if(input.keyweapon[3] && flamerammo > 0){
		currentweapon = 3;
	}
}

void Player::HandleDisconnect(World & world, Uint8 peerid){
	UnDeploy();
	if(world.IsAuthority()){
		Peer * peer = world.peerlist[peerid];
		if(peer){
			User * user = world.lobby.GetUserInfo(peer->accountid);
			if(user){
				char temp[256];
				sprintf(temp, "%s has left the game", user->name);
				world.ShowStatus(temp, 176, true);
				Team * team = GetTeam(world);
				if(team){
					user->statscopy = peer->stats;
					user->statsagency = team->agency;
				}
			}
		}
	}
}

void Player::OnDestroy(World & world){
	if(ai){
		delete ai;
		ai = 0;
	}
}

bool Player::InBase(World & world){
	if(y > world.map.height * 64){
		return true;
	}
	return false;
}

bool Player::InOwnBase(World & world){
	BaseDoor * basedoor = static_cast<BaseDoor *>(world.GetObjectFromId(basedoorentering));
	if(InBase(world) && basedoor && basedoor->teamid == teamid){
		return true;
	}
	return false;
}

bool Player::IsDisguised(void){
	if(disguised >= 100){
		return true;
	}
	return false;
}

void Player::UnDisguise(World & world){
	if(IsDisguised()){
		disguised = 12;
		EmitSound(world, world.resources.soundbank["disguise.wav"], 64);
	}
}

bool Player::Poison(Uint16 playerid){
	if(!poisonedby){
		poisonedby = playerid;
		return true;
	}
	return false;
}

bool Player::HasSecurityPass(void){
	for(int i = 0; i < 4; i++){
		if(inventoryitems[i] == INV_SECURITYPASS){
			return true;
		}
	}
	return false;
}

bool Player::CheckForBaseExit(World & world){
	Uint32 ypos = y - ((world.map.height * 64) + 718);
	ypos %= (1 * 26 * 64);
	if(InBase(world) && x <= 500 && ypos > 600){
		return true;
	}
	return false;
}

bool Player::CheckForLadder(World & world){
	Platform * ladder = world.map.TestAABB(x - abs(xv), y - height, x + abs(xv), y, Platform::LADDER);
	if(!justjumpedfromladder && input.keymoveup && ladder){
		Uint32 center = ((ladder->x2 - ladder->x1) / 2) + ladder->x1;
		if(abs(signed(center) - x) <= abs(ceil(float(xv)))){
			x = center;
			state = LADDER;
			if(IsDisguised()){
				res_bank = 124;
			}else{
				res_bank = 16;
			}
			res_index = 0;
			state_i = -1;
			return true;
		}
	}
	return false;
}

bool Player::CheckForGround(World & world, Platform & platform){
	justjumpedfromladder = false;
	Uint32 yt = platform.XtoY(x);
	if(y <= yt || ((platform.type == Platform::STAIRSUP || platform.type == Platform::STAIRSDOWN) && y <= yt + 1)){
		EmitSound(world, world.resources.soundbank["futstonr.wav"], 32);
		EmitSound(world, world.resources.soundbank["land11.wav"], 96);
		yv = 0;
		currentplatformid = platform.id;
		y = yt;
		xv /= 4;
		FollowGround(*this, world, xv/* - xv2*/);
		justlandedfromair = true;
		if(input.keymoveright || input.keymoveleft){
			state = RUNNING;
		}else{
			state = STANDING;
			if(!IsDisguised()){
				res_bank = 14;
				res_index = 0;
			}
		}/*else
		  if(input.keymoveleft && xv < 0){
		  state = RUNNING;
		  }else{
		  state = STANDING;
		  }*/
		state_i = -1;
		return true;
	}else
	if(y <= yt + 80 && !IsDisguised() && !world.map.TestAABB(x > platform.x2 ? platform.x2 - 1 : platform.x1 - 1, yt - 1, x > platform.x2 ? platform.x2 + 1 : platform.x1 + 1, yt - 1, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN)){
		//y = yt + 48;
		yv = -3;
		if(x > platform.x2){
			x = platform.x2;
			mirrored = true;
		}else{
			x = platform.x1;
			mirrored = false;
		}
		currentplatformid  = platform.id;
		state = CLIMBINGLEDGE;
		state_i = -1;
		y = (int(y / 3) * 3) - 1;
		if(y > yt + 48){
			y = yt + 48;
		}
		Uint32 yq = y;
		while(yq < yt + 48){
			state_i++;
			yq += 3;
		}
		//state_i = -1;
		return true;
	}
	return false;
}

bool Player::HandleAgainstWall(World & world){
	// handle when player is up against a wall
	y--;
	if(world.map.TestAABB(x, y - height, x, y, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true)){
		x++;
		if(!world.map.TestAABB(x, y - height, x, y, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true)){
			y++;
			FindCurrentPlatform(*this, world);
			xv = 0;
			/*state = STANDING;
			state_i = -1;*/
			return true;
		}else{
			x--;
			x--;
			if(!world.map.TestAABB(x, y - height, x, y, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true)){
				y++;
				FindCurrentPlatform(*this, world);
				xv = 0;
				/*state = STANDING;
				state_i = -1;*/
				return true;
			}
		}
	}
	return false;
}

bool Player::CanCreateBase(World & world){
	if(InBase(world)){
		return false;
	}
	if(world.map.TestAABB(x, y - 50, x, y - 1, Platform::STAIRSDOWN | Platform::STAIRSUP | Platform::RECTANGLE)){
		return false;
	}
	std::vector<Uint8> types;
	types.push_back(ObjectTypes::BASEDOOR);
	types.push_back(ObjectTypes::TERMINAL);
	std::vector<Object *> objects = world.TestAABB(x - 30, y - 50, x + 30, y, types);
	if(objects.size() > 0){
		return false;
	}
	return true;
}

Team * Player::GetTeam(World & world){
	Object * object = world.GetObjectFromId(teamid);
	if(object){
		Team * team = static_cast<Team *>(object);
		if(team){
			return team;
		}
	}
	std::vector<Team *> teams;
	for(std::vector<Uint16>::iterator it = world.objectsbytype[ObjectTypes::TEAM].begin(); it != world.objectsbytype[ObjectTypes::TEAM].end(); it++){
		Object * object = world.GetObjectFromId((*it));
		teams.push_back(static_cast<Team *>(object));
	}
	for(std::vector<Team *>::iterator it = teams.begin(); it != teams.end(); it++){
		Team * team = *it;
		for(int i = 0; i < team->numpeers; i++){
			if(world.peerlist[team->peers[i]]){
				Player * player = world.GetPeerPlayer(world.peerlist[team->peers[i]]->id);
				if(player && player->id == id){
					teamid = team->id;
					return team;
				}
			}
		}
	}
	return 0;
}

bool Player::AddInventoryItem(Uint8 id, bool settocurrent){
	for(int i = 0; i < 5; i++){
		if(inventoryitems[i] == INV_NONE){
			inventoryitems[i] = id;
			inventoryitemsnum[i] = 1;
			if(settocurrent){
				currentinventoryitem = i;
			}
			return true;
		}else
		if(inventoryitems[i] == id){
			Uint8 maxitems = 4;
			switch(id){
				case INV_BASEDOOR:
					maxitems = 1;
				break;
				case INV_SECURITYPASS:
					maxitems = 1;
				break;
			}
			if(inventoryitemsnum[i] >= maxitems){
				return false;
			}else{
				inventoryitemsnum[i]++;
				if(settocurrent){
					currentinventoryitem = i;
				}
				return true;
			}
		}
	}
	return false;
}

void Player::RemoveInventoryItem(Uint8 id){
	for(int i = 0; i < 4; i++){
		if(inventoryitems[i] == id){
			inventoryitemsnum[i]--;
			if(inventoryitemsnum[i] == 0){
				inventoryitems[i] = INV_NONE;
				if(currentinventoryitem == i && i > 0){
					currentinventoryitem = i - 1;
				}
				for(int j = i + 1; j < 4; j++){
					inventoryitems[j - 1] = inventoryitems[j];
					inventoryitemsnum[j - 1] = inventoryitemsnum[j];
				}
				inventoryitems[3] = INV_NONE;
				inventoryitemsnum[3] = 0;
			}
		}
	}
}

Uint8 Player::InventoryItemCount(Uint8 id){
	for(int i = 0; i < 4; i++){
		if(inventoryitems[i] == id){
			return inventoryitemsnum[i];
		}
	}
	return 0;
}

bool Player::BuyItem(World & world, Uint8 id){
	if(!BuyAvailable(world, id)){
		return false;
	}
	BuyableItem * buyableitem = 0;
	for(std::vector<BuyableItem *>::iterator it = world.buyableitems.begin(); it != world.buyableitems.end(); it++){
		if((*it)->id == id){
			buyableitem = (*it);
			break;
		}
	}
	if(buyableitem && credits >= buyableitem->price){
		Team * team = GetTeam(world);
		if(team && buyableitem->techchoice & team->disabledtech){
			return false;
		}
		BaseDoor * basedoor = static_cast<BaseDoor *>(world.GetObjectFromId(basedoorentering));
		if(!InBase(world) || (basedoor && basedoor->teamid != teamid)){
			return false;
		}
		int x1, y1, x2, y2;
		GetAABB(world.resources, &x1, &y1, &x2, &y2);
		std::vector<Uint8> types;
		types.push_back(ObjectTypes::INVENTORYSTATION);
		if(world.TestAABB(x1, y1, x2, y2, types).size() == 0){
			return false;
		}
		bool bought = false;
		switch(id){
			case World::BUY_LASER:{
				if(laserammo < 30){
					laserammo += 5;
					if(laserammo > 30){
						laserammo = 30;
					}
					bought = true;
				}
			}break;
			case World::BUY_ROCKET:{
				if(rocketammo < 30){
					rocketammo += 3;
					if(rocketammo > 30){
						rocketammo = 30;
					}
					bought = true;
				}
			}break;
			case World::BUY_FLAMER:{
				if(flamerammo < 75){
					flamerammo += 15;
					if(flamerammo > 75){
						flamerammo = 75;
					}
					bought = true;
				}
			}break;
			case World::BUY_HEALTH:{
				if(AddInventoryItem(INV_HEALTHPACK, true)){
					bought = true;
				}
			}break;
			case World::BUY_TRACT:{
				if(AddInventoryItem(INV_LAZARUSTRACT, true)){
					bought = true;
				}
			}break;
			case World::BUY_SECURITYPASS:{
				if(AddInventoryItem(INV_SECURITYPASS, true)){
					bought = true;
				}
			}break;
			case World::BUY_VIRUS:{
				if(AddInventoryItem(INV_VIRUS, true)){
					bought = true;
				}
			}break;
			case World::BUY_POISON:{
				if(AddInventoryItem(INV_POISON, true)){
					bought = true;
				}
			}break;
			case World::BUY_EMPB:{
				if(AddInventoryItem(INV_EMPBOMB, true)){
					bought = true;
				}
			}break;
			case World::BUY_SHAPEDB:{
				if(AddInventoryItem(INV_SHAPEDBOMB, true)){
					bought = true;
				}
			}break;
			case World::BUY_PLASMAB:{
				if(AddInventoryItem(INV_PLASMABOMB, true)){
					bought = true;
				}
			}break;
			case World::BUY_NEUTRONB:{
				if(AddInventoryItem(INV_NEUTRONBOMB, true)){
					bought = true;
				}
			}break;
			case World::BUY_DET:{
				if(AddInventoryItem(INV_PLASMADET, true)){
					bought = true;
				}
			}break;
			case World::BUY_FIXEDC:{
				if(AddInventoryItem(INV_FIXEDCANNON, true)){
					bought = true;
				}
			}break;
			case World::BUY_FLARE:{
				if(AddInventoryItem(INV_FLARE, true)){
					bought = true;
				}
			}break;
			case World::BUY_POISONFLARE:{
				if(AddInventoryItem(INV_POISONFLARE, true)){
					bought = true;
				}
			}break;
			case World::BUY_CAMERA:{
				if(AddInventoryItem(INV_CAMERA, true)){
					bought = true;
				}
			}break;
			case World::BUY_DOOR:{
				if(AddInventoryItem(INV_BASEDOOR, true)){
					bought = true;
				}
			}break;
			case World::BUY_DEFENSE:{
				for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
					Object * object = *it;
					if(object->type == ObjectTypes::WALLDEFENSE){
						WallDefense * walldefense = static_cast<WallDefense *>(object);
						Team * team = GetTeam(world);
						if(team && walldefense->teamid == team->id){
							if(walldefense->AddDefense()){
								bought = true;
							}
						}
					}
				}
			}break;
			case World::BUY_INFO:{
				Team * team = GetTeam(world);
				if(team){
					if(team->secretprogress < 180 && !team->beamingterminalid){
						team->secretprogress += 20;
						bought = true;
					}
				}
			}break;
			case World::BUY_GIVE0:{
				Team * team = GetTeam(world);
				if(team){
					if(team->peers[0]){
						Player * player = world.GetPeerPlayer(team->peers[0]);
						if(player){
							player->AddCredits(100);
							bought = true;
						}
					}
				}
			}break;
			case World::BUY_GIVE1:{
				Team * team = GetTeam(world);
				if(team){
					if(team->peers[1]){
						Player * player = world.GetPeerPlayer(team->peers[1]);
						if(player){
							player->AddCredits(100);
							bought = true;
						}
					}
				}
			}break;
			case World::BUY_GIVE2:{
				Team * team = GetTeam(world);
				if(team){
					if(team->peers[2]){
						Player * player = world.GetPeerPlayer(team->peers[2]);
						if(player){
							player->AddCredits(100);
							bought = true;
						}
					}
				}
			}break;
			case World::BUY_GIVE3:{
				Team * team = GetTeam(world);
				if(team){
					if(team->peers[3]){
						Player * player = world.GetPeerPlayer(team->peers[3]);
						if(player){
							player->AddCredits(100);
							bought = true;
						}
					}
				}
			}break;
		}
		if(bought){
			EmitSound(world, world.resources.soundbank["reload2.wav"], 96);
			world.BuyItem(id);
			credits -= buyableitem->price;
			return true;
		}
	}
	return false;
}

bool Player::RepairItem(World & world, Uint8 id){
	BuyableItem * buyableitem = 0;
	for(std::vector<BuyableItem *>::iterator it = world.buyableitems.begin(); it != world.buyableitems.end(); it++){
		if((*it)->id == id){
			buyableitem = (*it);
			break;
		}
	}
	if(buyableitem && credits >= buyableitem->repairprice){
		Team * team = GetTeam(world);
		if(team && team->disabledtech & buyableitem->techchoice){
			int x1, y1, x2, y2;
			GetAABB(world.resources, &x1, &y1, &x2, &y2);
			std::vector<Uint8> types;
			types.push_back(ObjectTypes::TECHSTATION);
			if(world.TestAABB(x1, y1, x2, y2, types, 0, 0, false).size() == 0){
				return false;
			}
			
			team->disabledtech &= ~(buyableitem->techchoice);
			EmitSound(world, world.resources.soundbank["reload2.wav"], 96);
			world.RepairItem(id);
			credits -= buyableitem->repairprice;
			return true;
		}
	}
	return false;
}

bool Player::VirusItem(World & world, Uint8 id){
	BuyableItem * buyableitem = 0;
	for(std::vector<BuyableItem *>::iterator it = world.buyableitems.begin(); it != world.buyableitems.end(); it++){
		if((*it)->id == id){
			buyableitem = (*it);
			break;
		}
	}
	if(buyableitem && InventoryItemCount(INV_VIRUS) > 0){
		BaseDoor * basedoor = static_cast<BaseDoor *>(world.GetObjectFromId(basedoorentering));
		if(basedoor){
			Team * team = static_cast<Team *>(world.GetObjectFromId(basedoor->teamid));
			if(team && !(team->disabledtech & buyableitem->techchoice)){
				int x1, y1, x2, y2;
				GetAABB(world.resources, &x1, &y1, &x2, &y2);
				std::vector<Uint8> types;
				types.push_back(ObjectTypes::TECHSTATION);
				if(world.TestAABB(x1, y1, x2, y2, types, 0, 0, false).size() == 0){
					return false;
				}
				
				team->disabledtech |= buyableitem->techchoice;
				EmitSound(world, world.resources.soundbank["reload2.wav"], 96);
				world.VirusItem(id);
				RemoveInventoryItem(INV_VIRUS);
				return true;
			}
		}
	}
	return false;
}

void Player::LoadAbilities(World & world){
	Peer * peer = GetPeer(world);
	if(peer){
		Team * team = GetTeam(world);
		User * user = world.lobby.GetUserInfo(peer->accountid);
		if(team && user){
			maxfuel += user->agency[team->agency].jetpack * 10;
			fuel = maxfuel;
			maxshield += user->agency[team->agency].shield * 20;
			shield = maxshield;
			maxhealth += user->agency[team->agency].endurance * 20;
			health = maxhealth;
			hackingbonus = user->agency[team->agency].hacking * 0.10;
			creditsbonus = user->agency[team->agency].contacts * 0.10;
			if(team->agency == Team::NOXIS){
				jumpimpulse = -3;
			}
			abilitiesloaded = true;
		}
	}
}

void Player::KillByGovt(World & world){
	Team * team = GetTeam(world);
	if(team){
		Object govtprojectile(ObjectTypes::PLASMAPROJECTILE);
		govtprojectile.healthdamage = 0xFFFF;
		govtprojectile.shielddamage = 0xFFFF;
		govtprojectile.ownerid = team->id;
		HandleHit(world, 50, 50, govtprojectile);
		for(int i = 0; i < 8; i++){
			PlasmaProjectile * plasmaprojectile = static_cast<PlasmaProjectile *>(world.CreateObject(ObjectTypes::PLASMAPROJECTILE));
			if(plasmaprojectile){
				plasmaprojectile->x = x;
				plasmaprojectile->y = y - 10;
				plasmaprojectile->xv = (world.Random() % 17) - 8;
				plasmaprojectile->yv = -(world.Random() % 37);
				plasmaprojectile->ownerid = govtprojectile.ownerid;
			}
		}
	}
}

void Player::AddCredits(int amount){
	if(credits + amount > 0xFFFF){
		credits = 0xFFFF;
	}else{
		credits += amount;
	}
}

void Player::UnDeploy(void){
	state = UNDEPLOYING;
	state_i = 0;
	collidable = false;
}

bool Player::CanExhaustInputQueue(World & world){
	bool otherplayersinview = false;
	for(std::vector<Uint16>::iterator it = world.objectsbytype[ObjectTypes::PLAYER].begin(); it != world.objectsbytype[ObjectTypes::PLAYER].end(); it++){
		Player * player = static_cast<Player *>(world.GetObjectFromId(*it));
		if(player->id != id){
			if(abs(player->x - x) < 400 && abs(player->y - y) < 400){
				otherplayersinview = true;
				break;
			}
		}
	}
	if(xv == 0 && yv == 0 && !otherplayersinview){
		if(state == STANDING || state == DEAD || state == CROUCHED){
			return true;
		}
	}
	return false;
}

Projectile * Player::Fire(World & world, Uint8 direction){
	if(direction == 200){
		return 0;
	}
	Uint8 delay = weaponfiredelay[currentweapon];
	Projectile * projectile = 0;
	if(world.tickcount - lastfire >= delay + 3){
		lastfire = world.tickcount;
		// fire
		Object * projectile = 0;
		Uint8 oldlaserammo = laserammo;
		Uint8 oldrocketammo = rocketammo;
		Uint8 oldflamerammo = flamerammo;
		switch(currentweapon){
			case 0:{
				projectile = world.CreateObject(ObjectTypes::BLASTERPROJECTILE);
			}break;
			case 1:{
				if(laserammo == 0){
					state_i--;
					currentweapon = 0;
				}else{
					projectile = (LaserProjectile *)world.CreateObject(ObjectTypes::LASERPROJECTILE);
					laserammo--;
				}
			}break;
			case 2:{
				if(rocketammo == 0){
					state_i--;
					currentweapon = 0;
				}else{
					projectile = world.CreateObject(ObjectTypes::ROCKETPROJECTILE);
					rocketammo--;
				}
			}break;
			case 3:{
				if(flamerammo == 0){
					state_i--;
					currentweapon = 0;
				}else{
					projectile = world.CreateObject(ObjectTypes::FLAMERPROJECTILE);
					if(flamersoundchannel == -1){
						flamersoundchannel = EmitSound(world, world.resources.soundbank["flamebg2.wav"], 128, true);
					}
					flamerammo--;
				}
			}break;
		}
		if(projectile){
			currentprojectileid = projectile->id;
			projectile->ownerid = id;
			projectile->x = x;
			projectile->y = y;
			projectile->mirrored = mirrored;
			Sint8 velocity = projectile->velocity;
			switch(direction){
				case 0: // standingshoot up
					projectile->x += 3 * (mirrored ? 1 : -1);
					projectile->y -= 100 + projectile->emitoffset;
					projectile->yv = -velocity;
				break;
				case 1: // standingshoot up/right
					projectile->x += 36 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 87 + (projectile->emitoffset * 0.70710678118655);
					projectile->yv = -velocity * 0.70710678118655;
					projectile->xv = velocity * 0.70710678118655;
				break;
				case 2: // standingshoot right
					projectile->x += 45 + projectile->emitoffset;
					projectile->y -= 55;
					projectile->xv = velocity;
				break;
				case 3: // standingshoot down/right
					projectile->x += 37 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 26 - (projectile->emitoffset * 0.70710678118655);
					projectile->yv = velocity * 0.70710678118655;
					projectile->xv = velocity * 0.70710678118655;
				break;
				case 4: // standingshoot down
					projectile->x += 9 * (mirrored ? -1 : 1);
					projectile->y -= 8 - projectile->emitoffset;
					projectile->yv = velocity;
				break;
				case 5: // standingshoot down/left
					projectile->x -= 37 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 26 - (projectile->emitoffset * 0.70710678118655);
					projectile->yv = velocity * 0.70710678118655;
					projectile->xv = -velocity * 0.70710678118655;
				break;
				case 6: // standingshoot left
					projectile->x -= 45 + projectile->emitoffset;
					projectile->y -= 55;
					projectile->xv = -velocity;
				break;
				case 7: // standingshoot up/left
					projectile->x -= 36 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 87 + (projectile->emitoffset * 0.70710678118655);
					projectile->yv = -velocity * 0.70710678118655;
					projectile->xv = -velocity * 0.70710678118655;
				break;
				case 8: // crounchedshoot right
					projectile->x += 49 + projectile->emitoffset;
					projectile->y -= 33;
					projectile->xv = velocity;
				break;
				case 9: // crouchedshoot left
					projectile->x -= 49 + projectile->emitoffset;
					projectile->y -= 33;
					projectile->xv = -velocity;
				break;
				case 10: // laddershoot up
					projectile->x += 0 * (mirrored ? 1 : -1);
					projectile->y -= 103 + projectile->emitoffset;
					projectile->yv = -velocity;
				break;
				case 11: // laddershoot up/right
					projectile->x += 36 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 83 + (projectile->emitoffset * 0.70710678118655);
					projectile->yv = -velocity * 0.70710678118655;
					projectile->xv = velocity * 0.70710678118655;
				break;
				case 12: // laddershoot right
					projectile->x += 45 + projectile->emitoffset;
					projectile->y -= 57;
					projectile->xv = velocity;
				break;
				case 13: // laddershoot down/right
					projectile->x += 37 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 26 - (projectile->emitoffset * 0.70710678118655);
					projectile->yv = velocity * 0.70710678118655;
					projectile->xv = velocity * 0.70710678118655;
				break;
				case 14: // laddershoot down
					projectile->x += 10 * (mirrored ? -1 : 1);
					projectile->y -= 8 - projectile->emitoffset;
					projectile->yv = velocity;
				break;
				case 15: // laddershoot down/left
					projectile->x -= 37 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 26 - (projectile->emitoffset * 0.70710678118655);
					projectile->yv = velocity * 0.70710678118655;
					projectile->xv = -velocity * 0.70710678118655;
				break;
				case 16: // laddershoot left
					projectile->x -= 45 + projectile->emitoffset;
					projectile->y -= 57;
					projectile->xv = -velocity;
				break;
				case 17: // laddershoot up/left
					projectile->x -= 36 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 83 + (projectile->emitoffset * 0.70710678118655);
					projectile->yv = -velocity * 0.70710678118655;
					projectile->xv = -velocity * 0.70710678118655;
				break;
				case 20: // airshoot up
					projectile->x += -1 * (mirrored ? 1 : -1);
					projectile->y -= 100 + projectile->emitoffset;
					projectile->yv = -velocity;
				break;
				case 21: // airshoot up/right
					projectile->x += 34 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 87 + (projectile->emitoffset * 0.70710678118655);
					projectile->yv = -velocity * 0.70710678118655;
					projectile->xv = velocity * 0.70710678118655;
				break;
				case 22: // airshoot right
					projectile->x += 48 + projectile->emitoffset;
					projectile->y -= 53;
					projectile->xv = velocity;
				break;
				case 23: // airshoot down/right
					projectile->x += 31 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 18 - (projectile->emitoffset * 0.70710678118655);
					projectile->yv = velocity * 0.70710678118655;
					projectile->xv = velocity * 0.70710678118655;
				break;
				case 24: // standingshoot down
					projectile->x += 4 * (mirrored ? -1 : 1);
					projectile->y -= 8 - projectile->emitoffset;
					projectile->yv = velocity;
				break;
				case 25: // airshoot down/left
					projectile->x -= 31 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 18 - (projectile->emitoffset * 0.70710678118655);
					projectile->yv = velocity * 0.70710678118655;
					projectile->xv = -velocity * 0.70710678118655;
				break;
				case 26: // airshoot left
					projectile->x -= 48 + projectile->emitoffset;
					projectile->y -= 53;
					projectile->xv = -velocity;
				break;
				case 27: // airshoot up/left
					projectile->x -= 34 + (projectile->emitoffset * 0.70710678118655);
					projectile->y -= 87 + (projectile->emitoffset * 0.70710678118655);
					projectile->yv = -velocity * 0.70710678118655;
					projectile->xv = -velocity * 0.70710678118655;
				break;
			}
			if(world.map.TestAABB(projectile->x, projectile->y, projectile->x, projectile->y, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN)){
				laserammo = oldlaserammo;
				rocketammo = oldrocketammo;
				flamerammo = oldflamerammo;
				if(flamersoundchannel != -1){
					world.audio.Stop(flamersoundchannel);
					flamersoundchannel = -1;
					state_i += 2;
				}
				world.DestroyObject(projectile->id);
				projectile = 0;
				currentprojectileid = 0;
				state_i--;
			}else{
				Peer * peer = GetPeer(world);
				if(peer){
					peer->stats.weaponfires[currentweapon]++;
				}
			}
		}
	}else{
		state_i--;
	}
	return projectile;
}

bool Player::ProcessJetpackState(World & world){
	if(currentplatformid && HandleAgainstWall(world)){
		return true;
	}
	currentplatformid = 0;
	if(fuellow){
		state = FALLING;
		fallingnudge = 0;
		state_i = -1;
		return true;
	}
	if(fuel == maxfuel){
		fuel--;
	}
	if(jetpacksoundchannel == -1){
		jetpacksoundchannel = EmitSound(world, world.resources.soundbank["jetpak1.wav"], 64, true);
	}
	/*if(currentplatformid){
		y--;
		if(world.map.TestAABB(x, y - height, x, y, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true)){
			y++;
			state = STANDING;
			state_i = -1;
			return true;
		}
	}*/
	res_bank = 13;
	res_index = (state_i / 4) + 1;
	if(res_index > 6){
		res_index = 6;
	}
	if(state_i >= (12 * 4)){
		state_i = 12 * 4;
	}
	if(input.keymoveleft){
		if(state_i % 2 == 0){
			xv += -1;
		}
		mirrored = true;
	}
	if(input.keymoveright){
		if(state_i % 2 == 0){
			xv += 1;
		}
		mirrored = false;
	}
	if(state_i % 2 == 0){
		yv -= 1;
	}
	int xv2 = xv;
	int yv2 = -30;
	Platform * platform1 = world.map.TestIncr(x, y - height, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true);
	if(platform1 && y >= platform1->y2){
		//float close = ((float)yv2 / -30);
		yv = 0;
		//yv = (yv * close) + (10 * (1 - close)) + 5;
		//xv = xv / 2;
		state_i = -1;
	}
	Uint8 xvmax = 14;
	/*if(hassecret){
		xvmax = 12;
	}*/
	if(xv > xvmax){
		xv = xvmax;
	}
	if(xv < -xvmax){
		xv = -xvmax;
	}
	if(yv < -9){
		yv = -9;
	}
	//if(yv > 0){
	//	yv = 0;
	//}
	xv2 = xv;
	yv2 = yv;
	//y--;
	Platform * platform = world.map.TestIncr(x, y - height, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, 0, true);
	for(int i = 0; i < 3; i++){
		int xv2 = xv * (0.33 * i);
		int yv2 = yv * (0.33 * i);
		Plume * plume = (Plume *)world.CreateObject(ObjectTypes::PLUME);
		if(plume){
			plume->yv += 20;
			plume->renderpass = 1;
			plume->type = rand() % 2;
			plume->SetPosition(x + xv2 + (mirrored ? 10 : -10) + ((rand() % 3) - 1), y + yv2 - 40 + ((rand() % 3) - 1));
		}
	}
	if(CheckForLadder(world)){
		return true;
	}
	x = x + xv2;
	y = y + yv2;
	if(platform){
		if(CheckForGround(world, *platform)){
			return true;
		}else{
			float xn, yn;
			platform->GetNormal(x, y, &xn, &yn);
			if(xn){
				xv = (xn * abs(xv));
			}
			if(yn){
				yv = (yn * abs(yv));
			}
			xv /= 2;
			yv /= 2;
			if(abs(xv) >= 2 || abs(yv) >= 2){
				EmitSound(world, world.resources.soundbank["land1.wav"], 96);
			}
		}
	}
	if(yv > 0){
		yv = 0;
	}
	if(!input.keyjetpack){
		state = FALLING;
		fallingnudge = 0;
		state_i = 4 * 2;
		return true;
	}
	return false;
}

bool Player::ProcessFallingState(World & world){
	currentplatformid = 0;
	if(state_i / 2 < 2){
		res_bank = 12;
		res_index = state_i / 2;
	}else{
		res_bank = 13;
		res_index = state_i / 4;
	}
	if(IsDisguised()){
		res_bank = 123;
		res_index = 0;
	}
	if(state_i >= 6 * 4){
		state_i = 6 * 4;
	}
	if(input.keymoveleft){
		mirrored = true;
		fallingnudge--;
		if(fallingnudge < -8){
			fallingnudge = -8;
		}
	}
	if(input.keymoveright){
		mirrored = false;
		fallingnudge++;
		if(fallingnudge > 8){
			fallingnudge = 8;
		}
	}
	xv += fallingnudge / 2;
	int xv2 = xv;
	int yv2 = yv;
	Platform * platform = world.map.TestIncr(x, y - height, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
	if(platform){
		int yt = platform->XtoY(x);
		if(platform->y2 - platform->y1 <= 1 && y >= yt){
			xv2 = xv;
			yv2 = yv;
			platform = world.map.TestIncr(x, y - height, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, platform);
		}
	}
	if(platform){
		if(CheckForGround(world, *platform)){
			return true;
		}else{
			float xn, yn;
			x = x + xv2;
			y = y + yv2;
			platform->GetNormal(x, y, &xn, &yn);
			xv = (xn * abs(xv)) / 2;
			yv = (yn * abs(yv)) / 2;
			if(abs(xv) >= 2 || abs(yv) >= 2){
				EmitSound(world, world.resources.soundbank["fall2b.wav"], 96);
			}
		}
	}else{
		if(CheckForLadder(world)){
			return true;
		}
		x = x + xv;
		y = y + yv;
	}
	yv += world.gravity;
	if(yv > world.maxyvelocity){
		yv = world.maxyvelocity;
	}
	xv -= (fallingnudge / 2);
	return false;
}

bool Player::ProcessStandingState(World & world){
	if(!currentplatformid){
		// this may have introduced a bug where the player sometimes snaps to a different area
		if(!FindCurrentPlatform(*this, world)){
			state = FALLING;
			fallingnudge = 0;
			state_i = -1;
			return true;
		}
	}
	if((input.keymoveright || input.keymoveleft) && !input.keymovedown){
		state = RUNNING;
		state_i = -1;
		return true;
	}
	if(input.keymoveup || input.keymovedown){
		Sint16 yt = input.keymovedown ? 1 : -1;
		Platform * ladder = world.map.TestAABB(x - 20, y + yt, x + 20, y + yt, Platform::LADDER);
		if(ladder){
			Uint32 center = ((ladder->x2 - ladder->x1) / 2) + ladder->x1;
			if(abs(signed(center) - x) < 5){
				x = center;
				state = LADDER;
				state_i = -1;
				return true;
			}
			if(center < x){
				mirrored = true;
			}else{
				mirrored = false;
			}
			state = RUNNING;
			state_i = -1;
			return true;
		}
	}
	if(input.keymovedown && !IsDisguised()){
		state = CROUCHING;
		state_i = -1;
		return true;
	}
	return false;
}

bool Player::ProcessLadderState(World &world){
	xv = 0;
	if(input.keyfire && state != LADDERSHOOT){
		state = LADDERSHOOT;
		state_i = -1;
		return true;
	}
	if(input.keymoveleft){
		mirrored = true;
	}
	if(input.keymoveright){
		mirrored = false;
	}
	if((input.keyjump && !oldinput.keyjump) || (input.keymoveleft && input.keymoveright && input.keyactivate)){
		if(input.keymovedown){
			state = FALLING;
			fallingnudge = 0;
			state_i = 4 * 2;
			return true;
		}else{
			Uint8 xvmax = 14;
			if(hassecret){
				xvmax = 11;
			}
			xvmax -= 4;
			if(input.keymoveleft){
				xv -= xvmax;
				mirrored = true;
			}
			if(input.keymoveright){
				xv += xvmax;
				mirrored = false;
			}
			justjumpedfromladder = true;
			state = JUMPING;
			state_i = -1;
			return true;
		}
	}
	if(input.keyactivate && !oldinput.keyactivate){
		state = FALLING;
		fallingnudge = 0;
		state_i = 4 * 2;
		return true;
	}
	if(input.keyjetpack && !oldinput.keyjetpack && !fuellow){
		justjumpedfromladder = true;
		state = JETPACK;
		state_i = -1;
		return true;
	}
	int ye = yv;
	int xe = xv;
	Platform * platform = world.map.TestIncr(x, y, x, y, &xe, &ye, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
	Platform * currentladder = world.map.TestAABB(x, y, x, y, Platform::LADDER);
	bool forceclimbup = false;
	bool forceclimbdown = false;
	if(currentladder && !input.keymoveup && !input.keymovedown){
		if((input.keymoveright || input.keymoveleft) && y - currentladder->y1 < 20){
			forceclimbup = true;
		}
		if((input.keymoveright || input.keymoveleft) && currentladder->y2 - y < 20){
			forceclimbdown = true;
		}
	}
	if(input.keymoveup || forceclimbup){
		yv = -5;
	}else
	if(input.keymovedown || forceclimbdown){
		yv = 5;
	}else{
		yv = 0;
	}
	if(input.keymoveup || forceclimbup){
		Platform * ladder = world.map.TestAABB(x, y - height + yv, x, y + yv, Platform::LADDER);
		if(!ladder){
			if(platform){
				currentplatformid = platform->id;
				y = platform->XtoY(x);
				if(input.keymoveright || input.keymoveleft){
					state = RUNNING;
					state_i = -1;
					return true;
				}
				state = STANDING;
				state_i = -1;
				return true;
			}else{
				state = FALLING;
				fallingnudge = 0;
				state_i = 4 * 2;
				return true;
			}
		}
	}else
	if(input.keymovedown || forceclimbdown){
		Platform * ladder = world.map.TestAABB(x, y + yv, x, y + yv, Platform::LADDER);
		if(!ladder){
			if(platform){
				currentplatformid = platform->id;
				y = platform->XtoY(x);
				if(input.keymoveright || input.keymoveleft){
					state = RUNNING;
					state_i = -1;
					return true;
				}
				state = STANDING;
				state_i = -1;
				return true;
			}else{
				state = FALLING;
				fallingnudge = 0;
				state_i = 4 * 2;
				return true;
			}
		}
		state_i -= 2;
		if(state_i > 99){ // wrapped around
			if(IsDisguised()){
				state_i = 19;
			}else{
				state_i = 20;
			}
		}
	}else{
		state_i = 0;
	}
	y += yv;
	if(IsDisguised()){
		if(state_i > 19){
			state_i = 0;
		}
		// 124:0-19 civilian climbing ladder
		res_bank = 124;
	}else{
		if(state_i > 20){
			state_i = 0;
		}
		res_bank = 16;
	}
	res_index = state_i;
	if(res_index == 4){
		EmitSound(world, world.resources.soundbank["ladder1.wav"], 24);
	}
	if(res_index == 15){
		EmitSound(world, world.resources.soundbank["ladder2.wav"], 24);
	}
	return false;
}

bool Player::OnGround(void){
	switch(state){
		case STANDING:
		case RUNNING:
		case CROUCHING:
		case UNCROUCHING:
		case CROUCHED:
		case CROUCHEDSHOOT:
		case CROUCHEDTHROWING:
		case STANDINGSHOOT:
		case ROLLING:
		case HACKING:
		case THROWING:
			return true;
		break;
	}
	return false;
}

void Player::Warp(World & world, Sint16 x, Sint16 y){
	if(!state_warp){
		warpx = x;
		warpy = y;
		state_warp = 1;
		EmitSound(world, world.resources.soundbank["transrev.wav"], 96);
	}
}

bool Player::PickUpItem(World & world, PickUp & pickup){
	bool islocalplayer = false;
	if(this == world.GetPeerPlayer(world.localpeerid)){
		islocalplayer = true;
	}
	bool pickedup = true;
	Uint8 invtype = INV_NONE;
	const char * invname = "?";
	switch(pickup.type){
		case PickUp::SECRET:{
			if(!hassecret){
				hassecret = true;
				Team * team = GetTeam(world);
				if(team){
					team->playerwithsecret = id;
				}
				secretteamid = pickup.quantity;
				tracetime = pickup.tracetime;
				if(islocalplayer){
					world.ShowStatus("You picked up a secret!");
				}
			}else{
				pickedup = false;
			}
		}break;
		case PickUp::FILES:{
			int filespickedup = pickup.quantity;
			if(pickup.quantity > maxfiles){
				filespickedup = maxfiles - files;
				files += maxfiles;
				oldfiles = files;
			}else{
				files += pickup.quantity;
				oldfiles = files;
			}
			if(islocalplayer){
				char temp[256];
				sprintf(temp, "Picked up %d files", filespickedup);
				world.ShowStatus(temp);
			}
			effecthacking = true;
			effecthackingcontinue = 5;
			EmitSound(world, world.resources.soundbank["power11.wav"], 64);
		}break;
		case PickUp::LASERAMMO:{
			laserammo += pickup.quantity;
			if(laserammo > 30){
				laserammo = 30;
			}
			if(islocalplayer){
				char temp[256];
				sprintf(temp, "Picked up %d Laser ammo", pickup.quantity);
				world.ShowStatus(temp);
			}
			EmitSound(world, world.resources.soundbank["reload2.wav"], 96);
		}break;
		case PickUp::ROCKETAMMO:{
			rocketammo += pickup.quantity;
			if(rocketammo > 30){
				rocketammo = 30;
			}
			if(islocalplayer){
				char temp[256];
				sprintf(temp, "Picked up %d Rocket ammo", pickup.quantity);
				world.ShowStatus(temp);
			}
			EmitSound(world, world.resources.soundbank["reload2.wav"], 96);
		}break;
		case PickUp::FLAMERAMMO:{
			flamerammo += pickup.quantity;
			if(flamerammo > 75){
				flamerammo = 75;
			}
			if(islocalplayer){
				char temp[256];
				sprintf(temp, "Picked up %d Flamer ammo", pickup.quantity);
				world.ShowStatus(temp);
			}
			EmitSound(world, world.resources.soundbank["reload2.wav"], 96);
		}break;
		case PickUp::NEUTRONBOMB:{
			if(!pickup.powerup){
				invtype = INV_NEUTRONBOMB;
				invname = "Neutron Bomb";
			}
		}break;
		case PickUp::EMPBOMB:{
			invtype = INV_EMPBOMB;
			invname = "E.M.P. Bomb";
		}break;
		case PickUp::PLASMABOMB:{
			invtype = INV_PLASMABOMB;
			invname = "Plasma Bomb";
		}break;
		case PickUp::SHAPEDBOMB:{
			invtype = INV_SHAPEDBOMB;
			invname = "Shaped Bomb";
		}break;
		case PickUp::PLASMADET:{
			invtype = INV_PLASMADET;
			invname = "Plasma Detonator";
		}break;
		case PickUp::FIXEDCANNON:{
			invtype = INV_FIXEDCANNON;
			invname = "Fixed Cannon";
		}break;
		case PickUp::FLARE:{
			invtype = INV_FLARE;
			invname = "Flare";
		}break;
		case PickUp::CAMERA:{
			invtype = INV_CAMERA;
			invname = "Camera";
		}break;
	}
	if(invtype != INV_NONE){
		bool added = false;
		for(int i = 0; i < pickup.quantity; i++){
			if(AddInventoryItem(invtype)){
				added = true;
			}
		}
		if(added){
			EmitSound(world, world.resources.soundbank["reload2.wav"], 96);
			if(islocalplayer){
				char temp[256];
				sprintf(temp, "Picked up %d %s", pickup.quantity, invname);
				world.ShowStatus(temp);
			}
		}else{
			pickedup = false;
		}
	}
	if(pickedup){
		if(pickup.powerup){
			const char * powerupname = "?";
			switch(pickup.type){
				case PickUp::SUPERSHIELD:{
					powerupname = "You've gained the Super Shield!";
					shield = maxshield * 2;
				}break;
				case PickUp::NEUTRONBOMB:{
					powerupname = "You've acquired a Neutron Bomb!";
					AddInventoryItem(INV_NEUTRONBOMB);
				}break;
				case PickUp::JETPACK:{
					powerupname = "Extra Jetpack propellant!";
					jetpackbonustime = world.tickcount + (20 * 24);
					fuel = maxfuel;
					fuellow = false;
				}break;
				case PickUp::HACKING:{
					powerupname = "Double hacking until codes changed!";
					hackingbonustime = world.tickcount + (40 * 24);
				}break;
				case PickUp::RADAR:{
					powerupname = "You can see your enemies on radar!";
					radarbonustime = world.tickcount + (30 * 24);
				}break;
			}
			Peer * peer = GetPeer(world);
			if(peer){
				peer->stats.powerupspickedup++;
			}
			if(islocalplayer){
				char temp[256];
				sprintf(temp, "%s", powerupname);
				world.ShowStatus(temp);
				EmitSound(world, world.resources.soundbank["power11.wav"], 96);
			}
		}
		/*	pickup.draw = false;
			pickup.quantity = pickup.poweruprespawntime;
		}else{
			pickup.draw = false;
			pickup.collidable = false;
			world.MarkDestroyObject(pickup.id);
		}*/
		return true;
	}
	return false;
}

Peer * Player::GetPeer(World & world){
	for(int i = 0; i < world.maxpeers; i++){
		Peer * peer = world.peerlist[i];
		if(peer){
			for(std::list<Uint16>::iterator it = peer->controlledlist.begin(); it != peer->controlledlist.end(); it++){
				Object * object = world.GetObjectFromId(*it);
				if(object){
					if(object->id == id){
						return peer;
					}
				}
			}
		}
	}
	return 0;
}

void Player::SetToRespawnPosition(World & world){
	xv = 0;
	yv = 0;
	Team * team = GetTeam(world);
	if(team){
		basedoorentering = team->basedoorid;
		y = ((team->number) * 26 * 64) + 640 + (world.map.height * 64) + 718;
		x = 895;
		FindCurrentPlatform(*this, world);
		/*Sint32 xv2 = 0;
		Sint32 yv2 = 1;
		Platform * platform = world.map.TestIncr(x, y - height, x, y, &xv2, &yv2, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
		if(platform){
			currentplatformid = platform->id;
		}*/
	}
}

void Player::DropAllItems(World & world){
	if(hassecret){
		PickUp * pickup = DropItem(world, PickUp::SECRET, secretteamid);
		if(pickup){
			pickup->tracetime = tracetime;
			tracetime = 0;
		}
		Team * team = GetTeam(world);
		if(team){
			team->playerwithsecret =  0;
		}
		hassecret = false;
	}
	if(files > 0){
		DropItem(world, PickUp::FILES, files);
		files = 0;
		oldfiles = 0;
	}
	if(laserammo > 0){
		DropItem(world, PickUp::LASERAMMO, laserammo);
		laserammo = 0;
	}
	if(rocketammo > 0){
		DropItem(world, PickUp::ROCKETAMMO, rocketammo);
		rocketammo = 0;
	}
	if(flamerammo > 0){
		DropItem(world, PickUp::FLAMERAMMO, flamerammo);
		flamerammo = 0;
	}
	for(int i = 3; i >= 0; i--){
		if(inventoryitems[i]){
			Uint8 num = inventoryitemsnum[i];
			Uint8 dropquantity = num;
			Uint8 droptype = PickUp::NONE;
			switch(inventoryitems[i]){
				case INV_EMPBOMB:{
					droptype = PickUp::EMPBOMB;
				}break;
				case INV_SHAPEDBOMB:{
					droptype = PickUp::SHAPEDBOMB;
				}break;
				case INV_PLASMABOMB:{
					droptype = PickUp::PLASMABOMB;
				}break;
				case INV_NEUTRONBOMB:{
					droptype = PickUp::NEUTRONBOMB;
				}break;
				case INV_PLASMADET:{
					droptype = PickUp::PLASMADET;
				}break;
				case INV_FIXEDCANNON:{
					droptype = PickUp::FIXEDCANNON;
				}break;
				case INV_FLARE:{
					droptype = PickUp::FLARE;
				}break;
				case INV_CAMERA:{
					droptype = PickUp::CAMERA;
				}break;
			}
			if(droptype != PickUp::NONE){
				DropItem(world, droptype, dropquantity);
			}
			Team * team = GetTeam(world);
			if(!(inventoryitems[i] == INV_BASEDOOR && (team && !team->basedoorid))){
				for(int j = 0; j < num; j++){
					RemoveInventoryItem(inventoryitems[i]);
				}
			}
		}
	}
}

PickUp * Player::DropItem(World & world, Uint8 type, Uint16 quantity){
	PickUp * pickup = (PickUp *)world.CreateObject(ObjectTypes::PICKUP);
	if(pickup){
		pickup->type = type;
		pickup->x = x;
		pickup->y = y - 1;
		pickup->xv = (world.Random() % 9) - 4;
		pickup->yv = -15;
		pickup->quantity = quantity;
	}
	return pickup;
}

bool Player::BuyAvailable(World & world, Uint8 id){
	for(std::vector<BuyableItem *>::iterator it = world.buyableitems.begin(); it != world.buyableitems.end(); it++){
		BuyableItem * buyableitem = *it;
		if(buyableitem->id == id){
			Peer * peer = GetPeer(world);
			if(peer && peer->techchoices & buyableitem->techchoice){
				Team * team = GetTeam(world);
				if(buyableitem->agencyspecific == -1 || (team && buyableitem->agencyspecific == team->agency)){
					return true;
				}
			}
		}
	}
	/*switch(id){
		case World::BUY_LASER:
		case World::BUY_ROCKET:
		case World::BUY_FLAMER:
		case World::BUY_HEALTH:
		case World::BUY_EMPB:
		case World::BUY_SHAPEDB:
		case World::BUY_PLASMAB:
		case World::BUY_NEUTRONB:
		case World::BUY_DET:
		case World::BUY_FIXEDC:
		case World::BUY_FLARE:
		case World::BUY_DOOR:
		case World::BUY_DEFENSE:
		case World::BUY_INFO:
			return true;
		break;
	}*/
	if(id == World::BUY_GIVE0){
		Team * team = GetTeam(world);
		if(team && team->numpeers >= 1 && team->peers[0] != world.localpeerid){
			return true;
		}
	}
	if(id == World::BUY_GIVE1){
		Team * team = GetTeam(world);
		if(team && team->numpeers >= 2 && team->peers[1] != world.localpeerid){
			return true;
		}
	}
	if(id == World::BUY_GIVE2){
		Team * team = GetTeam(world);
		if(team && team->numpeers >= 3 && team->peers[2] != world.localpeerid){
			return true;
		}
	}
	if(id == World::BUY_GIVE3){
		Team * team = GetTeam(world);
		if(team && team->numpeers >= 4 && team->peers[3] != world.localpeerid){
			return true;
		}
	}
	return false;
}