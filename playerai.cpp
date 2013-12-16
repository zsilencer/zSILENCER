#include "playerai.h"
#include <algorithm>
#include "basedoor.h"

PlayerAI::PlayerAI(Player & player) : player(player){
	direction = false;
	targetplatformset = 0;
	ladderjumping = false;
	linktype = LINK_NONE;
	linkladder = 0;
	state = IDLE;
}

void PlayerAI::Tick(World & world){
	Input zeroinput;
	player.input = zeroinput;

	if(state == IDLE){
		SetState(HACK);
	}
	if(player.state == Player::RESURRECTING){
		SetState(IDLE);
	}
	if(player.state == Player::RESPAWNING){
		SetState(EXITBASE);
	}
	if(player.hassecret){
		if(player.InBase(world)){
			if(player.InOwnBase(world)){
				SetState(RETURNSECRET);
			}else{
				SetState(EXITBASE);
			}
		}else{
			SetState(GOTOBASE);
		}
	}
	
	
	if(!targetplatformset && player.state != Player::HACKING){
		if(state == HACK){
			std::vector<Terminal *> terminals = FindNearestTerminals(world);
			if(terminals.size() > 0){
				for(std::vector<Terminal *>::iterator it = terminals.begin(); it != terminals.end(); it++){
					if(SetTarget(world, (*it)->x, (*it)->y)){
						break;
					}
				}
			}
		}
	}
	if(!FollowPath(world)){
		// Done following path
		ClearTarget();
	}
	
	if(state == HACK){
		std::vector<Uint8> types;
		types.push_back(ObjectTypes::TERMINAL);
		std::vector<Object *> collided = world.TestAABB(player.x, player.y - player.height, player.x, player.y, types);
		for(std::vector<Object *>::iterator it = collided.begin(); it != collided.end(); it++){
			switch((*it)->type){
				case ObjectTypes::TERMINAL:{
					Terminal * terminal = static_cast<Terminal *>(*it);
					if(terminal->state == Terminal::READY || terminal->state == Terminal::HACKERGONE){
						if(player.state != Player::HACKING){
							if(rand() % 3 == 0){
								player.input.keyactivate = true;
							}
							ClearTarget();
						}
					}
				}break;
			}
		}
	}
	
	if(state == EXITBASE){
		if(player.InBase(world)){
			if(!targetplatformset){
				BaseExit * baseexit = GetBaseExit(world);
				if(baseexit){
					SetTarget(world, baseexit->x + 1, baseexit->y);
				}
			}
		}else{
			SetState(IDLE);
		}
	}
	
	if(state == GOTOBASE){
		if(!player.InBase(world)){
			if(!targetplatformset){
				BaseDoor * basedoor = GetBaseDoor(world);
				if(basedoor){
					SetTarget(world, basedoor->x, basedoor->y);
				}
			}
			Team * team = player.GetTeam(world);
			if(team){
				std::vector<Uint8> types;
				types.push_back(ObjectTypes::BASEDOOR);
				std::vector<Object *> collided = world.TestAABB(player.x, player.y - player.height, player.x, player.y, types);
				for(std::vector<Object *>::iterator it = collided.begin(); it != collided.end(); it++){
					BaseDoor * basedoor = static_cast<BaseDoor *>(*it);
					if(basedoor->teamid == team->id){
						player.input.keyactivate = true;
					}
				}
			}
		}
	}
	
	if(state == RETURNSECRET){
		if(!targetplatformset){
			SecretReturn * secretreturn = GetSecretReturn(world);
			if(secretreturn){
				SetTarget(world, secretreturn->x, secretreturn->y + 30);
			}
		}
		if(!player.hassecret){
			SetState(EXITBASE);
		}
	}

	if(player.OnGround() && player.inventoryitems[player.currentinventoryitem] == Player::INV_BASEDOOR){
		player.input.keyuse = true;
	}
}

