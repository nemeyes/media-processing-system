#ifndef _DK_AUDIO_BASE_H_
#define _DK_AUDIO_BASE_H_

#if defined(WIN32)
#include <windows.h>
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#else
#include <pthreads.h>
#define EXP_CLASS
#endif

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <memory>

typedef struct _dk_circular_buffer_t dk_circular_buffer_t;
class EXP_CLASS dk_audio_base
{
public:
	typedef struct _abuffer_t
	{
		size_t amount;
		_abuffer_t * prev;
		_abuffer_t * next;
	} abuffer_t;

	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAIL,
		ERR_CODE_NOT_IMPLEMENTED,
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
		SUBMEDIA_TYPE_RGB24,
		SUBMEDIA_TYPE_YUY2,
		SUBMEDIA_TYPE_I420,
		SUBMEDIA_TYPE_YV12,
		SUBMEDIA_TYPE_NV12,
	} SUBMEDIA_TYPE;

	typedef struct _dk_audio_entity_t
	{
		void * data;
		size_t data_size;
		size_t data_capacity;
	} dk_audio_entity_t;

	dk_audio_base(void);
	virtual ~dk_audio_base(void);

	ERR_CODE push(uint8_t * bs, size_t size);
	ERR_CODE pop(uint8_t * bs, size_t & size);
	ERR_CODE init(abuffer_t * buffer);

private:
	abuffer_t * _root;
	dk_circular_buffer_t * _aqueue;

#if defined(WIN32)
	CRITICAL_SECTION _mutex;
#else
	pthread_mutex _mutex;
#endif
};

class EXP_CLASS dk_audio_decoder : public dk_audio_base
{
public:
	dk_audio_decoder(void);
	virtual ~dk_audio_decoder(void);

	virtual dk_audio_decoder::ERR_CODE initialize_decoder(void * config);
	virtual dk_audio_decoder::ERR_CODE release_decoder(void);

	virtual dk_audio_decoder::ERR_CODE decode(dk_audio_decoder::dk_audio_entity_t * encoded, dk_audio_decoder::dk_audio_entity_t * pcm);
};

class EXP_CLASS dk_audio_encoder : public dk_audio_base
{
public:
	dk_audio_encoder(void);
	virtual ~dk_audio_encoder(void);

	virtual dk_audio_encoder::ERR_CODE initialize_encoder(void * config);
	virtual dk_audio_encoder::ERR_CODE release_encoder(void);

	virtual dk_audio_encoder::ERR_CODE encode(dk_audio_encoder::dk_audio_entity_t * pcm, dk_audio_encoder::dk_audio_entity_t * encoded);
	virtual dk_audio_encoder::ERR_CODE encode(dk_audio_encoder::dk_audio_entity_t * pcm);
	virtual dk_audio_encoder::ERR_CODE get_queued_data(dk_audio_encoder::dk_audio_entity_t * encoded);

	virtual uint8_t * extradata(void);
	virtual size_t extradata_size(void);
};

class EXP_CLASS dk_audio_renderer : public dk_audio_base
{
public:
	dk_audio_renderer(void);
	virtual ~dk_audio_renderer(void);

	virtual dk_audio_renderer::ERR_CODE initialize_renderer(void * config);
	virtual dk_audio_renderer::ERR_CODE release_renderer(void);
	virtual dk_audio_renderer::ERR_CODE render(dk_audio_renderer::dk_audio_entity_t * pcm);
};






#endif