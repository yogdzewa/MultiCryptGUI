#pragma once

#define ENCRYPTION_MODE 1
#define DECRYPTION_MODE 0

#include "common.h"

typedef struct {
	unsigned char k[8];
	unsigned char c[4];
	unsigned char d[4];
} key_set;

void generate_key(unsigned char* key);
void generate_sub_keys(unsigned char* main_key, key_set* key_sets);
void process_message(unsigned char* message_piece, unsigned char* processed_piece, key_set* key_sets, int mode);

class DES
{
	key_set key_sets[17];
public:
	//Initialize with key
	DES(bytes key_8byte) 
	{
		assert(key_8byte.size() == 8);
		
		uint8_t main_key[8];
		memcpy_s(main_key, 8, key_8byte.begin()._Ptr, 8);
		generate_sub_keys(main_key, key_sets);
	}

	//If block is bigger than 64 bits, only 64 bits get processed.
	//If block is smaller, it's padded with zeros.
	bytes encryptBlock(bytes block)
	{
		if (block.size() < 8)
			block.resize(8, 0);

		bytes result(8);
		process_message(block.begin()._Ptr, result.begin()._Ptr, key_sets, ENCRYPTION_MODE);
		return result;
	}

	//Block size must strictly equal to 64 bits for decryption.
	bytes decryptBlock(bytes block)
	{
		assert(block.size() == 8);

		bytes result(8);
		process_message(block.begin()._Ptr, result.begin()._Ptr, key_sets, DECRYPTION_MODE);
		return result;
	}

	//Adds an 64-bit unsigned integer to the front of a message of any length, and encrypt.
	bytes encryptMessageWithLength(bytes message)
	{
		bytes result;

		uint64_t len = message.size();
		message.insert(message.begin(), { 0, 0, 0, 0, 0, 0, 0, 0 }); //Insert 64 bits to the front
		*((uint64_t*)(message.begin()._Ptr)) = len; //First DES block records the length of original message

		message.resize((message.size() / 8 + !!(message.size() % 8)) * 8, 0); //Pad message to 64*n bits
		result.resize(message.size());
		for (size_t blockBegin = 0; blockBegin < message.size(); blockBegin+=8)
		{
			bytes block(message.begin() + blockBegin, message.begin() + blockBegin + 8);
			block = encryptBlock(block);
			memcpy_s((result.begin() + blockBegin)._Ptr, 8, block.begin()._Ptr, 8);
		}

		return result;
	}

	//Reverse process of encryptMessageWithLength
	bytes decryptMessageWithLength(bytes encrypted)
	{
		bytes result(encrypted.size());
		for (size_t blockBegin = 0; blockBegin < encrypted.size(); blockBegin += 8)
		{
			bytes block(encrypted.begin() + blockBegin, encrypted.begin() + blockBegin + 8);
			block = decryptBlock(block);
			memcpy_s((result.begin() + blockBegin)._Ptr, 8, block.begin()._Ptr, 8);
		}

		uint64_t len;
		memcpy_s(&len, 8, result.begin()._Ptr, 8);
		bytes message(result.begin() + 8, result.begin() + 8 + len);
		return message;
	}
};