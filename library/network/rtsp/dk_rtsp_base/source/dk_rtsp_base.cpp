#include "dk_rtsp_base.h"
#include <memory>

#define AV_RB32(x)  \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) | \
               (((const uint8_t*)(x))[1] << 16) | \
               (((const uint8_t*)(x))[2] <<  8) | \
                ((const uint8_t*)(x))[3])

#define MIN(a,b) ((a) > (b) ? (b) : (a))


dk_rtsp_base::dk_rtsp_base(void)
{
}

dk_rtsp_base::~dk_rtsp_base(void)
{

}

bool dk_rtsp_base::is_vps(dk_rtsp_base::vsubmedia_type smt, uint8_t nal_unit_type)
{
	// VPS NAL units occur in H.265 only:
	return smt == dk_rtsp_base::vsubmedia_type_hevc && nal_unit_type == 32;
}

bool dk_rtsp_base::is_sps(dk_rtsp_base::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == dk_rtsp_base::vsubmedia_type_h264 ? nal_unit_type == 7 : nal_unit_type == 33;
}

bool dk_rtsp_base::is_pps(dk_rtsp_base::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == dk_rtsp_base::vsubmedia_type_h264 ? nal_unit_type == 8 : nal_unit_type == 34;
}

bool dk_rtsp_base::is_idr(dk_rtsp_base::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == dk_rtsp_base::vsubmedia_type_h264 ? nal_unit_type == 5 : nal_unit_type == 34;
}

bool dk_rtsp_base::is_vlc(dk_rtsp_base::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == dk_rtsp_base::vsubmedia_type_h264 ? (nal_unit_type <= 5 && nal_unit_type > 0) : (nal_unit_type <= 31);
}

const int dk_rtsp_base::find_nal_unit(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end)
{
	uint32_t i;
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


const uint8_t * dk_rtsp_base::find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state)
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