bool PlayerAI::CreatePathToPlatformSet(World & world, std::deque<PlatformSet *> & path, PlatformSet & to){
	PlatformSet * platformset = 0;
	Platform * currentplatform = world.map.platformids[player.currentplatformid];
	if(path.size() == 0){
		if(currentplatform){
			platformset = currentplatform->set;
			path.push_back(platformset);
		}
	}else{
		platformset = path.back();
	}
	if(platformset){
		if(platformset == &to){
			return true;
		}else{
			for(std::vector<PlatformSet *>::iterator it = world.map.platformsets.begin(); it != world.map.platformsets.end(); it++){
				if(std::find(path.begin(), path.end(), (*it)) == path.end()){
					if(FindAnyLink(world, *platformset, *(*it))){
						path.push_back((*it));
						if(CreatePathToPlatformSet(world, path, to)){
							return true;
						}
					}
				}
			}
		}
	}
	if(path.size() > 0){
		path.pop_back();
	}
	return false;
}

bool PlayerAI::FollowPath(World & world){
	Platform * currentplatform = world.map.platformids[player.currentplatformid];
	if(currentplatform && currentplatform->set == targetplatformset){
		if(platformsetpath.size() > 0){
			platformsetpath.pop_front();
			linktype = LINK_NONE;
			if(platformsetpath.size() > 0){
				targetplatformset = platformsetpath.front();
			}
		}
	}
	if(targetplatformset){
		if(currentplatform && targetplatformset != currentplatform->set){
			if(linktype == LINK_NONE){
				if(!FindAnyLink(world, *currentplatform->set, *targetplatformset)){
					ClearTarget();
				}
			}
			switch(linktype){
				case LINK_LADDER:{
					if(player.OnGround()){
						Sint16 center = linkladder->x1 + ((linkladder->x2 - linkladder->x1) / 2);
						if(center < player.x){
							player.input.keymoveleft = true;
						}else
							if(center > player.x){
								player.input.keymoveright = true;
							}
						if(linkladder->x1 <= player.x && linkladder->x2 >= player.x){
							if(linkladder->y1 < player.y){
								player.input.keymoveup = true;
								direction = true;
							}else{
								player.input.keymovedown = true;
								direction = false;
							}
						}
					}
				}break;
			}
		}
	}
	if(targetplatformset){
		if(currentplatform && targetplatformset == currentplatform->set){
			int diff = abs(targetx - player.x);
			if(diff > 8){
				if(targetx < player.x){
					player.input.keymoveleft = true;
				}else
				if(targetx > player.x){
					player.input.keymoveright = true;
				}
			}else{
				return false;
			}
		}
		if(player.state == Player::LADDER){
			ladderjumping = false;
			if(direction){
				player.input.keymoveup = true;
				if(rand() % 12 == 0){
					player.input.keyjump = true;
					player.input.keymoveleft = false;
					player.input.keymoveright = false;
					ladderjumping = true;
				}
			}else{
				player.input.keymovedown = true;
				if(rand() % 5 == 0){
					player.input.keyjump = true;
				}
			}
		}
		if(player.state == Player::FALLING || player.state == Player::JUMPING){
			player.input.keymoveleft = false;
			player.input.keymoveright = false;
			if(ladderjumping){
				if(rand() % 12 == 0){
					player.input.keymoveup = true;
				}
			}
		}
	}else{
		if(player.state == Player::LADDER){
			player.input.keymovedown = true;
			player.input.keyjump = true;
		}
	}
	
	return true;
}

bool PlayerAI::SetTarget(World & world, Sint16 x, Sint16 y){
	ClearTarget();
	Platform * platform = world.map.TestAABB(x, y, x, y, Platform::RECTANGLE | Platform::STAIRSUP | Platform::STAIRSDOWN);
	if(platform){
		if(CreatePathToPlatformSet(world, platformsetpath, *platform->set)){
			linktype = LINK_NONE;
			targetx = x;
			targety = y;
			targetplatformset = platformsetpath.front();
			//printf("created path of size %d\n", platformsetpath.size());
			return true;
		}else{
			//printf("could not create path to target\n");
			//ClearTarget();
		}
	}
	return false;
}

void PlayerAI::ClearTarget(void){
	targetplatformset = 0;
	platformsetpath.clear();
}

void PlayerAI::GetCurrentNode(int & x, int & y){
	x = player.x / 64;
	y = (player.y - 32) / 64;
}

Uint8 PlayerAI::GetNodeType(World & world, unsigned int x, unsigned int y){
	if(x > world.map.expandedwidth || y > world.map.expandedheight){
		return 0;
	}
	return world.map.nodetypes[(y * world.map.expandedwidth) + x];
}

