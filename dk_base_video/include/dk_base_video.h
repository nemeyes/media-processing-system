#ifndef _DK_BASE_VIDEO_H_
#define _DK_BASE_VIDEO_H_

#include <cstdint>

#if defined(WIN32)
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#else
#define EXP_CLASS
#endif

typedef struct _DK_VIDEO_ENTITY_T
{
	uint8_t * bitstream;
	size_t bitstream_size;
} DK_VIDEO_ENTITY_T;

class EXP_CLASS dk_base_video
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED
	} ERR_CODE;

	typedef enum _SUBMEDIA_TYPE
	{
		SUBMEDIA_TYPE_UNKNOWN = -1,
		SUBMEDIA_TYPE_H264,
		SUBMEDIA_TYPE_H264_BP,
		SUBMEDIA_TYPE_H264_HP,
		SUBMEDIA_TYPE_H264_MP,
		SUBMEDIA_TYPE_H264_EP,
		SUBMEDIA_TYPE_MP4V,
		SUBMEDIA_TYPE_MP4V_SP,
		SUBMEDIA_TYPE_MP4V_ASP,
		SUBMEDIA_TYPE_MJPEG,
		SUBMEDIA_TYPE_RGB32,
		SUBMEDIA_TYPE_YUY2,
		SUBMEDIA_TYPE_IYUV,
		SUBMEDIA_TYPE_YV12,
		SUBMEDIA_TYPE_NV12,
	} SUBMEDIA_TYPE;

	dk_base_video(void);
	virtual ~dk_base_video(void);

	ERR_CODE put_video(DK_VIDEO_ENTITY_T * bitstream);
	ERR_CODE get_video(DK_VIDEO_ENTITY_T * bitstream);
};

class EXP_CLASS dk_base_video_decoder : public dk_base_video
{
public:
	typedef struct EXP_CLASS _CONFIGURATION_T
	{
		int32_t width;
		int32_t height;
		SUBMEDIA_TYPE ist;
		SUBMEDIA_TYPE ost;
		_CONFIGURATION_T(void)
			: width(0)
			, height(0)
			, ist(SUBMEDIA_TYPE_UNKNOWN)
			, ost(SUBMEDIA_TYPE_UNKNOWN)
		{}

		_CONFIGURATION_T(const _CONFIGURATION_T & clone)
		{
			width = clone.width;
			height = clone.height;
			ist = clone.ist;
			ost = clone.ost;
		}

		_CONFIGURATION_T & operator=(const _CONFIGURATION_T & clone)
		{
			width = clone.width;
			height = clone.height;
			ist = clone.ist;
			ost = clone.ost;
			return (*this);
		}

	} CONFIGURATION_T;

	dk_base_video_decoder(void);
	virtual ~dk_base_video_decoder(void);

	virtual ERR_CODE initialize_decoder(CONFIGURATION_T * config);
	virtual ERR_CODE release_decoder(void);

	virtual ERR_CODE decode(DK_VIDEO_ENTITY_T * bitstream) = 0;

protected:
	CONFIGURATION_T _config;
};

class EXP_CLASS dk_base_video_encoder : public dk_base_video
{
public:
	dk_base_video_encoder(void);
	virtual ~dk_base_video_encoder(void);

	virtual ERR_CODE initialize_encoder(void);
	virtual ERR_CODE release_encoder(void);

	virtual ERR_CODE encode(DK_VIDEO_ENTITY_T * bitstream) = 0;
};








#endif