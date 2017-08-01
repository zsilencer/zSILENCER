#include "state.h"

State::State() : Object(ObjectTypes::STATE){
	requiresauthority = true;
	requiresmaptobeloaded = false;
	state = 0;
	oldstate = 0;
	issprite = false;
}

void State::Serialize(bool write, Serializer & data, Serializer * old){
	Object::Serialize(write, data, old);
	data.Serialize(write, state, old);
}