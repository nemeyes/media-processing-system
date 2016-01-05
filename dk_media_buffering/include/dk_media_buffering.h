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

class dk_video_buffer;
class EXP_CLASS dk_media_buffering
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED
	} ERR_CODE;

	static dk_media_buffering & instance(void);

	dk_media_buffering::ERR_CODE push_video(uint8_t * data, size_t size);
	dk_media_buffering::ERR_CODE pop_video(uint8_t * data, size_t & size);

	dk_media_buffering::ERR_CODE set_vps(uint8_t * vps, size_t size);
	dk_media_buffering::ERR_CODE set_sps(uint8_t * sps, size_t size);
	dk_media_buffering::ERR_CODE set_pps(uint8_t * pps, size_t size);
	dk_media_buffering::ERR_CODE get_vps(uint8_t * vps, size_t & size);
	dk_media_buffering::ERR_CODE get_sps(uint8_t * sps, size_t & size);
	dk_media_buffering::ERR_CODE get_pps(uint8_t * pps, size_t & size);

private:
	dk_media_buffering(void);
	~dk_media_buffering(void);
	dk_media_buffering(const dk_media_buffering & clone);

private:
	dk_video_buffer * _vbuffer;
};