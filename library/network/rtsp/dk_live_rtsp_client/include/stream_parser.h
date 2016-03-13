#ifndef _STREAM_PARSER_H_
#define _STREAM_PARSER_H_

#include <cstdlib>
#include <cstdint>
#include "dk_live_rtsp_client.h"

class stream_parser
{
public:
	static bool is_vps(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type);
	static bool is_sps(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type);
	static bool is_pps(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type);
	static bool is_idr(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type);
	static bool is_vlc(dk_live_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t nal_unit_type);
	static const int find_nal_unit(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end);
	
	static const uint8_t * find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state);
	

private:
	stream_parser(void);
	~stream_parser(void);

};








#endif