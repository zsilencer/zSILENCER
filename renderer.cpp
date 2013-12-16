#include "renderer.h"
#include "sprite.h"
#include "resources.h"
#include "objecttypes.h"
#include "button.h"
#include "textbox.h"
#include "textinput.h"
#include "selectbox.h"
#include "scrollbar.h"
#include "team.h"
#include "player.h"
#include "civilian.h"
#include "guard.h"
#include "robot.h"
#include "terminal.h"
#include "healmachine.h"
#include "creditmachine.h"
#include "secretreturn.h"
#include "basedoor.h"
#include "shrapnel.h"
#include "bodypart.h"
#include "rocketprojectile.h"
#include "surveillancemonitor.h"
#include "techstation.h"
#include "teambillboard.h"
#include "overlay.h"
#include "toggle.h"
#include "pickup.h"
#include "warper.h"
#include "fixedcannon.h"
#include "grenade.h"
#include "walldefense.h"
#include "wallprojectile.h"
#include "config.h"
#include <math.h>

Renderer::Renderer(World & world) : world(world), camera(640, 480){
	ambience_r = 0;
	ex = 0;
	ey = 0;
	state_i = 0;
	localplayer = 0;
	for(int i = 0; i < raindropscount; i++){
		raindropsx[i] = rand();
		raindropsy[i] = rand();
	}
	playerinbaseold = false;
}

void Renderer::Draw(Surface * surface, float frametime){
	// Uncomment below to find and view individual sprites
	/*SDL_FillRect(surface, 0, 112);
	const Uint8 * keystate = SDL_GetKeyboardState(NULL);
	if(keystate[SDL_SCANCODE_UP]){ ex++; }
	if(keystate[SDL_SCANCODE_DOWN]){ ex--; }
	if(keystate[SDL_SCANCODE_RIGHT]){ ey++; }
	if(keystate[SDL_SCANCODE_LEFT]){ ey--; }
	if(world.resources.spritebank[ex][ey]){
		BlitSurface(world.resources.spritebank[ex][ey], 0, surface, 0);
		char temp[1234];
		sprintf(temp, "(%d)(%d) %d x %d   %d %d", ex, ey, world.resources.spritebank[ex][ey]->w, world.resources.spritebank[ex][ey]->h, world.resources.spriteoffsetx[ex][ey], world.resources.spriteoffsety[ex][ey]);
		DrawText(surface, 10, 200, temp, 133, 7);
	}
	SDL_Delay(100);
	return;*/
	localplayer = world.GetPeerPlayer(world.localpeerid);
	// Uncomment this to follow a player when there is no local peer
	/*if(!localplayer && world.IsAuthority()){
		for(std::vector<Uint16>::iterator it = world.objectsbytype[ObjectTypes::PLAYER].begin(); it != world.objectsbytype[ObjectTypes::PLAYER].end(); it++){
			localplayer = static_cast<Player *>(world.GetObjectFromId((*it)));
			break;
		}
	}*/
	if(localplayer){
		if(localplayer->InBase(world) && !playerinbaseold){
			localplayer->oldx = localplayer->x;
			localplayer->oldy = localplayer->y;
			camera.SetPosition(localplayer->x, localplayer->y - localplayer->height);
			playerinbaseold = true;
		}
		if(!localplayer->InBase(world) && playerinbaseold){
			localplayer->oldx = localplayer->x;
			localplayer->oldy = localplayer->y;
			camera.SetPosition(localplayer->x, localplayer->y - localplayer->height);
			playerinbaseold = false;
		}
		if(localplayer->state == Player::RESPAWNING){
			localplayer->oldx = localplayer->x;
			localplayer->oldy = localplayer->y;
		}
		if(localplayer->state_warp == 12){
			localplayer->oldx = localplayer->x;
			localplayer->oldy = localplayer->y;
		}
		int px = localplayer->x + ((localplayer->oldx - localplayer->x) * frametime);
		int py = localplayer->y + ((localplayer->oldy - localplayer->y) * frametime);
		if(px && py){
			camera.Follow(world, px, py, 15, 100, 0, 30);
			camera.Smooth(frametime);
		}
	}
	if(world.map.loaded){
		// Clear the minimap
		Surface minimap;
		minimap.w = 172;
		minimap.h = 62;
		minimap.pixels = world.map.minimap.pixels;
		BlitSurface(&minimap, 0, &world.map.minimap.surface, 0);
		minimap.pixels = 0;
		//
	}
	//if((world.gameplaystate == World::INGAME && world.map.loaded) || world.gameplaystate != World::INGAME){
	
	//memset(lightmap, ambiencelevel, surface->w * surface->h);
	DrawWorld(surface, camera, true, true, 3, frametime);
	//}

	if(world.map.loaded){
		DrawHUD(surface, frametime);
		//DrawDebug(surface);
		DrawStatus(surface);
		DrawMessage(surface);
	}
	if(world.quitstate == 1 || world.quitstate == 2){
#ifdef OUYA
		const char * text = "Hit O To QUIT";
#else
		const char * text = "Hit Enter To Quit";
#endif
		DrawText(surface, (640 - (strlen(text) * 16)) / 2, 200, (char *)text, 136, 16, false, 202);
	}
}

void Renderer::Tick(void){
	// Update ambience level
	Sint8 ambience = world.map.ambience;
	if(localplayer && localplayer->InBase(world)){
		ambience = world.map.baseambience;
	}
	ambiencelevel = 128 + (ambience * 4.5) + ambience_r;
	/*static Uint8 t;
	if(localplayer){
		if(localplayer->state == Player::RUNNING){
			if(localplayer->mirrored){
				t++;
			}else{
				t--;
			}
		}
	}
	if(t > 128){
		t = 0;
	}
	ambiencelevel = t;*/
	//
	
	// Animate raindrops
	for(int i = 0; i < raindropscount; i++){
		if(rand() % 1000 == 0){
			raindropsx[i] = rand();
			raindropsy[i] = rand();
		}
		raindropsoldx[i] = raindropsx[i];
		raindropsoldy[i] = raindropsy[i];
		switch(i % 3){
			case 0:
				raindropsx[i] += 128;
				raindropsy[i] += 320;
			break;
			case 1:
				raindropsx[i] += 128;
				raindropsy[i] += 250;
			break;
			case 2:
				raindropsx[i] += 96;
				raindropsy[i] += 200;
			break;
		}
	}
	//
	if(world.illuminate > 0){
		world.illuminate--;
		ambience_r += 4;
		if(ambience_r > 40){
			ambience_r = 40;
		}
	}else
	if(ambience_r >= 4){
		ambience_r -= 4;
		if(ambience_r < 0){
			ambience_r = 0;;
		}
	}
	camera.Tick();
	state_i++;
}

