#pragma once

#include "common.h"
#include <deque>

class LFSR
{
	std::deque<bit> lfsr;
	bits feedback;

public:

	LFSR(const bits& _feedback, const bits& _initial)
	{
		assert(_feedback.size() == _initial.size());

		feedback = _feedback;
		lfsr.resize(_initial.size());
		for (size_t i = 0; i < _initial.size(); i++)
			lfsr[i] = _initial[i];
	}

	bit nextState()
	{
		bit newBit = 0;
		bit retVal = lfsr[lfsr.size() - 1];

		for (size_t i = 0; i < lfsr.size(); i++)
			newBit ^= lfsr[i] * feedback[i];

		lfsr.pop_back();
		lfsr.push_front(newBit);
		return retVal;
	}

	//Least Significant Bit first
	byte generateByte()
	{
		byte result = 0;
		for (int pos = 0; pos < 8; pos++)
			result += nextState()<<pos;
		return result;
	}
};

class JK 
{
	bit q;
public:
	JK(bit initial) { q = initial; }
	JK() { q = 0; }
	bit nextState(bit J, bit K)
	{
		switch (J*2 + K)
		{
		case 0: return q;
		case 1: return q = 0;
		case 2: return q = 1;
		case 3: return q = !q;
		}
	}
};

class LFSR_JK
{
	LFSR lfsr1;
	LFSR lfsr2;
	JK jk;

public:
	/*
	Example:
		Let parameters be:
		lfsr1_feedback = stringToBits("10010001")
		lfsr1_initial  = stringToBits("10000100")
		
		After initialization, LFSR1 holds 10000100
										  ^  ^   ^--- After a single step, XOR of these bits will replace the leftmost bit of LFSR1.
													  Rest of the bits rotates right, and LFSR1 outputs the rightmost bit to JK-Trigger.
						 	   feedback = 10010001

		Same holds for LFSR2.

		jk_initial defines the Q value that the JK-Trigger holds at the beginning.
	*/
	LFSR_JK(const bits& lfsr1_feedback, const bits& lfsr1_initial, const bits& lfsr2_feedback, const bits& lfsr2_initial, bit jk_initial) :
		lfsr1(lfsr1_feedback, lfsr1_initial),
		lfsr2(lfsr2_feedback, lfsr2_initial),
		jk(jk_initial)
	{}

	//Generates one bit of key, changes keystream generator state accordingly.
	bit nextState()
	{
		return jk.nextState(lfsr1.nextState(), lfsr2.nextState());
	}

	//Generates one byte of key, *LEAST SIGNIFICANT BIT GENERATED FIRST*
	byte keyStreamByte()
	{
		byte result = 0;
		for (int pos = 0; pos < 8; pos++)
			result += nextState() << pos;
		return result;
	}

	//Generates key of specified length in bytes.
	bytes keyStreamBytes(size_t len) {
		bytes result(len);
		for (size_t i = 0; i < len; i++)
			result[i] = keyStreamByte();
		return result;
	}
};