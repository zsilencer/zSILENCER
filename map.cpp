#include <math.h>
#include <algorithm>
#include "map.h"
#include "platform.h"
#include "civilian.h"
#include "guard.h"
#include "robot.h"
#include "terminal.h"
#include "vent.h"
#include "healmachine.h"
#include "creditmachine.h"
#include "secretreturn.h"
#include "surveillancemonitor.h"
#include "techstation.h"
#include "inventorystation.h"
#include "teambillboard.h"
#include "overlay.h"
#include "team.h"
#include "warper.h"
#include "walldefense.h"
#include "pickup.h"
#include "baseexit.h"

Map::Map(){
	for(unsigned int i = 0; i < 4; i++){
		fg[i] = 0;
		bg[i] = 0;
		fgflipped[i] = 0;
		bgflipped[i] = 0;
		fglum[i] = 0;
		bglum[i] = 0;
	}
	width = 0;
	height = 0;
	ambience = 0;
	memset(description, 0, sizeof(description));
	loaded = false;
	nodetypes = 0;
}

Map::~Map(){
	Unload();
}

Map::Header::Header(){
	firstbyte = 0;
	version = 0;
	maxplayers = 0;
	maxteams = 0;
	width = 0;
	height = 0;
	parallax = 0;
	ambience = 0;
	flags = 0;
	memset(description, 0, sizeof(description));
	minimapcompressedsize = 0;
	levelsize = 0;
}

bool Map::Load(const char * filename, World & world){
	Unload();
	bool result = LoadFile(filename, world, 0);
	if(result){
		std::vector<Team *> teams;
		for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
			Object * object = *it;
			if(object->type == ObjectTypes::TEAM){
				teams.push_back(static_cast<Team *>(object));
			}
		}
		bool result2 = false;
		//Uint32 yoffset = height + 10;
		for(std::vector<Team *>::iterator it = teams.begin(); it != teams.end(); it++){
			result2 = LoadBase(*(*it), world);
			//result2 = LoadFile("XBASE15A.SIL", world, yoffset);
			//yoffset += 26 + 10;
		}
		if(result2){
			loaded = true;
			// Put stairs at the beginning so that they collide first in tests, otherwise players can sometimes hit rectangle directly under triangle edge and start walking on it
			std::sort(platforms.begin(), platforms.end(), CompareType);
			CalculateAdjacentPlatforms();
			CalculatePlatformSets();
			CalculateNodes();
			CalculatePlatformSetConnections();
			CalculateRainPuddleLocations();
			return true;
		}
	}
	return false;
}

bool Map::LoadBase(Team & team, World & world){
	return LoadFile("XBASE15A.SIL", world, &team);
}

bool Map::LoadHeader(SDL_RWops * file, Map::Header & header){
	Uint8 padding = 0;
	if(!SDL_RWread(file, &header.firstbyte, 1, 1)){ return false; }
	if(!SDL_RWread(file, &header.version, 1, 1)){ return false; }
	if(!SDL_RWread(file, &header.maxplayers, 1, 1)){ return false; }
	if(!SDL_RWread(file, &header.maxteams, 1, 1)){ return false; }
	if(!SDL_RWread(file, &header.width, 2, 1)){ return false; }
	if(!SDL_RWread(file, &header.height, 2, 1)){ return false; }
	header.width = SDL_SwapBE16(header.width);
	header.height = SDL_SwapBE16(header.height);
	if(!SDL_RWread(file, &padding, 1, 1)){ return false; }
	if(!SDL_RWread(file, &header.parallax, 1, 1)){ return false; }
	if(!SDL_RWread(file, &header.ambience, 1, 1)){ return false; }
	//ambience -= 128;
	if(!SDL_RWread(file, &padding, 1, 1)){ return false; }
	if(!SDL_RWread(file, &padding, 1, 1)){ return false; }
	if(!SDL_RWread(file, &header.flags, 4, 1)){ return false; }
	header.flags = SDL_SwapBE32(header.flags);
	if(!SDL_RWread(file, &header.description, 1, 0x80)){ return false; }
	header.description[0x80 - 1] = 0;
	if(!SDL_RWread(file, &header.minimapcompressedsize, 4, 1)){ return false; }
	header.minimapcompressedsize = SDL_SwapLE32(header.minimapcompressedsize);
	if(!SDL_RWread(file, header.minimapcompressed, 1, header.minimapcompressedsize)){ return false; }
	if(!SDL_RWread(file, &header.levelsize, 4, 1)){ return false; }
	header.levelsize = SDL_SwapLE32(header.levelsize);
	return true;
}

bool Map::UncompressMinimap(Uint8 (*pixels)[172 * 62], const Uint8 * compressed, int compressedsize){
	unsigned long minimapsizeuncompressed = sizeof(*pixels);
	if(uncompress(*pixels, &minimapsizeuncompressed, compressed, compressedsize) != Z_OK){
		return false;
	}
	return true;
}

