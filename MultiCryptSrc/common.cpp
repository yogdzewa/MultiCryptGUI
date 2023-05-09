#include "common.h" 
#include <string.h>

bytes stringToBytes(const std::string& s)
{
	bytes b(s.length());
	memcpy_s(b.begin()._Ptr, s.length(), s.begin()._Ptr, s.length());
	return b;
}

std::string bytesToString(const bytes& b)
{
	std::string s;
	s.resize(b.size());
	memcpy_s((void*)s.begin()._Ptr, b.size(), b.begin()._Ptr, b.size());
	return s;
}

bits stringToBits(const std::string& s)
{
	bits b;
	for (size_t i = 0; i < s.length(); i++)
	{
		if (s[i] == '1')
			b.push_back(1);
		else if (s[i] == '0')
			b.push_back(0);
	}
	return b;
}

bytes concatBytes(const bytes& a, const bytes& b)
{
	bytes result = a;
	result.insert(result.end(), b.begin(), b.end());
	return result;
}

std::string bytesToHexdump(const bytes& b)
{
	std::stringstream ss;
	for (size_t i = 0; i < b.size(); i++)
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)b[i];
	return ss.str();
}

int8_t parseHexDigit(char ch)
{
	if ('0' <= ch && ch <= '9')
		return ch - '0';
	else if ('a' <= ch && ch <= 'h')
		return ch - 'a' + 10;
	else if ('A' <= ch && ch <= 'H')
		return ch - 'A' + 10;
	else
		return -1;
}

bytes hexdumpToBytes(const std::string& s)
{
	bytes result;

	int16_t currByte= -1;
	for (size_t i = 0; i < s.length(); i++)
	{
		int8_t tmp = parseHexDigit(s[i]);
		if (tmp >= 0) //valid hex digit
		{
			if (currByte == -1) //first digit
				currByte = tmp << 4;
			else
			{
				currByte += tmp;
				result.push_back(currByte);
				currByte = -1;
			}
		}
	}
	
	return result;
}

bytes xorBytes(const bytes& randBytes, const bytes& message)
{
	size_t len = std::min(randBytes.size(), message.size());
	bytes result(len);
	for (size_t i = 0; i < len; i++)
		result[i] = randBytes[i] ^ message[i];
	return result;
}