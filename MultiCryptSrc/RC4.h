#pragma once
#include "common.h"

class RC4
{
	std::array<byte, 256> sbox;
	int i = 0, j = 0;

public:
	RC4() {
		for (int i = 0; i < 256; i++)
			sbox[i] = i;
	}

	void permuteSbox(const bytes& key) {
		int j = 0;
		for (int i = 0; i < 256; i++)
		{
			j = (j + sbox[i] + key[i % key.size()]) % 256;
			std::swap(sbox[i], sbox[j]);
		}
	}

	byte nextRandByte() {
		i = (i + 1) % 256;
		j = (j + sbox[i]) % 256;
		std::swap(sbox[i], sbox[j]);
		return sbox[(sbox[i] + sbox[j]) % 256];
	}

	bytes generateRandBytes(size_t len) {
		bytes result(len);
		for (size_t i = 0; i < len; i++)
			result[i] = nextRandByte();
		return result;
	}
};

/*
Usage:
	RC4 keygen;
	keygen.permuteSbox(key);
	keystream = keygen.generateRnadBytes(len);
	xorBytes(keysteram, message);
*/