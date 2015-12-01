#ifndef _DK_VIDEO_BASE_H_
#define _DK_VIDEO_BASE_H_

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <memory>

#if defined(WIN32)
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#else
#define EXP_CLASS
#endif

typedef struct _dk_video_entity_t
{
	uint8_t *	data;
	size_t		data_size;
	size_t		data_capacity;
} dk_video_entity_t;

class EXP_CLASS dk_video_base
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED,
		ERR_CODE_NOT_IMPLEMENTED,
	} ERR_CODE;

	typedef enum _SUBMEDIA_TYPE
	{
		SUBMEDIA_TYPE_UNKNOWN = -1,
		SUBMEDIA_TYPE_AVC, //H264 bitstream without start code(avcc)
		SUBMEDIA_TYPE_H264,//H264 bitstream with start code(annex B)
		SUBMEDIA_TYPE_H264_BP,
		SUBMEDIA_TYPE_H264_HP,
		SUBMEDIA_TYPE_H264_MP,
		SUBMEDIA_TYPE_H264_EP,
		SUBMEDIA_TYPE_MP4V,
		SUBMEDIA_TYPE_MP4V_SP,
		SUBMEDIA_TYPE_MP4V_ASP,
		SUBMEDIA_TYPE_MJPEG,
		SUBMEDIA_TYPE_RGB32,
		SUBMEDIA_TYPE_RGB24,
		SUBMEDIA_TYPE_YUY2,
		SUBMEDIA_TYPE_I420,
		SUBMEDIA_TYPE_YV12,
		SUBMEDIA_TYPE_NV12,
	} SUBMEDIA_TYPE;

	dk_video_base(void);
	virtual ~dk_video_base(void);

	ERR_CODE put_video(dk_video_entity_t * bitstream);
	ERR_CODE get_video(dk_video_entity_t * bitstream);
};

class EXP_CLASS dk_video_decoder : public dk_video_base
{
public:
	dk_video_decoder(void);
	virtual ~dk_video_decoder(void);

	virtual ERR_CODE initialize_decoder(void * config);
	virtual ERR_CODE release_decoder(void);

	virtual ERR_CODE decode(dk_video_entity_t * bitstream, dk_video_entity_t * decoded);
	virtual ERR_CODE decode(dk_video_entity_t * bitstream);
};

class EXP_CLASS dk_video_encoder : public dk_video_base
{
public:
	dk_video_encoder(void);
	virtual ~dk_video_encoder(void);

	virtual ERR_CODE initialize_encoder(void);
	virtual ERR_CODE release_encoder(void);

	virtual ERR_CODE encode(dk_video_entity_t * bitstream) = 0;
};








#endif