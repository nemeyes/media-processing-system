#ifndef _DK_RTSP_BASE_H_
#define _DK_RTSP_BASE_H_

#include <cstdint>
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

class EXP_CLASS dk_rtsp_base
{
public:
	typedef enum _media_type_t
	{
		unknown_media_type = -1,
		media_type_video = 0,
		media_type_audio
	} media_type_t;

	typedef enum _vsubmedia_type_t
	{
		unknown_video_type = -1,
		vsubmedia_type_jpeg = 0,
		vsubmedia_type_mpeg4,
		vsubmedia_type_h264,
		vsubmedia_type_hevc,
	} vsubmedia_type_t;

	typedef enum _asubmedia_type_t
	{
		unknown_audio_type = -1,
		asubmedia_type_mp3 = 0,
		asubmedia_type_aac = 1,
		asubmedia_type_celt,
	} asubmedia_type_t;

	typedef enum _error_code
	{
		error_code_success = 0,
		error_code_fail
	} error_code;

	typedef enum _recv_option_t
	{
		recv_audio_video = 0,
		recv_video,
		recv_audio
	} recv_option_t;

	typedef enum _transport_option_t
	{
		rtp_over_udp = 0,
		rtp_over_tcp,
		rtp_over_http
	} transport_option_t;

	typedef enum _focus_option_t
	{
		focus_on_nothing = 0,
		focus_on_video,
		focus_on_audio
	} focus_option_t;

	dk_rtsp_base(void);
	virtual ~dk_rtsp_base(void);
	
	static bool is_vps(dk_rtsp_base::vsubmedia_type_t smt, uint8_t nal_unit_type);
	static bool is_sps(dk_rtsp_base::vsubmedia_type_t smt, uint8_t nal_unit_type);
	static bool is_pps(dk_rtsp_base::vsubmedia_type_t smt, uint8_t nal_unit_type);
	static bool is_idr(dk_rtsp_base::vsubmedia_type_t smt, uint8_t nal_unit_type);
	static bool is_vlc(dk_rtsp_base::vsubmedia_type_t smt, uint8_t nal_unit_type);
	static const int find_nal_unit(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end);
	static const uint8_t * find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state);
};


#endif