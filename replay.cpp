#include "replay.h"
#include "world.h"
#include "user.h"
#include "player.h"

Replay::Replay(){
	file = 0;
	isrecording = false;
	isplaying = false;
	uniqueport = 0;
	inputsize = 0;
	gamestarted = false;
	showallnames = false;
	x = 0;
	y = 0;
	oldx = 0;
	oldy = 0;
	xv = 0;
	yv = 0;
	speed = 1;
	ffmpeg = 0;
	ffmpegvideo = true;
	tick = 0;
}

Replay::~Replay(){
	EndRecording();
	EndPlaying();
}

void Replay::BeginRecording(const char * filename){
	CDDataDir();
	file = SDL_RWFromFile(filename, "wb");
	if(file){
		isrecording = true;
	}
}

void Replay::EndRecording(void){
	if(file){
		SDL_RWclose(file);
	}
	file = 0;
	isrecording = false;
}

void Replay::BeginPlaying(const char * filename, const char * outfilename, bool video){
	if(!filename){
		return;
	}
	CDDataDir();
	file = SDL_RWFromFile(filename, "rb");
	if(file){
		isplaying = true;
	}
#ifdef POSIX
	if(outfilename){
		ffmpegvideo = video;
		if(video){
			char cmd[256];
			sprintf(cmd, "ffmpeg -v debug -y -f rawvideo -vcodec rawvideo -s 640x480 -pix_fmt rgb24 -r 50 -i pipe: -an -vcodec libx264 -preset veryslow -qp 0 %s", outfilename);
			ffmpeg = popen(cmd, "w");
			tick = 0;
		}else{
			char cmd[256];
			sprintf(cmd, "ffmpeg -v debug -y -f s16le -ar 44100 -ac 1 -acodec pcm_s16le -i pipe: -vn -acodec libvo_aacenc %s", outfilename);
			ffmpeg = popen(cmd, "w");
		}
	}
#endif
	gamestarted = false;
}

void Replay::EndPlaying(void){
	if(file){
		SDL_RWclose(file);
	}
	file = 0;
#ifdef POSIX
	if(ffmpeg){
		pclose(ffmpeg);
	}
	ffmpeg = 0;
#endif
	isplaying = false;
}

void Replay::WriteHeader(World & world){
	Serializer data;
	for(int i = 0; i < strlen(world.version) + 1; i++){
		SDL_RWwrite(file, &world.version[i], 1, 1);
	}
	data.Put(world.randomseed);
	data.Put(world.tickcount);
	world.gameinfo.Serialize(Serializer::WRITE, data);
	Uint32 datasize = data.BitsToBytes(data.offset);
	SDL_RWwrite(file, &datasize, sizeof(datasize), 1);
	SDL_RWwrite(file, data.data, data.BitsToBytes(data.offset), 1);
}

bool Replay::ReadHeader(World & world){
	Uint8 byte;
	int i = 0;
	int read;
	char versionstring[16 + 1];
	memset(versionstring, 0, sizeof(versionstring));
	do{
		read = SDL_RWread(file, &byte, 1, 1);
		versionstring[i++] = byte;
	}while(read && byte && i < 16);
	if(strcmp(versionstring, world.version) != 0){
		return false;
	}
	Serializer data;
	Uint32 datasize = 0;
	SDL_RWread(file, &datasize, sizeof(datasize), 1);
	if(datasize > data.size){
		return false;
	}
	read = SDL_RWread(file, data.data, datasize, 1);
	if(read == 0){
		return false;
	}
	data.offset = datasize * 8;
	data.Get(world.randomseed);
	data.Get(world.tickcount);
	world.gameinfo.Serialize(Serializer::READ, data);
	return true;
}

