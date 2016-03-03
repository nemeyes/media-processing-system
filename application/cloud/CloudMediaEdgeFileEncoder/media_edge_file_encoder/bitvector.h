#ifndef _BIT_VECTOR_H_
#define _BIT_VECTOR_H_

#include <cstdint>

#define DEBUG_PRINT(x) do {x = x;} while (0)

typedef enum
{
	BASELINE = 66,
	MAIN = 77,
	EXTENDED = 88,
	HIGH = 100,
	FREXT_Hi10P = 110,
	FREXT_Hi422 = 122,
	FREXT_Hi444 = 244,
	FREXT_CAVLC444 = 44,
} H264_PROFILE;

class CBitVector
{
public:
	CBitVector(unsigned char* baseBytePtr,
		unsigned baseBitOffset,
		unsigned totNumBits);

	void setup(unsigned char* baseBytePtr,
		unsigned baseBitOffset,
		unsigned totNumBits);

	void putBits(unsigned from, unsigned numBits); // "numBits" <= 32
	void put1Bit(unsigned bit);

	unsigned getBits(unsigned numBits); // "numBits" <= 32
	unsigned get1Bit();

	void skipBits(unsigned numBits);

	unsigned curBitIndex() const { return fCurBitIndex; }
	unsigned totNumBits() const { return fTotNumBits; }
	unsigned numBitsRemaining() const { return fTotNumBits - fCurBitIndex; }

	unsigned get_expGolomb();
	// Returns the value of the next bits, assuming that they were encoded using an exponential-Golomb code of order 0

private:
	unsigned char* fBaseBytePtr;
	unsigned fBaseBitOffset;
	unsigned fTotNumBits;
	unsigned fCurBitIndex;
};

extern void shift_bits(unsigned char* toBasePtr, unsigned toBitOffset, unsigned char const* fromBasePtr, unsigned fromBitOffset, unsigned numBits);
extern void remove_emulation_bytes(uint8_t *dst, uint8_t *src, uint32_t max_size, uint32_t num_bytes_in_nal_unit, uint32_t *copy_size);
extern uint32_t log2bin(uint32_t value);
#endif