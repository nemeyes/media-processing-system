#pragma once
#include <cstdint>

#if defined(WIN32)
#include <windows.h>
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#else
#define EXP_CLASS
#endif

class eap_video_buffer;
class EXP_CLASS eap_media_buffering
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED
	} ERR_CODE;

	static eap_media_buffering & instance(void);

	eap_media_buffering::ERR_CODE push_video(uint8_t * es, size_t size);
	eap_media_buffering::ERR_CODE pop_video(uint8_t * es, size_t & size);
	eap_media_buffering::ERR_CODE get_video_resolution(int32_t & width, int32_t & height);

	eap_media_buffering::ERR_CODE set_vps(uint8_t * vps, size_t size);
	eap_media_buffering::ERR_CODE set_sps(uint8_t * sps, size_t size);
	eap_media_buffering::ERR_CODE set_pps(uint8_t * pps, size_t size);
	eap_media_buffering::ERR_CODE get_vps(uint8_t * vps, size_t & size);
	eap_media_buffering::ERR_CODE get_sps(uint8_t * sps, size_t & size);
	eap_media_buffering::ERR_CODE get_pps(uint8_t * pps, size_t & size);

private:
	eap_media_buffering(void);
	~eap_media_buffering(void);
	eap_media_buffering(const eap_media_buffering & clone);

private:
	eap_video_buffer * _vbuffer;
	int32_t _width;
	int32_t _height;
};