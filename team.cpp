#include "team.h"
#include "overlay.h"
#include "terminal.h"
#include "player.h"

Team::Team() : Object(ObjectTypes::TEAM){
	agency = NOXIS;
	numpeers = 0;
	color = 0;
	requiresauthority = true;
	requiresmaptobeloaded = false;
	overlayid = 0;
	secrets = 0;
	secretdelivered = 0;
	secretprogress = 0;
	number = 0;
	basedoorid = 0;
	beamingterminalid = 0;
	for(int i = 0; i < 4; i++){
		peers[i] = 0;
		peeroverlayids[i] = 0;
		peerreadyoverlayids[i] = 0;
		peerleveloverlayids[i] = 0;
	}
	peerschecksum = 0;
	oldsecretprogress = 0;
	issprite = false;
	playerwithsecret = 0;
	disabledtech = 0;
}

void Team::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, agency, old);
	data.Serialize(write, secrets, old);
	data.Serialize(write, secretprogress, old);
	data.Serialize(write, secretdelivered, old);
	data.Serialize(write, basedoorid, old);
	data.Serialize(write, beamingterminalid, old);
	data.Serialize(write, numpeers, old);
	data.Serialize(write, number, old);
	for(int i = 0; i < 4; i++){
		data.Serialize(write, peers[i], old);
	}
	data.Serialize(write, playerwithsecret, old);
	data.Serialize(write, disabledtech, old);
}