void Renderer::DrawWorld(Surface * surface, Camera & camera, bool drawminimap, bool drawluminance, int recursion, float frametime){
	recursion--;
	if(recursion < 0){
		return;
	}
	if(world.map.loaded){
		Sint16 cameray;
		camera.GetPosition(0, &cameray);
		if(cameray < world.map.height * 64){
			DrawParallax(surface, camera);
		}
		DrawBackground(surface, camera, drawluminance);
		DrawRain(surface, camera, frametime);
		DrawRainPuddles(surface, camera);
	}
	std::vector<Object *> objectlights;
	for(unsigned int renderpass = 0; renderpass < 4; renderpass++){
		for(std::list<Object *>::iterator i = world.objectlist.begin(); i != world.objectlist.end(); i++){
			Object * object = *i;
			Uint8 lightingsurfacebank = 0;
			Uint8 lightingsurfaceindex = 0;
			int oldx = object->oldx;
			int oldy = object->oldy;
			if(oldx == 0){
				oldx = object->x;
			}
			if(oldy == 0){
				oldy = object->y;
			}
			int nudgex = (oldx - object->x) * frametime;
			int nudgey = (oldy - object->y) * frametime;
			// stop objects from nudging back and forth when they havent been updated
			if(world.tickcount - 1 > object->lasttick){
				nudgex = 0;
				nudgey = 0;
			}
			if(object->issprite && object->draw && camera.IsVisible(world, *object)){
				if(object->renderpass == renderpass){
					object->x += nudgex;
					object->y += nudgey;
					Surface * src = world.resources.spritebank[object->res_bank][object->res_index];
					Rect dstrect;
					if(src){
						Surface * effectsurface = 0;
						dstrect.x = object->x - world.resources.spriteoffsetx[object->res_bank][object->res_index] + camera.GetXOffset();
						dstrect.y = object->y - world.resources.spriteoffsety[object->res_bank][object->res_index] + camera.GetYOffset();
						switch(object->type){
							case ObjectTypes::PLAYER:{
								Player * player = static_cast<Player *>(object);
								effectsurface = CreateSurfaceCopy(src);
								EffectTeamColor(effectsurface, 0, player->suitcolor);
								if(player->state_hit){
									EffectHit(effectsurface, 0, player->hitx, player->hity, player->state_hit);
								}
								if(player->effectshieldcontinue){
									EffectShieldDamage(effectsurface, 0, 205);
								}
								if(player->effecthacking){
									EffectHacking(effectsurface, 0, 190);
								}
								if(player->hassecret || (player->disguised != 0 && player->disguised != 100)){
									EffectHacking(effectsurface, 0, 124);
								}
								if(player->state_warp){
									EffectWarp(effectsurface, 0, player->state_warp);
								}else{
									DrawShadow(surface, object);
								}
								if(player->res_index == 5 || (player->currentweapon == 3 && player->res_index == 4 && player->flamersoundchannel != -1)){
									switch(player->res_bank){
										case 21:
											lightingsurfacebank = 223;
										break;
										case 22:
											lightingsurfacebank = 225;
										break;
										case 23:
											lightingsurfacebank = 226;
										break;
										case 24:
											lightingsurfacebank = 227;
										break;
										case 25:
											lightingsurfacebank = 228;
										break;
										case 36:
											lightingsurfacebank = 224;
										break;
									}
								}
							}break;
							case ObjectTypes::CIVILIAN:{
								Civilian * civilian = static_cast<Civilian *>(object);
								effectsurface = CreateSurfaceCopy(src);
								EffectTeamColor(effectsurface, 0, civilian->suitcolor);
								if(civilian->state_hit){
									EffectHit(effectsurface, 0, civilian->hitx, civilian->hity, civilian->state_hit);
								}
								if(civilian->tractteamid){
									EffectHacking(effectsurface, 0, 124);
								}
								if(civilian->state_warp){
									EffectWarp(effectsurface, 0, civilian->state_warp);
								}else{
									DrawShadow(surface, object);
								}
							}break;
							case ObjectTypes::WALLDEFENSE:{
								WallDefense * walldefense = static_cast<WallDefense *>(object);
								if(walldefense->state_hit){
									effectsurface = CreateSurfaceCopy(src);
									EffectHit(effectsurface, 0, walldefense->hitx, walldefense->hity, walldefense->state_hit);
								}
							}break;
							case ObjectTypes::GRENADE:{
								Grenade * grenade = static_cast<Grenade *>(object);
								effectsurface = CreateSurfaceCopy(src);
								EffectTeamColor(effectsurface, 0, grenade->color);
								DrawShadow(surface, object);
							}break;
							case ObjectTypes::FIXEDCANNON:{
								FixedCannon * fixedcannon = static_cast<FixedCannon *>(object);
								effectsurface = CreateSurfaceCopy(src);
								EffectTeamColor(effectsurface, 0, fixedcannon->suitcolor);
								if(fixedcannon->state_hit){
									EffectHit(effectsurface, 0, fixedcannon->hitx, fixedcannon->hity, fixedcannon->state_hit);
								}
								DrawShadow(surface, object);
							}break;
							case ObjectTypes::BODYPART:{
								BodyPart * bodypart = static_cast<BodyPart *>(object);
								effectsurface = CreateSurfaceCopy(src);
								EffectTeamColor(effectsurface, 0, bodypart->suitcolor);
							}break;
							case ObjectTypes::GUARD:{
								Guard * guard = static_cast<Guard *>(object);
								effectsurface = CreateSurfaceCopy(src);
								if(guard->weapon == 2){
									EffectRampColor(effectsurface, 0, 2);
								}
								if(guard->state_hit){
									EffectHit(effectsurface, 0, guard->hitx, guard->hity, guard->state_hit);
								}
								if(guard->state_warp){
									EffectWarp(effectsurface, 0, guard->state_warp);
								}else{
									DrawShadow(surface, object);
								}
							}break;
							case ObjectTypes::ROBOT:{
								Robot * robot = static_cast<Robot *>(object);
								effectsurface = CreateSurfaceCopy(src);
								if(robot->state_hit){
									EffectHit(effectsurface, 0, robot->hitx, robot->hity, robot->state_hit);
								}
								if(robot->damaging){
									EffectShieldDamage(effectsurface, 0, 120);
								}
								if(robot->state_warp){
									EffectWarp(effectsurface, 0, robot->state_warp);
								}else{
									DrawShadow(surface, object);
								}
							}break;
							case ObjectTypes::BASEDOOR:{
								if(localplayer){
									Team * team = localplayer->GetTeam(world);
									if(team){
										BaseDoor * basedoor = static_cast<BaseDoor *>(object);
										if(basedoor->discoveredby[team->number]){
											effectsurface = CreateSurfaceCopy(src);
											EffectTeamColor(effectsurface, 0, basedoor->color);
										}else{
											src = 0;
										}
									}
								}
							}break;
							case ObjectTypes::SHRAPNEL:{
								Shrapnel * shrapnel = static_cast<Shrapnel *>(object);
								effectsurface = CreateSurfaceCopy(src);
								if(shrapnel->res_bank == 110){
									EffectRampColor(effectsurface, 0, 202);
								}
								EffectBrightness(effectsurface, 0, shrapnel->GetBrightness());
							}break;
							case ObjectTypes::SCROLLBAR:{
								ScrollBar * scrollbar = static_cast<ScrollBar *>(object);
								if(!scrollbar->draw){
									src = 0;
								}
							}break;
							case ObjectTypes::HEALMACHINE:{
								HealMachine * healmachine = static_cast<HealMachine *>(object);
								src = 0;
								Rect dstrect;
								Uint8 index = healmachine->state_i;
								if(index > 5){
									index = 5;
								}
								dstrect.w = world.resources.spritewidth[172][index];
								dstrect.h = world.resources.spriteheight[172][index];
								dstrect.x = object->x - world.resources.spriteoffsetx[172][index] + camera.GetXOffset();
								dstrect.y = object->y - world.resources.spriteoffsety[172][index] + camera.GetYOffset();
								BlitSurface(world.resources.spritebank[172][index], 0, surface, &dstrect);
							}break;
							case ObjectTypes::TEAMBILLBOARD:{
								TeamBillboard * teambillboard = static_cast<TeamBillboard *>(object);
								// 151:0 is team billboard border
								// 151:1 is team billboard background
								// 151:2-6 is team billboard team names noxis to blackrose
								// 151:7-13 is team billboard animation 1
								// 151:14-20 is team billboard animation 2
								Team * team = static_cast<Team *>(world.GetObjectFromId(teambillboard->teamid));
								if(!team || (team && !team->playerwithsecret)){
									Rect dstrect;
									dstrect.x = object->x - world.resources.spriteoffsetx[151][1] + camera.GetXOffset();
									dstrect.y = object->y - world.resources.spriteoffsety[151][1] + camera.GetYOffset();
									dstrect.w = world.resources.spritewidth[151][1];
									dstrect.h = world.resources.spriteheight[151][1];
									BlitSurface(world.resources.spritebank[151][1], 0, surface, &dstrect);
									/*dstrect.x = object->x - world.resources.spriteoffsetx[151][object->res_index] + camera.GetXOffset();
									dstrect.y = object->y - world.resources.spriteoffsety[151][object->res_index] + camera.GetYOffset();
									dstrect.w = world.resources.spritewidth[151][object->res_index];
									dstrect.h = world.resources.spriteheight[151][object->res_index];
									BlitSurface(world.resources.spritebank[151][object->res_index], 0, surface, &dstrect);*/
									Uint8 index = ((state_i / 4) % 6) + 7;
									dstrect.x = object->x - world.resources.spriteoffsetx[151][index] + camera.GetXOffset();
									dstrect.y = object->y - world.resources.spriteoffsety[151][index] + camera.GetYOffset();
									dstrect.w = world.resources.spritewidth[151][index];
									dstrect.h = world.resources.spriteheight[151][index];
									BlitSurface(world.resources.spritebank[151][index], 0, surface, &dstrect);
									index = ((state_i / 4) % 6) + 14;
									dstrect.x = object->x - world.resources.spriteoffsetx[151][index] + camera.GetXOffset();
									dstrect.y = object->y - world.resources.spriteoffsety[151][index] + camera.GetYOffset();
									dstrect.w = world.resources.spritewidth[151][index];
									dstrect.h = world.resources.spriteheight[151][index];
									BlitSurface(world.resources.spritebank[151][index], 0, surface, &dstrect);
									index = teambillboard->agency + 2;
									dstrect.x = object->x - world.resources.spriteoffsetx[151][index] + camera.GetXOffset();
									dstrect.y = object->y - world.resources.spriteoffsety[151][index] + camera.GetYOffset();
									dstrect.w = world.resources.spritewidth[151][index];
									dstrect.h = world.resources.spriteheight[151][index];
									BlitSurface(world.resources.spritebank[151][index], 0, surface, &dstrect);
								}
							}break;
							case ObjectTypes::TERMINAL:{
								Terminal * terminal = static_cast<Terminal *>(object);
								if(terminal->state == Terminal::HACKING || terminal->state == Terminal::HACKERGONE || (terminal->isbig && terminal->state == Terminal::READY)){
									Sint16 yoffset = -99;
									Rect dstrect;
									Rect srcrect;
									dstrect.x = object->x - world.resources.spriteoffsetx[180][2] + camera.GetXOffset();
									dstrect.y = object->y - world.resources.spriteoffsety[180][2] + camera.GetYOffset() + yoffset;
									dstrect.w = world.resources.spritewidth[180][2];
									dstrect.h = world.resources.spriteheight[180][2];
									DrawRampColored(world.resources.spritebank[180][2], 0, surface, &dstrect);
									dstrect.x = object->x - world.resources.spriteoffsetx[180][1] + camera.GetXOffset();
									dstrect.y = object->y - world.resources.spriteoffsety[180][1] + camera.GetYOffset() + yoffset;
									BlitSurface(world.resources.spritebank[180][1], 0, surface, &dstrect);
									dstrect.x = object->x - world.resources.spriteoffsetx[180][0] + camera.GetXOffset();
									dstrect.y = object->y - world.resources.spriteoffsety[180][0] + camera.GetYOffset() + yoffset;
									srcrect.w = ((1 - ((float)terminal->GetPercent() / 100)) * (world.resources.spritewidth[180][0]));
									srcrect.h = world.resources.spriteheight[180][0];
									srcrect.x = 0;
									srcrect.y = 0;
									BlitSurface(world.resources.spritebank[180][0], &srcrect, surface, &dstrect);
								}
							}break;
							case ObjectTypes::SURVEILLANCEMONITOR:{
								SurveillanceMonitor * surveillancemonitor = static_cast<SurveillanceMonitor *>(object);
								if(surveillancemonitor->drawscreen){
									Rect dstrect;
									dstrect.w = surveillancemonitor->camera.w / surveillancemonitor->scalefactor;
									dstrect.h = surveillancemonitor->camera.h / surveillancemonitor->scalefactor;
									dstrect.x = surveillancemonitor->renderxoffset + object->x - world.resources.spriteoffsetx[object->res_bank][object->res_index] + camera.GetXOffset();
									dstrect.y = surveillancemonitor->renderyoffset + object->y - world.resources.spriteoffsety[object->res_bank][object->res_index] + camera.GetYOffset();
									Surface newsurface(dstrect.w, dstrect.h, 1);
									//memset(newsurface->pixels, 1, newsurface->w * newsurface->h);
									Object * objectfollowing = 0;
									if(surveillancemonitor->objectfollowing){
										objectfollowing = world.GetObjectFromId(surveillancemonitor->objectfollowing);
									}
									if(objectfollowing){
										surveillancemonitor->camera.x += (objectfollowing->oldx - objectfollowing->x) * frametime;
										surveillancemonitor->camera.y += (objectfollowing->oldy - objectfollowing->y) * frametime;
									}
									DrawWorldScaled(&newsurface, surveillancemonitor->camera, recursion, frametime, surveillancemonitor->scalefactor);
									if(objectfollowing){
										surveillancemonitor->camera.x -= (objectfollowing->oldx - objectfollowing->x) * frametime;
										surveillancemonitor->camera.y -= (objectfollowing->oldy - objectfollowing->y) * frametime;
									}
									EffectRampColor(&newsurface, 0, 198);
									BlitSurface(&newsurface, 0, surface, &dstrect);
								}
							}break;
							case ObjectTypes::OVERLAY:{
								Overlay * overlay = static_cast<Overlay *>(object);
								if(object->effectbrightness != 128){
									if(!effectsurface){
										effectsurface = CreateSurfaceCopy(src);
									}
									EffectBrightness(effectsurface, 0, object->effectbrightness);
								}
								switch(overlay->res_bank){
									case 222:{
										src = 0;
										objectlights.push_back(object);
										//lightingsurfacebank = overlay->res_bank;
										//lightingsurfaceindex = overlay->res_index;
									}break;
								}
							}break;
							case ObjectTypes::TECHSTATION:{
								TechStation * techstation = static_cast<TechStation *>(object);
								if(techstation->state_hit){
									effectsurface = CreateSurfaceCopy(src);
									EffectHit(effectsurface, 0, techstation->hitx, techstation->hity, techstation->state_hit);
								}
							}break;
							default:{
								if(object->effectcolor){
									if(!effectsurface){
										effectsurface = CreateSurfaceCopy(src);
									}
									EffectColor(effectsurface, 0, object->effectcolor);
								}
								if(object->effectbrightness != 128){
									if(!effectsurface){
										effectsurface = CreateSurfaceCopy(src);
									}
									EffectBrightness(effectsurface, 0, object->effectbrightness);
								}
							}break;
						}
						if(effectsurface){
							src = effectsurface;
						}
						if(src){
							BlitSprite(object, camera, surface, &dstrect, src, 0);
						}
						if(effectsurface){
							delete effectsurface;
						}
					}
					switch(object->type){
						case ObjectTypes::PLAYER:{
							Player * player = static_cast<Player *>(object);
							if(player->state == Player::JETPACK || player->state == Player::JETPACKSHOOT){
								Rect dstrect;
								dstrect.x = -world.resources.spriteoffsetx[218][0] + camera.GetXOffset() + object->x;
								dstrect.y = -world.resources.spriteoffsety[218][0] + camera.GetYOffset() + object->y;
								if(object->mirrored){
									dstrect.x = object->x - (world.resources.spritewidth[218][0] - world.resources.spriteoffsetx[218][0]) + camera.GetXOffset();
									DrawMirrored(world.resources.spritebank[218][0], 0, surface, &dstrect);
								}else{
									BlitSurface(world.resources.spritebank[218][0], 0, surface, &dstrect);
								}
							}
							////// 210:0-14 decal running
							////// 211:0-5 decal runstart
							////// 212:0-3 decal runstop
							// 213:0-1 decal jumping?
							// 214:0-6 decal inair?
							////// 215:0-4 decal crouching
							////// 216:0-16 decal throwing
							////// 217:0-16 decal throwing crouched
							Uint8 decalres_bank = 0;
							Uint8 decalres_index = 0;
							switch(object->res_bank){
								case 66:
									decalres_bank = 211;
									decalres_index = object->res_index;
								break;
								case 11:
									decalres_bank = 210;
									decalres_index = object->res_index;
								break;
								case 67:
									decalres_bank = 212;
									decalres_index = object->res_index;
								break;
								case 12:
									decalres_bank = 213;
									decalres_index = object->res_index;
								break;
								case 13:
									decalres_bank = 214;
									decalres_index = object->res_index;
								break;
								case 17:
									decalres_bank = 215;
									decalres_index = object->res_index;
								break;
								case 113:
									decalres_bank = 216;
									decalres_index = object->res_index;
								break;
								case 114:
									decalres_bank = 217;
									decalres_index = object->res_index;
								break;
							}
							if(decalres_bank && world.resources.spritebank[decalres_bank][decalres_index]){
								Rect dstrect;
								dstrect.x = -world.resources.spriteoffsetx[decalres_bank][decalres_index] + camera.GetXOffset() + object->x;
								dstrect.y = -world.resources.spriteoffsety[decalres_bank][decalres_index] + camera.GetYOffset() + object->y;
								if(object->mirrored){
									dstrect.x = object->x - (world.resources.spritewidth[decalres_bank][decalres_index] - world.resources.spriteoffsetx[decalres_bank][decalres_index]) + camera.GetXOffset();
									DrawMirrored(world.resources.spritebank[decalres_bank][decalres_index], 0, surface, &dstrect);
								}else{
									BlitSurface(world.resources.spritebank[decalres_bank][decalres_index], 0, surface, &dstrect);
								}
							}
							if(localplayer && player->GetTeam(world) == localplayer->GetTeam(world) && player != localplayer){
								Peer * peer = player->GetPeer(world);
								if(peer){
									User * user = world.lobby.GetUserInfo(peer->accountid);
									if(user && !user->retrieving){
										char username[64];
										strcpy(username, user->name);
										DrawText(surface, player->x + camera.GetXOffset() - ((strlen(username) * 6) / 2), player->y + camera.GetYOffset() - 80, username, 133, 6);
									}
								}
							}
						}break;
						case ObjectTypes::BUTTON:{
							Button * button = static_cast<Button *>(object);
							if(button->text[0]){
								Sint16 xoff;
								Sint16 yoff;
								button->GetTextOffset(world, &xoff, &yoff);
								DrawText(surface, xoff, yoff, button->text, button->textbank, button->textwidth, true, button->effectcolor, button->effectbrightness);
							}
						}break;
						case ObjectTypes::SCROLLBAR:{
							ScrollBar * scrollbar = static_cast<ScrollBar *>(object);
							if(scrollbar->draw){
								Uint8 oldres_index = scrollbar->res_index;
								scrollbar->res_index = scrollbar->barres_index;
								src = world.resources.spritebank[object->res_bank][object->res_index];
								float pos = float(scrollbar->scrollposition) / scrollbar->scrollmax;
								Uint16 scrollbararea = world.resources.spriteheight[object->res_bank][object->res_index];
								Rect srcrect;
								srcrect.w = src->w;
								srcrect.h = scrollbararea - (scrollbar->scrollmax);
								if(src->h - (scrollbar->scrollmax) < 32){
									srcrect.h = 32;
								}
								srcrect.x = 0;
								srcrect.y = 0;
								int dsty = ((scrollbararea - srcrect.h) * (pos));
								//if(dsty > (scrollbararea - srcrect.h)){
								//	dsty = (scrollbararea - srcrect.h);
								//}
								dstrect.y += dsty;
								dstrect.x += 1;
								dstrect.y += 16;
								BlitSprite(object, camera, surface, &dstrect, src, &srcrect);
								scrollbar->res_index = oldres_index;
							}
						}break;
						case ObjectTypes::SELECTBOX:{
							SelectBox * selectbox = static_cast<SelectBox *>(object);
							unsigned int line = 0;
							unsigned int i = 0;
							for(std::deque<char *>::iterator it = selectbox->items.begin(); it != selectbox->items.end(); it++, i++){
								if(i < selectbox->scrolled){
									continue;
								}
								if(line > selectbox->height / selectbox->lineheight){
									break;
								}
								char * text = (*it);
								if(i == selectbox->selecteditem){
									Rect dstrect;
									dstrect.x = selectbox->x;
									dstrect.y = selectbox->y + (line * selectbox->lineheight);
									dstrect.w = selectbox->width;
									dstrect.h = 11;
									DrawFilledRectangle(surface, dstrect.x, dstrect.y, dstrect.x + dstrect.w, dstrect.y + dstrect.h, 180);
								}
								DrawText(surface, selectbox->x, selectbox->y + (line * selectbox->lineheight), text, 133, 6);
								line++;
							}
						}break;
						case ObjectTypes::TEXTINPUT:{
							TextInput * textinput = static_cast<TextInput *>(object);
							DrawTextInput(surface, *textinput);
						}break;
						case ObjectTypes::TEXTBOX:{
							TextBox * textbox = static_cast<TextBox *>(object);
							unsigned int line = 0;
							unsigned int i = 0;
							for(std::deque<char *>::iterator it = textbox->text.begin(); it != textbox->text.end(); it++, i++){
								if(i < textbox->scrolled){
									continue;
								}
								int y = textbox->y + (line * textbox->lineheight);
								if(line > textbox->height / textbox->lineheight){
									break;
								}
								if(textbox->bottomtotop){
									int size = (textbox->text.size() * textbox->lineheight);
									if(size > textbox->height){
										size = (ceil(float(textbox->height) / textbox->lineheight)) * textbox->lineheight;
									}
									y += (textbox->height - size);
								}
								Uint8 color = (*it)[strlen((*it)) + 1];
								Uint8 brightness = (*it)[strlen((*it)) + 2];
								DrawText(surface, textbox->x, y, (*it), textbox->res_bank, textbox->fontwidth, false, color, brightness);
								line++;
							}
						}break;
						case ObjectTypes::TOGGLE:{
							Toggle * toggle = static_cast<Toggle *>(object);
							if(toggle->text[0]){
								DrawText(surface, toggle->x - ((strlen(toggle->text) * 9) / 2), toggle->y, toggle->text, 134, 9);
							}
						}break;
						case ObjectTypes::HEALMACHINE:{
							HealMachine * healmachine = static_cast<HealMachine *>(object);
							if(healmachine->state_i > 0){
								Rect dstrect;
								dstrect.w = world.resources.spritewidth[172][6];
								dstrect.h = world.resources.spriteheight[172][6];
								dstrect.x = object->x - world.resources.spriteoffsetx[172][6] + camera.GetXOffset();
								dstrect.y = object->y - world.resources.spriteoffsety[172][6] + camera.GetYOffset();
								DrawColored(world.resources.spritebank[172][6], 0, surface, &dstrect);
							}
						}break;
						case ObjectTypes::SECRETRETURN:{
							// secret 152:1-8
							// nosecret 152:9-13
							SecretReturn * secretreturn = static_cast<SecretReturn *>(object);
							Uint8 numsecrets = 0;
							Team * team = static_cast<Team *>(world.GetObjectFromId(secretreturn->teamid));
							if(team){
								numsecrets = team->secrets;
							}
							for(int i = 0; i < 3; i++){
								Uint8 index = secretreturn->frame[i];
								if(numsecrets > i){
									index = (((state_i / 4)) % 8) + 1;
								}
								dstrect.w = world.resources.spritewidth[152][index];
								dstrect.h = world.resources.spriteheight[152][index];
								dstrect.x = object->x - world.resources.spriteoffsetx[152][index] + camera.GetXOffset() + (i * 45);
								dstrect.y = object->y - world.resources.spriteoffsety[152][index] + camera.GetYOffset();
								BlitSurface(world.resources.spritebank[152][index], 0, surface, &dstrect);
							}
						}break;
						case ObjectTypes::TECHSTATION:{
							TechStation * techstation = static_cast<TechStation *>(object);
							Uint8 index = ((state_i / 2) % 16) + 1;
							if(techstation->health == 0){
								index = 1;
							}
							dstrect.w = world.resources.spritewidth[106][index];
							dstrect.h = world.resources.spriteheight[106][index];
							dstrect.x = object->x - world.resources.spriteoffsetx[106][index] + camera.GetXOffset();
							dstrect.y = object->y - world.resources.spriteoffsety[106][index] + camera.GetYOffset();
							BlitSurface(world.resources.spritebank[106][index], 0, surface, &dstrect);
							index = (state_i / 4) % 4;
							if(techstation->health == 0){
								index = 0;
							}
							switch(techstation->type){
								default:
								case 0:
									index += 17;
								break;
								case 1:
									index += 21;
								break;
								case 2:
									index += 25;
								break;
							}
							dstrect.w = world.resources.spritewidth[106][index];
							dstrect.h = world.resources.spriteheight[106][index];
							dstrect.x = object->x - world.resources.spriteoffsetx[106][index] + camera.GetXOffset();
							dstrect.y = object->y - world.resources.spriteoffsety[106][index] + camera.GetYOffset();
							BlitSurface(world.resources.spritebank[106][index], 0, surface, &dstrect);
						}break;
					}
					if(object->type == ObjectTypes::ROCKETPROJECTILE && &camera == &this->camera){
						RocketProjectile * rocketprojectile = static_cast<RocketProjectile *>(object);
						if(rocketprojectile->JustHit()){
							world.Illuminate();
						}
					}
					if(object->res_bank == 46 && object->res_index == 17){
						lightingsurfacebank = 221;
						lightingsurfaceindex = 0;
					}
					if(drawluminance && lightingsurfacebank){
						dstrect.x = object->x - world.resources.spriteoffsetx[lightingsurfacebank][lightingsurfaceindex] + camera.GetXOffset();
						dstrect.y = object->y - world.resources.spriteoffsety[lightingsurfacebank][lightingsurfaceindex] + camera.GetYOffset();
						Surface * src = world.resources.spritebank[lightingsurfacebank][lightingsurfaceindex];
						if(object->mirrored){
							Surface * srcmirrored = new Surface(src->w, src->h);
							Rect dstrect0;
							dstrect0.x = dstrect0.y = 0;
							DrawMirrored(src, 0, srcmirrored, &dstrect0);
							src = srcmirrored;
							dstrect.x = object->x - (world.resources.spritewidth[lightingsurfacebank][lightingsurfaceindex] - world.resources.spriteoffsetx[lightingsurfacebank][lightingsurfaceindex]) + camera.GetXOffset();
						}
						DrawLight(surface, src, &dstrect);
						if(object->mirrored){
							delete src;
						}
					}
					object->x -= nudgex;
					object->y -= nudgey;
				}
			}
			if(object->type == ObjectTypes::OVERLAY && object->draw){
				Overlay * overlay = static_cast<Overlay *>(object);
				if(overlay->text){
					DrawText(surface, overlay->x, overlay->y, overlay->text, overlay->textbank, overlay->textwidth, overlay->drawalpha, overlay->effectcolor, overlay->effectbrightness, overlay->textcolorramp);
				}
			}
			if(object->type == ObjectTypes::CREDITMACHINE && renderpass == 3){
				CreditMachine * creditmachine = static_cast<CreditMachine *>(object);
				Uint8 index = creditmachine->state_i;
				Rect dstrect;
				dstrect.w = world.resources.spritewidth[81][index];
				dstrect.h = world.resources.spriteheight[81][index];
				dstrect.x = object->x - world.resources.spriteoffsetx[81][index] + camera.GetXOffset();
				dstrect.y = object->y - world.resources.spriteoffsety[81][index] + camera.GetYOffset();
				BlitSurface(world.resources.spritebank[81][index], 0, surface, &dstrect);
			}
			if(object->type == ObjectTypes::WARPER && renderpass == 3){
				Warper * warper = static_cast<Warper *>(object);
				Rect dstrect;
				dstrect.w = world.resources.spritewidth[85][1];
				dstrect.h = world.resources.spriteheight[85][1];
				dstrect.x = object->x - world.resources.spriteoffsetx[85][1] + camera.GetXOffset();
				dstrect.y = object->y - world.resources.spriteoffsety[85][1] + camera.GetYOffset();
				BlitSurface(world.resources.spritebank[85][1], 0, surface, &dstrect);
				char text[16];
				sprintf(text, "%d", warper->GetCountdown());
				DrawText(surface, object->x + camera.GetXOffset() - 3, object->y + camera.GetYOffset(), text, 133, 6, false, 126, 128, true);
			}
			if(object->type == ObjectTypes::WALLPROJECTILE && renderpass == object->renderpass){
				WallProjectile * wallprojectile = static_cast<WallProjectile *>(object);
				if(wallprojectile->state_i > 8){
					object->x += nudgex;
					object->y += nudgey;
					for(int i = 1; i <= 3; i++){
						int x1 = (object->x - (object->xv * (i / float(3))));
						int y1 = (object->y - (object->yv * (i / float(3))));
						int x2 = object->x;
						int y2 = object->y;
						DrawLine(surface, x1 + camera.GetXOffset(), y1 + camera.GetYOffset(), x2 + camera.GetXOffset(), y2 + camera.GetYOffset(), 203, 4 - i);
					}
					object->x -= nudgex;
					object->y -= nudgey;
				}
			}
			if(object->issprite && object->renderpass == renderpass){
				if(drawminimap && world.map.loaded){
					DrawMiniMap(object);
				}
			}
		}
	}
	if(world.map.loaded){
		if(drawluminance){
			DrawForegroundLuminance(surface, camera);
			for(std::vector<Object *>::iterator it = objectlights.begin(); it != objectlights.end(); it++){
				Surface * src = world.resources.spritebank[(*it)->res_bank][(*it)->res_index];
				Rect dstrect;
				dstrect.x = (*it)->x - world.resources.spriteoffsetx[(*it)->res_bank][(*it)->res_index] + camera.GetXOffset();
				dstrect.y = (*it)->y - world.resources.spriteoffsety[(*it)->res_bank][(*it)->res_index] + camera.GetYOffset();
				DrawLight(surface, src, &dstrect);
			}
		}
		DrawForeground(surface, camera);
	}
}

