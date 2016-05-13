#ifndef _STREAM_PARSER_H_
#define _STREAM_PARSER_H_

#include <cstdlib>
#include <cstdint>
#include "dk_rtmp_client.h"

namespace debuggerking
{
	class stream_parser
	{
	public:
		static bool is_sps(int32_t smt, uint8_t nal_unit_type);
		static bool is_pps(int32_t smt, uint8_t nal_unit_type);
		static bool is_idr(int32_t smt, uint8_t nal_unit_type);
		static bool is_vlc(int32_t smt, uint8_t nal_unit_type);
		static const int32_t find_nal_unit(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end);

		static const uint8_t * find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state);


	private:
		stream_parser(void);
		stream_parser(const stream_parser & clone);

	};
};

#endif