bool Map::LoadFile(const char * filename, World & world, Team * team){
	SDL_RWops * file = SDL_RWFromFile(filename, "rb");
	if(!file){
		return false;
	}
	Header header;
	Uint8 levelcompressed[65535];
	Uint32 numactors = 0;
	Uint32 numplatforms = 0;
	Sint32 x1, y1, x2, y2, type1, type2 = 0;

	if(!LoadHeader(file, header)){
		return false;
	}
	if(!SDL_RWread(file, levelcompressed, 1, header.levelsize)){ return false; }
	SDL_RWclose(file);
	
	if(header.width > 256 || header.height > 256){
		return false;
	}
	
	Uint32 yoffset = 0;
	
	if(team == 0){ // team is 0 for main map
		expandedwidth = (header.width > 47 ? header.width : 47);
		expandedheight = header.height + ((26 + 10) * world.maxteams);
		Map::width = header.width;
		Map::height = header.height;
		/*if(parallax > 3){
			parallax = 3;
		}*/
		Map::parallax = header.parallax;
		Map::ambience = header.ambience;
		/*if(uncompress(minimap.pixels, &minimapsizeuncompressed, header.minimapcompressed, header.minimapcompressedsize) != Z_OK){
			return false;
		}*/
		if(!UncompressMinimap(&minimap.pixels, header.minimapcompressed, header.minimapcompressedsize)){
			return false;
		}
		minimap.Recolor(16 * 4);
		strcpy(Map::description, description);
		for(unsigned int i = 0; i < 4; i++){
			fg[i] = new Uint16[expandedwidth * expandedheight];
			bg[i] = new Uint16[expandedwidth * expandedheight];
			fgflipped[i] = new bool[expandedwidth * expandedheight];
			bgflipped[i] = new bool[expandedwidth * expandedheight];
			fglum[i] = new bool[expandedwidth * expandedheight];
			bglum[i] = new bool[expandedwidth * expandedheight];
			for(int j = 0; j < expandedwidth * expandedheight; j++){
				fg[i][j] = 0;
				bg[i][j] = 0;
				fgflipped[i][j] = false;
				bgflipped[i][j] = false;
				fglum[i][j] = false;
				bglum[i][j] = false;
			}
		}
	}else{
		baseambience = header.ambience;
		yoffset = height + 10 + (team->number * (26));
	}
	
	int result;
	unsigned int maxlevelsize = 0;
	Uint8 * level;
	do{
		maxlevelsize += 100000;
		level = new Uint8[maxlevelsize];
		unsigned long levelsizeuncompressed = maxlevelsize;
		
		result = uncompress(level, &levelsizeuncompressed, levelcompressed, header.levelsize);

		if(result != Z_OK){
			delete[] level;
		}
	}while(result == Z_BUF_ERROR);
	
	if(result != Z_OK){
		return false;
	}
	
	unsigned int i = 0;
	for(unsigned int y = yoffset; y < yoffset + header.height; y++){
		for(unsigned int x = 0; x < header.width; x++){
			bg[0][(y * expandedwidth) + x] = ((level[i + 0]) | (level[i + 1] << 8));
			bgflipped[0][(y * expandedwidth) + x] = level[i + 2] ? true : false;
			bglum[0][(y * expandedwidth) + x] = level[i + 3] ? true : false ;
			bg[1][(y * expandedwidth) + x] = ((level[i + 4]) | (level[i + 5] << 8));
			bgflipped[1][(y * expandedwidth) + x] = level[i + 6] ? true : false;
			bglum[1][(y * expandedwidth) + x] = level[i + 7] ? true : false;
			bg[2][(y * expandedwidth) + x] = ((level[i + 8]) | (level[i + 9] << 8));
			bgflipped[2][(y * expandedwidth) + x] = level[i + 10] ? true : false;
			bglum[2][(y * expandedwidth) + x] = level[i + 11] ? true : false;
			bg[3][(y * expandedwidth) + x] = ((level[i + 12]) | (level[i + 13] << 8));
			bgflipped[3][(y * expandedwidth) + x] = level[i + 14] ? true : false;
			bglum[3][(y * expandedwidth) + x] = level[i + 15] ? true : false;
			
			fg[0][(y * expandedwidth) + x] = ((level[i + 20]) | (level[i + 21] << 8));
			fgflipped[0][(y * expandedwidth) + x] = level[i + 22] ? true : false;
			fglum[0][(y * expandedwidth) + x] = level[i + 23] ? true : false;
			fg[1][(y * expandedwidth) + x] = ((level[i + 24]) | (level[i + 25] << 8));
			fgflipped[1][(y * expandedwidth) + x] = level[i + 26] ? true : false;
			fglum[1][(y * expandedwidth) + x] = level[i + 27] ? true : false;
			fg[2][(y * expandedwidth) + x] = ((level[i + 28]) | (level[i + 29] << 8));
			fgflipped[2][(y * expandedwidth) + x] = level[i + 30] ? true : false;
			fglum[2][(y * expandedwidth) + x] = level[i + 31] ? true : false;
			fg[3][(y * expandedwidth) + x] = ((level[i + 32]) | (level[i + 33] << 8));
			fgflipped[3][(y * expandedwidth) + x] = level[i + 34] ? true : false;
			fglum[3][(y * expandedwidth) + x] = level[i + 35] ? true : false;
			i += 36;
		}
	}
	i = header.width * header.height * 36;
	memcpy(&numactors, &level[i], sizeof(numactors));
	numactors = SDL_SwapLE32(numactors);
	i += sizeof(Uint32) + sizeof(Uint32);
	for(unsigned int a = 0; a < numactors; a++){
		Uint32 actorid;
		Uint32 actorx;
		Uint32 actory;
		Uint32 actordirection;
		Sint32 actortype;
		Uint32 actormatchid;
		Uint32 actorsubplane;
		Uint32 actorunknown;
		Uint32 actorsecurityid;
		memcpy(&actorid, &level[i], sizeof(actorid));
		actorid = SDL_SwapLE32(actorid);
		i += sizeof(actorid);
		memcpy(&actorx, &level[i], sizeof(actorx));
		actorx = SDL_SwapLE32(actorx);
		i += sizeof(actorx);
		memcpy(&actory, &level[i], sizeof(actory));
		actory = SDL_SwapLE32(actory);
		i += sizeof(actory);
		memcpy(&actordirection, &level[i], sizeof(actordirection));
		actordirection = SDL_SwapLE32(actordirection);
		i += sizeof(actordirection);
		memcpy(&actortype, &level[i], sizeof(actortype));
		actortype = SDL_SwapLE32(actortype);
		i += sizeof(actortype);
		memcpy(&actormatchid, &level[i], sizeof(actormatchid));
		actormatchid = SDL_SwapLE32(actormatchid);
		i += sizeof(actormatchid);
		memcpy(&actorsubplane, &level[i], sizeof(actorsubplane));
		actorsubplane = SDL_SwapLE32(actorsubplane);
		i += sizeof(actorsubplane);
		memcpy(&actorunknown, &level[i], sizeof(actorunknown));
		actorunknown = SDL_SwapLE32(actorunknown);
		i += sizeof(actorunknown);
		memcpy(&actorsecurityid, &level[i], sizeof(actorsecurityid));
		actorsecurityid = SDL_SwapLE32(actorsecurityid);
		i += sizeof(actorsecurityid);
		actory += yoffset * 64;
		//printf("(%u, %u) %d id:%u type:%d match:%u subp:%u unk:%x secid:%d\n", actorx, actory, actordirection, actorid, actortype, actormatchid, actorsubplane, actorunknown, actorsecurityid);
		
		switch(actorid){
			case 0:{
				// agent guard (has blaster)
				// 0: patrol
				// 1: guard
				if(world.SecurityIDCanSpawn(actorsecurityid)){
					Guard * guard = (Guard *)world.CreateObject(ObjectTypes::GUARD);
					if(guard){
						guard->x = actorx;
						guard->y = actory;
						guard->weapon = 0;
						guard->mirrored = actordirection ? true : false;
						guard->originalx = guard->x;
						guard->originaly = guard->y;
						guard->originalmirrored = guard->mirrored;
						if(actortype == 0){
							guard->patrol = true;
						}
					}
				}
			}break;
			case 1:{
				Civilian * civilian = (Civilian *)world.CreateObject(ObjectTypes::CIVILIAN);
				if(civilian){
					civilian->x = actorx;
					civilian->y = actory;
					switch(actortype){
						case 0:
							civilian->speed = 4;
						break;
						case 1:
							civilian->speed = 5;
						break;
					}
				}
			}break;
			case 2:{
				// captain guard (has laser)
				// 0: patrol
				// 1: guard
				if(world.SecurityIDCanSpawn(actorsecurityid)){
					if(actortype == 0 || actortype == 1){
						Guard * guard = (Guard *)world.CreateObject(ObjectTypes::GUARD);
						if(guard){
							guard->x = actorx;
							guard->y = actory;
							guard->weapon = 1;
							guard->mirrored = actordirection ? true : false;
							guard->originalx = guard->x;
							guard->originaly = guard->y;
							guard->originalmirrored = guard->mirrored;
							if(actortype == 0){
								guard->patrol = true;
							}
						}
					}
				}
			}break;
			case 3:{
				// trooper guard (has rocket)
				// 0: patrol
				// 1: guard
				// 2: magistrate's laser soldier
				// 3: magistrate's rocket soldier
				if(world.SecurityIDCanSpawn(actorsecurityid) && actortype <= 1){
					Guard * guard = (Guard *)world.CreateObject(ObjectTypes::GUARD);
					if(guard){
						guard->x = actorx;
						guard->y = actory;
						guard->weapon = 2;
						guard->mirrored = actordirection ? true : false;
						guard->originalx = guard->x;
						guard->originaly = guard->y;
						guard->originalmirrored = guard->mirrored;
						if(actortype == 0){
							guard->patrol = true;
						}
					}
				}
			}break;
			case 6:{
				// robot
				// 0: patrol
				// 1: guard
				if(world.SecurityIDCanSpawn(actorsecurityid)){
					Robot * robot = (Robot *)world.CreateObject(ObjectTypes::ROBOT);
					if(robot){
						robot->x = actorx;
						robot->y = actory;
						if(actortype == 0){
							robot->patrol = true;
						}
						robot->originalx = robot->x;
						robot->originaly = robot->y;
					}
				}
			}break;
			case 36:{
				// Player start location
				XY xy;
				xy.x = actorx;
				xy.y = actory;
				playerstartlocations.push_back(xy);
			}break;
			case 37:{
				// surveillance camera
				XY xy;
				xy.x = actorx;
				xy.y = actory;
				surveillancecameras.push_back(xy);
			}break;
			case 47:{
				// 49:0 is small candle doodad
				// 50:0 is large candle doodad
				// 51:0 is small canister doodad
				// 52:0 is large canister doodad
				// 53:0 is downward arrow poster doodad
				// 54:0-9 is man in tank doodad
				Overlay * overlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
				if(overlay){
					switch(actortype){
						case 0:
							overlay->res_bank = 49;
						break;
						case 1:
							overlay->res_bank = 50;
						break;
						case 2:
							overlay->res_bank = 51;
						break;
						case 3:
							overlay->res_bank = 52;
						break;
						case 4:
							overlay->res_bank = 53;
						break;
						case 5:
							overlay->res_bank = 54;
						break;
						case 6:
							overlay->res_bank = 55;
						break;
						case 7:
							overlay->res_bank = 56;
						break;
						case 8:
							overlay->res_bank = 57;
						break;
						case 9:
							overlay->res_bank = 58;
						break;
						default:
							//overlay->res_bank = 53;
						break;
					}
					overlay->res_index = 0;
					overlay->x = actorx;
					overlay->y = actory;
				}
			}break;
			case 50:{
				// surveillance monitor
				SurveillanceMonitor * surveillancemonitor = (SurveillanceMonitor *)world.CreateObject(ObjectTypes::SURVEILLANCEMONITOR);
				if(surveillancemonitor){
					// 4:1   5:1   6:2,4   7:0
					switch(actortype){
						case 4:
						case 5:{
							surveillancemonitor->SetSize(0);
						}break;
						case 6:
							surveillancemonitor->SetSize(1);
						break;
						case 7:
							surveillancemonitor->SetSize(2);
							if(team){
								SurveillanceMonitor * surv1 = (SurveillanceMonitor *)world.CreateObject(ObjectTypes::SURVEILLANCEMONITOR);
								if(surv1){
									surv1->SetSize(10);
									surv1->x = actorx;
									surv1->y = actory;
									surv1->teamid = team->id;
								}
								SurveillanceMonitor * surv2 = (SurveillanceMonitor *)world.CreateObject(ObjectTypes::SURVEILLANCEMONITOR);
								if(surv2){
									surv2->SetSize(11);
									surv2->x = actorx;
									surv2->y = actory;
									surv2->teamid = team->id;
								}
								SurveillanceMonitor * surv3 = (SurveillanceMonitor *)world.CreateObject(ObjectTypes::SURVEILLANCEMONITOR);
								if(surv3){
									surv3->SetSize(12);
									surv3->x = actorx;
									surv3->y = actory;
									surv3->teamid = team->id;
								}
							}
						break;
					}
					surveillancemonitor->x = actorx;
					surveillancemonitor->y = actory;
					if(team){
						surveillancemonitor->teamid = team->id;
					}
				}
			}break;
			case 54:{
				Terminal * terminal = (Terminal *)world.CreateObject(ObjectTypes::TERMINAL);
				if(terminal){
					terminal->x = actorx;
					terminal->y = actory;
					if(actortype == 0){
						terminal->SetSize(0);
					}else{
						terminal->SetSize(1);
					}
				}
				/*if(actortype != 0){
					BigTerminal * bigterminal = (BigTerminal *)world.CreateObject(ObjectTypes::BIGTERMINAL);
					if(bigterminal){
						bigterminal->x = actorx;
						bigterminal->y = actory;
					}
				}else{
					SmallTerminal * smallterminal = (SmallTerminal *)world.CreateObject(ObjectTypes::SMALLTERMINAL);
					if(smallterminal){
						smallterminal->x = actorx;
						smallterminal->y = actory;
					}
				}*/
			}break;
			case 56:{
				// inventory station
				InventoryStation * inventorystation = (InventoryStation *)world.CreateObject(ObjectTypes::INVENTORYSTATION);
				if(inventorystation){
					inventorystation->x = actorx;
					inventorystation->y = actory;
					if(team){
						inventorystation->teamid = team->id;
					}
				}
			}break;
			case 57:{
				HealMachine * healmachine = (HealMachine *)world.CreateObject(ObjectTypes::HEALMACHINE);
				if(healmachine){
					healmachine->x = actorx;
					healmachine->y = actory;
				}
			}break;
			case 58:{
				// secret return
				SecretReturn * secretreturn = (SecretReturn *)world.CreateObject(ObjectTypes::SECRETRETURN);
				if(secretreturn){
					secretreturn->x = actorx;
					secretreturn->y = actory;
					if(team){
						secretreturn->teamid = team->id;
					}
				}
			}break;
			case 61:{
				Warper * warper = (Warper *)world.CreateObject(ObjectTypes::WARPER);
				if(warper){
					warper->actormatch = actormatchid;
					warper->x = actorx;
					warper->y = actory;
					warper->FindMatch(world);
				}
			}break;
			case 63:{
				// powerup
				// 0: super shield
				// 1: neutron bomb
				// 2: jet pack
				// 3: invisible
				// 4: hacking bonus
				// 5: see enemies
				// 6: neutron depositor
				bool valid = false;
				switch(actortype){
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
						valid = true;
					break;
				}
				if(valid){
					PickUp * pickup = (PickUp *)world.CreateObject(ObjectTypes::PICKUP);
					if(pickup){
						pickup->powerup = true;
						switch(actortype){
							default:
							case 0:
								pickup->type = PickUp::SUPERSHIELD;
							break;
							case 1:
								pickup->type = PickUp::NEUTRONBOMB;
							break;
							case 2:
								pickup->type = PickUp::JETPACK;
							break;
							case 3:
								pickup->type = PickUp::INVISIBLE;
							break;
							case 4:
								pickup->type = PickUp::HACKING;
							break;
							case 5:
								pickup->type = PickUp::RADAR;
							break;
							case 6:
								pickup->type = PickUp::DEPOSITOR;
							break;
						}
						pickup->x = actorx;
						pickup->y = actory;
						pickup->draw = false;
						pickup->poweruprespawntime = 60;
						pickup->quantity = pickup->poweruprespawntime;
					}
				}
			}break;
			case 64:{
				Vent * vent = (Vent *)world.CreateObject(ObjectTypes::VENT);
				if(vent){
					vent->x = actorx;
					vent->y = actory;
				}
			}break;
			case 65:{
				// base exit?
				BaseExit * baseexit = (BaseExit *)world.CreateObject(ObjectTypes::BASEEXIT);
				if(baseexit){
					baseexit->x = actorx;
					baseexit->y = actory;
					if(team){
						baseexit->teamid = team->id;
					}
				}
			}break;
			case 66:{
				TechStation * techstation = (TechStation *)world.CreateObject(ObjectTypes::TECHSTATION);
				if(techstation){
					techstation->x = actorx;
					techstation->y = actory;
					techstation->type = actortype;
					if(team){
						techstation->teamid = team->id;
					}
				}
			}break;
			case 67:{
				// 0: base defense
				// 1: guard defense
				if(world.SecurityIDCanSpawn(actorsecurityid) || actortype == 0){
					WallDefense * walldefense = (WallDefense *)world.CreateObject(ObjectTypes::WALLDEFENSE);
					if(walldefense){
						walldefense->x = actorx;
						walldefense->y = actory;
						if(team){
							walldefense->teamid = team->id;
						}else{
							walldefense->AddDefense();
						}
					}
				}
			}break;
			case 68:{
				// team billboard
				TeamBillboard * teambillboard = (TeamBillboard *)world.CreateObject(ObjectTypes::TEAMBILLBOARD);
				if(teambillboard){
					teambillboard->x = actorx;
					teambillboard->y = actory;
					if(team){
						teambillboard->agency = team->agency;
						teambillboard->teamid = team->id;
					}else{
						teambillboard->agency = 0;
					}
				}
				SurveillanceMonitor * surveillancemonitor = (SurveillanceMonitor *)world.CreateObject(ObjectTypes::SURVEILLANCEMONITOR);
				surveillancemonitor->x = actorx;
				surveillancemonitor->y = actory;
				surveillancemonitor->SetSize(3);
				surveillancemonitor->drawscreen = false;
				if(team){
					surveillancemonitor->teamid = team->id;
				}
			}break;
			case 69:{
				// computer doodad thing
				Overlay * overlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
				if(overlay){
					overlay->res_bank = 171;
					overlay->res_index = 0;
					overlay->x = actorx;
					overlay->y = actory;
				}
			}break;
			case 70:{
				CreditMachine * creditmachine = (CreditMachine *)world.CreateObject(ObjectTypes::CREDITMACHINE);
				if(creditmachine){
					creditmachine->x = actorx;
					creditmachine->y = actory;
				}
			}break;
		}
	}
	memcpy(&numplatforms, &level[i], sizeof(numplatforms));
	numplatforms = SDL_SwapLE32(numplatforms);
	i += sizeof(Uint32) + sizeof(Uint32);
	
	//FILE * fileo = fopen("platforms.txt", "w");
	for(unsigned int j = 0; j < numplatforms; j++){
		memcpy(&x1, &level[i], sizeof(x1));
		x1 = SDL_SwapLE32(x1);
		i += sizeof(Uint32);
		memcpy(&y1, &level[i], sizeof(y1));
		y1 = SDL_SwapLE32(y1);
		y1 += (yoffset * 64);
		i += sizeof(Uint32);
		memcpy(&x2, &level[i], sizeof(x2));
		x2 = SDL_SwapLE32(x2);
		i += sizeof(Uint32);
		memcpy(&y2, &level[i], sizeof(y2));
		y2 = SDL_SwapLE32(y2);
		y2 += (yoffset * 64);
		i += sizeof(Uint32);
		memcpy(&type1, &level[i], sizeof(type1));
		type1 = SDL_SwapLE32(type1);
		i += sizeof(Uint32);
		memcpy(&type2, &level[i], sizeof(type2));
		type2 = SDL_SwapLE32(type2);
		i += sizeof(Uint32);
		
		Uint8 type = 255;
		bool valid = false;
		if(type1 == 0 && type2 == 0){
			type = Platform::RECTANGLE;
			valid = true;
		}else
		if(type1 == 1 && type2 == 0){
			type = Platform::LADDER;
			valid = true;
		}else
		if(type1 == 0 && type2 == 1){
			type = Platform::STAIRSUP;
			valid = true;
		}else
		if(type1 == 0 && type2 == 2){
			type = Platform::STAIRSDOWN;
			valid = true;
		}else
		if(type1 == 2 && type2 == 0){
			type = Platform::TRACK;
			valid = true;
		}else
		if(type1 == 3 && type2 == 0){
			type = Platform::OUTSIDEROOM;
			valid = true;
		}else
		if(type1 == 3 && type2 == 1){
			type = Platform::SPECIFICROOM;
			valid = true;
		}else{
			//char temp[1234];
			//sprintf(temp, "TRACK: (%d, %d) (%d, %d)", x1, y1, x2, y2);
			//OutputDebugStringA(temp);
		}
		//printf("%d (%d, %d) (%d, %d)\n", type, x1, y1, x2, y2);
		
		if(valid){
			Platform * newplatform = new Platform(type, currentid, x1, y1, x2, y2);
			platformids[currentid] = newplatform;
			platforms.push_back(newplatform);
			currentid++;
		}
	}
	//fclose(fileo);
	
	//delete[] levelcompressed;
	//delete[] level;
	
	return true;
}

