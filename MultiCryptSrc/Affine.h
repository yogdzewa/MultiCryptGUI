#pragma once

#include "common.h"

class Affine
{
	uint32_t multiplier;
	uint32_t offset;
	uint32_t modular_inverse;

	struct Tuple { int32_t number; int32_t identity1; int32_t identity2; };
	static int32_t mod256Inverse(uint32_t n)
	{
		Tuple A = { 256, 0, 1 };
		Tuple B = { n, 1, 0 };
		Tuple C;

		while (true)
		{
			int32_t quotient, remainder;
			quotient = A.number / B.number;
			remainder = A.number % B.number;

			//Subtract multiples of B from A
			C = { remainder,
						A.identity1 - quotient * B.identity1,
						A.identity2 - quotient * B.identity2 };

			if (C.number == 1)
				break;
			if (C.number == 0)
				return -1;

			//Enter next round with B and Remainder.
			//A.number always bigger than B.number.
			if (B.number > C.number)
			{
				A = B;
				B = C;
			}
			else
				A = C;
		}
		
		while (C.identity1 < 0) C.identity1 += 256;

		return C.identity1 % 256;
	}

public:
	// EncryptedByte = [(Byte * multiplier) + offset] % 256 
	Affine(uint32_t _multiplier, uint32_t _offset) : multiplier(_multiplier), offset(_offset), modular_inverse(mod256Inverse(_multiplier))
	{}

	byte encryptByte(byte b) const {
		return ((uint32_t)(b * multiplier) % 256 + offset) % 256;
	}

	byte decryptByte(byte b) const {
		return ((((uint32_t)b + 256 - offset) % 256) * modular_inverse) % 256;
	}

	bytes encryptBytes(const bytes& b) const {
		bytes result(b.size());
		for (size_t i = 0; i < b.size(); i++)
			result[i] = encryptByte(b[i]);
		return result;
	}

	bytes decryptBytes(const bytes& b) const {
		bytes result(b.size());
		for (size_t i = 0; i < b.size(); i++)
			result[i] = decryptByte(b[i]);
		return result;
	}
};