void Team::Tick(World & world){
	int newpeerschecksum = numpeers;
	for(int i = 0; i < numpeers; i++){
		if(world.peerlist[peers[i]]){
			newpeerschecksum += world.peerlist[peers[i]]->ip;
		}
	}
	if(secretprogress - oldsecretprogress >= 20){
		Player * localplayer = world.GetPeerPlayer(world.localpeerid);
		if(localplayer && this == localplayer->GetTeam(world)){
			Audio::GetInstance().Play(world.resources.soundbank["select2.wav"], 32);
		}
		oldsecretprogress = secretprogress;
	}
	if(secretdelivered){
		secrets++;
		world.SendSound("cathdoor.wav");
		Player * player = static_cast<Player *>(world.GetObjectFromId(secretdelivered));
		if(player){
			Peer * peer = player->GetPeer(world);
			if(peer){
				User * user = world.lobby.GetUserInfo(peer->accountid);
				bool stolen = false;
				if(player->secretteamid != id){
					stolen = true;
				}
				int remaining = 3 - secrets;
				char text[128];
				sprintf(text, "%s returned a %s\n( %d remaining )\n\nTeam awarded 1000 credits", user->name, stolen ? "stolen secret" : "secret", remaining);
				if(!world.intutorialmode){
					world.ShowMessage(text, 128, 0, true);
				}
			}
		}
		for(int i = 0; i < numpeers; i++){
			Player * player = world.GetPeerPlayer(peers[i]);
			if(player){
				player->AddCredits(1000);
			}
		}
		world.Illuminate();
		if(secrets >= 3){
			// game won
			if(!world.winningteamid){
				world.winningteamid = id;
				bool isourteam = false;
				for(int i = 0; i < numpeers; i++){
					Peer * peer = world.peerlist[peers[i]];
					if(peer){
						if(peer->id == world.localpeerid){
							isourteam = true;
						}
					}
				}
				Uint8 type = 10;
				char fs[64];
				strcpy(fs, "MISSION SUCCESS\n");
				if(!isourteam){
					strcpy(fs, "MISSION FAILED\n");
					type = 11;
				}
				for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
					Object * object = *it;
					if(object->type == ObjectTypes::PLAYER){
						Player * player = static_cast<Player *>(object);
						Team * team = player->GetTeam(world);
						if(team && team->id == world.winningteamid){
							player->UnDeploy();
						}else{
							Peer * peer = player->GetPeer(world);
							if(peer){
								world.KillByGovt(*peer);
							}
						}
					}
				}
				char message[256];
				sprintf(message, "%sAll secrets retrieved\nby %s agents:\n\n", fs, GetAgencyName());
				for(int i = 0; i < numpeers; i++){
					Peer * peer = world.peerlist[peers[i]];
					if(peer){
						User * user = world.lobby.GetUserInfo(peer->accountid);
						strcat(message, user->name);
						strcat(message, "\n");
					}
				}
				world.ShowMessage(message, 255, type);
			}
		}
		secretdelivered = 0;
	}
	if(secretprogress >= 180 && oldsecretprogress > 0){
		char teamtext[256];
		char enemytext[256];
		sprintf(teamtext, "TOP SECRET LOCATION DETERMINED\n\nApproximate time : 60 seconds");
		sprintf(enemytext, "ENEMY BEAMING DETECTED\n\nTracking location on radar");
		for(int i = 0; i < world.maxpeers; i++){
			Peer * peer = world.peerlist[i];
			if(peer){
				if(world.GetPeerTeam(peer->id) == this){
					world.ShowMessage(teamtext, 128, 0, true, peer);
				}else{
					world.ShowMessage(enemytext, 128, 0, true, peer);
				}
			}
		}
		world.SendSound("typerev6.wav");
		secretprogress = 0;
		oldsecretprogress = 0;
		if(world.IsAuthority()){
			std::vector<Terminal *> terminals;
			for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
				Object * object = (*it);
				if(object->type == ObjectTypes::TERMINAL){
					Terminal * terminal = static_cast<Terminal *>(object);
					if(terminal && terminal->isbig && terminal->state != Terminal::SECRETBEAMING && terminal->state != Terminal::SECRETREADY){
						terminals.push_back(terminal);
					}
				}
			}
			if(terminals.size() > 0){
				Terminal * terminal = terminals[world.Random() % terminals.size()];
				terminal->state = Terminal::SECRETBEAMING;
				terminal->state_i = 0;
				beamingterminalid = terminal->id;
				terminal->beamingtime = 60;
			}
		}
	}
	if(world.gameplaystate == World::INLOBBY){
		if(!overlayid){
			Overlay * overlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
			if(overlay){
				overlay->res_bank = 181;
				overlay->res_index = agency;
				overlay->x = 420;
				overlay->y = 70 + (number * 55);
				overlay->renderpass = 1;
				overlay->draw = world.choosingtech ? false : true;
				overlayid = overlay->id;
			}
		}
		if(newpeerschecksum != peerschecksum){
			for(int i = 0; i < 4; i++){
				if(peeroverlayids[i]){
					world.MarkDestroyObject(peeroverlayids[i]);
					peeroverlayids[i] = 0;
				}
				if(peerleveloverlayids[i]){
					world.MarkDestroyObject(peerleveloverlayids[i]);
					peerleveloverlayids[i] = 0;
				}
				if(peerreadyoverlayids[i]){
					world.MarkDestroyObject(peerreadyoverlayids[i]);
					peerreadyoverlayids[i] = 0;
				}
			}
		}
		for(int i = 0; i < numpeers; i++){
			Peer * peer = world.peerlist[peers[i]];
			if(peer){
				if(!peeroverlayids[i]){
					User * user = world.lobby.GetUserInfo(peer->accountid);
					if(!user->retrieving){
						Overlay * overlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
						if(overlay){
							overlay->text = new char[64];
							sprintf(overlay->text, "%s", user->name);
							overlay->textbank = 133;
							overlay->textwidth = 6;
							overlay->x = 473;
							overlay->y = 72 + (number * 55) + (i * 13);
							overlay->draw = world.choosingtech ? false : true;
							peeroverlayids[i] = overlay->id;
						}
						Overlay * leveloverlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
						if(leveloverlay){
							leveloverlay->text = new char[64];
							sprintf(leveloverlay->text, "L:%d", user->agency[agency].level);
							leveloverlay->textbank = 132;
							leveloverlay->textwidth = 4;
							leveloverlay->effectcolor = 170;
							leveloverlay->x = 473 + (strlen(user->name) * 6) + 3;
							leveloverlay->y = 72 + (number * 55) + (i * 13);
							leveloverlay->draw = world.choosingtech ? false : true;
							peerleveloverlayids[i] = leveloverlay->id;
						}
					}
				}
				if(!peerreadyoverlayids[i]){
					Overlay * overlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
					if(overlay){
						overlay->x = 455;
						overlay->y = 70 + (number * 55) + (i * 13);
						overlay->res_bank = 7;
						overlay->res_index = 19;
						overlay->draw = world.choosingtech ? false : true;
						peerreadyoverlayids[i] = overlay->id;
					}
				}else{
					Overlay * overlay = static_cast<Overlay *>(world.GetObjectFromId(peerreadyoverlayids[i]));
					if(overlay){
						if(peer->isready){
							overlay->res_index = 18;
						}else{
							overlay->res_index = 19;
						}
					}
				}
			}
		}
	}
	if(numpeers == 0){
		DestroyOverlays(world);
		world.MarkDestroyObject(id);
	}
	peerschecksum = newpeerschecksum;
}