void Map::Unload(void){
	loaded = false;
	currentid = 1;
	platformids.clear();
	playerstartlocations.clear();
	surveillancecameras.clear();
	rainpuddlelocations.clear();
	for(unsigned int i = 0; i < 4; i++){
		if(fg[i]){
			delete[] fg[i];
			fg[i] = 0;
		}
		if(bg[i]){
			delete[] bg[i];
			bg[i] = 0;
		}
		if(fgflipped[i]){
			delete[] fgflipped[i];
			fgflipped[i] = 0;
		}
		if(bgflipped[i]){
			delete[] bgflipped[i];
			bgflipped[i] = 0;
		}
		if(fglum[i]){
			delete[] fglum[i];
			fglum[i] = 0;
		}
		if(bglum[i]){
			delete[] bglum[i];
			bglum[i] = 0;
		}
	}
	for(std::vector<Platform *>::iterator i = platforms.begin(); i != platforms.end(); i++){
		delete (*i);
	}
	platforms.clear();
	for(std::vector<PlatformSet *>::iterator i = platformsets.begin(); i != platformsets.end(); i++){
		delete (*i);
	}
	platformsets.clear();
	if(nodetypes){
		delete[] nodetypes;
		nodetypes = 0;
	}
}

void Map::MiniMapCoords(int & x, int & y){
	x = (x / float(width * 64)) * 172;
	y = (y / float(height * 64)) * 62;
}