Platform * PlayerAI::FindClosestLadderToPlatform(World & world, PlatformSet & from, PlatformSet & to, Sint16 x){
	Platform * ladder = 0;
	std::vector<Platform *> ladders = world.map.LaddersToPlatform(from, to);
	for(std::vector<Platform *>::iterator it = ladders.begin(); it != ladders.end(); it++){
		Platform * newladder = *it;
		if(!ladder){
			ladder = newladder;
		}
		if(abs(x - newladder->x1) < abs(x - ladder->x1)){
			ladder = newladder;
		}
	}
	return ladder;
}

bool PlayerAI::FindLink(World & world, int type, PlatformSet & from, PlatformSet & to){
	switch(type){
		default:
		case LINK_NONE:
			linktype = LINK_NONE;
			return false;
		break;
		case LINK_LADDER:
			Platform * ladder = FindClosestLadderToPlatform(world, from, to, player.x);
			if(ladder){
				linktype = LINK_LADDER;
				linkladder = ladder;
				return true;
			}
		break;
	}
	linktype = LINK_NONE;
	return false;
}

bool PlayerAI::FindAnyLink(World & world, PlatformSet & from, PlatformSet & to){
	int starttype = (rand() % (LINK_MAXENUM - 1)) + 1;
	int type = starttype;
	do{
		if(FindLink(world, type, from, to)){
			return true;
		}
		type++;
		if(type >= LINK_MAXENUM){
			type = 1;
		}
	}while(type != starttype);
	return false;
}

std::vector<Terminal *> PlayerAI::FindNearestTerminals(World & world){
	Team * team = player.GetTeam(world);
	std::vector<Terminal * >terminals;
	for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
		if((*it)->type == ObjectTypes::TERMINAL){
			Terminal * terminal = static_cast<Terminal *>(*it);
			if(terminal->state == Terminal::READY || terminal->state == Terminal::HACKERGONE || (terminal->state == Terminal::SECRETREADY && team && team->beamingterminalid == terminal->id)){
				/*if(!nearestterminal){
					nearestterminal = terminal;
				}
				if(abs(terminal->x - player.x) < abs(nearestterminal->x - player.x)){
					nearestterminal = terminal;
				}*/
				terminals.push_back(terminal);
			}
		}
	}
	std::random_shuffle(terminals.begin(), terminals.end());
	std::sort(terminals.begin(), terminals.end(), TerminalSort);
	return terminals;
}

BaseExit * PlayerAI::GetBaseExit(World & world){
	if(player.InBase(world)){
		BaseDoor * basedoor = static_cast<BaseDoor *>(world.GetObjectFromId(player.basedoorentering));
		if(basedoor){
			for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
				if((*it)->type == ObjectTypes::BASEEXIT){
					BaseExit * baseexit = static_cast<BaseExit *>(*it);
					if(baseexit->teamid == basedoor->teamid){
						return baseexit;
					}
				}
			}
		}
	}
	return 0;
}

SecretReturn * PlayerAI::GetSecretReturn(World & world){
	if(player.InOwnBase(world)){
		Team * team = player.GetTeam(world);
		for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
			if((*it)->type == ObjectTypes::SECRETRETURN){
				SecretReturn * secretreturn = static_cast<SecretReturn *>(*it);
				if(secretreturn->teamid == team->id){
					return secretreturn;
				}
			}
		}
	}
	return 0;
}

BaseDoor * PlayerAI::GetBaseDoor(World & world){
	if(!player.InBase(world)){
		Team * team = player.GetTeam(world);
		if(team){
			for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
				if((*it)->type == ObjectTypes::BASEDOOR){
					BaseDoor * basedoor = static_cast<BaseDoor *>(*it);
					if(basedoor->teamid == team->id){
						return basedoor;
					}
				}
			}
		}
	}
	return 0;
}

void PlayerAI::SetState(Uint8 state){
	if(PlayerAI::state != state){
		PlayerAI::state = state;
		ClearTarget();
	}
}

bool PlayerAI::TerminalSort(Terminal * a, Terminal * b){
	if(a->state == Terminal::SECRETREADY && b->state != Terminal::SECRETREADY){
		return true;
	}
	return false;
}