#include "object.h"
#include "resources.h"
#include "audio.h"

Object::Object(Uint8 type){
	Object::type = type;
	id = 0;
	requiresauthority = false;
	requiresmaptobeloaded = true;
	snapshotinterval = -1;
	lasttick = 0;
	wasdestroyed = false;
	issprite = true;
	isphysical = false;
	ishittable = false;
	isbipedal = false;
	isprojectile = false;
	iscontrollable = false;
}

bool Object::RequiresAuthority(void){
	return requiresauthority;
}

void Object::Tick(World & world){

}

void Object::Serialize(bool write, Serializer & data, Serializer * old){
	// type and id are not delta compressed so we can look up the object
	data.Serialize(write, type);
	data.Serialize(write, id);
	if(Serializer::WRITE == write && old){
		old->readoffset += sizeof(type) * 8;
		old->readoffset += sizeof(id) * 8;
	}
	//
	if(issprite){
		Sprite::Serialize(write, data, old);
	}
	if(isphysical){
		Physical::Serialize(write, data, old);
	}
	if(ishittable){
		Hittable::Serialize(write, data, old);
	}
	if(isbipedal){
		Bipedal::Serialize(write, data, old);
	}
	if(isprojectile){
		Projectile::Serialize(write, data, old);
	}
}

void Object::OnDestroy(World & world){
	
}

void Object::HandleHit(World & world, Uint8 x, Uint8 y, Object & projectile){

}

void Object::HandleInput(Input & input){
	
}

void Object::HandleDisconnect(World & world, Uint8 peerid){
	
}

int Object::EmitSound(class World & world, Mix_Chunk * chunk, Uint8 volume, bool loop){
	return Audio::GetInstance().EmitSound(world, id, chunk, volume, loop);
}