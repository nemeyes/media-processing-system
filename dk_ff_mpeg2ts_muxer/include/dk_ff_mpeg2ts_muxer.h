#ifndef _DK_FF_MPEG2TS_MUXER_H_
#define _DK_FF_MPEG2TS_MUXER_H_

#include <cstdint>

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LIB)
#  define EXP_DLL __declspec(dllexport)
# else
#  define EXP_DLL __declspec(dllimport)
# endif
#else
# define EXP_DLL
#endif

class ff_mpeg2ts_muxer_core;
class EXP_DLL dk_ff_mpeg2ts_muxer
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED
	} ERR_CODE;

	typedef enum _CODEC_TYPE
	{
		CODEC_TYPE_H264
	} CODEC_TYPE;

	typedef struct EXP_DLL _configuration_t
	{
		int stream_index;
		int width;
		int height;
		int bitrate;
		int fps;
		unsigned char extradata[500];
		int extradata_size;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;


	dk_ff_mpeg2ts_muxer(void);
	~dk_ff_mpeg2ts_muxer(void);

	dk_ff_mpeg2ts_muxer::ERR_CODE initialize(configuration_t * config);
	dk_ff_mpeg2ts_muxer::ERR_CODE release(void);

	dk_ff_mpeg2ts_muxer::ERR_CODE put_video_stream(uint8_t * buffer, size_t nb, int64_t pts, bool keyframe);

private:
	ff_mpeg2ts_muxer_core * _core;

};







#endif