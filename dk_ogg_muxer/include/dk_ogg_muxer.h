#ifndef _DK_OGG_MUXER_H_
#define _DK_OGG_MUXER_H_

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LIB)
#  define EXP_CLASS __declspec(dllexport)
# else
#  define EXP_CLASS __declspec(dllimport)
# endif
#else
# define EXP_CLASS
#endif

class ogg_muxer;
class EXP_CLASS dk_ogg_muxer
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAIL
	} ERR_CODE;

	typedef enum _MEDIA_TYPE_T
	{
		MEDIA_TYPE_VIDEO = 0,
		MEDIA_TYPE_AUDIO
	} MEDIA_TYPE_T;

	typedef enum _VIDEO_SUBMEDIA_TYPE_T
	{
		UNKNOWN_VIDEO_TYPE = -1,
		SUBMEDIA_TYPE_H264 = 0,
	} VIDEO_SUBMEDIA_TYPE_T;

	typedef enum _AUDIO_SUBMEDIA_TYPE_T
	{
		UNKNOWN_AUDIO_TYPE = -1,
		SUBMEDIA_TYPE_OPUS = 0, //platform endian
	} AUDIO_SUBMEDIA_TYPE_T;

	typedef struct _configuration_t
	{


	} configuration_t;

	dk_ogg_muxer(void);
	~dk_ogg_muxer(void);





};














#endif