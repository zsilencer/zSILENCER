#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <string.h>
#include <vector>

class Serializer
{
public:
	Serializer(unsigned int size = 1024);
	Serializer(char * data, unsigned int size);
	void Resize(unsigned int newsize);
	void Copy(Serializer & from);
	bool MoreToRead(void);
	bool MoreBytesToRead(void);
	static unsigned int BitsToBytes(unsigned int bits);
	template <class type>
	void Serialize(bool write, type & value, Serializer * old){
		if(old){
			SerializeDelta(write, value, *old);
		}else{
			Serialize(write, value);
		}
	}
	template <class type>
	void Serialize(bool write, type & value){
		if(write){
			Put(value);
		}else{
			Get(value);
		}
	}
	template <class type>
	void SerializeDelta(bool write, type & value, type & oldvalue){
		if(write){
			if(value != oldvalue){
				PutBit(true);
				Put(value);
			}else{
				PutBit(false);
			}
		}else{
			if(GetBit()){
				Get(value);
			}
		}
	}
	template <class type>
	void SerializeDelta(bool write, type & value, Serializer & oldvalue){
		type old;
		if(write){
			oldvalue.Get(old);
		}
		SerializeDelta(write, value, old);
	}
	template <class type>
	void Put(type & value){
		PutBits((char *)&value, sizeof(value) * 8);
	}
	void Put(bool & value){
		PutBit(value);
	}
	void PutBit(bool bit);
	void PutBits(const char * source, int numberofbits);
	template <class type>
	bool Get(type & value){
		return GetBits((char *)&value, sizeof(value) * 8);
	}
	bool Get(bool & value){
		value = GetBit();
		return value;
	}
	bool GetBit(void);
	bool GetBits(char * output, int numberofbits);

	enum {READ = 0, WRITE};
	char * data;
	unsigned int size;
	unsigned int offset;
	unsigned int readoffset;
	
private:
	std::vector<char> datap;
};

#endif