void Renderer::DrawMiniMap(Object * object){
	// 104:2 minimap teammate, 3 minimap enemy, 4 minimap rocket,
	switch(object->type){
		case ObjectTypes::TERMINAL:{
			Terminal * terminal = static_cast<Terminal *>(object);
			if(terminal->state != Terminal::INACTIVE){
				if(terminal->isbig){
					if(terminal->state == Terminal::SECRETREADY){
						Uint8 color = enemycolor;
						if(Config::GetInstance().teamcolors){
							if(localplayer){
								Team * team = localplayer->GetTeam(world);
								if(team && team->beamingterminalid == terminal->id){
									color = teamcolor;
								}
							}
						}else{
							for(std::vector<Uint16>::iterator it = world.objectsbytype[ObjectTypes::TEAM].begin(); it != world.objectsbytype[ObjectTypes::TEAM].end(); it++){
								Team * team = static_cast<Team*>(world.GetObjectFromId((*it)));
								if(team && team->beamingterminalid == terminal->id){
									color = team->GetColor();
									break;
								}
							}
						}
						MiniMapCircle(terminal->x, terminal->y, TeamColorToIndex(color));
					}else
					if(terminal->state == Terminal::SECRETBEAMING){
						int x1 = terminal->x;
						int y1 = terminal->y;
						world.map.MiniMapCoords(x1, y1);
						int radius = terminal->beamingtime * 2;
						Uint8 color = enemycolor;
						if(Config::GetInstance().teamcolors){
							Player * player = world.GetPeerPlayer(world.localpeerid);
							if(player){
								Team * team = player->GetTeam(world);
								if(team){
									if(team->beamingterminalid == terminal->id){
										color = teamcolor;
									}
								}
							}
						}else{
							for(std::vector<Uint16>::iterator it = world.objectsbytype[ObjectTypes::TEAM].begin(); it != world.objectsbytype[ObjectTypes::TEAM].end(); it++){
								Team * team = static_cast<Team *>(world.GetObjectFromId((*it)));
								if(team && team->beamingterminalid == terminal->id){
									color = team->GetColor();
									break;
								}
							}
						}
						DrawCircle(&world.map.minimap.surface, x1, y1, radius, TeamColorToIndex(color));
					}else
					if(terminal->state == Terminal::READY || terminal->state == Terminal::HACKING || terminal->state == Terminal::HACKERGONE || (terminal->state == Terminal::BEAMING && state_i % 2)){
						MiniMapBlit(104, 28, terminal->x, terminal->y);
					}
				}else{
					if(terminal->state != Terminal::BEAMING){
						MiniMapBlit(104, 1, terminal->x, terminal->y);
					}
				}
			}
		}break;
		case ObjectTypes::BASEDOOR:{
			if(localplayer){
				Team * team = localplayer->GetTeam(world);
				if(team){
					BaseDoor * basedoor = static_cast<BaseDoor *>(object);
					if(basedoor->discoveredby[team->number]){
						Uint8 color = 0;
						Team * team = (Team *)world.GetObjectFromId(basedoor->teamid);
						if(team){
							if(Config::GetInstance().teamcolors){
								if(localplayer && localplayer->teamid != team->id){
									color = enemycolor;
								}else{
									color = teamcolor;
								}
							}else{
								color = team->GetColor();
							}
							MiniMapBlit(104, 5, basedoor->x, basedoor->y, false, color);
						}
					}
				}
			}
		}break;
		case ObjectTypes::PICKUP:{
			PickUp * pickup = static_cast<PickUp *>(object);
			if(pickup->type == PickUp::SECRET){
				MiniMapCircle(pickup->x, pickup->y, 192);
			}
		}break;
		case ObjectTypes::PLASMAPROJECTILE:
		case ObjectTypes::ROCKETPROJECTILE:{
			MiniMapBlit(104, 4, object->x, object->y, true);
		}break;
		case ObjectTypes::PLAYER:{
			Player * player = static_cast<Player *>(object);
			if(player == localplayer){
				int x = player->x;
				int y = player->y;
				world.map.MiniMapCoords(x, y);
				x -= 1;
				y -= 1;
				if(y < 62 - 4 && x < 172 - 4){
					Rect dstrect;
					dstrect.w = 172;
					dstrect.h = 3;
					dstrect.x = 0;
					dstrect.y = y;
					EffectBrightness(&world.map.minimap.surface, &dstrect, 150);
					dstrect.w = 3;
					dstrect.h = 62;
					dstrect.x = x;
					dstrect.y = 0;
					EffectBrightness(&world.map.minimap.surface, &dstrect, 150);
				}
			}
			if(player->hassecret){
				Team * team = player->GetTeam(world);
				Uint8 color;
				if(Config::GetInstance().teamcolors){
					if(localplayer && player->GetTeam(world) != localplayer->GetTeam(world)){
						color = enemycolor;
					}else{
						color = teamcolor;
					}
				}else{
					color = team->GetColor();
				}
				MiniMapCircle(player->x, player->y, TeamColorToIndex(color));
			}
			bool onteam = false;
			if(localplayer && player != localplayer && player->GetTeam(world) == localplayer->GetTeam(world)){
				onteam = true;
			}
			bool satelliteability = false;
			if(localplayer){
				Team * team = localplayer->GetTeam(world);
				Uint8 color = 0;
				if(Config::GetInstance().teamcolors){
					if(localplayer && player->GetTeam(world) != localplayer->GetTeam(world)){
						color = enemycolor;
					}else{
						color = teamcolor;
					}
				}else{
					Team * playerteam = player->GetTeam(world);
					if(playerteam){
						color = playerteam->GetColor();
					}
				}
				if(team && team->agency == Team::STATIC && state_i % 64 < 32){
					satelliteability = true;
				}
				if(localplayer->radarbonustime > world.tickcount){
					satelliteability = true;
				}
				if((satelliteability && !player->IsDisguised()) || player == localplayer || onteam){
					MiniMapBlit(104, 0, player->x, player->y, false, color);
				}
			}
		}
	}
}

void Renderer::DrawWorldScaled(Surface * surface, Camera & camera, int recursion, float frametime, int factor){
	Surface systemscreen(surface->w * factor, surface->h * factor);
	DrawWorld(&systemscreen, camera, false, false, recursion, frametime);
	Rect dstrect;
	dstrect.x = 0;
	dstrect.y = 0;
	dstrect.w = surface->w;
	dstrect.h = surface->h;
	DrawScaled(&systemscreen, 0, surface, &dstrect, factor);
}

void Renderer::BlitSurface(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	BlitSurfaceUpper(src, srcrect, dst, dstrect);
}