bool Replay::ReadToNextTick(World & world){
	Uint8 code;
	do{
		int read = SDL_RWread(file, &code, 1, 1);
		if(!read){
			return false;
		}
		switch(code){
			case RPL_GAMEINFO:{
				printf("RPL_GAMEINFO\n");
				Uint32 gameinfosize;
				SDL_RWread(file, &gameinfosize, sizeof(gameinfosize), 1);
				Serializer data;
				SDL_RWread(file, data.data, gameinfosize, 1);
				data.offset = gameinfosize * 8;
				world.gameinfo.Serialize(Serializer::READ, data);
			}break;
			case RPL_NEWPEER:{
				printf("RPL_NEWPEER\n");
				Uint8 agency;
				Uint32 accountid;
				SDL_RWread(file, &agency, 1, 1);
				SDL_RWread(file, &accountid, sizeof(accountid), 1);
				world.AddPeer((char *)"local", uniqueport++, agency, accountid);
			}break;
			case RPL_START:{
				printf("RPL_START\n");
				gamestarted = true;
			}break;
			case RPL_USERINFO:{
				printf("RPL_USERINFO\n");
				Uint32 size;
				SDL_RWread(file, &size, sizeof(size), 1);
				Serializer data;
				SDL_RWread(file, data.data, data.BitsToBytes(size), 1);
				data.offset = size;
				User user;
				user.Serialize(Serializer::READ, data);
				User * userp = world.lobby.GetUserInfo(user.accountid);
				data.readoffset = 0;
				userp->Serialize(Serializer::READ, data);
				userp->retrieving = false;
			}break;
			case RPL_CHANGETEAM:{
				printf("RPL_CHANGETEAM\n");
				Uint8 peerid;
				SDL_RWread(file, &peerid, 1, 1);
				world.ChangeTeam(peerid);
			}break;
			case RPL_TECH:{
				printf("RPL_TECH\n");
				Uint8 peerid;
				Uint32 techchoices;
				SDL_RWread(file, &peerid, 1, 1);
				SDL_RWread(file, &techchoices, sizeof(techchoices), 1);
				world.SetTech(peerid, techchoices);
			}break;
			case RPL_CHAT:{
				printf("RPL_CHAT\n");
				Uint8 peerid;
				Uint8 to;
				Uint8 msgsize;
				char msg[256];
				SDL_RWread(file, &peerid, 1, 1);
				SDL_RWread(file, &to, 1, 1);
				SDL_RWread(file, &msgsize, 1, 1);
				SDL_RWread(file, msg, msgsize, 1);
				msg[msgsize] = 0;
				Peer * peer = world.peerlist[peerid];
				if(peer){
					if(to == 0 || world.GetPeerTeam(peer->id) == world.GetPeerTeam(world.localpeerid)){
						world.DisplayChatMessage(peer->accountid, msg);
					}
				}
			}break;
			case RPL_STATION:{
				printf("RPL_STATION\n");
				Uint8 peerid;
				Uint8 action;
				Uint8 itemid;
				SDL_RWread(file, &peerid, 1, 1);
				SDL_RWread(file, &action, 1, 1);
				SDL_RWread(file, &itemid, 1, 1);
				Peer * peer = world.peerlist[peerid];
				if(peer){
					Player * player = world.GetPeerPlayer(peer->id);
					if(player){
						switch(action){
							case STA_BUY: player->BuyItem(world, itemid); break;
							case STA_REPAIR: player->RepairItem(world, itemid); break;
							case STA_VIRUS: player->VirusItem(world, itemid); break;
						}
					}
				}
			}break;
			case RPL_INPUT:{
				//printf("RPL_INPUT\n");
				Uint8 peerid;
				SDL_RWread(file, &peerid, 1, 1);
				Serializer * data = new Serializer;
				SDL_RWread(file, data->data, GetInputSize(), 1);
				data->offset = GetInputSize() * 8;
				world.inputqueue[peerid].push_back(data);
			}break;
			case RPL_DISCONNECT:{
				printf("RPL_DISCONNECT\n");
				Uint8 peerid;
				SDL_RWread(file, &peerid, 1, 1);
				world.HandleDisconnect(peerid);
			}break;
			case RPL_TICK:{
				//printf("RPL_TICK\n");
			}break;
			default:{
				printf("Unknown replay code: %d\n", code);
				return false;
			}break;
		}
	}while(code != RPL_TICK);
	return true;
}

