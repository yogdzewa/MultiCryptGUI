#pragma once
#include "common.h"

class DiffieHellman
{
	uint32_t g, n, x, y, yy, k, key;
public:
	void initAgreement(uint32_t _g, uint32_t _n) {
		g = _g;
		n = _n;
	}

	uint32_t generatePubkey(uint32_t _x) {
		x = _x;
		uint64_t result = 1;
		for (uint32_t i = 0; i < x; i++)
			result = (result * g) % n;
		y = result;
		return result;
	}

	uint32_t generatePubkey() {
		srand(time(NULL));
		generatePubkey(rand() % n);
	}

	uint32_t generateSharedKey(uint32_t pubkey_from_other_peer) {
		yy = pubkey_from_other_peer;
		uint64_t result = 1;
		for (uint32_t i = 0; i < x; i++)
			result = (result * yy) % n;
		key = result;
		return result;
	}
};

/*

Usage:

	DiffieHellman Alice;					DiffieHellman Bob;
	Alice.initAgreement(g, n);				Bob.initAgreement(g, n);
	pub_A = Alice.generatePubkey();			pub_B = Bob.generatePubkey();
				|										|
	[Alice gives pub_A to Bob]				[Bob gives pub_B to Alice]
				|										|
		        v										v
	   Alice.generateSharedKey(pub_B) == Bob.generateSharedKey(pub_A)
*/