#ifndef _DK_MPEG2TS_MUXER_H_
#define _DK_MPEG2TS_MUXER_H_

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

class mpeg2ts_demuxer;
class EXP_DLL dk_mpeg2ts_demuxer
{
public:
	static int const TS_PACKET_LENGTH;

	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED
	} ERR_CODE;

	/*typedef struct EXP_DLL _configuration_t
	{
		typedef struct EXP_DLL _video_configuration_t
		{
			int32_t stream_index;
			int32_t width;
			int32_t height;
			int32_t bitrate;
			int32_t fps;
			int32_t extradata_size;
			uint8_t extradata[500];
			_video_configuration_t(void);
			_video_configuration_t(const _video_configuration_t & clone);
			_video_configuration_t operator=(const _video_configuration_t & clone);
		} video_configuration_t;

		video_configuration_t vconfig;

		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;*/


	dk_mpeg2ts_demuxer(void);
	~dk_mpeg2ts_demuxer(void);

	dk_mpeg2ts_demuxer::ERR_CODE initialize(void);
	dk_mpeg2ts_demuxer::ERR_CODE release(void);

	dk_mpeg2ts_demuxer::ERR_CODE demultiplexing(uint8_t * buffer, size_t nb);
private:
	mpeg2ts_demuxer * _core;

};







#endif