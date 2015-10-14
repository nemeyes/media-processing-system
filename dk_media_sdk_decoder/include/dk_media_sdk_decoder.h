#ifndef _DK_MEDIA_SDK_DECODER_H_
#define _DK_MEDIA_SDK_DECODER_H_

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

class media_sdk_decoder_core;
class EXP_DLL dk_media_sdk_decoder
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED,
		ERR_CODE_UNSUPPORTED_FUNCTION
	} ERR_CODE;

	typedef enum _CODEC_TYPE
	{
		CODEC_TYPE_H264,
		CODEC_TYPE_HEVC,
		CODEC_TYPE_MPEG4,
		CODEC_TYPE_MJPEG
	} CODEC_TYPE;

	dk_media_sdk_decoder(void);
	~dk_media_sdk_decoder(void);

	dk_media_sdk_decoder::ERR_CODE initialize(unsigned int width, unsigned int height);
	dk_media_sdk_decoder::ERR_CODE release(void);
	dk_media_sdk_decoder::ERR_CODE decode(unsigned char * input, size_t isize, unsigned int stride, unsigned char * output, size_t & osize);


private:
	media_sdk_decoder_core * _core;

};
















#endif