void Map::RandomPlayerStartLocation(World & world, Sint16 & x, Sint16 & y){
	if(playerstartlocations.size() == 0){
		return;
	}
	int index = world.Random() % playerstartlocations.size();
	int i = 0;
	for(std::vector<XY>::iterator it = playerstartlocations.begin(); it != playerstartlocations.end(); it++){
		if(i == index){
			XY xy = (*it);
			x = xy.x;
			y = xy.y;
			return;
		}
		i++;
	}
}

void Map::CalculateRainPuddleLocations(void){
	for(unsigned int i = 0 ; i < platforms.size(); i++){
		Platform * platform = platforms[i];
		if(platform->type == Platform::RECTANGLE){
			for(int x = platform->x1 + 8; x < platform->x2 - 32; x += 32){
				if(TestAABB(x, platform->y1, x + 32, platform->y1, Platform::OUTSIDEROOM)){
					if(!TestAABB(x, platform->y1, x + 32, platform->y1, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN, platform)){
						XY xy;
						xy.x = x;
						xy.y = platform->y1 - 4;
						rainpuddlelocations.push_back(xy);
					}
				}
			}
		}
	}
}

void Map::CalculateAdjacentPlatforms(void){
	for(unsigned int i = 0 ; i < platforms.size(); i++){
		switch(platforms[i]->type){
			case Platform::RECTANGLE:
				for(unsigned int j = 0; j < platforms.size(); j++){
					if(platforms[i] == platforms[j]){
						continue;
					}
					switch(platforms[j]->type){
						case Platform::RECTANGLE:
							if(!platforms[i]->adjacentl){
								if(platforms[j]->x2 == platforms[i]->x1 && platforms[j]->y1 == platforms[i]->y1){
									if(!TestAABB(platforms[j]->x2, platforms[j]->y1 - 1, platforms[j]->x2, platforms[j]->y1 - 1, Platform::RECTANGLE)){
										platforms[i]->adjacentl = platforms[j];
									}
								}
							}
							if(!platforms[i]->adjacentr){
								if(platforms[j]->x1 == platforms[i]->x2 && platforms[j]->y1 == platforms[i]->y1){
									if(!TestAABB(platforms[j]->x1, platforms[j]->y1 - 1, platforms[j]->x1, platforms[j]->y1 - 1, Platform::RECTANGLE)){
										platforms[i]->adjacentr = platforms[j];
									}
								}
							}
						break;
						case Platform::STAIRSUP:
							if(platforms[j]->x2 == platforms[i]->x1 && platforms[j]->y1 == platforms[i]->y1){
								platforms[i]->adjacentl = platforms[j];
							}
							if(platforms[j]->x1 == platforms[i]->x2 && platforms[j]->y2 == platforms[i]->y1){
								platforms[i]->adjacentr = platforms[j];
							}
						break;
						case Platform::STAIRSDOWN:
							if(platforms[j]->x2 == platforms[i]->x1 && platforms[j]->y2 == platforms[i]->y1){
								platforms[i]->adjacentl = platforms[j];
							}
							if(platforms[j]->x1 == platforms[i]->x2 && platforms[j]->y1 == platforms[i]->y1){
								platforms[i]->adjacentr = platforms[j];
							}
						break;
					}
				}
			break;
			case Platform::STAIRSUP:
				for(unsigned int j = 0; j < platforms.size(); j++){
					switch(platforms[j]->type){
						case Platform::RECTANGLE:
							if(!platforms[i]->adjacentl){
								if(platforms[j]->x2 == platforms[i]->x1 && platforms[j]->y1 == platforms[i]->y2){
									platforms[i]->adjacentl = platforms[j];
								}
							}
							if(!platforms[i]->adjacentr){
								if(platforms[j]->x1 == platforms[i]->x2 && platforms[j]->y1 == platforms[i]->y1){
									platforms[i]->adjacentr = platforms[j];
								}
							}
						break;
						case Platform::STAIRSUP:
							if(platforms[j]->x2 == platforms[i]->x1 && platforms[j]->y1 == platforms[i]->y2){
								platforms[i]->adjacentl = platforms[j];
							}
							if(platforms[j]->x1 == platforms[i]->x2 && platforms[j]->y2 == platforms[i]->y1){
								platforms[i]->adjacentr = platforms[j];
							}
						break;
						case Platform::STAIRSDOWN:
							if(platforms[j]->x2 == platforms[i]->x1 && platforms[j]->y2 == platforms[i]->y2){
								platforms[i]->adjacentl = platforms[j];
							}
							if(platforms[j]->x1 == platforms[i]->x2 && platforms[j]->y1 == platforms[i]->y1){
								platforms[i]->adjacentr = platforms[j];
							}
						break;
					}
				}
			break;
			case Platform::STAIRSDOWN:
				for(unsigned int j = 0; j < platforms.size(); j++){
					switch(platforms[j]->type){
						case Platform::RECTANGLE:
							if(!platforms[i]->adjacentl){
								if(platforms[j]->x2 == platforms[i]->x1 && platforms[j]->y1 == platforms[i]->y1){
									platforms[i]->adjacentl = platforms[j];
								}
							}
							if(!platforms[i]->adjacentr){
								if(platforms[j]->x1 == platforms[i]->x2 && platforms[j]->y1 == platforms[i]->y2){
									platforms[i]->adjacentr = platforms[j];
								}
							}
						break;
						case Platform::STAIRSUP:
							if(platforms[j]->x2 == platforms[i]->x1 && platforms[j]->y1 == platforms[i]->y1){
								platforms[i]->adjacentl = platforms[j];
							}
							if(platforms[j]->x1 == platforms[i]->x2 && platforms[j]->y2 == platforms[i]->y2){
								platforms[i]->adjacentr = platforms[j];
							}
						break;
						case Platform::STAIRSDOWN:
							if(platforms[j]->x2 == platforms[i]->x1 && platforms[j]->y2 == platforms[i]->y1){
								platforms[i]->adjacentl = platforms[j];
							}
							if(platforms[j]->x1 == platforms[i]->x2 && platforms[j]->y1 == platforms[i]->y2){
								platforms[i]->adjacentr = platforms[j];
							}
						break;
					}
				}
			break;
		}
	}
}