bool Renderer::BlitSurfaceUpper(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	Rect fulldst;
	int srcx, srcy, w, h;
	if(!src || !dst){
		return false;
	}
	// If the destination rectangle is NULL, use the entire dest surface
	if(dstrect == NULL){
		fulldst.x = fulldst.y = 0;
		dstrect = &fulldst;
	}
	// Clip the source rectangle to the source surface
	if(srcrect){
		int maxw, maxh;
		srcx = srcrect->x;
		w = srcrect->w;
		if(srcx < 0){
			w += srcx;
			dstrect->x -= srcx;
			srcx = 0;
		}
		maxw = src->w - srcx;
		if(maxw < w){
			w = maxw;
		}
		srcy = srcrect->y;
		h = srcrect->h;
		if(srcy < 0){
			h += srcy;
			dstrect->y -= srcy;
			srcy = 0;
		}
		maxh = src->h - srcy;
		if(maxh < h){
			h = maxh;
		}
	}else{
		srcx = srcy = 0;
		w = src->w;
		h = src->h;
	}
	// Clip the destination rectangle to the destination surface
	int dx, dy;
	dx = 0 - dstrect->x;
	if(dx > 0){
		w -= dx;
		dstrect->x += dx;
		srcx += dx;
	}
	dx = dstrect->x + w - 0 - dst->w;
	if(dx > 0){
		w -= dx;
	}
	dy = 0 - dstrect->y;
	if(dy > 0){
		h -= dy;
		dstrect->y += dy;
		srcy += dy;
	}
	dy = dstrect->y + h - 0 - dst->h;
	if(dy > 0){
		h -= dy;
	}
	if(w > 0 && h > 0){
		Rect sr;
		sr.x = srcx;
		sr.y = srcy;
		sr.w = dstrect->w = w;
		sr.h = dstrect->h = h;
		if(src->rlepixels){
			BlitSurfaceRLE(src, &sr, dst, dstrect);
		}else{
			BlitSurfaceFast(src, &sr, dst, dstrect);
		}
		return true;
	}
	dstrect->w = dstrect->h = 0;
	return false;
}

void Renderer::BlitSurfaceSlow(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	// A slow blit, x and y bounds are checked for safety by GetPixel/SetPixel
	int maxh = srcrect->h + srcrect->y;
	int maxw = srcrect->w + srcrect->x;
	for(int ys = srcrect->y, yd = dstrect->y; ys < maxh; ys++, yd++){
		for(int xs = srcrect->x, xd = dstrect->x; xs < maxw; xs++, xd++){
			Uint8 pixel = GetPixel(src, xs, ys);
			if(pixel != 0){
				SetPixel(dst, xd, yd, pixel);
			}
		}
	}
}

void Renderer::BlitSurfaceFast(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	// This function is slightly faster than BlitSurfaceSlow but there is no bounds checking, doesnt matter if used with blitsurfaceupper
	int srcw = src->w;
	int dstw = dst->w;
	int maxh = (srcrect->h + srcrect->y) * srcw;
	int maxw = srcrect->w + srcrect->x;
	void * srcpixels = src->pixels;
	void * dstpixels = dst->pixels;
	for(int ys = (srcrect->y * srcw), yd = (dstrect->y * dstw); ys < maxh; ys += srcw, yd += dstw){
		for(int xs = srcrect->x, xd = dstrect->x; xs < maxw; xs++, xd++){
			Uint8 pixel = ((Uint8 *)srcpixels)[xs + ys];
			if(pixel){
				((Uint8 *)dstpixels)[xd + yd] = pixel;
			}
		}
	}
}

void Renderer::BlitSurfaceRLE(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	Uint8 * rlepixels = src->rlepixels;
    int w = src->w;
	int x = dstrect->x;
	int y = dstrect->y;
	Uint8 * dstbuf = (Uint8 *)dst->pixels + y * dst->w + x;
    Uint8 * srcbuf = (Uint8 *)rlepixels;
	// Skip lines at the top if necessary
	int vskip = srcrect->y;
	int ofs = 0;
	if(vskip){
		while(1){
			int run;
			ofs += *(Uint8 *)srcbuf;
			run = ((Uint8 *)srcbuf)[1];
			srcbuf += sizeof(Uint8) * 2;
			if(run){
				srcbuf += run;
				ofs += run;
			}else
			if(!ofs){
				return;
			}
			if(ofs == w){
				ofs = 0;
				if(!--vskip){
					break;
				}
			}
		}
	}
    // If left or right edge clipping needed, call clipped blit
    if(srcrect->x || srcrect->w != src->w) {
		BlitSurfaceRLEClipped(w, srcbuf, srcrect, dst, dstrect);
    }else{
		do{
			int linecount = srcrect->h;
			int ofs = 0;
			while(1){
				unsigned run;
				ofs += *(Uint8 *)srcbuf;
				run = ((Uint8 *)srcbuf)[1];
				srcbuf += 2 * sizeof(Uint8);
				if(run){
					memcpy(dstbuf + ofs, srcbuf, run);
					srcbuf += run;
					ofs += run;
				}else
				if(!ofs){
					break;
				}
				if(ofs == w){
					ofs = 0;
					dstbuf += dst->w;
					if(!--linecount){
						break;
					}
				}
			}
		}while(0);
    }
	return;
}

void Renderer::BlitSurfaceRLEClipped(int w, Uint8 * srcbuf, Renderer::Rect * srcrect, Surface * dst, Renderer::Rect * dstrect){
	Uint8 * dstbuf = (Uint8 *)dst->pixels + dstrect->y * dst->w + dstrect->x;
	do{
		int linecount = srcrect->h;
		int ofs = 0;
		int left = srcrect->x;
		int right = left + srcrect->w;
		dstbuf -= left;
		while(1){
			int run;
			ofs += *(Uint8 *)srcbuf;
			run = ((Uint8 *)srcbuf)[1];
			srcbuf += 2 * sizeof(Uint8);
			if(run){
				// Clip to left and right borders
				if(ofs < right){
				int start = 0;
				int len = run;
				int startcol;
				if(left - ofs > 0){
					start = left - ofs;
					len -= start;
					if(len <= 0){
						goto nocopy;
					}
				}
				startcol = ofs + start;
				if(len > right - startcol){
					len = right - startcol;
				}
				memcpy(dstbuf + startcol, srcbuf + start, len);
			}
			nocopy:
			srcbuf += run;
			ofs += run;
			}else
			if(!ofs){
				break;
			}
			if(ofs == w){
				ofs = 0;
				dstbuf += dst->w;
				if(!--linecount){
					break;
				}
			}
		}
	}while(0);
}

void Renderer::BlitSprite(Object * object, Camera & camera, Surface * dst, Rect * dstrect, Surface * src, Rect * srcrect){
	if(src){
		if(object->drawcheckered){
			DrawCheckered(src, srcrect, dst, dstrect);
		}else
		if(object->drawalpha){
			DrawAlphaed(src, srcrect, dst, dstrect);
		}else{
			if(object->mirrored){
				dstrect->x = object->x - (world.resources.spritewidth[object->res_bank][object->res_index] - world.resources.spriteoffsetx[object->res_bank][object->res_index]) + camera.GetXOffset();
				DrawMirrored(src, srcrect, dst, dstrect);
			}else{
				BlitSurface(src, srcrect, dst, dstrect);
			}
		}
	}
}

void Renderer::DrawText(Surface * surface, Uint16 x, Uint16 y, const char * text, Uint8 bank, Uint8 width, bool alpha, Uint8 color, Uint8 brightness, bool rampcolor){
	Uint16 xc = 0;
	for(unsigned int i = 0; i < strlen(text); i++){
		if(text[i] == ' ' || text[i] == '\xA0' /* nbsp */){
			xc += width;
		}else{
			Uint8 ioffset = 33;
			if(bank == 132){
				ioffset = 34;
			}
			Surface * glyph = world.resources.spritebank[bank][Uint8(text[i] - ioffset)];
			if(glyph){
				Surface * es = 0;
				if(color || brightness){
					es = CreateSurfaceCopy(glyph);
					glyph = es;
					if(color){
						if(rampcolor){
							EffectRampColor(glyph, 0, color);
						}else{
							EffectColor(glyph, 0, color);
						}
					}
					if(brightness != 128){
						EffectBrightness(glyph, 0, brightness);
					}
				}
				Rect dstrect;
				dstrect.x = x + xc;
				dstrect.y = y;
				dstrect.w = surface->w;
				dstrect.h = surface->h;
				xc += width;
				if(alpha){
					DrawAlphaed(glyph, 0, surface, &dstrect);
				}else{
					BlitSurface(glyph, 0, surface, &dstrect);
				}
				if(es){
					//SDL_FreeSurface(es);
					delete es;
				}
			}
		}
	}
}

void Renderer::DrawTextInput(Surface * surface, TextInput & textinput){
	char * text = &textinput.text[textinput.scrolled];
	char password[256];
	if(textinput.password){
		memset(password, '*', strlen(text));
		password[strlen(text)] = 0;
		text = password;
	}
	Uint8 effectbrightness = textinput.effectbrightness;
	if(textinput.inactive){
		effectbrightness = 64;
	}
	DrawText(surface, textinput.x, textinput.y, text, textinput.res_bank, textinput.fontwidth, false, textinput.effectcolor, effectbrightness);
	if(!textinput.inactive && textinput.showcaret && state_i % 32 < 16){
		Rect dstrect;
		dstrect.x = textinput.x + (strlen(text) * textinput.fontwidth);
		dstrect.y = textinput.y - 1;
		dstrect.w = 1;
		dstrect.h = textinput.height * 0.8f;
		DrawFilledRectangle(surface, dstrect.x, dstrect.y, dstrect.x + dstrect.w, dstrect.y + dstrect.h, textinput.caretcolor);
		//SDL_FillRect(surface, &dstrect, textinput.caretcolor);
	}
}

void Renderer::DrawTinyText(Surface * surface, Uint16 x, Uint16 y, const char * text, Uint8 tint, Uint8 brightness){
	int width = strlen(text) * 4;
	for(int i = 0; i < strlen(text); i++){
		if(text[i] == '1'){
			width -= 1;
		}
	}
	for(int i = 0, j = 0; i < strlen(text); i++, j += 4){
		char ch[2];
		ch[0] = text[i];
		ch[1] = 0;
		DrawText(surface, x - (width / 2) + j, y, ch, 132, 4, false, tint, brightness);
		if(ch[0] == '1'){
			j -= 1;
		}
	}
}

