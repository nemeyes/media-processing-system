#ifndef _DK_FF_MPEG2TS_MUXER_H_
#define _DK_FF_MPEG2TS_MUXER_H_

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

	typedef struct EXP_DLL _configuration_t
	{
		int stream_index;
		int width;
		int height;
		int bitrate;
		int fps;
		unsigned char extra_data[500];
		int extra_data_size;
		_configuration_t(void)
			: stream_index(0)
			, width(0)
			, height(0)
			, bitrate(0)
			, fps(30)
			, extra_data_size(0)
		{
			memset(extra_data, 0x00, sizeof(extra_data));
		}

		_configuration_t(const _configuration_t & clone)
		{
			stream_index = clone.stream_index;
			width = clone.width;
			height = clone.height;
			bitrate = clone.bitrate;
			fps = clone.fps;
			bitrate = clone.bitrate;
			extra_data_size = clone.extra_data_size;
			memset(extra_data, 0x00, sizeof(extra_data));
		}

		_configuration_t operator=(const _configuration_t & clone)
		{
			stream_index = clone.stream_index;
			width = clone.width;
			height = clone.height;
			bitrate = clone.bitrate;
			fps = clone.fps;
			bitrate = clone.bitrate;
			extra_data_size = clone.extra_data_size;
			memset(extra_data, 0x00, sizeof(extra_data));
			return (*this);
		}
	} configuration_t;


	dk_ff_mpeg2ts_muxer(void);
	~dk_ff_mpeg2ts_muxer(void);

	dk_ff_mpeg2ts_muxer::ERR_CODE initialize(configuration_t & config);
	dk_ff_mpeg2ts_muxer::ERR_CODE release(void);

	dk_ff_mpeg2ts_muxer::ERR_CODE put_video_stream(unsigned char * buffer, size_t nb, long long pts, bool keyframe);

private:
	ff_mpeg2ts_muxer_core * _core;

};







#endif