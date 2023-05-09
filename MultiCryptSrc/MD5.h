#pragma once
/**
 * @file md5.h
 * @The header file of md5.
 * @author Jiewei Wei
 * @mail weijieweijerry@163.com
 * @github https://github.com/JieweiWei
 * @data Oct 19 2014
 *
 */

/*
	------------------ NOTE ----------------------
	   TO SEE THE USAGE, PLEASE SKIP TO LINE 85.
	------------------ NOTE ----------------------
*/

#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))
#define FF(a, b, c, d, x, s, ac) { \
  (a) += F ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
  (a) += G ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
  (a) += H ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
  (a) += I ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#include <string>
#include <cstring>
#include "common.h"
using std::string;
typedef unsigned char byte;
typedef unsigned int bit32;
class __MD5 {
public:
	__MD5(const bytes& message);
	const byte* getDigest();
	string toStr();
private:
	void init(const byte* input, size_t len);
	void transform(const byte block[64]);
	void encode(const bit32* input, byte* output, size_t length);
	void decode(const byte* input, bit32* output, size_t length);
private:
	bool finished;
	bit32 state[4];
	bit32 count[2];
	byte buffer[64];
	byte digest[16];
	static const byte PADDING[64];
	static const char HEX_NUMBERS[16];
};

class MD5
{
public:
	static bytes digest(const bytes& message)
	{
		__MD5 md5(message);
		const byte* b = md5.getDigest();
		bytes result(16);
		memcpy_s(result.begin()._Ptr, 16, b, 16);
		return result;
	}
};
