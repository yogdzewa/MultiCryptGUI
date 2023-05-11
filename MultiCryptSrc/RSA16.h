#pragma once
#include "common.h"

static const std::vector<uint8_t> u8PrimesList = {
   2,3,5,7,11,13,17,19,23,29,31,
   37,41,43,47,53,59,61,67,71,73,
   79,83,89,97,101,103,107,109,113,
   127,131,137,139,149,151,157,163,
   167,173,179,181,191,193,197,199,
   211,223,227,229,233,239,241,251
};

class RSA16
{
	uint8_t primeA, primeB;
	uint16_t N, Totient, E, D;

public:
	struct Tuple { uint32_t number; int32_t identity1; int32_t identity2; };
	static uint32_t gcd(uint32_t a, uint32_t b)
	{
		Tuple A = { std::max(a,b), 0, 1 };
		Tuple B = { std::min(a,b), 1, 0 };
		Tuple C;

		while (true)
		{
			uint32_t quotient, remainder;
			quotient = A.number / B.number;
			remainder = A.number % B.number;

			//Subtract multiples of B from A
			C = { remainder,
						(int32_t)(A.identity1 - quotient * B.identity1),
						(int32_t)(A.identity2 - quotient * B.identity2) };

			if (C.number == 1)
				return 1;
			if (C.number == 0)
				return B.number;

			//Enter next round with B and Remainder.
			A = B;
			B = C;
		}
	}
	static uint32_t modinv(uint32_t a, uint32_t mod)
	{
		if ( a == 1 )
			return 1;
		Tuple A = { mod, 0, 1 };
		Tuple B = { a, 1, 0 };
		Tuple C;

		while (true)
		{
			uint32_t quotient, remainder;
			quotient = A.number / B.number;
			remainder = A.number % B.number;

			//Subtract multiples of B from A
			C = { remainder,
						(int)(A.identity1 - quotient * B.identity1),
						(int)(A.identity2 - quotient * B.identity2) };

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

		while (C.identity1 < 0) C.identity1 += mod;
		return C.identity1 % mod;
	}
	static int32_t lcm(uint32_t a, uint32_t b) { return a * b / gcd(a, b); }

	struct PubKey { uint16_t N, E; };
	struct PrivKey { uint16_t N, D; };
	PubKey getPubKey() const { return { N,E }; }
	PrivKey getPrivKey() const { return { N, D }; }

	struct AllInfo {
		uint8_t primeA, primeB;
		uint16_t N, Totient, E, D;
	};
	AllInfo getAllInfo() const { return { primeA, primeB, N, Totient, E, D }; }

	//Create an instance for both encryption and decryption
	//All key generation is done automatically. Use getPubKey/getPrivKey/getAllInfo to see the parameters generated.
	RSA16()
	{
		srand(time(NULL));
		N = 0;
		while (N < 0x100 || N>= 0xFFFF) //Ensure public modulo is big enough to encrypt on byte
		{
			primeA = u8PrimesList[rand() % u8PrimesList.size()];
			primeB = u8PrimesList[rand() % u8PrimesList.size()];
			N = primeA * primeB;
		}

		Totient = lcm(primeA - 1, primeB - 1);

		E = Totient;
		while (gcd(E, Totient) > 1)
			E = (rand() % (Totient - 2) - 1) + 2;

		D = modinv(E, Totient);
	}

	//Create an instance for encryption from public key.
	//Any operation involving private key will be undefined.
	RSA16(const PubKey& k)
	{
		N = k.N;
		E = k.E;
	}

	//Create an instance for decryption from private key.
	//Any operation involving publik key will be undefined.
	RSA16(const PrivKey& k) 
	{
		N = k.N;
		D = k.D;
	}

	//Create an instance for both encryption and decryption from AllInfo.
	RSA16(const AllInfo& i)
	{
		primeA = i.primeA;
		primeB = i.primeB;
		N = i.N;
		Totient = i.Totient;
		E = i.E;
		D = i.D;
	}

	//b^exp mod N
	template <class inputType, class outputType>
	outputType raise(inputType b, uint16_t exponent) const
	{
		if (b == 0)
			return 0;
		uint32_t result = 1;
		for (int i = 0; i < exponent; i++)
			result = (result * b) % N;

		return result;
	}

	enum enumKeys { PUBLIC_KEY, PRIVATE_KEY };

	//Encrypt always takes byte and gives u16
	uint16_t encryptByteWithKey(byte b, enumKeys k) const
	{return raise<byte, uint16_t>(b, (k == PUBLIC_KEY) ? E : D);}

	//Decrypt always takes u16 and gives byte
	byte decryptByteWithKey(uint16_t e, enumKeys k) const
	{return raise<uint16_t, byte>(e, (k == PUBLIC_KEY) ? E : D);}

	//Encryption and decryption
	uint16_t encryptByte(byte b) const
	{return encryptByteWithKey(b, PUBLIC_KEY);}
	uint8_t decryptByte(uint16_t e) const
	{return decryptByteWithKey(e, PRIVATE_KEY);}

	//Signing and verification
	uint16_t signByte(byte b) const
	{return encryptByteWithKey(b, PRIVATE_KEY);}
	bool verifySignedByte(uint16_t signed_byte, byte original_byte) const
	{return decryptByteWithKey(signed_byte, PUBLIC_KEY) == original_byte;}

	typedef std::vector<uint16_t> rsa16_msg;

	//Encryption and decryption
	rsa16_msg encryptBytes(const bytes& b) const {
		std::vector<uint16_t> result(b.size());
		for (size_t i = 0; i < b.size(); i++)
			result[i] = encryptByte(b[i]);
		return result;
	}
	bytes decryptBytes(const rsa16_msg& b) const {
		bytes result(b.size());
		for (size_t i = 0; i < b.size(); i++)
			result[i] = decryptByte(b[i]);
		return result;
	}

	//Signing and verification
	rsa16_msg signBytes(const bytes& b) const {
		std::vector<uint16_t> result(b.size());
		for (size_t i = 0; i < b.size(); i++)
			result[i] = signByte(b[i]);
		return result;
	}

	bool verifySignedBytes(const rsa16_msg& signed_bytes, const bytes& original_bytes) const {
		for (size_t i = 0; i < signed_bytes.size(); i++)
			if (verifySignedByte(signed_bytes[i], original_bytes[i]) == false)
				return false;
		return true;
	}

	byte decryptSignedByte(uint16_t e) const {
		return decryptByteWithKey(e, PUBLIC_KEY);
	}

	bytes decryptSignedBytes(const rsa16_msg& b) const {
		bytes result(b.size());
		for (size_t i = 0; i < b.size(); i++)
			result[i] = decryptSignedByte(b[i]);
		return result;
	}

};