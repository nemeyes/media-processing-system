#ifndef _DK_RTSP_BASE_H_
#define _DK_RTSP_BASE_H_

#include <dk_basic_type.h>
#if !defined(WIN32)
#include <pthread.h>
#define EXP_CLASS
#else
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#endif

namespace debuggerking
{
	class EXP_CLASS rtsp_base : public foundation
	{
	public:
		typedef enum _transport_option
		{
			rtp_over_udp = 0,
			rtp_over_tcp,
			rtp_over_http
		} transport_option;

		rtsp_base(void);
		virtual ~rtsp_base(void);
	
		static bool is_vps(int32_t smt, uint8_t nal_unit_type);
		static bool is_sps(int32_t smt, uint8_t nal_unit_type);
		static bool is_pps(int32_t smt, uint8_t nal_unit_type);
		static bool is_idr(int32_t smt, uint8_t nal_unit_type);
		static bool is_vlc(int32_t smt, uint8_t nal_unit_type);
		static const int32_t find_nal_unit(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end);
		static const uint8_t * find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state);
	};
};

#endif