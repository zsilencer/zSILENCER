#include "secretreturn.h"

SecretReturn::SecretReturn() : Object(ObjectTypes::SECRETRETURN){
	requiresauthority = false;
	res_bank = 152;
	res_index = 0;
	teamid = 0;
}

void SecretReturn::Tick(World & world){
	
}