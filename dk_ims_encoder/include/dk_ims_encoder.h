#ifndef _DK_INTEL_MEDIA_SDK_ENCODER_H_
#define _DK_INTEL_MEDIA_SDK_ENCODER_H_

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

class ims_enc_core;
class EXP_DLL dk_ims_encoder
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

	dk_ims_encoder(void);
	~dk_ims_encoder(void);

public:


private:
	ims_enc_core * _core;

};
















#endif