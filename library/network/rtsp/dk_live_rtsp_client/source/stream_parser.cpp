#include "stream_parser.h"
#include "dk_live_rtsp_client.h"

#define AV_RB32(x)  \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) | \
               (((const uint8_t*)(x))[1] << 16) | \
               (((const uint8_t*)(x))[2] <<  8) | \
                ((const uint8_t*)(x))[3])

#define MIN(a,b) ((a) > (b) ? (b) : (a))

bool stream_parser::is_vps(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type)
{
	// VPS NAL units occur in H.265 only:
	return smt == dk_live_rtsp_client::SUBMEDIA_TYPE_HEVC && nal_unit_type == 32;
}

bool stream_parser::is_sps(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type)
{
	return smt == dk_live_rtsp_client::SUBMEDIA_TYPE_H264 ? nal_unit_type == 7 : nal_unit_type == 33;
}

bool stream_parser::is_pps(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type)
{
	return smt == dk_live_rtsp_client::SUBMEDIA_TYPE_H264 ? nal_unit_type == 8 : nal_unit_type == 34;
}

bool stream_parser::is_idr(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type)
{
	return smt == dk_live_rtsp_client::SUBMEDIA_TYPE_H264 ? nal_unit_type == 5 : nal_unit_type == 34;
}

bool stream_parser::is_vlc(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type)
{
	return smt == dk_live_rtsp_client::SUBMEDIA_TYPE_H264 ? (nal_unit_type <= 5 && nal_unit_type > 0) : (nal_unit_type <= 31);
}

const int stream_parser::find_nal_unit(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end)
{
	int i;
	// find start
	*nal_start = 0;
	*nal_end = 0;

	i = 0;
	//( next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) && 
		   (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0 || bitstream[i + 3] != 0x01))
	{
		i++; // skip leading zero
		if (i + 4 >= size) 
		{ 
			return 0; 
		} // did not find nal start
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) // ( next_bits( 24 ) != 0x000001 )
	{
		i++;
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) 
	{ 
		/* error, should never happen */ 
		return 0; 
	}

	i += 3;
	*nal_start = i;

	//( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0) &&
		   (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01))
	{
		i++;
		// FIXME the next line fails when reading a nal that ends exactly at the end of the data
		if (i + 3 >= size) 
		{ 
			*nal_end = size; 
			return -1; 
		} // did not find nal end, stream ended first
	}

	*nal_end = i;
	return (*nal_end - *nal_start);
}


const uint8_t * stream_parser::find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state)
{
	int i;
	if (begin >= end)
		return end;

	for (i = 0; i < 3; i++)
	{
		uint32_t tmp = *state << 8;
		*state = tmp + *(begin++);
		if (tmp == 0x100 || begin == end)
			return begin;
	}

	while (begin < end)
	{
		if (begin[-1] > 1)
			begin += 3;
		else if (begin[-2])
			begin += 2;
		else if (begin[-3] | (begin[-1] - 1))
			begin++;
		else
		{
			begin++;
			break;
		}
	}

	begin = MIN(begin, end) - 4;
	*state = AV_RB32(begin);
	return begin + 4;
}

/*
static __forceinline uint16_t AV_RL16(const void *p)
{
	uint16_t v;
	__asm{
		ld.ub %0, %1
		"ldins.b % 0:l, % 2"
		: "=&r"(v)
		: "m"(*(const uint8_t*)p), "RKs12"(*((const uint8_t*)p + 1))
	};
	return v;
}

static __forceinline uint16_t AV_RB16(const void *p)
{
	uint16_t v;
	__asm("ld.ub    %0,   %2  \n\t"
		"ldins.b  %0:l, %1  \n\t"
		: "=&r"(v)
		: "RKs12"(*(const uint8_t*)p), "m"(*((const uint8_t*)p + 1)));
	return v;
}

static __forceinline uint32_t AV_RB24(const void *p)
{
	uint32_t v;
	__asm("ld.ub    %0,   %3  \n\t"
		"ldins.b  %0:l, %2  \n\t"
		"ldins.b  %0:u, %1  \n\t"
		: "=&r"(v)
		: "RKs12"(*(const uint8_t*)p),
		"RKs12"(*((const uint8_t*)p + 1)),
		"m"    (*((const uint8_t*)p + 2)));
	return v;
}

static __forceinline uint32_t AV_RL24(const void *p)
{
	uint32_t v;
	__asm("ld.ub    %0,   %1  \n\t"
		"ldins.b  %0:l, %2  \n\t"
		"ldins.b  %0:u, %3  \n\t"
		: "=&r"(v)
		: "m"    (*(const uint8_t*)p),
		"RKs12"(*((const uint8_t*)p + 1)),
		"RKs12"(*((const uint8_t*)p + 2)));
	return v;
}

static __forceinline uint32_t AV_RB32(const void *p)
{
	uint32_t v;
	__asm("ld.ub    %0,   %4  \n\t"
		"ldins.b  %0:l, %3  \n\t"
		"ldins.b  %0:u, %2  \n\t"
		"ldins.b  %0:t, %1  \n\t"
		: "=&r"(v)
		: "RKs12"(*(const uint8_t*)p),
		"RKs12"(*((const uint8_t*)p + 1)),
		"RKs12"(*((const uint8_t*)p + 2)),
		"m"    (*((const uint8_t*)p + 3)));
	return v;
}

static __forceinline uint32_t AV_RL32(const void *p)
{
	uint32_t v;
	__asm("ld.ub    %0,   %1  \n\t"
		"ldins.b  %0:l, %2  \n\t"
		"ldins.b  %0:u, %3  \n\t"
		"ldins.b  %0:t, %4  \n\t"
		: "=&r"(v)
		: "m"    (*(const uint8_t*)p),
		"RKs12"(*((const uint8_t*)p + 1)),
		"RKs12"(*((const uint8_t*)p + 2)),
		"RKs12"(*((const uint8_t*)p + 3)));
	return v;
}

static __forceinline uint64_t AV_RB64(const void *p)
{
	union { uint64_t v; uint32_t hl[2]; } v;
	v.hl[0] = AV_RB32(p);
	v.hl[1] = AV_RB32((const uint32_t*)p + 1);
	return v.v;
}

static __forceinline uint64_t AV_RL64(const void *p)
{
	union { uint64_t v; uint32_t hl[2]; } v;
	v.hl[1] = AV_RL32(p);
	v.hl[0] = AV_RL32((const uint32_t*)p + 1);
	return v.v;
}
*/