#include "stream_parser.h"
#include "dk_rtsp_client.h"

#define AV_RB32(x)  \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) | \
               (((const uint8_t*)(x))[1] << 16) | \
               (((const uint8_t*)(x))[2] <<  8) | \
                ((const uint8_t*)(x))[3])

#define MIN(a,b) ((a) > (b) ? (b) : (a))

bool stream_parser::is_vps(dk_rtsp_client::SUBMEDIA_TYPE_T smt, unsigned char nal_unit_type)
{
	// VPS NAL units occur in H.265 only:
	return smt == dk_rtsp_client::SUBMEDIA_TYPE_H265 && nal_unit_type == 32;
}

bool stream_parser::is_sps(dk_rtsp_client::SUBMEDIA_TYPE_T smt, unsigned char nal_unit_type)
{
	return smt == dk_rtsp_client::SUBMEDIA_TYPE_H264 ? nal_unit_type == 7 : nal_unit_type == 33;
}

bool stream_parser::is_pps(dk_rtsp_client::SUBMEDIA_TYPE_T smt, unsigned char nal_unit_type)
{
	return smt == dk_rtsp_client::SUBMEDIA_TYPE_H264 ? nal_unit_type == 8 : nal_unit_type == 34;
}

bool stream_parser::is_vlc(dk_rtsp_client::SUBMEDIA_TYPE_T smt, unsigned char nal_unit_type)
{
	return smt == dk_rtsp_client::SUBMEDIA_TYPE_H264 ? (nal_unit_type <= 5 && nal_unit_type > 0) : (nal_unit_type <= 31);
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