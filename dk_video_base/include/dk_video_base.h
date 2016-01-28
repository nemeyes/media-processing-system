#ifndef _DK_VIDEO_BASE_H_
#define _DK_VIDEO_BASE_H_

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
class EXP_CLASS dk_video_base
{
public:
	typedef struct _vbuffer_t
	{
		size_t amount;
		long long pts;
		_vbuffer_t * prev;
		_vbuffer_t * next;
	} vbuffer_t;

	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAIL,
		ERR_CODE_NOT_IMPLEMENTED,
		ERR_CODE_UNSUPPORTED_FUNCTION,
		ERR_CODE_INVALID_ENCODING_DEVICE,
		ERR_CODE_ENCODING_UNDER_PROCESSING,
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
		SUBMEDIA_TYPE_HEVC,
		SUBMEDIA_TYPE_RGB32,
		SUBMEDIA_TYPE_RGB24,
		SUBMEDIA_TYPE_YUY2,
		SUBMEDIA_TYPE_I420,
		SUBMEDIA_TYPE_YV12,
		SUBMEDIA_TYPE_NV12,
	} SUBMEDIA_TYPE;

	typedef enum _MEMORY_TYPE
	{
		MEMORY_TYPE_HOST = 0,
		MEMORY_TYPE_DX9,
		MEMORY_TYPE_DX9EX,
		MEMORY_TYPE_DX10,
		MEMORY_TYPE_DX10_1,
		MEMORY_TYPE_DX11_1,
		MEMORY_TYPE_DX11_2,
		MEMORY_TYPE_DX11_3,
		MEMORY_TYPE_DX12,
		MEMORY_TYPE_OPENGL,
		MEMORY_TYPE_OPENCL,
		MEMORY_TYPE_CUDA,
	} MEMORY_TYPE;

	typedef enum _PIC_TYPE
	{
		PICTURE_TYPE_NONE = 0,
		PICTURE_TYPE_IDR,
		PICTURE_TYPE_I,
		PICTURE_TYPE_P,
		PICTURE_TYPE_B
	} PIC_TYPE;

	typedef struct _dk_video_entity_t
	{
		MEMORY_TYPE	mem_type;
		void *		surface;
		uint8_t *	data;
		size_t		data_size;
		size_t		data_capacity;
		PIC_TYPE	pic_type;
		bool		gen_spspps;
		bool		gen_idr;
		bool		flush;
		_dk_video_entity_t(void)
			: mem_type(dk_video_base::MEMORY_TYPE_HOST)
			, surface(nullptr)
			, data(nullptr)
			, data_size(0)
			, data_capacity(0)
			, pic_type(dk_video_base::PICTURE_TYPE_NONE)
			, gen_spspps(false)
			, gen_idr(false)
			, flush(false)
		{
		}

		_dk_video_entity_t(const _dk_video_entity_t & clone)
		{
			mem_type = clone.mem_type;
			surface = clone.surface;
			data = clone.data;
			data_size = clone.data_size;
			data_capacity = clone.data_capacity;
			pic_type = clone.pic_type;
			gen_spspps = clone.gen_spspps;
			gen_idr = clone.gen_idr;
			flush = clone.flush;
		}

		_dk_video_entity_t operator=(const _dk_video_entity_t & clone)
		{
			mem_type = clone.mem_type;
			surface = clone.surface;
			data = clone.data;
			data_size = clone.data_size;
			data_capacity = clone.data_capacity;
			pic_type = clone.pic_type;
			gen_spspps = clone.gen_spspps;
			gen_idr = clone.gen_idr;
			flush = clone.flush;
			return (*this);
		}

		~_dk_video_entity_t(void)
		{

		}
	} dk_video_entity_t;

	dk_video_base(bool use_builtin_queue = true);
	virtual ~dk_video_base(void);

	ERR_CODE push(uint8_t * bs, size_t size);
	ERR_CODE pop(uint8_t * bs, size_t & size);
	ERR_CODE init(vbuffer_t * buffer);

private:
	bool _use_builtin_queue;
	vbuffer_t * _root;
	dk_circular_buffer_t * _vqueue;

#if defined(WIN32)
	CRITICAL_SECTION _mutex;
#else
	pthread_mutex _mutex;
#endif
};

class EXP_CLASS dk_video_decoder : public dk_video_base
{
public:
	dk_video_decoder(void);
	virtual ~dk_video_decoder(void);

	virtual dk_video_decoder::ERR_CODE initialize_decoder(void * config);
	virtual dk_video_decoder::ERR_CODE release_decoder(void);

	virtual dk_video_decoder::ERR_CODE decode(dk_video_decoder::dk_video_entity_t * bitstream, dk_video_decoder::dk_video_entity_t * decoded);
	virtual dk_video_decoder::ERR_CODE decode(dk_video_decoder::dk_video_entity_t * bitstream);
	virtual dk_video_decoder::ERR_CODE get_queued_data(dk_video_decoder::dk_video_entity_t * decoded);
};

class EXP_CLASS dk_video_encoder : public dk_video_base
{
public:
	typedef enum _ENCODER_STATE
	{
		ENCODER_STATE_NONE,
		ENCODER_STATE_INITIALIZING,
		ENCODER_STATE_INITIALIZED,
		ENCODER_STATE_ENCODING,
		ENCODER_STATE_ENCODED,
		ENCODER_STATE_RELEASING,
		ENCODER_STATE_RELEASED
	} ENCODER_STATE;

	dk_video_encoder(bool use_builtin_queue = true);
	virtual ~dk_video_encoder(void);

	virtual dk_video_encoder::ENCODER_STATE state(void);

	virtual dk_video_encoder::ERR_CODE initialize_encoder(void * config);
	virtual dk_video_encoder::ERR_CODE release_encoder(void);

	virtual dk_video_encoder::ERR_CODE encode(dk_video_encoder::dk_video_entity_t * input, dk_video_encoder::dk_video_entity_t * bitstream);
	virtual dk_video_encoder::ERR_CODE encode(dk_video_encoder::dk_video_entity_t * input);
	virtual dk_video_encoder::ERR_CODE get_queued_data(dk_video_encoder::dk_video_entity_t * input);

	virtual dk_video_encoder::ERR_CODE encode_async(dk_video_encoder::dk_video_entity_t * input);
	virtual dk_video_encoder::ERR_CODE check_encoding_finish(void);

	virtual void on_acquire_bitstream(uint8_t * bistream, size_t size) = 0;

	static const int next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end);

};

class EXP_CLASS dk_video_renderer : public dk_video_base
{
public:
	dk_video_renderer(void);
	virtual ~dk_video_renderer(void);

	virtual dk_video_renderer::ERR_CODE initialize_renderer(void * config);
	virtual dk_video_renderer::ERR_CODE release_renderer(void);
	virtual dk_video_renderer::ERR_CODE render(dk_video_renderer::dk_video_entity_t * decoded);
};








#endif