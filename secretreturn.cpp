#include "secretreturn.h"

SecretReturn::SecretReturn() : Object(ObjectTypes::SECRETRETURN){
	requiresauthority = false;
	res_bank = 152;
	res_index = 0;
	teamid = 0;
	for(int i = 0; i < 3; i++){
		frame[i] = 0;
		framespeed[i] = 6;
	}
	state_i = 0;
}

void SecretReturn::Tick(World & world){
	for(int i = 0; i < 3; i++){
		if(state_i % framespeed[i] == 0){
			frame[i]++;
			if(frame[i] > 13){
				frame[i] = 9;
				framespeed[i] = (rand() % 3) + 5;
			}
		}
	}
	state_i++;
}