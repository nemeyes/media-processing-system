#include "dk_bit_vector.h"

static unsigned char const singleBitMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

#define MAX_LENGTH 32

void shift_bits(unsigned char* toBasePtr, unsigned toBitOffset, unsigned char const* fromBasePtr, unsigned fromBitOffset, unsigned numBits)
{
	if (numBits == 0)
		return;

	/* Note that from and to may overlap, if from>to */
	unsigned char const* fromBytePtr = fromBasePtr + fromBitOffset / 8;
	unsigned fromBitRem = fromBitOffset % 8;
	unsigned char* toBytePtr = toBasePtr + toBitOffset / 8;
	unsigned toBitRem = toBitOffset % 8;

	while (numBits-- > 0)
	{
		unsigned char fromBitMask = singleBitMask[fromBitRem];
		unsigned char fromBit = (*fromBytePtr)&fromBitMask;
		unsigned char toBitMask = singleBitMask[toBitRem];

		if (fromBit != 0)
		{
			*toBytePtr |= toBitMask;
		}
		else
		{
			*toBytePtr &= ~toBitMask;
		}

		if (++fromBitRem == 8)
		{
			++fromBytePtr;
			fromBitRem = 0;
		}
		if (++toBitRem == 8)
		{
			++toBytePtr;
			toBitRem = 0;
		}
	}
}

void remove_emulation_bytes(uint8_t *dst, uint8_t *src, uint32_t max_size, uint32_t num_bytes_in_nal_unit, uint32_t *copy_size)
{
	unsigned int i;
	if (num_bytes_in_nal_unit > max_size) return;
	*copy_size = 0;
	for (i = 0; i < num_bytes_in_nal_unit; i++)
	{
		if (i + 2 < num_bytes_in_nal_unit && src[i] == 0 && src[i + 1] == 0 && src[i + 2] == 3)
		{
			dst[(*copy_size)++] = src[i++];
			dst[(*copy_size)++] = src[i++];
		}
		else
		{
			dst[(*copy_size)++] = src[i];
		}
	}
}

uint32_t __inline log2bin(uint32_t value)
{
	uint32_t n = 0;
	while (value)
	{
		value >>= 1;
		n++;
	}
	return n;
}


CBitVector::CBitVector(unsigned char* baseBytePtr, unsigned baseBitOffset, unsigned totNumBits)
{
	setup(baseBytePtr, baseBitOffset, totNumBits);
}

void CBitVector::setup(unsigned char* baseBytePtr, unsigned baseBitOffset, unsigned totNumBits)
{
	fBaseBytePtr = baseBytePtr;
	fBaseBitOffset = baseBitOffset;
	fTotNumBits = totNumBits;
	fCurBitIndex = 0;
}

void CBitVector::putBits(unsigned from, unsigned numBits)
{
	if (numBits == 0)
		return;

	unsigned char tmpBuf[4];
	unsigned overflowingBits = 0;

	if (numBits > MAX_LENGTH)
	{
		numBits = MAX_LENGTH;
	}

	if (numBits > fTotNumBits - fCurBitIndex)
	{
		overflowingBits = numBits - (fTotNumBits - fCurBitIndex);
	}

	tmpBuf[0] = (unsigned char)(from >> 24);
	tmpBuf[1] = (unsigned char)(from >> 16);
	tmpBuf[2] = (unsigned char)(from >> 8);
	tmpBuf[3] = (unsigned char)from;

	shift_bits(fBaseBytePtr, fBaseBitOffset + fCurBitIndex, /* to */tmpBuf, MAX_LENGTH - numBits, /* from */numBits - overflowingBits /* num bits */);
	fCurBitIndex += numBits - overflowingBits;
}

void CBitVector::put1Bit(unsigned bit)
{
	// The following is equivalent to "putBits(..., 1)", except faster:
	if (fCurBitIndex >= fTotNumBits)
	{ /* overflow */
		return;
	}
	else
	{
		unsigned totBitOffset = fBaseBitOffset + fCurBitIndex++;
		unsigned char mask = singleBitMask[totBitOffset % 8];
		if (bit)
		{
			fBaseBytePtr[totBitOffset / 8] |= mask;
		}
		else
		{
			fBaseBytePtr[totBitOffset / 8] &= ~mask;
		}
	}
}

unsigned CBitVector::getBits(unsigned numBits)
{
	if (numBits == 0)
		return 0;

	unsigned char tmpBuf[4];
	unsigned overflowingBits = 0;

	if (numBits > MAX_LENGTH)
	{
		numBits = MAX_LENGTH;
	}

	if (numBits > fTotNumBits - fCurBitIndex)
	{
		overflowingBits = numBits - (fTotNumBits - fCurBitIndex);
	}

	shift_bits(tmpBuf, 0, /* to */fBaseBytePtr, fBaseBitOffset + fCurBitIndex, /* from */numBits - overflowingBits /* num bits */);
	fCurBitIndex += numBits - overflowingBits;

	unsigned result = (tmpBuf[0] << 24) | (tmpBuf[1] << 16) | (tmpBuf[2] << 8) | tmpBuf[3];
	result >>= (MAX_LENGTH - numBits); // move into low-order part of word
	result &= (0xFFFFFFFF << overflowingBits); // so any overflow bits are 0
	return result;
}

unsigned CBitVector::get1Bit()
{
	// The following is equivalent to "getBits(1)", except faster:
	if (fCurBitIndex >= fTotNumBits)
	{ /* overflow */
		return 0;
	}
	else
	{
		unsigned totBitOffset = fBaseBitOffset + fCurBitIndex++;
		unsigned char curFromByte = fBaseBytePtr[totBitOffset / 8];
		unsigned result = (curFromByte >> (7 - (totBitOffset % 8))) & 0x01;
		return result;
	}
}

void CBitVector::skipBits(unsigned numBits)
{
	if (numBits > fTotNumBits - fCurBitIndex)
	{ /* overflow */
		fCurBitIndex = fTotNumBits;
	}
	else
	{
		fCurBitIndex += numBits;
	}
}

unsigned CBitVector::get_expGolomb()
{
	unsigned numLeadingZeroBits = 0;
	unsigned codeStart = 1;

	while (get1Bit() == 0 && fCurBitIndex < fTotNumBits)
	{
		++numLeadingZeroBits;
		codeStart *= 2;
	}

	return codeStart - 1 + getBits(numLeadingZeroBits);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////