void Map::CalculatePlatformSets(void){
	for(unsigned int i = 0 ; i < platforms.size(); i++){
		switch(platforms[i]->type){
			case Platform::RECTANGLE:
			case Platform::STAIRSUP:
			case Platform::STAIRSDOWN:
				Platform & leftmost = GetLeftmostPlatform(*platforms[i]);
				if(!leftmost.set){
					PlatformSet * platformset = new PlatformSet;
					Platform * platform = &leftmost;
					do{
						if(platform->set){
							break;
						}
						platformset->platforms.push_back(platform);
						platformset->length += platform->GetLength();
						platform->set = platformset;
						platform = platform->adjacentr;
					}while(platform);
					platformsets.push_back(platformset);
				}
			break;
		}
	}
	for(unsigned int i = 0 ; i < platforms.size(); i++){
		if(platforms[i]->type == Platform::LADDER){
			Platform * ladder = platforms[i];
			Sint16 center = abs(ladder->x2 - ladder->x1) + ladder->x1;
			Platform * top = TestAABB(center, ladder->y1, center, ladder->y1, Platform::RECTANGLE);
			if(top){
				PlatformSet * set = top->set;
				if(set){
					set->ladders.push_back(ladder);
				}
			}
			Platform * bottom = TestAABB(center, ladder->y2, center, ladder->y2, Platform::RECTANGLE);
			if(bottom){
				PlatformSet * set = bottom->set;
				if(set){
					set->ladders.push_back(ladder);
				}
			}
		}
	}
}