void Replay::WriteGameInfo(LobbyGame & gameinfo){
	Uint8 code = RPL_GAMEINFO;
	SDL_RWwrite(file, &code, 1, 1);
	Serializer data;
	gameinfo.Serialize(Serializer::WRITE, data);
	Uint32 gameinfosize = data.BitsToBytes(data.offset);
	SDL_RWwrite(file, &gameinfosize, sizeof(gameinfosize), 1);
	SDL_RWwrite(file, data.data, data.BitsToBytes(data.offset), 1);
}

void Replay::WriteNewPeer(Uint8 agency, Uint32 accountid){
	Uint8 code = RPL_NEWPEER;
	SDL_RWwrite(file, &code, 1, 1);
	SDL_RWwrite(file, &agency, 1, 1);
	SDL_RWwrite(file, &accountid, sizeof(accountid), 1);
}

void Replay::WriteStart(void){
	Uint8 code = RPL_START;
	SDL_RWwrite(file, &code, 1, 1);
}

void Replay::WriteUserInfo(User & user){
	Uint8 code = RPL_USERINFO;
	SDL_RWwrite(file, &code, 1, 1);
	Serializer data;
	user.Serialize(Serializer::WRITE, data);
	Uint32 size = data.offset;
	SDL_RWwrite(file, &size, sizeof(size), 1);
	SDL_RWwrite(file, data.data, data.BitsToBytes(data.offset), 1);
}

void Replay::WriteChangeTeam(Uint8 peerid){
	Uint8 code = RPL_CHANGETEAM;
	SDL_RWwrite(file, &code, 1, 1);
	SDL_RWwrite(file, &peerid, 1, 1);
}

void Replay::WriteSetTech(Uint8 peerid, Uint32 techchoices){
	Uint8 code = RPL_TECH;
	SDL_RWwrite(file, &code, 1, 1);
	SDL_RWwrite(file, &peerid, 1, 1);
	SDL_RWwrite(file, &techchoices, sizeof(techchoices), 1);
}

void Replay::WriteChat(Uint8 peerid, Uint8 to, char * msg){
	Uint8 code = RPL_CHAT;
	SDL_RWwrite(file, &code, 1, 1);
	SDL_RWwrite(file, &peerid, 1, 1);
	SDL_RWwrite(file, &to, 1, 1);
	Uint8 msgsize = strlen(msg);
	SDL_RWwrite(file, &msgsize, 1, 1);
	SDL_RWwrite(file, msg, msgsize, 1);
}

void Replay::WriteStation(Uint8 peerid, Uint8 action, Uint8 itemid){
	Uint8 code = RPL_STATION;
	SDL_RWwrite(file, &code, 1, 1);
	SDL_RWwrite(file, &peerid, 1, 1);
	SDL_RWwrite(file, &action, 1, 1);
	SDL_RWwrite(file, &itemid, 1, 1);
}

void Replay::WriteInputCommand(World & world, Uint8 peerid, Serializer & data){
	Uint8 code = RPL_INPUT;
	SDL_RWwrite(file, &code, 1, 1);
	SDL_RWwrite(file, &peerid, 1, 1);
	int dataoffset = data.BitsToBytes(data.readoffset);
	SDL_RWwrite(file, &data.data[dataoffset], data.BitsToBytes(data.offset) - dataoffset, 1);
}

void Replay::WriteDisconnect(Uint8 peerid){
	Uint8 code = RPL_DISCONNECT;
	SDL_RWwrite(file, &code, 1, 1);
	SDL_RWwrite(file, &peerid, 1, 1);
}

void Replay::WriteTick(void){
	Uint8 code = RPL_TICK;
	SDL_RWwrite(file, &code, 1, 1);
}

bool Replay::IsRecording(void){
	return isrecording;
}

bool Replay::IsPlaying(void){
	return isplaying;
}

bool Replay::GameStarted(void){
	return gamestarted;
}

bool Replay::ShowAllNames(void){
	return showallnames;
}

int Replay::GetInputSize(void){
	if(!inputsize){
		Input input;
		input.mousex = 0xFFFF;
		input.mousey = 0xFFFF;
		Serializer data;
		input.Serialize(Serializer::WRITE, data);
		inputsize = sizeof(Uint32)/*tickcount*/ + sizeof(Uint32)/*lasttick*/ + data.BitsToBytes(data.offset);
	}
	return inputsize;
}