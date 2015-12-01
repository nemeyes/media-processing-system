#ifndef _DK_AVC_RAW_STREAM_READER_H_
#define _DK_AVC_RAW_STREAM_READER_H_

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

class avc_raw_stream_reader;
class EXP_DLL dk_avc_raw_stream_reader
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED,
		ERR_CODE_UNSUPPORTED_FUNCTION
	} ERR_CODE;

	typedef struct EXP_DLL _configuration_t
	{
		_configuration_t(void)
		{
		}

		_configuration_t(const _configuration_t & clone)
		{
		}

		_configuration_t operator=(const _configuration_t & clone)
		{
			return (*this);
		}
	} configuration_t;

	dk_avc_raw_stream_reader(void);
	~dk_avc_raw_stream_reader(void);

	dk_avc_raw_stream_reader::ERR_CODE initialize(dk_avc_raw_stream_reader::configuration_t * conf);
	dk_avc_raw_stream_reader::ERR_CODE release(void);
	dk_avc_raw_stream_reader::ERR_CODE read(dk_video_entity * bitstream);

private:
	avc_raw_stream_reader * _core;


};








#endif