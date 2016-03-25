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
		long long pts;
		size_t amount;
		_vbuffer_t * prev;
		_vbuffer_t * next;
	} vbuffer_t;

	typedef enum _err_code
	{
		err_code_success,
		err_code_fail,
		err_code_not_implemented,
		err_code_unsupported_function,
		err_code_invalid_encoding_device,
		err_code_encoding_under_processing,
	} err_code;

	typedef enum _submedia_type
	{
		submedia_type_unknown = -1,
		submedia_type_avc, //h264 bitstream without start code(avcc)
		submedia_type_h264,//h264 bitstream with start code(annex b)
		submedia_type_h264_bp,
		submedia_type_h264_hp,
		submedia_type_h264_mp,
		submedia_type_h264_ep,
		submedia_type_mp4v,
		submedia_type_mp4v_sp,
		submedia_type_mp4v_asp,
		submedia_type_jpeg,
		submedia_type_hevc,
		submedia_type_hevc_mp,
		submedia_type_vc1,
		submedia_type_mvc,
		submedia_type_vp8,
		submedia_type_rgb32,
		submedia_type_rgb24,
		submedia_type_yuy2,
		submedia_type_i420,
		submedia_type_yv12,
		submedia_type_nv12,
	} submedia_type;

	typedef enum _memory_type
	{
		memory_type_host = 0,
		memory_type_dx9,
		memory_type_dx9ex,
		memory_type_dx10,
		memory_type_dx10_1,
		memory_type_dx11,
		memory_type_dx11_1,
		memory_type_dx11_2,
		memory_type_dx11_3,
		memory_type_dx12,
		memory_type_opengl,
		memory_type_opencl,
		memory_type_cuda,
	} memory_type;

	typedef enum _pic_type
	{
		picture_type_none = 0,
		picture_type_idr,
		picture_type_i,
		picture_type_p,
		picture_type_b
	} pic_type;

	typedef struct _dk_video_entity_t
	{
		long long	pts;
		memory_type	mem_type;
		void *		surface;
		uint8_t *	data;
		size_t		data_size;
		size_t		data_capacity;
		pic_type	pic_type;
		bool		gen_spspps;
		bool		gen_idr;
		bool		flush;
		_dk_video_entity_t(void)
			: mem_type(dk_video_base::memory_type_host)
			, surface(nullptr)
			, data(nullptr)
			, data_size(0)
			, data_capacity(0)
			, pic_type(dk_video_base::picture_type_none)
			, gen_spspps(false)
			, gen_idr(false)
			, flush(false)
			, pts(0)
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
			pts = clone.pts;
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
			pts = clone.pts;
			return (*this);
		}

		~_dk_video_entity_t(void)
		{

		}
	} dk_video_entity_t;

	dk_video_base(bool use_builtin_queue = true);
	virtual ~dk_video_base(void);

	err_code push(uint8_t * bs, size_t size, long long pts);
	err_code pop(uint8_t * bs, size_t & size, long long & pts);
	err_code init(vbuffer_t * buffer);

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

	virtual dk_video_decoder::err_code initialize_decoder(void * config);
	virtual dk_video_decoder::err_code release_decoder(void);

	virtual dk_video_decoder::err_code decode(dk_video_decoder::dk_video_entity_t * bitstream, dk_video_decoder::dk_video_entity_t * decoded);
	virtual dk_video_decoder::err_code decode(dk_video_decoder::dk_video_entity_t * bitstream);
	virtual dk_video_decoder::err_code get_queued_data(dk_video_decoder::dk_video_entity_t * decoded);
};

class EXP_CLASS dk_video_encoder : public dk_video_base
{
public:
	typedef enum _encoder_state
	{
		encoder_state_none,
		encoder_state_initializing,
		encoder_state_initialized,
		encoder_state_encoding,
		encoder_state_encoded,
		encoder_state_releasing,
		encoder_state_released
	} encoder_state;

	typedef EXP_CLASS struct _configuration_t
	{
		dk_video_encoder::memory_type mem_type;
		void * d3d_device;
		dk_video_encoder::submedia_type cs;
		int32_t width;
		int32_t height;
		dk_video_encoder::submedia_type codec;
		int32_t bitrate;
		int32_t fps;
		int32_t keyframe_interval;
		int32_t numb;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t & operator=(const _configuration_t & clone);
	} configuration_t;

	dk_video_encoder(bool use_builtin_queue = true);
	virtual ~dk_video_encoder(void);

	virtual dk_video_encoder::encoder_state state(void);

	virtual dk_video_encoder::err_code initialize_encoder(void * config);
	virtual dk_video_encoder::err_code release_encoder(void);

	virtual dk_video_encoder::err_code encode(dk_video_encoder::dk_video_entity_t * input, dk_video_encoder::dk_video_entity_t * bitstream);
	virtual dk_video_encoder::err_code encode(dk_video_encoder::dk_video_entity_t * input);
	virtual dk_video_encoder::err_code get_queued_data(dk_video_encoder::dk_video_entity_t * input);

	virtual dk_video_encoder::err_code encode_async(dk_video_encoder::dk_video_entity_t * input);
	virtual dk_video_encoder::err_code check_encoding_finish(void);

	virtual void on_acquire_bitstream(uint8_t * bistream, size_t size) = 0;

	static const int next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end);

};

class EXP_CLASS dk_video_renderer : public dk_video_base
{
public:
	dk_video_renderer(void);
	virtual ~dk_video_renderer(void);

	virtual dk_video_renderer::err_code initialize_renderer(void * config);
	virtual dk_video_renderer::err_code release_renderer(void);
	virtual dk_video_renderer::err_code render(dk_video_renderer::dk_video_entity_t * decoded);
};








#endif