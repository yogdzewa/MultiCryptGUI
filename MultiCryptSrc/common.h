#pragma once

#include <vector>
#include <cstdint>
#include <array>
#include <string>
#include <iomanip>
#include <sstream>
#include <assert.h>

typedef uint8_t byte;
typedef std::vector<uint8_t> bytes;

//Never store values other than 0 and 1.
typedef uint8_t bit;
typedef std::vector<uint8_t> bits;

//Conversion between string and raw byte vector
bytes stringToBytes(const std::string& s);
std::string bytesToString(const bytes& b);

//Conversion between byte vector and readable hexdump text
std::string bytesToHexdump(const bytes& b);
bytes hexdumpToBytes(const std::string& s);

//Returns hex value of character.
//Returns -1 when fail.
int8_t parseHexDigit(char ch);

//Utility function to use with keystream generator (RC4/LFSR-JK)
bytes xorBytes(const bytes& randBytes, const bytes& message);

//Takes: "00101101"
//Gives: vector<uint8_t>{0,0,1,0,1,1,0,1}
bits stringToBits(const std::string& s);

template <class T>
bytes castVariableToBytes(const T& var)
{
	bytes b(sizeof(T));
	memcpy_s(b.begin()._Ptr, sizeof(T), &var, sizeof(T));
	return b;
}

template <class T>
T& castBytesToVariable(const bytes& b)
{
	return *(T*)(b.begin()._Ptr);
}

template <class T>
bytes castVectorToBytes(const std::vector<T>& vec)
{
	bytes b(sizeof(T) * vec.size());
	memcpy_s(b.begin()._Ptr, sizeof(T) * vec.size(), vec.begin()._Ptr, sizeof(T) * vec.size());
	return b;
}

bytes concatBytes(const bytes& a, const bytes& b);