void Team::OnDestroy(World & world){
	DestroyOverlays(world);
}

bool Team::AddPeer(Uint8 id){
	Uint8 maxplayers = 4;
	if(agency == BLACKROSE){
		maxplayers = 1;
	}
	for(int i = 0; i < numpeers; i++){
		if(peers[i] == id){
			return false;
		}
	}
	if(numpeers < maxplayers){
		peers[numpeers] = id;
		numpeers++;
		return true;
	}
	return false;
}

void Team::RemovePeer(Uint8 id){
	for(int i = 0; i < numpeers; i++){
		if(peers[i] == id){
			for(int j = i; j < numpeers; j++){
				peers[j] = peers[j + 1];
			}
			numpeers--;
		}
	}
}

void Team::DestroyOverlays(World & world){
	for(int i = 0; i < 4; i++){
		if(peeroverlayids[i]){
			world.MarkDestroyObject(peeroverlayids[i]);
			peeroverlayids[i] = 0;
		}
		if(peerreadyoverlayids[i]){
			world.MarkDestroyObject(peerreadyoverlayids[i]);
			peerreadyoverlayids[i] = 0;
		}
		if(peerleveloverlayids[i]){
			world.MarkDestroyObject(peerleveloverlayids[i]);
			peerleveloverlayids[i] = 0;
		}
	}
	if(overlayid){
		world.MarkDestroyObject(overlayid);
		overlayid = 0;
	}
}

Uint8 Team::GetColor(void){
	if(color){
		return color;
	}
	Uint8 basecolor = 14;
	Uint8 shade = 8;
	switch(number){
		case 0:
			basecolor = 10;
			shade = 7;
		break;
		case 1:
			basecolor = 14;
		break;
		case 2:
			basecolor = 13;
		break;
		case 3:
			basecolor = 9;
			shade = 8;
		break;
		case 4:
			basecolor = 15;
			shade = 11;
		break;
		case 5:
			basecolor = 12;
			shade = 10;
		break;
	}
	if(agency == BLACKROSE){
		basecolor = 8;
		shade = 8 + (number - 4);
	}
	return ((shade << 4) + basecolor);
}

const char * Team::GetAgencyName(void){
	switch(agency){
		default:
		case 0:
			return "Noxis";
		break;
		case 1:
			return "Lazarus";
		break;
		case 2:
			return "Caliber";
		break;
		case 3:
			return "Static";
		break;
		case 4:
			return "Black Rose";
		break;
	}
}

Uint32 Team::GetAvailableTech(World & world){
	Uint32 tech = 0;
	for(int i = 0; i < 4; i++){
		Peer * peer = world.peerlist[peers[i]];
		if(peer){
			tech |= peer->techchoices;
		}
	}
	return tech;
}

void Team::ShowOverlays(World & world, bool show){
	Object * object;
	object = world.GetObjectFromId(overlayid);
	if(object){
		object->draw = show;
	}
	for(int i = 0; i < 4; i++){
		object = world.GetObjectFromId(peeroverlayids[i]);
		if(object){
			object->draw = show;
		}
		object = world.GetObjectFromId(peerleveloverlayids[i]);
		if(object){
			object->draw = show;
		}
		object = world.GetObjectFromId(peerreadyoverlayids[i]);
		if(object){
			object->draw = show;
		}
	}
}