void Map::CalculatePlatformSetConnections(void){
	
}

void Map::CalculateNodes(void){
	if(nodetypes){
		delete nodetypes;
	}
	nodetypes = new Uint8[expandedwidth * expandedheight];
	for(int y = 0; y < expandedheight; y++){
		for(int x = 0; x < expandedwidth; x++){
			Platform * platform = TestAABB((x * 64) + 16, (y * 64) + 16, (x * 64) + 48, (y * 64) + 48, Platform::RECTANGLE | Platform::STAIRSDOWN | Platform::STAIRSUP | Platform::LADDER);
			if(platform){
				nodetypes[(y * expandedwidth) + x] = platform->type;
			}else{
				nodetypes[(y * expandedwidth) + x] = 0;
			}
		}
	}
}

int Map::TeamNumberFromY(Sint16 y){
	if(y < (height + 10) * 64){
		return -1; // in main map
	}
	// returns the team number of the base at the y position
	return (y - ((height + 10) * 64)) / (26 * 64);
}

int Map::AdjacentPlatformsLength(Platform & platform){
	Platform * leftmost = &GetLeftmostPlatform(platform);
	int length = 0;
	while(leftmost){
		length += (leftmost->x2 - leftmost->x1);
		leftmost = leftmost->adjacentr;
	}
	return length;
}

Platform & Map::GetLeftmostPlatform(Platform & platform){
	Platform * leftmost = &platform;
	while(leftmost->adjacentl){
		leftmost = leftmost->adjacentl;
	}
	return *leftmost;
}

