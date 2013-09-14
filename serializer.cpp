#include "serializer.h"

Serializer::Serializer(unsigned int size){
	offset = 0;
	readoffset = 0;
	Serializer::size = size;
	data = new char[size];
	outsidedata = false;
}

Serializer::Serializer(char * data, unsigned int size){
	offset = size * 8;
	readoffset = 0;
	Serializer::data = data;
	Serializer::size = size;
	outsidedata = true;
}

Serializer::~Serializer(){
	if(!outsidedata){
		delete[] data;
	}
}

void Serializer::Resize(unsigned int newsize){
	char * newdata = new char[newsize];
	memcpy(newdata, data, BitsToBytes(offset));
	if(!outsidedata){
		delete[] data;
	}
	outsidedata = false;
	data = newdata;
	size = newsize;
}

void Serializer::Copy(Serializer & from){
	if(from.size > size){
		Resize(from.size);
	}
	memcpy(data, from.data, BitsToBytes(from.offset));
	offset = from.offset;
	readoffset = from.readoffset;
}

bool Serializer::MoreToRead(void){
	return readoffset < offset;
}

bool Serializer::MoreBytesToRead(void){
	return BitsToBytes(readoffset) < BitsToBytes(offset);
}

unsigned int Serializer::BitsToBytes(unsigned int bits){
	return (((bits)+7)>>3);
}

void Serializer::PutBit(bool bit){
	if(BitsToBytes(offset + 1) > size){
		Resize(size + 1024);
	}
	if(bit){
		unsigned int offsetmod8 = offset & 7;
		if(offsetmod8 == 0){
			data[offset >> 3] = (char)0x80;
		}else{
			data[offset >> 3] |= 0x80 >> (offsetmod8);
		}
	}else{
		if((offset & 7) == 0){
			data[offset >> 3] = 0;
		}
	}
	offset++;
}

void Serializer::PutBits(const char * source, int numberofbits){
	if(BitsToBytes(offset + numberofbits) > size){
		Resize(size + 1024);
	}
	if(numberofbits <= 0){
		return;
	}
	int offsetmod8 = offset & 7;
	int byteoffset = 0;
	char databyte;
	while(numberofbits > 0){
		databyte = *(source + byteoffset);
		if(numberofbits < 8){
			databyte <<= 8 - numberofbits;
		}
		if(offsetmod8 == 0){
			*(data + (offset >> 3)) = databyte;
		}else{
			*(data + (offset >> 3)) |= (unsigned char)databyte >> (offsetmod8);
			if(8 - (offsetmod8) < 8 && 8 - (offsetmod8) < numberofbits){
				*(data + (offset >> 3) + 1) = (databyte << (8 - (offsetmod8)));
			}
		}
		if(numberofbits >= 8){
			offset += 8;
		}else{
			offset += numberofbits;
		}
		numberofbits -= 8;
		byteoffset++;
	}
}

bool Serializer::GetBit(void){
	bool bit = (data[readoffset >> 3] & (0x80 >> (readoffset & 7))) ? true : false;
	readoffset++;
	return bit;
}

bool Serializer::GetBits(char * output, int numberofbits){
	if(numberofbits <= 0){
		return false;
	}
	if(readoffset + numberofbits > offset){
		return false;
	}
	int readoffsetmod8 = readoffset & 7;
	int byteoffset = 0;
	memset(output, 0, BitsToBytes(numberofbits));
	while(numberofbits > 0){
		*(output + byteoffset) |= (*(data + (readoffset >> 3))) << (readoffsetmod8);
		if(readoffsetmod8 > 0 && numberofbits > 8 - (readoffsetmod8)){
			*(output + byteoffset) |= (unsigned char)(*(data + (readoffset >> 3) + 1)) >> (8 - (readoffsetmod8));
		}	
		numberofbits -= 8;
		if(numberofbits < 0){
			*(output + byteoffset) >>= -numberofbits;
			readoffset += 8 + numberofbits;
		}else{
			readoffset += 8;
		}
		byteoffset++;
	}
	return true;
}