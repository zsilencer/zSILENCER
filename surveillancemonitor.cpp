#include "surveillancemonitor.h"
#include "team.h"
#include "basedoor.h"
#include "secretreturn.h"
#include "techstation.h"
#include "inventorystation.h"
#include "player.h"

SurveillanceMonitor::SurveillanceMonitor() : Object(ObjectTypes::SURVEILLANCEMONITOR){
	res_bank = 65;
	res_index = 0;
	renderxoffset = 0;
	renderyoffset = 0;
	objectfollowing = 0;
	teamid = 0;
	surveillancecamera = -1;
	drawscreen = true;
	scalefactor = 2;
	size = 0;
	state_i = 0;
	// 65 is screen borders 0:large 1:small 2:base screen
	// 151:0 is teambillboard border
}

void SurveillanceMonitor::Tick(World & world){
	if(teamid){
		if(res_bank == 151){
			Team * team = static_cast<Team *>(world.GetObjectFromId(teamid));
			if(team){
				objectfollowing = team->beamingterminalid;
				if(!objectfollowing){
					objectfollowing = team->playerwithsecret;
				}
				if(objectfollowing){
					drawscreen = true;
				}else{
					drawscreen = false;
				}
			}
		}else{
			if(!objectfollowing){
				Team * team = static_cast<Team *>(world.GetObjectFromId(teamid));
				if(team){
					switch(size){
						case 2:{
							BaseDoor * basedoor = static_cast<BaseDoor *>(world.GetObjectFromId(team->basedoorid));
							if(basedoor){
								objectfollowing = basedoor->id;
							}
						}break;
						case 10:{
							for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
								Object * object = *it;
								if(object->type == ObjectTypes::SECRETRETURN){
									SecretReturn * secretreturn = static_cast<SecretReturn *>(object);
									if(secretreturn->teamid == team->id){
										objectfollowing = secretreturn->id;
									}
								}
							}
						}break;
						case 11:{
							for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
								Object * object = *it;
								if(object->type == ObjectTypes::INVENTORYSTATION){
									InventoryStation * inventorystation = static_cast<InventoryStation *>(object);
									if(inventorystation->teamid == team->id){
										objectfollowing = inventorystation->id;
									}
								}
							}
						}break;
						case 12:{
							for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
								Object * object = *it;
								if(object->type == ObjectTypes::TECHSTATION){
									TechStation * techstation = static_cast<TechStation *>(object);
									if(techstation->teamid == team->id && techstation->type == 2){
										objectfollowing = techstation->id;
									}
								}
							}
						}break;
					}
				}
			}
		}
	}else{
		if(world.map.surveillancecameras.size() > 0){
			if(state_i == 0){
				surveillancecamera = -1;
			}
			if(surveillancecamera == -1){
				surveillancecamera = rand() % world.map.surveillancecameras.size();
			}
			camera.SetPosition(world.map.surveillancecameras[surveillancecamera].x, world.map.surveillancecameras[surveillancecamera].y);
		}
	}
	/*if(!objectfollowing){
		if(teamid){
			Team * team = static_cast<Team *>(world.GetObjectFromId(teamid));
			if(team){
				BaseDoor * basedoor = static_cast<BaseDoor *>(world.GetObjectFromId(team->basedoorid));
				if(basedoor){
					objectfollowing = basedoor->id;
				}
			}
		}else{
			int index = rand() % world.objectlist.size();
			int i = 0;
			for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++, i++){
				if(i == index){
					objectfollowing = (*it)->id;
					break;
				}
			}
		}
	}*/
	if(objectfollowing){
		//if(rand() % 500 == 0){
		//	objectfollowing = 0;
		//}
		Object * object = world.GetObjectFromId(objectfollowing);
		if(object){
			camera.SetPosition(object->x, object->y - (world.resources.spriteoffsety[object->res_bank][object->res_index] / 2));
		}else{
			objectfollowing = 0;
		}
	}
	state_i++;
}

void SurveillanceMonitor::SetSize(Uint8 size){
	SurveillanceMonitor::size = size;
	switch(size){
		default:
		case 0:
			res_index = 0;
			renderxoffset = 8;
			renderyoffset = 11;
			camera.w = 99 * 2;
			camera.h = 97 * 2;
		break;
		case 1:
			res_index = 1;
			renderxoffset = 6;
			renderyoffset = 7;
			camera.w = 78 * 2;
			camera.h = 41 * 2;
		break;
		case 2:
			res_index = 2;
			renderxoffset = 5;
			renderyoffset = 5;
			camera.w = 101 * 2;
			camera.h = 97 * 2;
		break;
		case 3:
			res_bank = 151;
			res_index = 0;
			renderxoffset = 7;
			renderyoffset = 6;
			camera.w = 189 * 2;
			camera.h = 135 * 2;
		break;
		case 10:
			res_index = 2;
			renderxoffset = 5;
			renderyoffset = 106;
			camera.w = 25 * 4;
			camera.h = 25 * 4;
			scalefactor = 4;
		break;
		case 11:
			res_index = 2;
			renderxoffset = 36;
			renderyoffset = 106;
			camera.w = 25 * 4;
			camera.h = 25 * 4;
			scalefactor = 4;
		break;
		case 12:
			res_index = 2;
			renderxoffset = 67;
			renderyoffset = 106;
			camera.w = 25 * 4;
			camera.h = 25 * 4;
			scalefactor = 4;
		break;
	}
}