void Renderer::DrawShadow(Surface * surface, Object * object){
	unsigned int width = world.resources.spritewidth[object->res_bank][object->res_index];
	Sint16 x = object->x;
	Sint16 y = object->y;
	int xv = 0;
	int yv = 300;
	Platform * platform = world.map.TestIncr(x, y - 1, x, y - 1, &xv, &yv, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
	if(platform && platform->XtoY(x) == y + yv){
		int i = 0;
		int y2 = -1;
		if(object->mirrored){
			x -= (width - world.resources.spriteoffsetx[object->res_bank][object->res_index]);
		}else{
			x -= world.resources.spriteoffsetx[object->res_bank][object->res_index];
		}
		for(int y1 = y + yv + camera.GetYOffset() - 1; y1 < y + yv + camera.GetYOffset() + 3; y1++){
			for(int x1 = x + camera.GetXOffset() + (abs(y2) * 4) - 4; x1 < x + camera.GetXOffset() + width - (abs(y2) * 4) + 4; x1++){
				if(i % 2){
					SetPixel(surface, x1, y1, 115);
				}
				i++;
			}
			i++;
			y2++;
		}
	}
}

void Renderer::DrawRain(Surface * surface, Camera & camera, float frametime){
	// 175:0 is rain
	for(int i = 0; i < raindropscount; i++){
		Rect dstrect;
		dstrect.x = ((raindropsx[i] / 16) % (640 + world.resources.spritewidth[175][0])) - world.resources.spritewidth[175][0];
		dstrect.y = ((raindropsy[i] / 16) % (480 + world.resources.spriteheight[175][0])) - world.resources.spriteheight[175][0];
		dstrect.x += ((raindropsoldx[i] / 16) - (raindropsx[i] / 16)) * frametime;
		dstrect.y += ((raindropsoldy[i] / 16) - (raindropsy[i] / 16)) * frametime;
		int x1 = dstrect.x - camera.GetXOffset();
		int y1 = dstrect.y - camera.GetYOffset();
		if(world.map.TestAABB(x1, y1, x1 + world.resources.spritewidth[175][0], y1 + world.resources.spriteheight[175][0], Platform::OUTSIDEROOM)){
			Platform * platform = world.map.TestAABB(x1, y1, x1 + world.resources.spritewidth[175][0], y1 + world.resources.spriteheight[175][0], Platform::STAIRSUP | Platform::STAIRSDOWN | Platform::RECTANGLE);
			if(!platform){
				BlitSurface(world.resources.spritebank[175][0], 0, surface, &dstrect);
			}
		}
	}
}

void Renderer::DrawRainPuddles(Surface * surface, Camera & camera){
	// 186:0-9 is rain puddle
	Rect dstrect;
	int j = 0;
	for(std::vector<Map::XY>::iterator it = world.map.rainpuddlelocations.begin(); it != world.map.rainpuddlelocations.end(); it++){
		Sint16 x = (*it).x;
		Sint16 y = (*it).y;
		dstrect.x = x + camera.GetXOffset();
		dstrect.y = y + camera.GetYOffset();
		int div = j % 2 ? 4 : 3;
		BlitSurface(world.resources.spritebank[186][(((state_i) / div) + (j)) % 10], 0, surface, &dstrect);
		j++;
	}
}

inline void Renderer::SetPixel(Surface * surface, unsigned int x, unsigned int y, Uint8 color){
	unsigned int surfacew = surface->w;
    if(x >= surfacew){
		return;
    }
    if(y >= surface->h){
		return;
    }
	((Uint8 *)surface->pixels)[x + (y * surfacew)] = color;
}

inline Uint8 Renderer::GetPixel(Surface * surface, unsigned int x, unsigned int y){
	unsigned int surfacew = surface->w;
	if(x >= surfacew){
		return 0;
    }
    if(y >= surface->h){
		return 0;
    }
	return ((Uint8 *)surface->pixels)[x + (y * surfacew)];
}

void Renderer::DrawDebug(Surface * surface){
	char temp[1234];
	sprintf(temp, "%d %d %d %d %d %d", world.localinput.keymovedown, world.localinput.keymoveup, world.localinput.keymoveleft, world.localinput.keymoveright, world.localinput.keyjump, world.localinput.keyjetpack);
	DrawText(surface, 10, 30, temp, 133, 7, false, -16);
	sprintf(temp, "mode: %s(%d), snapshots: %d, input packets: %d, ambience: %d objects: %d", world.mode ? "REPLICA" : "AUTHORITY", world.localpeerid, world.totalsnapshots, world.totalinputpackets, world.map.ambience, int(world.objectlist.size()));
	DrawText(surface, 10, 40, temp, 133, 7, false, -64);
	for(int i = 0; i < world.maxpeers; i++){
		if(world.peerlist[i]){
			char controlled[1234];
			controlled[0] = 0;
			for(std::list<Uint16>::iterator it = world.peerlist[i]->controlledlist.begin(); it != world.peerlist[i]->controlledlist.end(); it++){
				Object * object = world.GetObjectFromId(*it);
				if(object){
					sprintf(controlled, " %d(%d, %d) ", (*it), object->x, object->y);
				}
			}
			char temp[1234];
			sprintf(temp, "peerlist(%d)->controlled = (%s) ", i, controlled);
			DrawText(surface, 10, 50 + (10 * i), temp, 133, 7, false, -48);
			
			/*for(int j = 0; j < world.maxoldsnapshots; j++){
				Serializer * oldsnapshot = world.oldsnapshots[i][j];
				if(oldsnapshot){
					sprintf(temp, "peerlist(%d)->oldsnapshots[%d] = offset:%d", i, j, oldsnapshot->offset);
					DrawText(surface, 10, 100 + (10 * j), temp, 133, 7, false, -48);
				}
			}*/
		}
	}
}

void Renderer::DrawMessage(Surface * surface){
	if(!world.message_i){
		return;
	}
	int linelength = (int)strchr(world.message, '\n');
	if(!linelength){
		linelength = strlen(world.message);
	}else{
		linelength -= (int)world.message;
	}
	int liney = 60;
	Uint8 color = 208;
	int textwidth = 11;
	int textbank = 135;
	int lineheight = 20;
	switch(world.messagetype){
		case 1:
			color = 128;
			liney = 190;
			textbank = 134;
			textwidth = 10;
		case 2:
			
		break;
		case 3:
			color = 208;
			liney = 10;
			textbank = 134;
			textwidth = 10;
			lineheight = 14;
		break;
		case 10: // won
			color = 224;
		break;
		case 11: // lost
			color = 153;
		break;
		case 20: // connection lost
			color = 153;
			liney = 200;
		break;
	}
	int nextline = linelength;
	int line = 0;
	for(int i = 0; i < strlen(world.message); i++){
		if(i >= world.message_i){
			break;
		}
		Uint8 brightness = 128;
		if(world.messagetype < 10){
			if(world.message_i - world.messagetime + 8 >= 0){
				brightness -= (world.message_i - world.messagetime + 8) * 8;
			}
			if(world.message_i % 32 >= 16){
				brightness += ((16 - (world.message_i % 16)) * 2);
			}else{
				brightness += ((world.message_i % 16) * 2);
			}
		}
		if(world.message_i - i <= 5){
			brightness += 40 - ((world.message_i - i) * 8);
		}
		char temp[2];
		temp[0] = world.message[i];
		temp[1] = 0;
		if(world.messagetype >= 10){
			if(line == 0){
				textbank = 136;
				textwidth = 25;
			}else{
				textwidth = 13;
				textbank = 135;
			}
		}
		Uint8 brightness2 = (int(brightness) - 64) < 8 ? 8 : brightness - 64;
		DrawText(surface, ((640 - (linelength * textwidth)) / 2) + (textwidth * (linelength - nextline)) + 1, liney + 1, temp, textbank, textwidth, false, color, brightness2);
		DrawText(surface, ((640 - (linelength * textwidth)) / 2) + (textwidth * (linelength - nextline)), liney, temp, textbank, textwidth, false, color, brightness);
		nextline--;
		if(nextline < 0){
			linelength = (int)strchr(&world.message[i + 1], '\n');
			if(!linelength){
				linelength = strlen(&world.message[i + 1]);
			}else{
				linelength -= (int)&world.message[i + 1];
			}
			nextline = linelength;
			if(line == 0 && world.messagetype >= 10){
				liney += 40;
			}else{
				liney += lineheight;
			}
			line++;
		}
	}
}

void Renderer::DrawStatus(Surface * surface){
	int liney = 0;
	for(std::deque<char *>::iterator it = world.statusmessages.begin(); it != world.statusmessages.end(); it++){
		char * text = *it;
		char * time = &text[strlen(text) + 1];
		char * color = &text[strlen(text) + 2];
		Uint8 brightness = 128;
		if(*time <= 16){
			brightness -= (16 - *time) * 8;
		}
		Uint8 brightness2 = (int(brightness) - 64) < 8 ? 8 : brightness - 64;
		DrawText(surface, (640 - (strlen(text) * 7)) / 2 + 1, 370 + liney + 1, text, 133, 7, false, *color, brightness2);
		DrawText(surface, (640 - (strlen(text) * 7)) / 2, 370 + liney, text, 133, 7, false, *color, brightness);
		liney -= 10;
	}
}

void Renderer::DrawScaled(Surface * src, Rect * srcrect, Surface *dst, Rect * dstrect, int factor){
	for(int y = 0, y2 = 0; y < src->h; y += factor, y2++){
		for(int x = 0, x2 = 0; x < src->w; x += factor, x2++){
			Uint8 color = GetPixel(src, x, y);
			if(color){
				SetPixel(dst, dstrect->x + x2, dstrect->y + y2, color);
			}
		}
	}
}

void Renderer::DrawCheckered(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	if(src->w % 2 == 0){
		for(int y = 0; y < src->h; y++){
			for(int x = y % 2; x < src->w; x += 2){
				Uint8 color = GetPixel(src, x, y);
				if(color){
					SetPixel(dst, dstrect->x + x, dstrect->y + y, color);
				}
			}
		}
	}else{
		unsigned int i = 0;
		for(int y = 0; y < src->h; y++){
			for(int x = 0; x < src->w; x++){
				if(i % 2){
					Uint8 color = GetPixel(src, x, y);
					if(color){
						SetPixel(dst, dstrect->x + x, dstrect->y + y, color);
					}
				}
				i++;
			}
			i++;
		}
	}
}

void Renderer::DrawColored(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	for(int y = 0; y < src->h; y++){
		for(int x = 0; x < src->w; x++){
			Uint8 srcpixel = GetPixel(src, x, y);
			Uint8 newcolor = palette.Color(GetPixel(dst, dstrect->x + x, dstrect->y + y), srcpixel);
			if(srcpixel){
				SetPixel(dst, dstrect->x + x, dstrect->y + y, newcolor);
			}
		}
	}
}

void Renderer::DrawRampColored(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	for(int y = 0; y < src->h; y++){
		for(int x = 0; x < src->w; x++){
			Uint8 srcpixel = GetPixel(src, x, y);
			Uint8 color = palette.RampColor(GetPixel(dst, dstrect->x + x, dstrect->y + y), srcpixel);
			if(srcpixel){
				SetPixel(dst, dstrect->x + x, dstrect->y + y, color);
			}
		}
	}
}

void Renderer::DrawBrightened(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect, Uint8 brightness){
	for(int y = 0; y < src->h; y++){
		for(int x = 0; x < src->w; x++){
			Uint8 srcpixel = GetPixel(src, x, y);
			Uint8 color = palette.Brightness(srcpixel, brightness);
			if(srcpixel){
				SetPixel(dst, dstrect->x + x, dstrect->y + y, color);
			}
		}
	}
}

void Renderer::DrawAlphaed(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	for(int y = 0; y < src->h; y++){
		for(int x = 0; x < src->w; x++){
			Uint8 srcpixel = GetPixel(src, x, y);
			Uint8 color = palette.Alpha(srcpixel, GetPixel(dst, dstrect->x + x, dstrect->y + y));
			if(srcpixel){
				SetPixel(dst, dstrect->x + x, dstrect->y + y, color);
			}
		}
	}
}

Surface * Renderer::CreateSurfaceCopy(Surface * src){
	Surface * effectsurface = new Surface(src->w, src->h);
	BlitSurface(src, 0, effectsurface, 0);
	return effectsurface;
}

void Renderer::EffectHacking(Surface * dst, Rect * dstrect, Uint8 color){
	Uint8 index = state_i % 8;
	if(index == 0){
		ex = (rand() % 64);
		ey = (rand() % 64);
	}
	int dstw = dst->w;
	int dsth = dst->h;
	for(int y = 0; y < dsth; y++){
		for(int x = 0; x < dstw; x++){
			Uint8 overlay = GetPixel(world.resources.spritebank[178][index], x + ex, y + ey);
			Uint8 pixel = GetPixel(dst, x, y);
			if(overlay && pixel){
				//SetPixel(dst, x, y, palette.Mix(color, pixel));
				SetPixel(dst, x, y, palette.RampColorMin(pixel, color));
			}
		}
	}
}

void Renderer::EffectTeamColor(Surface * dst, Rect * dstrect, Uint8 values){
	Uint8 brightness = (values >> 4) * 16;
	Uint8 color = (values & 0x0F) * 16;
	int dstw = dst->w;
	int dsth = dst->h;
	for(int y = 0; y < dsth; y++){
		for(int x = 0; x < dstw; x++){
			Uint8 * pixel = &((Uint8 *)dst->pixels)[x + (y * dstw)];
			if(*pixel >= 195 && *pixel <= 208){
				*pixel = palette.Color(*pixel/* - 195 + 114*/, color);
				*pixel = palette.Brightness(*pixel/* - 195 + 114*/, brightness);
            }else
            if(*pixel >= 81 && *pixel <= 92){
				*pixel = palette.Color(*pixel + 128/* - 81 + 114*/, color);
				*pixel = palette.Brightness(*pixel/* - 195 + 114*/, brightness);
            }
        }
    }
}

Uint8 Renderer::TeamColorToIndex(Uint8 values){
	Uint8 brightness = (values >> 4) * 16;
	Uint8 color = (values & 0x0F) * 16;
	Uint8 index = palette.Color(204, color);
	index = palette.Brightness(index, brightness);
	return index;
}

void Renderer::EffectBrightness(Surface * dst, Rect * dstrect, Uint8 brightness){
	Rect zerodstrect;
	if(!dstrect){
		zerodstrect.w = dst->w;
		zerodstrect.h = dst->h;
		zerodstrect.x = 0;
		zerodstrect.y = 0;
		dstrect = &zerodstrect;
	}
	for(Uint32 x = dstrect->x; x < dstrect->x + dstrect->w; x++){
        for(Uint32 y = dstrect->y; y < dstrect->y + dstrect->h; y++){
			Uint8 * pixel = &((Uint8 *)dst->pixels)[x + (y * dst->w)];
			*pixel = palette.Brightness(*pixel, brightness);
        }
    }
}

void Renderer::EffectColor(Surface * dst, Rect * dstrect, Uint8 color){
	Rect zerodstrect;
	if(!dstrect){
		zerodstrect.w = dst->w;
		zerodstrect.h = dst->h;
		zerodstrect.x = 0;
		zerodstrect.y = 0;
		dstrect = &zerodstrect;
	}
	for(Uint32 x = dstrect->x; x < dstrect->w + dstrect->x; x++){
        for(Uint32 y = dstrect->y; y < dstrect->h + dstrect->y; y++){
			Uint8 * pixel = &((Uint8 *)dst->pixels)[x + (y * dst->w)];
			*pixel = palette.Color(*pixel, color);
        }
    }
}

void Renderer::EffectRampColor(Surface * dst, Rect * dstrect, Uint8 color){
	Rect zerodstrect;
	if(!dstrect){
		zerodstrect.w = dst->w;
		zerodstrect.h = dst->h;
		zerodstrect.x = 0;
		zerodstrect.y = 0;
		dstrect = &zerodstrect;
	}
	for(Uint32 x = dstrect->x; x < dstrect->w + dstrect->x; x++){
        for(Uint32 y = dstrect->y; y < dstrect->h + dstrect->y; y++){
			Uint8 * pixel = &((Uint8 *)dst->pixels)[x + (y * dst->w)];
			if(*pixel){
				*pixel = palette.RampColor(*pixel, color);
			}
        }
    }
}

void Renderer::EffectRampColorPlus(Surface * dst, Rect * dstrect, Uint8 color, Uint8 plus){
	Rect zerodstrect;
	if(!dstrect){
		zerodstrect.w = dst->w;
		zerodstrect.h = dst->h;
		zerodstrect.x = 0;
		zerodstrect.y = 0;
		dstrect = &zerodstrect;
	}
	for(Uint32 x = dstrect->x; x < dstrect->w + dstrect->x; x++){
        for(Uint32 y = dstrect->y; y < dstrect->h + dstrect->y; y++){
			Uint8 * pixel = &((Uint8 *)dst->pixels)[x + (y * dst->w)];
			if(*pixel){
				*pixel = palette.RampColorPlus(*pixel, color, plus);
			}
        }
    }
}

void Renderer::EffectHit(Surface * dst, Rect * dstrect, Uint8 hitx, Uint8 hity, Uint8 state_hit){
	Uint8 hit_type = state_hit / 32;
	Uint8 index = (state_hit % 32) - 1;
	if(index > 7){
		index = 7 - (index - 7);
	}
	if(index <= 7){
		Uint8 color = 0;
		switch(hit_type){
			case 0:{
				color = 146;
			}break;
			case 1:{
				color = 146;
			}break;
			case 2:{
				color = 194;
			}break;
		}
		int xoffset = ((float(hitx) / 100) * dst->w) - world.resources.spriteoffsetx[153][index];
		int yoffset = ((float(hity) / 100) * dst->h) - world.resources.spriteoffsety[153][index];
		for(int y = 0; y < world.resources.spritebank[153][index]->h; y++){
			for(int x = 0; x < world.resources.spritebank[153][index]->w; x++){
				Uint8 overlay = GetPixel(world.resources.spritebank[153][index], x, y);
				Uint8 pixel = GetPixel(dst, x + xoffset, y + yoffset);
				if(overlay && pixel){
					//Uint8 newcolor = palette.RampColorMin(pixel, color);
					Uint8 newcolor = palette.RampColor(pixel, color) + 6;
					if(newcolor > color + 15){
						newcolor = color + 15;
					}
					SetPixel(dst, x + xoffset, y + yoffset, newcolor);
				}
			}
		}
	}
	if(hit_type == 1){
		EffectShieldDamage(dst, 0, 205);
	}
}

void Renderer::EffectShieldDamage(Surface * dst, Rect * dstrect, Uint8 color){
	// 177:0-7 is shield damage stencil
	for(int y = 0; y < dst->h; y++){
		for(int x = 0; x < dst->w; x++){
			Uint8 overlay = GetPixel(world.resources.spritebank[177][state_i % 8], x, y);
			Uint8 pixel = GetPixel(dst, x, y);
			if(overlay && pixel){
				SetPixel(dst, x, y, palette.RampColorMin(pixel, color, 14));//SetPixel(dst, x, y, color);//palette.Mix(205, pixel));
			}
		}
	}
}

void Renderer::EffectWarp(Surface * dst, Rect * dstrect, Uint8 state_warp){
	Uint8 index = 7;
	int yoffset = (state_warp - 8) * 12;
	if(state_warp >= 12){
		yoffset = (state_warp - 20) * -12;
	}
	int i = 0;
	for(int y = 0; y < dst->h; y++){
		for(int x = 0; x < dst->w; x++){
			Uint8 overlay = GetPixel(world.resources.spritebank[153][index], x, y + yoffset);
			Uint8 pixel = GetPixel(dst, x, y);
			if(pixel){
				if(overlay && i % 2){
					SetPixel(dst, x, y, 128);
				}else
				if(!overlay && y > -yoffset){
					SetPixel(dst, x, y, 0);
				}
			}
			i++;
		}
		i++;
	}
}

void Renderer::MiniMapBlit(Uint8 res_bank, Uint8 res_index, int x, int y, bool alpha, Uint8 teamcolor){
	Rect dstrect;
	int xm = x, ym = y;
	world.map.MiniMapCoords(xm, ym);
	dstrect.x = xm - world.resources.spriteoffsetx[res_bank][res_index];
	dstrect.y = ym - world.resources.spriteoffsety[res_bank][res_index];
	if(teamcolor || alpha){
		Surface * newsurface = CreateSurfaceCopy(world.resources.spritebank[res_bank][res_index]);
		if(teamcolor){
			EffectTeamColor(newsurface, 0, teamcolor);
		}
		if(alpha){
			DrawAlphaed(newsurface, 0, &world.map.minimap.surface, &dstrect);
		}else{
			BlitSurface(newsurface, 0, &world.map.minimap.surface, &dstrect);
		}
		delete newsurface;
	}else{
		BlitSurface(world.resources.spritebank[res_bank][res_index], 0, &world.map.minimap.surface, &dstrect);
	}
}

void Renderer::MiniMapCircle(int x, int y, Uint8 color){
	int x1 = x;
	int y1 = y;
	world.map.MiniMapCoords(x1, y1);
	int radius = ((state_i % 16) / 2) + 3;
	Uint8 newcolor = color - ((state_i % 16) / 2) + 4;
	Uint8 oldramp = (color - 2) / 16;
	Uint8 newramp = (newcolor - 2) / 16;
	if(newramp < oldramp){
		newcolor = (oldramp * 16) + 2;
	}else
	if(newramp > oldramp){
		newcolor = (oldramp * 16) + 2 + 15;
	}
	DrawCircle(&world.map.minimap.surface, x1, y1, radius, newcolor);
}

void Renderer::DrawMirrored(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect){
	int srch = src->h;
	int srcw = src->w;
	int dstrectx = dstrect->x;
	int dstrecty = dstrect->y;
	for(int y = 0; y < srch; y++){
        for(int x = 0; x < srcw; x++){
			Uint8 color = GetPixel(src, srcw - (x + 1), y);
			if(color){
				SetPixel(dst, dstrectx + x, dstrecty + y, color);
			}
        }
    }
}

void Renderer::DrawLight(Surface * surface, Surface * src, Rect * rect){
	if(rect->x <= surface->w && rect->y <= surface->h && rect->x >= -src->w && rect->y >= -src->h){
		int surfacew = surface->w;
		int surfaceh = surface->h;
		int minw = rect->x;
		int minx1 = 0;
		if(minw < 0){
			minx1 += abs(minw);
			minw = 0;
		}
		int minh = rect->y;
		int miny1 = 0;
		if(minh < 0){
			miny1 += abs(minh);
			minh = 0;
		}
		int maxw = rect->x + src->w;
		if(maxw > surfacew){
			maxw = surfacew;
		}
		int maxh = rect->y + src->h;
		if(maxh > surfaceh){
			maxh = surfaceh;
		}
		int srcw = src->w;
		Uint8 * pixels = (Uint8 *)src->pixels;
		for(int y1 = miny1, y2 = minh; y2 < maxh; y1++, y2++){
			unsigned int i = (y1 * srcw) + minx1;
			unsigned int i2 = (y2 * surfacew) + minw;
			for(int x2 = minw; x2 < maxw; x2++){
				int lum = pixels[i];
				if(lum){
					Uint8 * surfacepixel = &surface->pixels[i2];
					Uint8 newcolor = palette.Light(*surfacepixel, lum % 16);
					*surfacepixel = newcolor;
				}
				/*
				// Unoptimized lighting algorithm
				int lum = pixels[i] % 16;
				Uint8 * surfacepixel = &surface->pixels[i2];
				if(lum && *surfacepixel < 114 && *surfacepixel > 1){
					int lum2 = (lum * 8) + ambiencelevel;
					if(lum2 > 128){
						lum = 128;
					}else{
						lum = lum2;
					}
					Uint8 newcolor = palette.Brightness(*surfacepixel, lum);
					int newcolorbrightness = (newcolor - 2) % 16;
					Uint8 oldcolorbrightness = (*surfacepixel - 2) % 16;
					if(newcolorbrightness >= oldcolorbrightness * (float(ambiencelevel) / 128)){
						*surfacepixel = newcolor + 112;
					}
				}*/
				i++;
				i2++;
			}
		}
	}
}

void Renderer::DrawTile(Surface * surface, Surface * tile, Rect * rect){
	BlitSurface(tile, 0, surface, rect);
}

void Renderer::DrawParallax(Surface * surface, Camera & camera){
	for(int y = 0; y < 12; y++){
		for(int x = 0; x < 20; x++){
			Rect dstrect;
			dstrect.x = x * 64;
			dstrect.y = y * 64;
			dstrect.w = 64;
			dstrect.h = 64;
			int cx = -(camera.GetXOffset() - 320);
			int cy = -(camera.GetYOffset() - 240);
			dstrect.x = dstrect.x -ceil(((float)cx / (world.map.width * 64)) * 320);
			dstrect.y = dstrect.y -ceil(((float)cy / (world.map.height * 64)) * 240);
			DrawTile(surface, world.resources.spritebank[world.map.parallax][(y * 20) + x], &dstrect);
		}
	}
}

void Renderer::DrawBackground(Surface * surface, Camera & camera, bool drawluminance){
	Rect dstrect;
	Surface * tile;
	int minx = (camera.x - (camera.w / 2)) / 64;
	if(minx < 0){
		minx = 0;
	}
	int maxx = ceil(float(camera.x + (camera.w / 2)) / 64);
	if(maxx > world.map.expandedwidth){
		maxx = world.map.expandedwidth;
	}
	int miny = (camera.y - (camera.h / 2)) / 64;
	if(miny < 0){
		miny = 0;
	}
	int maxy = ceil(float(camera.y + (camera.h / 2)) / 64);
	if(maxy > world.map.expandedheight){
		maxy = world.map.expandedheight;
	}
	for(int y = miny; y < maxy; y++){
		for(int x = minx; x < maxx; x++){
			int i = (y * world.map.expandedwidth) + x;
			for(int j = 0; j < 4; j++){
				if(world.map.bg[j][i]){
					dstrect.x = (x * 64) + camera.GetXOffset();
					dstrect.y = (y * 64) + camera.GetYOffset();
					if(world.map.bgflipped[j][i]){
						tile = world.resources.tileflippedbank[(world.map.bg[j][i] & 0xFF00) >> 8][world.map.bg[j][i] & 0xFF];
					}else{
						tile = world.resources.tilebank[(world.map.bg[j][i] & 0xFF00) >> 8][world.map.bg[j][i] & 0xFF];
					}
					if(tile){
						if(world.map.bglum[j][i]){
							if(drawluminance){
								DrawLight(surface, tile, &dstrect);
							}
						}else{
							DrawTile(surface, tile, &dstrect);
						}
					}
				}
			}
		}
	}
}

void Renderer::DrawForeground(Surface * surface, Camera & camera){
	Rect dstrect;
	Surface * tile;
	int minx = (camera.x - (camera.w / 2)) / 64;
	if(minx < 0){
		minx = 0;
	}
	int maxx = ceil(float(camera.x + (camera.w / 2)) / 64);
	if(maxx > world.map.expandedwidth){
		maxx = world.map.expandedwidth;
	}
	int miny = (camera.y - (camera.h / 2)) / 64;
	if(miny < 0){
		miny = 0;
	}
	int maxy = ceil(float(camera.y + (camera.h / 2)) / 64);
	if(maxy > world.map.expandedheight){
		maxy = world.map.expandedheight;
	}
	for(int y = miny; y < maxy; y++){
		for(int x = minx; x < maxx; x++){
			int i = (y * world.map.expandedwidth) + x;
			for(int j = 0; j < 4; j++){
				if(world.map.fg[j][i]){
					dstrect.x = (x * 64) + camera.GetXOffset();
					dstrect.y = (y * 64) + camera.GetYOffset();
					if(world.map.fgflipped[j][i]){
						tile = world.resources.tileflippedbank[(world.map.fg[j][i] & 0xFF00) >> 8][world.map.fg[j][i] & 0xFF];
					}else{
						tile = world.resources.tilebank[(world.map.fg[j][i] & 0xFF00) >> 8][world.map.fg[j][i] & 0xFF];
					}
					if(tile){
						if(!world.map.fglum[j][i]){
							DrawTile(surface, tile, &dstrect);
						}
					}
				}
			}
			/*if(world.map.nodetypes[i]){
				DrawFilledRectangle(surface, (x * 64) + camera.GetXOffset() + 28, (y * 64) + camera.GetYOffset() + 28, (x * 64) + camera.GetXOffset() + 36, (y * 64) + camera.GetYOffset() + 36, 224);
			}*/
		}
	}
}

void Renderer::DrawForegroundLuminance(Surface * surface, Camera & camera){
	Rect dstrect;
	Surface * tile;
	int minx = (camera.x - (camera.w / 2)) / 64;
	if(minx < 0){
		minx = 0;
	}
	int maxx = ceil(float(camera.x + (camera.w / 2)) / 64);
	if(maxx > world.map.expandedwidth){
		maxx = world.map.expandedwidth;
	}
	int miny = (camera.y - (camera.h / 2)) / 64;
	if(miny < 0){
		miny = 0;
	}
	int maxy = ceil(float(camera.y + (camera.h / 2)) / 64);
	if(maxy > world.map.expandedheight){
		maxy = world.map.expandedheight;
	}
	for(int y = miny; y < maxy; y++){
		for(int x = minx; x < maxx; x++){
			int i = (y * world.map.expandedwidth) + x;
			for(int j = 0; j < 4; j++){
				if(world.map.fg[j][i]){
					dstrect.x = (x * 64) + camera.GetXOffset();
					dstrect.y = (y * 64) + camera.GetYOffset();
					if(world.map.fgflipped[j][i]){
						tile = world.resources.tileflippedbank[(world.map.fg[j][i] & 0xFF00) >> 8][world.map.fg[j][i] & 0xFF];
					}else{
						tile = world.resources.tilebank[(world.map.fg[j][i] & 0xFF00) >> 8][world.map.fg[j][i] & 0xFF];
					}
					if(tile){
						if(world.map.fglum[j][i]){
							DrawLight(surface, tile, &dstrect);
						}
					}
				}
			}
		}
	}
}

void Renderer::DrawHUD(Surface * surface, float frametime){
	Player * player = 0;
	Peer * peer = world.peerlist[world.localpeerid];
	if(peer){
		for(std::list<Uint16>::iterator it = peer->controlledlist.begin(); it != peer->controlledlist.end(); it++){
			Object * object = world.GetObjectFromId(*it);
			if(object){
				if(object->type == ObjectTypes::PLAYER){
					player = static_cast<Player *>(object);
				}
			}
		}
	}
	Rect dstrect;
	
	if(localplayer){
		// Draw main status bar
		
		// Draw system 1, system 2 screens
		// 135x44
		
		if(world.systemcameraactive[0]){
			dstrect.x = -world.resources.spriteoffsetx[95][2];
			dstrect.y = -world.resources.spriteoffsety[92][2] + 381;
			BlitSurface(world.resources.spritebank[95][2], 0, surface, &dstrect);
			
			Surface systemscreen(135, 44, 1);
			Camera camera(135 * 2, 44 * 2);
			Object * followobject = world.GetObjectFromId(world.systemcamerafollow[0]);
			int px = 0;
			int py = 0;
			if(followobject){
				px = followobject->x + ((followobject->oldx - followobject->x) * frametime);
				py = followobject->y + ((followobject->oldy - followobject->y) * frametime);
			}
			camera.Follow(world, px + world.systemcamerax[0], py + world.systemcameray[0], 0, 0, 0, 0);
			DrawWorldScaled(&systemscreen, camera, 3, frametime);
			EffectRampColor(&systemscreen, 0, 190);
			dstrect.x = 5;
			dstrect.y = 349;
			BlitSurface(&systemscreen, 0, surface, &dstrect);
		}
		if(world.systemcameraactive[1]){
			dstrect.x = -world.resources.spriteoffsetx[95][11];
			dstrect.y = -world.resources.spriteoffsety[92][11] + 318;
			BlitSurface(world.resources.spritebank[95][11], 0, surface, &dstrect);
			
			Surface systemscreen(135, 44, 1);
			Camera camera(135 * 2, 44 * 2);
			Object * followobject = world.GetObjectFromId(world.systemcamerafollow[1]);
			int px = 0;
			int py = 0;
			if(followobject){
				px = followobject->x + ((followobject->oldx - followobject->x) * frametime);
				py = followobject->y + ((followobject->oldy - followobject->y) * frametime);
			}
			camera.Follow(world, px + world.systemcamerax[1], py + world.systemcameray[1], 0, 0, 0, 0);
			DrawWorldScaled(&systemscreen, camera, 3, frametime);
			EffectRampColor(&systemscreen, 0, 190);
			dstrect.x = 500;
			dstrect.y = 348;
			BlitSurface(&systemscreen, 0, surface, &dstrect);
		}
		//

		
		dstrect.x = 235;
		dstrect.y = 419;
		BlitSurface(&world.map.minimap.surface, 0, surface, &dstrect);
		dstrect.x = -world.resources.spriteoffsetx[94][0];
		dstrect.y = -world.resources.spriteoffsety[94][0];
		BlitSurface(world.resources.spritebank[94][0], 0, surface, &dstrect);
		if(player){
			if(player->fuellow){
				dstrect.x = -world.resources.spriteoffsetx[95][8];
				dstrect.y = -world.resources.spriteoffsety[95][8];
				BlitSurface(world.resources.spritebank[95][8], 0, surface, &dstrect);
			}
			Rect srcrect;
			srcrect.x = 0;
			srcrect.y = 0;
			srcrect.w = (((float)player->fuel/player->maxfuel) * world.resources.spritebank[95][6]->w);
			srcrect.h = world.resources.spritebank[95][6]->h;
			dstrect.x = -world.resources.spriteoffsetx[95][6];
			dstrect.y = -world.resources.spriteoffsety[95][6];
			BlitSurface(world.resources.spritebank[95][6], &srcrect, surface, &dstrect);
			dstrect.x = -world.resources.spriteoffsetx[95][5];
			dstrect.y = -world.resources.spriteoffsety[95][5];
			BlitSurface(world.resources.spritebank[95][5], 0, surface, &dstrect);
			srcrect.w = world.resources.spritebank[95][0]->w;
			srcrect.h = world.resources.spritebank[95][0]->h;
			srcrect.y = srcrect.h - (((float)player->health/player->maxhealth) * srcrect.h);
			dstrect.x = -world.resources.spriteoffsetx[95][0];
			dstrect.y = -world.resources.spriteoffsety[95][0];
			dstrect.y += srcrect.y;
			BlitSurface(world.resources.spritebank[95][0], &srcrect, surface, &dstrect);
			srcrect.w = world.resources.spritebank[95][1]->w;
			srcrect.h = world.resources.spritebank[95][1]->h;
			srcrect.y = srcrect.h - (((float)player->shield/player->maxshield) * srcrect.h);
			if(srcrect.y < 0){
				srcrect.y = 0;
			}
			dstrect.x = -world.resources.spriteoffsetx[95][1];
			dstrect.y = -world.resources.spriteoffsety[95][1];
			dstrect.y += srcrect.y;
			if(player->shield > player->maxshield){
				Surface * effectsurface = CreateSurfaceCopy(world.resources.spritebank[95][1]);
				Uint8 brightness = 136;
				Uint8 time = 6;
				if(state_i % (time * 2) < time){
					brightness += (state_i % time) * 2;
				}else{
					brightness += (time - (state_i % time)) * 2;
				}
				EffectBrightness(effectsurface, 0, brightness);
				BlitSurface(effectsurface, &srcrect, surface, &dstrect);
				delete effectsurface;
			}else{
				BlitSurface(world.resources.spritebank[95][1], &srcrect, surface, &dstrect);
			}
			srcrect.y = 0;
			srcrect.w = (((float)player->files/player->maxfiles) * world.resources.spritebank[95][7]->w);
			srcrect.h = world.resources.spritebank[95][7]->h;
			dstrect.x = -world.resources.spriteoffsetx[95][7];
			dstrect.y = -world.resources.spriteoffsety[95][7];
			BlitSurface(world.resources.spritebank[95][7], &srcrect, surface, &dstrect);
			srcrect.w = surface->w;
			srcrect.h = surface->h;
			Uint8 currentammo = 0;
			switch(player->currentweapon){
				case 0:
					currentammo = 99;
					dstrect.x = -world.resources.spriteoffsetx[96][1];
					dstrect.y = -world.resources.spriteoffsety[96][1];
					BlitSurface(world.resources.spritebank[96][1], &srcrect, surface, &dstrect);
					dstrect.x = -world.resources.spriteoffsetx[96][5];
					dstrect.y = -world.resources.spriteoffsety[96][5];
					BlitSurface(world.resources.spritebank[96][5], &srcrect, surface, &dstrect);
				break;
				case 1:
					currentammo = player->laserammo;
					dstrect.x = -world.resources.spriteoffsetx[96][2];
					dstrect.y = -world.resources.spriteoffsety[96][2];
					BlitSurface(world.resources.spritebank[96][2], &srcrect, surface, &dstrect);
					dstrect.x = -world.resources.spriteoffsetx[96][6];
					dstrect.y = -world.resources.spriteoffsety[96][6];
					BlitSurface(world.resources.spritebank[96][6], &srcrect, surface, &dstrect);
				break;
				case 2:
					currentammo = player->rocketammo;
					dstrect.x = -world.resources.spriteoffsetx[96][3];
					dstrect.y = -world.resources.spriteoffsety[96][3];
					BlitSurface(world.resources.spritebank[96][3], &srcrect, surface, &dstrect);
					dstrect.x = -world.resources.spriteoffsetx[96][7];
					dstrect.y = -world.resources.spriteoffsety[96][7];
					BlitSurface(world.resources.spritebank[96][7], &srcrect, surface, &dstrect);
				break;
				case 3:
					currentammo = player->flamerammo;
					dstrect.x = -world.resources.spriteoffsetx[96][4];
					dstrect.y = -world.resources.spriteoffsety[96][4];
					BlitSurface(world.resources.spritebank[96][4], &srcrect, surface, &dstrect);
					dstrect.x = -world.resources.spriteoffsetx[96][8];
					dstrect.y = -world.resources.spriteoffsety[96][8];
					BlitSurface(world.resources.spritebank[96][8], &srcrect, surface, &dstrect);
				break;
			}
			dstrect.x = -world.resources.spriteoffsetx[96][0];
			dstrect.y = -world.resources.spriteoffsety[96][0] + (player->currentweapon * 14);
			BlitSurface(world.resources.spritebank[96][0], &srcrect, surface, &dstrect);
			
			char ammo[64] = "";
			sprintf(ammo, "%s%d", currentammo < 10 ? " " : "", currentammo);
			DrawText(surface, 117, 457, ammo, 135, 12, true);
			strcpy(ammo, "99");
			DrawTinyText(surface, 10, 414, ammo);
			if(player->laserammo > 0){
				sprintf(ammo, "%s%d", player->laserammo < 10 ? " " : "", player->laserammo);
				DrawTinyText(surface, 10, 428, ammo);
			}
			if(player->rocketammo > 0){
				sprintf(ammo, "%s%d", player->rocketammo < 10 ? " " : "", player->rocketammo);
				DrawTinyText(surface, 10, 442, ammo);
			}
			if(player->flamerammo > 0){
				sprintf(ammo, "%s%d", player->flamerammo < 10 ? " " : "", player->flamerammo);
				DrawTinyText(surface, 10, 456, ammo);
			}
			char credits[64] = "";
			sprintf(credits, "%d", player->credits);
			DrawText(surface, 572, 456, credits, 135, 12, false, 202);
			
			if(player->health && (float)player->health / player->maxhealth <= 0.5 && state_i % 8 <= 3){
				dstrect.x = -world.resources.spriteoffsetx[95][3];
				dstrect.y = -world.resources.spriteoffsety[95][3];
				BlitSurface(world.resources.spritebank[95][3], &srcrect, surface, &dstrect);
			}
			char health[64] = "";
			sprintf(health, "%d", player->health);
			DrawTinyText(surface, 158, 463, health, 161);
			
			if(player->shield && (float)player->shield / player->maxshield <= 0.5 && state_i % 8 <= 3){
				dstrect.x = -world.resources.spriteoffsetx[95][4];
				dstrect.y = -world.resources.spriteoffsety[95][4];
				BlitSurface(world.resources.spritebank[95][4], &srcrect, surface, &dstrect);
			}
			char shield[64] = "";
			sprintf(shield, "%d", player->shield);
			DrawTinyText(surface, 481, 463, shield, 202);
			
			// Draw inventory
			
			dstrect.x = -world.resources.spriteoffsetx[94][2];
			dstrect.y = -world.resources.spriteoffsety[94][2];
			BlitSurface(world.resources.spritebank[94][2], 0, surface, &dstrect);
			
			int xoffsets[] = {612, 584, 556, 528};
			int yoffsets[] = {13, 13, 11, 7};
			for(int i = 0; i < 4; i++){
				Uint8 invindex = InvIdToResIndex(player->inventoryitems[i]);
				dstrect.x = -world.resources.spriteoffsetx[97][invindex] + xoffsets[i];
				dstrect.y = -world.resources.spriteoffsety[97][invindex] + yoffsets[i];
				if(player->currentinventoryitem == i){
					BlitSurface(world.resources.spritebank[97][invindex], 0, surface, &dstrect);
				}else{
					if(world.resources.spritebank[97][invindex]){
						Surface * effectsurface = CreateSurfaceCopy(world.resources.spritebank[97][invindex]);
						EffectBrightness(effectsurface, 0, 32);
						BlitSurface(effectsurface, 0, surface, &dstrect);
						delete effectsurface;
					}
				}
				
				if(player->inventoryitemsnum[i] > 1){
					char text[10];
					sprintf(text, "%d", player->inventoryitemsnum[i]);
					DrawText(surface, xoffsets[i] + 20, yoffsets[i] + 20, text, 132, 6);
				}
			}
		

			
			// Draw teams
			
			std::vector<Team *> teams;
			for(std::vector<Uint16>::iterator it = world.objectsbytype[ObjectTypes::TEAM].begin(); it != world.objectsbytype[ObjectTypes::TEAM].end(); it++){
				teams.push_back(static_cast<Team *>(world.GetObjectFromId((*it))));
			}
			if(teams.size() == 1){
				dstrect.x = -world.resources.spriteoffsetx[94][1];
				dstrect.y = -world.resources.spriteoffsety[94][1];
				BlitSurface(world.resources.spritebank[94][1], 0, surface, &dstrect);
			}else{
				dstrect.x = -world.resources.spriteoffsetx[103][0];
				dstrect.y = -world.resources.spriteoffsety[103][0] - 133 + ((teams.size() - 1) * 20);
				BlitSurface(world.resources.spritebank[103][0], 0, surface, &dstrect);
				dstrect.x = -world.resources.spriteoffsetx[103][1];
				dstrect.y = -world.resources.spriteoffsety[103][1];
				BlitSurface(world.resources.spritebank[103][1], 0, surface, &dstrect);
			}
			int teamyoffset = 5;
			for(std::vector<Team *>::iterator it = teams.begin(); it != teams.end(); it++){
				Team * team = *it;
				dstrect.x = 5;
				dstrect.y = teamyoffset + 1;
				Surface * newsurface = CreateSurfaceCopy(world.resources.spritebank[181][team->agency]);
				EffectTeamColor(newsurface, 0, team->GetColor());
				DrawScaled(newsurface, 0, surface, &dstrect);
				delete newsurface;
				for(int i = 0; i < team->numpeers; i++){
					if(world.peerlist[team->peers[i]]){
						Player * player = world.GetPeerPlayer(world.peerlist[team->peers[i]]->id);
						if(player){
							Uint8 index = (player->state == Player::DEAD || player->state == Player::DYING ? 8 : 4);
							dstrect.x = -world.resources.spriteoffsetx[103][index + i] + 25 + (17 * i);
							dstrect.y = -world.resources.spriteoffsety[103][index + i] + teamyoffset;
							if(player->InBase(world) || player->hassecret){
								Surface * effectsurface = CreateSurfaceCopy(world.resources.spritebank[103][index + i]);
								Uint8 plus = 0;
								Uint8 time = 6;
								Uint8 shift = 2;
								Uint8 color = 210;
								if(player->hassecret){
									time = 6;
									color = 114;
									shift = 0;
								}
								if((state_i >> shift) % (time * 2) < time){
									plus += ((state_i >> shift) % time);
								}else{
									plus += time - ((state_i >> shift) % time);
								}
								EffectRampColorPlus(effectsurface, 0, color, plus);
								BlitSurface(effectsurface, 0, surface, &dstrect);
								delete effectsurface;
							}else{
								BlitSurface(world.resources.spritebank[103][index + i], 0, surface, &dstrect);
							}
						}
					}
				}
				for(int i = 0; i < 3; i++){
					Uint8 index = team->secrets > i ? 2 : 3;
					dstrect.x = -world.resources.spriteoffsetx[103][index] - (9 * (3 - i)) + 11;
					dstrect.y = -world.resources.spriteoffsety[103][index] + teamyoffset;
					BlitSurface(world.resources.spritebank[103][index], 0, surface, &dstrect);
				}
				teamyoffset += 20;
			}
			
			
			// Draw secrets
			
			Team * team = player->GetTeam(world);
			
			if(team && team->basedoorid){
				int secretprogress = team->secretprogress;
				
				int yoffset = 60;
				if(teams.size() >= 3){
					yoffset += (teams.size() * 20) - 65;
				}
				if(!team->beamingterminalid){
					int lineheight = 13;
					dstrect.x = -world.resources.spriteoffsetx[187][0];
					dstrect.y = -world.resources.spriteoffsety[187][0] + yoffset;
					BlitSurface(world.resources.spritebank[187][0], 0, surface, &dstrect);
					Uint8 color = 0;
					Uint8 brightness = 136;
					const char * names[] = {"Guv Net", "OS", "Protocol", "Cypher Lock 1", "Cypher Lock 2", "Cypher Lock 3", "Header", "Schedule", "Location"};
					for(int i = 0; i < sizeof(names) / sizeof(char *); i++){
						secretprogress -= 20;
						if(secretprogress < ((player->state == Player::HACKING && player->state_i == 16 && state_i % 16 < 8) ? -20 : 0)){
							color = 114;
							brightness = 96;
						}
						DrawText(surface, 10, 54 + (i * lineheight) + yoffset, (char *)names[i], 133, 6, false, color, brightness);
					}
				}else{
					dstrect.x = -world.resources.spriteoffsetx[187][1];
					dstrect.y = -world.resources.spriteoffsety[187][1] + yoffset;
					BlitSurface(world.resources.spritebank[187][1], 0, surface, &dstrect);
				}
				if(world.highlightsecrets){
					Surface * effectsurface = CreateSurfaceCopy(world.resources.spritebank[86][2]);
					Uint8 brightness = 120;
					if(state_i % 32 < 16){
						brightness += (state_i % 16);
					}else{
						brightness += 16 - (state_i % 16);
					}
					EffectBrightness(effectsurface, 0, brightness);
					dstrect.x = -world.resources.spriteoffsetx[86][2];
					dstrect.y = -world.resources.spriteoffsety[86][2] + yoffset;
					BlitSurface(effectsurface, 0, surface, &dstrect);
					delete effectsurface;
				}
				if(world.highlightminimap){
					Surface * effectsurface = CreateSurfaceCopy(world.resources.spritebank[86][1]);
					Uint8 brightness = 120;
					if(state_i % 32 < 16){
						brightness += (state_i % 16);
					}else{
						brightness += 16 - (state_i % 16);
					}
					EffectBrightness(effectsurface, 0, brightness);
					dstrect.x = -world.resources.spriteoffsetx[86][1];
					dstrect.y = -world.resources.spriteoffsety[86][1];
					BlitSurface(effectsurface, 0, surface, &dstrect);
					delete effectsurface;
				}
			}
			
			Uint8 tracetime = 0;
			if(team && team->beamingterminalid){
				Terminal * terminal = static_cast<Terminal *>(world.GetObjectFromId(team->beamingterminalid));
				if(terminal){
					if(terminal->tracetime > 0){
						tracetime = terminal->tracetime;
					}
				}
			}
			if(player->tracetime > 0){
				tracetime = player->tracetime;
			}
			if(tracetime > 0){
				char temp[256];
				sprintf(temp, "Government Trace Time: %d", tracetime);
				DrawText(surface, 20, 350, temp, 133, 6, false, 0, 136);
			}
			
			// Draw buy interface
			// 102:0 buy background
			// 102:1 buy highlight
			// 102:2 buy up arrow
			// 102:3 buy down array
			
			if(player->buyinterfaceid || player->techinterfaceid){
				Interface * iface = (Interface *)world.GetObjectFromId(player->buyinterfaceid);
				if(!iface){
					iface = (Interface *)world.GetObjectFromId(player->techinterfaceid);
				}
				if(iface){
					dstrect.x = -world.resources.spriteoffsetx[102][0];
					dstrect.y = -world.resources.spriteoffsety[102][0];
					BlitSurface(world.resources.spritebank[102][0], 0, surface, &dstrect);
					
					SelectBox * selectbox = (SelectBox *)iface->GetObjectWithUid(world, 1);
					unsigned int line = 0;
					unsigned int i = 0;
					for(std::deque<char *>::iterator it = selectbox->items.begin(); it != selectbox->items.end(); it++, i++){
						if(i < selectbox->scrolled){
							continue;
						}
						if(line >= 5){
							break;
						}
						
						int yoffset = line * 25;
						
						Uint8 brightness = 128;

						if(i == selectbox->selecteditem){
							dstrect.x = -world.resources.spriteoffsetx[102][1];
							dstrect.y = -world.resources.spriteoffsety[102][1] + yoffset;
							BlitSurface(world.resources.spritebank[102][1], 0, surface, &dstrect);
							if(state_i % 16 >= 8){
								brightness += ((state_i % 8) / 1);
							}else{
								brightness += 8 - ((state_i % 8) / 1);
							}
						}
						
						BuyableItem * buyableitem = 0;
						Uint32 id = selectbox->IndexToId(i);
						for(std::vector<BuyableItem *>::iterator itb = world.buyableitems.begin(); itb != world.buyableitems.end(); itb++){
							if((*itb)->id == id){
								buyableitem = (*itb);
								break;
							}
						}
						
						dstrect.x = -world.resources.spriteoffsetx[buyableitem->res_bank][buyableitem->res_index] + 169;
						dstrect.y = -world.resources.spriteoffsety[buyableitem->res_bank][buyableitem->res_index] + 139 + yoffset;
						if(brightness != 128){
							if(world.resources.spritebank[buyableitem->res_bank][buyableitem->res_index]){
								Surface * effectsurface = CreateSurfaceCopy(world.resources.spritebank[buyableitem->res_bank][buyableitem->res_index]);
								EffectBrightness(effectsurface, 0, brightness);
								BlitSurface(effectsurface, 0, surface, &dstrect);
								delete effectsurface;
							}
						}else{
							BlitSurface(world.resources.spritebank[buyableitem->res_bank][buyableitem->res_index], 0, surface, &dstrect);
						}
						
						char * itemname = (*it);
						char temp[64];
						if(buyableitem->id == World::BUY_GIVE0){
							Peer * peer = world.peerlist[team->peers[0]];
							if(peer){
								User * user = world.lobby.GetUserInfo(peer->accountid);
								strcpy(temp, itemname);
								strcat(temp, user->name);
								itemname = temp;
							}
						}else
						if(buyableitem->id == World::BUY_GIVE1){
							Peer * peer = world.peerlist[team->peers[1]];
							if(peer){
								User * user = world.lobby.GetUserInfo(peer->accountid);
								strcpy(temp, itemname);
								strcat(temp, user->name);
								itemname = temp;
							}
						}else
						if(buyableitem->id == World::BUY_GIVE2){
							Peer * peer = world.peerlist[team->peers[2]];
							if(peer){
								User * user = world.lobby.GetUserInfo(peer->accountid);
								strcpy(temp, itemname);
								strcat(temp, user->name);
								itemname = temp;
							}
						}else
						if(buyableitem->id == World::BUY_GIVE3){
							Peer * peer = world.peerlist[team->peers[3]];
							if(peer){
								User * user = world.lobby.GetUserInfo(peer->accountid);
								strcpy(temp, itemname);
								strcat(temp, user->name);
								itemname = temp;
							}
						}
						char price[10];
						if(player->buyinterfaceid){
							if(team && (team->disabledtech & buyableitem->techchoice)){
								strcpy(price, "DOWN");
							}else{
								sprintf(price, "%d", buyableitem->price);
							}
						}else{
							if(!player->InOwnBase(world)){
								BaseDoor * basedoor = static_cast<BaseDoor *>(world.GetObjectFromId(player->basedoorentering));
								if(basedoor){
									Team * otherteam = static_cast<Team *>(world.GetObjectFromId(basedoor->teamid));
									if(otherteam && otherteam->disabledtech & buyableitem->techchoice){
										strcpy(price, "DOWN");
									}else{
										sprintf(price, "%d", buyableitem->repairprice);
									}
								}
							}else{
								if(team && team->disabledtech & buyableitem->techchoice){
									sprintf(price, "%d", buyableitem->repairprice);
								}else{
									strcpy(price, "UP");
								}
							}
						}
						DrawText(surface, 222, 145 + yoffset, itemname, 134, 9, false, 0, brightness);
						DrawText(surface, 440 - ((strlen(price) * 9) / 2), 145 + yoffset, price, 134, 9, false, 0, brightness);
						yoffset += 25;
						
						line++;
					}
					
					char text[256];
					if(player->buyinterfaceid || player->InOwnBase(world)){
						sprintf(text, "Available Credits: %d", player->credits);
					}else{
						sprintf(text, "Viruses Available: %d", player->InventoryItemCount(Player::INV_VIRUS));
					}
					DrawText(surface, 320 - ((strlen(text) * 9) / 2), 275, text, 134, 9);
				}
			}
		
			// Draw chat
			
			if(world.showchat_i || player->chatinterfaceid){
				Rect dstrect;
				dstrect.x = 400;
				dstrect.y = 280;
				dstrect.w = 231;
				dstrect.h = 30;
				DrawMessageBackground(surface, &dstrect);
				int yoffset = 10;
				for(int i = 0; i < world.chatlines.size(); i++){
					if(player->chatinterfaceid && i == 0 && world.chatlines.size() == 5){
						continue;
					}
					char text[36 + 1];
					memset(text, 0, sizeof(text) - 1);
					strncpy(text, world.chatlines[i].c_str(), sizeof(text));
					text[36] = 0;
					DrawText(surface, dstrect.x + 10, dstrect.y + yoffset, text, 133, 6, false, 0, 136);
					yoffset += 10;
				}
				if(player->chatinterfaceid){
					Interface * iface = (Interface *)world.GetObjectFromId(player->chatinterfaceid);
					if(iface){
						TextInput * textinput = (TextInput *)iface->GetObjectWithUid(world, 1);
						if(textinput){
							const char * textprepend = "(ALL):";
							if(player->chatwithteam){
								textprepend = "(TEAM):";
							}
							DrawText(surface, dstrect.x + 10, dstrect.y + yoffset, textprepend, 133, 6, false, 0, 136);
							textinput->x = dstrect.x + (strlen(textprepend) * 6) + 10;
							textinput->y = dstrect.y + yoffset;
							DrawTextInput(surface, *textinput);
						}
					}
				}
			}
		}
		
	}
}

void Renderer::DrawMessageBackground(Surface * surface, Rect * dstrect){
	// 188 is chat window.  0-topleft 1-top 2-topright 3-left 4-center 5-right 6-bottomleft 7-bottom 8-bottomright
	Rect dstrect2;
	Rect srcrect;
	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = 0;
	srcrect.h = 0;
	int x = 0;
	int y = 0;

	dstrect2.x = -world.resources.spriteoffsetx[188][0] + dstrect->x;
	dstrect2.y = -world.resources.spriteoffsety[188][0] + dstrect->y;
	BlitSurface(world.resources.spritebank[188][0], 0, surface, &dstrect2);
	x += world.resources.spritewidth[188][0];
	while(x < dstrect->w - world.resources.spritewidth[188][2]){
		dstrect2.x = -world.resources.spriteoffsetx[188][1] + dstrect->x + x;
		dstrect2.y = -world.resources.spriteoffsety[188][1] + dstrect->y;
		srcrect.w = dstrect->w - x - 36;
		if(srcrect.w > world.resources.spritewidth[188][1]){
			srcrect.w = world.resources.spritewidth[188][1];
		}
		srcrect.h = world.resources.spriteheight[188][1];
		BlitSurface(world.resources.spritebank[188][1], &srcrect, surface, &dstrect2);
		x += srcrect.w;
	}
	dstrect2.x = -world.resources.spriteoffsetx[188][2] + dstrect->x + dstrect->w - 36;// - world.resources.spritewidth[188][2];
	dstrect2.y = -world.resources.spriteoffsety[188][2] + dstrect->y;
	BlitSurface(world.resources.spritebank[188][2], 0, surface, &dstrect2);
	
	x = 0;
	y = dstrect->h;
	
	dstrect2.x = -world.resources.spriteoffsetx[188][6] + dstrect->x;
	dstrect2.y = -world.resources.spriteoffsety[188][6] + dstrect->y + y;
	BlitSurface(world.resources.spritebank[188][6], 0, surface, &dstrect2);
	x += world.resources.spritewidth[188][6];
	while(x < dstrect->w - world.resources.spritewidth[188][8]){
		dstrect2.x = -world.resources.spriteoffsetx[188][7] + dstrect->x + x;
		dstrect2.y = -world.resources.spriteoffsety[188][7] + dstrect->y + y;
		srcrect.w = dstrect->w - x - 36;
		if(srcrect.w > world.resources.spritewidth[188][7]){
			srcrect.w = world.resources.spritewidth[188][7];
		}
		srcrect.h = world.resources.spriteheight[188][7];
		BlitSurface(world.resources.spritebank[188][7], &srcrect, surface, &dstrect2);
		x += srcrect.w;
	}
	dstrect2.x = -world.resources.spriteoffsetx[188][8] + dstrect->x + dstrect->w - 36;// - world.resources.spritewidth[188][8];
	dstrect2.y = -world.resources.spriteoffsety[188][8] + dstrect->y + y;
	BlitSurface(world.resources.spritebank[188][8], 0, surface, &dstrect2);
}

Uint8 Renderer::GetAmbienceLevel(void){
	return ambiencelevel;
}

void Renderer::DrawLine(Surface * surface, int x1, int y1, int x2, int y2, Uint8 color, int thickness){
	int dx = x2 - x1;
	int dy = y2 - y1;
	int step;
	int error;
	int y = y1;
	int x = x1;
	float slope = 0;
	if(dx){
		slope = (float)dy / dx;
	}else{
		if(y2 > y1){
			for(y = y1; y < y2; y++){
				DrawFilledRectangle(surface, x, y, x + thickness, y + thickness, color);
			}
		}else{
			for(y = y1; y > y2; y--){
				DrawFilledRectangle(surface, x, y, x + thickness, y + thickness, color);
			}
		}
	}
	if(slope > -1 && slope < 1){
		error = -dx / 2;
		y = y1;
		y1 < y2 ? step = 1 : step = -1;
		if(x2 > x1){
			for(x = x1; x < x2; x++){
				DrawFilledRectangle(surface, x, y, x + thickness, y + thickness, color);
				error += dy * step;
				if(error >= 0){
					y += step;
					error -= dx;
				}
			}
		}else{
			for(x = x1; x > x2; x--){
				DrawFilledRectangle(surface, x, y, x + thickness, y + thickness, color);
				error += dy * -step;
				if(error <= 0){
					y += step;
					error -= dx;
				}
			}
		}
	}else{
		error = -dy / 2;
		x = x1;
		x1 < x2 ? step = 1 : step = -1;
		if(y2 > y1){
			for(y = y1; y < y2; y++){
				DrawFilledRectangle(surface, x, y, x + thickness, y + thickness, color);
				error += dx * step;
				if(error >= 0){
					x += step;
					error -= dy;
				}
			}
		}else{
			for(y = y1; y > y2; y--){
				DrawFilledRectangle(surface, x, y, x + thickness, y + thickness, color);
				error += dx * -step;
				if(error <= 0){
					x += step;
					error -= dy;
				}
			}
		}
	}
	DrawFilledRectangle(surface, x, y, x + thickness, y + thickness, color);
}

void Renderer::DrawFilledRectangle(Surface * surface, int x1, int y1, int x2, int y2, Uint8 color){
	for(int x = x1; x < x2; x++){
		for(int y = y1; y < y2; y++){
			SetPixel(surface, x, y, color);
		}
	}
}

void Renderer::DrawCircle(Surface *surface, int x, int y, int radius, Uint8 color){
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
	
	SetPixel(surface, x, y + radius, color);
	SetPixel(surface, x, y - radius, color);
	SetPixel(surface, x + radius, y, color);
	SetPixel(surface, x - radius, y, color);
	
	while(x1 < y1){
		// ddF_x == 2 * x + 1;
		// ddF_y == -2 * y;
		// f == x*x + y*y - radius*radius + 2*x - y + 1;
		if(f >= 0){
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;
		SetPixel(surface, x + x1, y + y1, color);
		SetPixel(surface, x - x1, y + y1, color);
		SetPixel(surface, x + x1, y - y1, color);
		SetPixel(surface, x - x1, y - y1, color);
		SetPixel(surface, x + y1, y + x1, color);
		SetPixel(surface, x - y1, y + x1, color);
		SetPixel(surface, x + y1, y - x1, color);
		SetPixel(surface, x - y1, y - x1, color);
	}
}

Uint8 Renderer::InvIdToResIndex(Uint8 id){
	// 97:0 base door, 1 health pack, 2 laz tract, 3 security pass, 4 camera, 5 poison
	// 6-9 bomb, 10 flare, 11 cannon, 12 plasma det, 13 poison flare, 14 virus, 15 base def
	// 16 laser ammo, 17 rockets, 18 flamer
	switch(id){
		default:
		case Player::INV_NONE:
			return 0xFF;
		break;
		case Player::INV_HEALTHPACK:
			return 1;
		break;
		case Player::INV_LAZARUSTRACT:
			return 2;
		break;
		case Player::INV_SECURITYPASS:
			return 3;
		break;
		case Player::INV_VIRUS:
			return 14;
		break;
		case Player::INV_POISON:
			return 5;
		break;
		case Player::INV_EMPBOMB:
			return 6;
		break;
		case Player::INV_SHAPEDBOMB:
			return 7;
		break;
		case Player::INV_PLASMABOMB:
			return 8;
		break;
		case Player::INV_NEUTRONBOMB:
			return 9;
		break;
		case Player::INV_PLASMADET:
			return 12;
		break;
		case Player::INV_FIXEDCANNON:
			return 11;
		break;
		case Player::INV_FLARE:
			return 10;
		break;
		case Player::INV_POISONFLARE:
			return 13;
		break;
		case Player::INV_CAMERA:
			return 4;
		break;
		case Player::INV_BASEDOOR:
			return 0;
		break;
	}
}
