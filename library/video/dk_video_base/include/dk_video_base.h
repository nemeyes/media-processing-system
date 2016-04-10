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
		long long timestamp;
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

	typedef struct EXP_CLASS  _codec_type_t
	{
		static const int32_t  unknown_codec_type = -1;
		static const int32_t  codec_h264 = 0;
		static const int32_t  codec_hevc = 1;
	} codec_type_t;

	typedef struct EXP_CLASS _submedia_type_t
	{
		static const int32_t unknown_submedia_type = -1;
		static const int32_t submedia_type_avc = 0;
		static const int32_t submedia_type_h264 = 1;
		static const int32_t submedia_type_h264_bp = 2;
		static const int32_t submedia_type_h264_hp = 3;
		static const int32_t submedia_type_h264_mp = 4;
		static const int32_t submedia_type_h264_ep = 5;
		static const int32_t submedia_type_mp4v = 6;
		static const int32_t submedia_type_mp4v_sp = 7;
		static const int32_t submedia_type_mp4v_asp = 8;
		static const int32_t submedia_type_jpeg = 9;
		static const int32_t submedia_type_hevc = 10;
		static const int32_t submedia_type_hevc_mp = 11;
		static const int32_t submedia_type_vc1 = 12;
		static const int32_t submedia_type_mvc = 13;
		static const int32_t submedia_type_vp8 = 14;
		static const int32_t submedia_type_rgb32 = 15;
		static const int32_t submedia_type_rgb24 = 16;
		static const int32_t submedia_type_yuy2 = 17;
		static const int32_t submedia_type_i420 = 18;
		static const int32_t submedia_type_yv12 = 19;
		static const int32_t submedia_type_nv12 = 20;
	} submedia_type_t;

	typedef struct EXP_CLASS _entropy_coding_mode_t
	{
		static const int32_t unknown_entropy_coding_mode = -1;
		static const int32_t entropy_coding_mode_cabac = 0;
		static const int32_t entropy_coding_mode_cavlc = 1;
	} entropy_coding_mode_t;

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


	/*
	enum nal_unit_type_e
	{
		NAL_UNKNOWN     = 0,
		NAL_SLICE       = 1,
		NAL_SLICE_DPA   = 2,
		NAL_SLICE_DPB   = 3,
		NAL_SLICE_DPC   = 4,
		NAL_SLICE_IDR   = 5,    // ref_idc != 0
		NAL_SEI = 6,    // ref_idc == 0
		NAL_SPS = 7,
		NAL_PPS = 8,
		NAL_AUD = 9,
		NAL_FILLER = 12,
		// ref_idc == 0 for 6,9,10,11,12
	};
	*/




	typedef struct _dk_video_entity_t
	{
		long long	timestamp;
		memory_type	mem_type;
		void *		surface;
		uint8_t *	data;
		size_t		data_size;
		size_t		data_capacity;
		pic_type	pic_type;
		int32_t		width;
		int32_t		height;
		bool		gen_spspps;
		bool		gen_idr;
		bool		gen_intra;
		bool		flush;
		_dk_video_entity_t(void)
			: timestamp(0)
			, mem_type(dk_video_base::memory_type_host)
			, surface(nullptr)
			, data(nullptr)
			, data_size(0)
			, data_capacity(0)
			, pic_type(dk_video_base::picture_type_none)
			, width(0)
			, height(0)
			, gen_spspps(false)
			, gen_idr(false)
			, gen_intra(false)
			, flush(false)
		{
		}

		_dk_video_entity_t(const _dk_video_entity_t & clone)
		{
			timestamp = clone.timestamp;
			mem_type = clone.mem_type;
			surface = clone.surface;
			data = clone.data;
			data_size = clone.data_size;
			data_capacity = clone.data_capacity;
			pic_type = clone.pic_type;
			width = clone.width;
			height = clone.height;
			gen_spspps = clone.gen_spspps;
			gen_idr = clone.gen_idr;
			gen_intra = clone.gen_intra;
			flush = clone.flush;
		}

		_dk_video_entity_t operator=(const _dk_video_entity_t & clone)
		{
			timestamp = clone.timestamp;
			mem_type = clone.mem_type;
			surface = clone.surface;
			data = clone.data;
			data_size = clone.data_size;
			data_capacity = clone.data_capacity;
			pic_type = clone.pic_type;
			width = clone.width;
			height = clone.height;
			gen_spspps = clone.gen_spspps;
			gen_idr = clone.gen_idr;
			gen_intra = clone.gen_intra;
			flush = clone.flush;
			return (*this);
		}

		~_dk_video_entity_t(void)
		{

		}
	} dk_video_entity_t;

	dk_video_base(bool use_builtin_queue = true);
	virtual ~dk_video_base(void);

	dk_video_base::err_code push(uint8_t * bs, size_t size, long long timestamp);
	dk_video_base::err_code pop(uint8_t * bs, size_t & size, long long & timestamp);
	dk_video_base::err_code init(vbuffer_t * buffer);

	void set_extradata(uint8_t * extradata, size_t extradata_size);
	void set_vps(uint8_t * vps, size_t vps_size);
	void set_sps(uint8_t * sps, size_t sps_size);
	void set_pps(uint8_t * pps, size_t pps_size);

	uint8_t * get_extradata(size_t & extradata_size);
	uint8_t * get_vps(size_t & vps_size);
	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);

	static const int next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end);

private:
	bool _use_builtin_queue;
	vbuffer_t * _root;
	dk_circular_buffer_t * _vqueue;

#if defined(WIN32)
	CRITICAL_SECTION _mutex;
#else
	pthread_mutex _mutex;
#endif

private:
	uint8_t _extradata[260];
	uint8_t _vps[260];
	uint8_t _sps[260];
	uint8_t _pps[260];
	size_t _extradata_size;
	size_t _vps_size;
	size_t _sps_size;
	size_t _pps_size;
};

class EXP_CLASS dk_video_decoder : public dk_video_base
{
public:
	typedef struct EXP_CLASS _configuration_t
	{
		dk_video_decoder::memory_type mem_type;
		void * d3d_device;
		int32_t iwidth;
		int32_t iheight;
		int32_t istride;
		int32_t owidth;
		int32_t oheight;
		int32_t ostride;
		int32_t sarwidth;
		int32_t sarheight;
		int32_t codec;
		int32_t cs;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t & operator=(const _configuration_t & clone);
	} configuration_t;

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

	typedef struct EXP_CLASS _configuration_t
	{
		dk_video_encoder::memory_type mem_type;
		void * d3d_device;
		int32_t cs;
		int32_t width;
		int32_t height;
		int32_t codec;
		int32_t bitrate;
		int32_t fps;
		int32_t keyframe_interval;
		int32_t numb;
		int32_t entropy_coding_mode;
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

	virtual uint8_t * spspps(uint32_t & spspps_size);

protected:
	uint8_t _spspps[260];
	uint32_t _spspps_size;

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