Platform & Map::GetRightmostPlatform(Platform & platform){
	Platform * rightmost = &platform;
	while(rightmost->adjacentr){
		rightmost = rightmost->adjacentr;
	}
	return *rightmost;
}

std::vector<Platform *> Map::LaddersToPlatform(PlatformSet & from, PlatformSet & to){
	std::vector<Platform *> ladders;
	for(std::vector<Platform *>::iterator it = from.ladders.begin(); it != from.ladders.end(); it++){
		Platform * ladder = *it;
		std::vector<Platform *>::iterator found = std::find(to.ladders.begin(), to.ladders.end(), ladder);
		if(found != to.ladders.end()){
			ladders.push_back(ladder);
		}
	}
	return ladders;
}

Platform * Map::TestAABB(int x1, int y1, int x2, int y2, Uint8 type, Platform * except, bool ignorethin){
	for(std::vector<Platform *>::iterator i = platforms.begin(); i != platforms.end(); i++){
		Platform * platform = (*i);
		if(platform != except){
			if(TestAABB(x1, y1, x2, y2, platform, type, ignorethin)){
				return platform;
			}
		}
	}
	return 0;
}

bool Map::TestAABB(int x1, int y1, int x2, int y2, Platform * platform, Uint8 type, bool ignorethin){
	if(platform->type & type){
		switch(platform->type){
			case Platform::OUTSIDEROOM:
			case Platform::SPECIFICROOM:
			case Platform::LADDER:
			case Platform::RECTANGLE:{
				if(((x1 <= platform->x1 && x2 >= platform->x1) || (x1 <= platform->x2 && x2 >= platform->x2) || (x1 >= platform->x1 && x2 <= platform->x2)) &&
				   ((y1 <= platform->y1 && y2 >= platform->y1) || (y1 <= platform->y2 && y2 >= platform->y2) || (y1 >= platform->y1 && y2 <= platform->y2))){
					if(ignorethin && (platform->y1 == platform->y2)){
						return false;
					}
					return true;
				}
			}break;
			case Platform::STAIRSUP:{
				int edgex = platform->x1 - platform->x2;
				int edgey = platform->y2 - platform->y1;
				int normalx = edgey;
				int normaly = -edgex;
				int min1 = (x1 * normalx) + (y1 * normaly);
				int max1 = (x2 * normalx) + (y2 * normaly);
				int min2 = (platform->x2 * normalx) + (platform->y1 * normaly);
				int max2 = (platform->x2 * normalx) + (platform->y2 * normaly);
				if(((x1 <= platform->x1 && x2 >= platform->x1) || (x1 <= platform->x2 && x2 >= platform->x2) || (x1 >= platform->x1 && x2 <= platform->x2)) &&
				   ((y1 <= platform->y1 && y2 >= platform->y1) || (y1 <= platform->y2 && y2 >= platform->y2) || (y1 >= platform->y1 && y2 <= platform->y2)) &&
				   (max1 >= min2 && min1 <= max2)){
					return true;
				}
			}break;
			case Platform::STAIRSDOWN:{
				int edgex = platform->x2 - platform->x1;
				int edgey = platform->y2 - platform->y1;
				int normalx = edgey;
				int normaly = -edgex;
				int min1 = (x1 * normalx) + (y2 * normaly);
				int max1 = (x2 * normalx) + (y1 * normaly);
				int min2 = (platform->x1 * normalx) + (platform->y2 * normaly);
				int max2 = (platform->x1 * normalx) + (platform->y1 * normaly);
				if(((x1 <= platform->x1 && x2 >= platform->x1) || (x1 <= platform->x2 && x2 >= platform->x2) || (x1 >= platform->x1 && x2 <= platform->x2)) &&
				   ((y1 <= platform->y1 && y2 >= platform->y1) || (y1 <= platform->y2 && y2 >= platform->y2) || (y1 >= platform->y1 && y2 <= platform->y2)) &&
				   (max1 >= min2 && min1 <= max2)){
					return true;
				}
			}break;
		}
	}
	return false;
}

Platform * Map::TestLine(int x1, int y1, int x2, int y2, int * xe, int * ye, Uint8 type){
	// This function is unstable when the tested line segment starts at a line segment comprising a
	// platform and must only be used for finding collision/distance when they are not touching.
	Platform * hit = 0;
	int xe1 = 0, ye1 = 0;
	for(std::vector<Platform *>::iterator i = platforms.begin(); i != platforms.end(); i++){
		Platform * platform = (*i);
		if(TestAABB(x1, y1, x2, y2, platform, type)){ // broadphase
			float x[4], y[4], xed = x2, yed = y2;
			int segments = 0;
			switch(platform->type){
				case Platform::LADDER:
				case Platform::RECTANGLE:{
					segments = 4;
					x[0] = (float)platform->x1; y[0] = (float)platform->y1;
					x[1] = (float)platform->x2; y[1] = (float)platform->y1;
					x[2] = (float)platform->x2; y[2] = (float)platform->y2;
					x[3] = (float)platform->x1; y[3] = (float)platform->y2;
				}break;
				case Platform::STAIRSUP:{
					segments = 2;
					x[0] = (float)platform->x1; y[0] = (float)platform->y2;
					x[1] = (float)platform->x2; y[1] = (float)platform->y1;
					x[2] = (float)platform->x2; y[2] = (float)platform->y2;
				}break;
				case Platform::STAIRSDOWN:{
					segments = 2;
					x[0] = (float)platform->x1; y[0] = (float)platform->y1;
					x[1] = (float)platform->x2; y[1] = (float)platform->y2;
					x[2] = (float)platform->x1; y[2] = (float)platform->y2;
				}break;
			}
			for(int i = 0; i < segments; i++){
				if(LineSegmentIntersection((float)x1, (float)y1, (float)x2, (float)y2, x[i], y[i], x[(i + 1) % segments], y[(i + 1) % segments], &xed, &yed)){
					if(abs(x1 - xed) <= abs((float)x1 - xe1) || abs(y1 - yed) <= abs((float)y1 - ye1)){
						xe1 = xed;
						ye1 = yed;
						if(xe){
							*xe = xe1;
						}
						if(ye){
							*ye = ye1;
						}
						hit = platform;
					}
				}
			}
		}
	}
	return hit;
}

