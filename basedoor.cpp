#include "basedoor.h"
#include "player.h"

BaseDoor::BaseDoor() : Object(ObjectTypes::BASEDOOR){
	requiresauthority = true;
	state_i = 0;
	res_bank = 101;
	res_index = 0;
	teamid = 0;
	renderpass = 1;
	teamnumber = 0;
	for(int i = 0; i < World::maxteams; i++){
		discoveredby[i] = false;
		enteredby[i] = false;
	}
}

void BaseDoor::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state_i, old);
	data.Serialize(write, color, old);
	data.Serialize(write, teamid, old);
	data.Serialize(write, teamnumber, old);
	for(int i = 0; i < World::maxteams; i++){
		data.Serialize(write, discoveredby[i], old);
	}
}

void BaseDoor::Tick(World & world){
	if(state_i == 0){
		Audio::GetInstance().EmitSound(id, world.resources.soundbank["portal1.wav"], 64);
	}
	if(state_i < 41){
		CheckForPlayersInView(world);
		res_bank = 101;
		res_index = state_i;
	}
	if(state_i >= 41){
		res_bank = 100;
		res_index = (state_i - 41) / 4;
	}
	if(state_i >= 41 + (4 * 4)){
		state_i = 40;
	}
	state_i++;
}

void BaseDoor::Respawn(void){
	state_i = 0;
	for(int i = 0; i < World::maxteams; i++){
		discoveredby[i] = false;
		enteredby[i] = false;
	}
}

void BaseDoor::SetTeam(Team & team){
	teamid = team.id;
	teamnumber = team.number;
	color = team.GetColor();
}

void BaseDoor::CheckForPlayersInView(World & world){
	std::vector<Uint8> types;
	types.push_back(ObjectTypes::PLAYER);
	std::vector<Object *> objects = world.TestAABB(x - 320, y - 240, x + 320, y + 240, types);
	for(std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++){
		Player * player = static_cast<Player *>(*it);
		Team * team = player->GetTeam(world);
		if(team){
			discoveredby[team->number] = true;
		}
	}
}