Platform * Map::TestIncr(int x1, int y1, int x2, int y2, int * xv, int * yv, Uint8 type, Platform * except, bool ignorethin){
	// broadphase
	std::vector<Platform *> test;
	for(std::vector<Platform *>::iterator i = platforms.begin(); i != platforms.end(); i++){
		Platform * platform = (*i);
		if(platform == except){
			continue;
		}
		int xb1 = x1 + (*xv < 0 ? *xv : 0);
		int yb1 = y1 + (*yv < 0 ? *yv : 0);
		int xb2 = x2 + (*xv > 0 ? *xv : 0);
		int yb2 = y2 + (*yv > 0 ? *yv : 0);
		/*int xb1 = x1 + *xv;
		int yb1 = y1 + *yv;
		int xb2 = x2 + *xv;
		int yb2 = y2 + *yv;*/
		if(TestAABB(xb1, yb1, xb2, yb2, platform, type, ignorethin)){
			test.push_back(platform);
		}
	}
	if(test.size() == 0){
		return 0;
	}
	int dx = *xv;
	int dy = *yv;
	int step;
	int error;
	int oldxv = *xv;
	int oldyv = *yv;
	int yv0 = 0;
	int xv0 = 0;
	*xv = 0;
	*yv = 0;
	float slope = 0;
	if(dx){
		slope = (float)dy / dx;
	}else{
		if(oldyv > 0){
			while(*yv < oldyv){
				for(std::vector<Platform *>::iterator i = test.begin(); i != test.end(); i++){
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, *i, type)){
						*yv = yv0;
						return *i;
					}
				}
				yv0 = *yv;
				(*yv)++;
			}
		}else{
			while(*yv > oldyv){
				for(std::vector<Platform *>::iterator i = test.begin(); i != test.end(); i++){
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, *i, type)){
						*yv = yv0;
						return *i;
					}
				}
				yv0 = *yv;
				(*yv)--;
			}
		}
	}
	if(slope > -1 && slope < 1){
		error = -dx / 2;
		oldyv > 0 ? step = 1 : step = -1;
		if(oldxv > 0){
			while(*xv < oldxv){
				for(std::vector<Platform *>::iterator i = test.begin(); i != test.end(); i++){
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, *i, type)){
						*yv = yv0;
						*xv = xv0;
						return *i;
					}
				}
				error += dy * step;
				if(error >= 0){
					yv0 = *yv;
					*yv += step;
					error -= dx;
				}
				xv0 = *xv;
				(*xv)++;
			}
		}else{
			while(*xv > oldxv){
				for(std::vector<Platform *>::iterator i = test.begin(); i != test.end(); i++){
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, *i, type)){
						*yv = yv0;
						*xv = xv0;
						return *i;
					}
				}
				error += dy * -step;
				if(error <= 0){
					yv0 = *yv;
					*yv += step;
					error -= dx;
				}
				xv0 = *xv;
				(*xv)--;
			}
		}
	}else{
		error = -dy / 2;
		oldxv > 0 ? step = 1 : step = -1;
		if(oldyv > 0){
			while(*yv < oldyv){
				for(std::vector<Platform *>::iterator i = test.begin(); i != test.end(); i++){
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, *i, type)){
						*xv = xv0;
						*yv = yv0;
						return *i;
					}
				}
				error += dx * step;
				if(error >= 0){
					xv0 = *xv;
					*xv += step;
					error -= dy;
				}
				yv0 = *yv;
				(*yv)++;
			}
		}else{
			while(*yv > oldyv){
				for(std::vector<Platform *>::iterator i = test.begin(); i != test.end(); i++){
					if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, *i, type)){
						*xv = xv0;
						*yv = yv0;
						return *i;
					}
				}
				error += dx * -step;
				if(error <= 0){
					xv0 = *xv;
					*xv += step;
					error -= dy;
				}
				yv0 = *yv;
				(*yv)--;
			}
		}
	}
	for(std::vector<Platform *>::iterator i = test.begin(); i != test.end(); i++){
		if(TestAABB(x1 + *xv, y1 + *yv, x2 + *xv, y2 + *yv, *i, type)){
			*xv = xv0;
			*yv = yv0;
			return *i;
		}
	}
	return 0;
}

bool Map::LineSegmentIntersection(float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Dx, float Dy, float * X, float * Y){
	float distAB, theCos, theSin, newX, ABpos;

	// Fail if either line segment is zero-length.
	if((Ax == Bx && Ay == By) || (Cx == Dx && Cy == Dy)){
		return false;
	}

	// Fail if the segments share an end-point.
	if((Ax == Cx && Ay == Cy) || (Bx == Cx && By == Cy) || (Ax == Dx && Ay == Dy) || (Bx == Dx && By == Dy)){
		return false;
	}

	// (1) Translate the system so that point A is on the origin.
	Bx -= Ax; By -= Ay;
	Cx -= Ax; Cy -= Ay;
	Dx -= Ax; Dy -= Ay;

	// Discover the length of segment A-B.
	distAB = sqrt(Bx * Bx + By * By);

	// (2) Rotate the system so that point B is on the positive X axis.
	theCos = Bx / distAB;
	theSin = By / distAB;
	newX = Cx * theCos + Cy * theSin;
	Cy = Cy * theCos - Cx * theSin; Cx = newX;
	newX = Dx * theCos + Dy * theSin;
	Dy = Dy * theCos - Dx * theSin; Dx = newX;

	// Fail if segment C-D doesn't cross line A-B.
	if((Cy < 0.0 && Dy < 0.0) || (Cy >= 0.0 && Dy >= 0.0)){
		return false;
	}

	// (3) Discover the position of the intersection point along line A-B.
	ABpos = Dx + (Cx - Dx) * Dy / (Dy - Cy);

	// Fail if segment C-D crosses line A-B outside of segment A-B.
	if(ABpos < 0.0 || ABpos > distAB){
		return false;
	}

	// (4) Apply the discovered position to line A-B in the original coordinate system.
	*X = Ax + ABpos * theCos;
	*Y = Ay + ABpos * theSin;

	// Success.
	return true;
}

bool Map::CompareType(Platform * a, Platform * b){
	if((a->type == Platform::STAIRSDOWN || a->type == Platform::STAIRSUP) && (b->type != Platform::STAIRSDOWN && b->type != Platform::STAIRSUP)){
		return true;
	}
	return false;
}