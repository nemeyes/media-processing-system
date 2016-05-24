#ifndef _DK_VIDEO_BASE_H_
#define _DK_VIDEO_BASE_H_

#include <dk_basic_type.h>

#if defined(WIN32)
#include <windows.h>
#include <atlbase.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <d3d11.h>
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


namespace debuggerking
{
	typedef struct _circular_buffer_t circular_buffer_t;
	class EXP_CLASS video_base : public foundation
	{
	public:
		typedef struct _buffer_t
		{
			long long timestamp;
			size_t amount;
			_buffer_t * prev;
			_buffer_t * next;
		} buffer_t;

		typedef struct _mode_t
		{
			static const int32_t none = 0;
			static const int32_t sync = 1;
			static const int32_t async = 2;
		} mode_t;

		typedef struct EXP_CLASS _configuration_t
		{
			int32_t mode;
			size_t	buffer_size;
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t & operator=(const _configuration_t & clone);
		} configuration_t;

		typedef struct _entity_t
		{
			long long	timestamp;
			int32_t		mem_type;
			void *		surface;
			uint8_t *	data;
			size_t		data_size;
			size_t		data_capacity;
			int32_t		pic_type;
			int32_t		width;
			int32_t		height;
			bool		gen_spspps;
			bool		gen_idr;
			bool		gen_intra;
			bool		flush;
			_entity_t(void)
				: timestamp(0)
				, mem_type(video_base::video_memory_type_t::host)
				, surface(nullptr)
				, data(nullptr)
				, data_size(0)
				, data_capacity(0)
				, pic_type(video_base::video_picture_type_t::unknown)
				, width(0)
				, height(0)
				, gen_spspps(false)
				, gen_idr(false)
				, gen_intra(false)
				, flush(false)
			{
			}
			_entity_t(const _entity_t & clone)
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
			_entity_t operator=(const _entity_t & clone)
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
			~_entity_t(void)
			{
			}
		} entity_t;

		video_base(void);
		virtual ~video_base(void);

		virtual int32_t initialize(configuration_t * config);
		virtual int32_t release(void);

		int32_t push(uint8_t * bs, size_t size, long long timestamp);
		int32_t pop(uint8_t * bs, size_t & size, long long & timestamp);
		int32_t init(buffer_t * buffer);

		void set_extradata(uint8_t * extradata, size_t extradata_size);
		void set_vps(uint8_t * vps, size_t vps_size);
		void set_sps(uint8_t * sps, size_t sps_size);
		void set_pps(uint8_t * pps, size_t pps_size);
		uint8_t * get_extradata(size_t & extradata_size);
		uint8_t * get_vps(size_t & vps_size);
		uint8_t * get_sps(size_t & sps_size);
		uint8_t * get_pps(size_t & pps_size);

#if 0
		int32_t initialize_d3d11(ID3D11Device * d3d11_device, int32_t iwidth, int32_t iheight, int32_t ifps, int32_t owidth, int32_t oheight, int32_t ofps);
		int32_t release_d3d11(void);

		int32_t convert_d3d11_rgb32_to_nv12(ID3D11Texture2D * rgb32, ID3D11Texture2D * nv12, int32_t iwidth, int32_t iheight, int32_t owidth, int32_t oheight);
#endif
		int32_t convert_yv12pitch_to_nv12(uint8_t * src_y, uint8_t * src_u, uint8_t * src_v, uint8_t * dst_y, uint8_t * dst_u, int32_t width, int32_t height, uint32_t src_stride, uint32_t dst_stride);
		int32_t convert_yv12pitch_to_yv12(uint8_t * src_y, uint8_t * src_u, uint8_t * src_v, uint8_t * dst_y, uint8_t * dst_u, int32_t width, int32_t height, uint32_t src_stride, uint32_t dst_stride);

		static const int32_t next_nalu(uint8_t * bitstream, size_t size, int32_t * nal_start, int32_t * nal_end);

	private:
		configuration_t * _config;
		buffer_t * _root;
		circular_buffer_t * _queue;

	#if defined(WIN32)
		CRITICAL_SECTION _mutex;
	#else
		pthread_mutex _mutex;
	#endif

	protected:
		uint8_t _extradata[260];
		uint8_t _vps[260];
		uint8_t _sps[260];
		uint8_t _pps[260];
		size_t _extradata_size;
		size_t _vps_size;
		size_t _sps_size;
		size_t _pps_size;

#if 0
		ATL::CComPtr<ID3D11VideoDevice> _d3d11_video_device;
		ATL::CComPtr<ID3D11DeviceContext> _d3d11_device_context;
		ATL::CComPtr<ID3D11VideoProcessorEnumerator> _d3d11_video_processor_enum;
		ATL::CComPtr<ID3D11VideoProcessor> _d3d11_video_processor;
#endif
	};

	class EXP_CLASS video_decoder : public video_base
	{
	public:
		typedef struct EXP_CLASS _configuration_t : public video_base::configuration_t
		{
			int32_t mem_type;
			void *	device;
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

		video_decoder(void);
		virtual ~video_decoder(void);

		virtual int32_t initialize_decoder(void * config);
		virtual int32_t release_decoder(void);

		virtual int32_t decode(video_decoder::entity_t * bitstream, video_decoder::entity_t * decoded);
		virtual int32_t decode(video_decoder::entity_t * bitstream);
		virtual int32_t get_queued_data(video_decoder::entity_t * decoded);
		virtual void after_decoding_callback(uint8_t * decoded, size_t size) = 0;
	};

	class EXP_CLASS video_encoder : public video_base
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

		typedef struct EXP_CLASS _configuration_t : public video_base::configuration_t
		{
			int32_t mem_type;
			void *	device;
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

		video_encoder(void);
		virtual ~video_encoder(void);

		virtual video_encoder::encoder_state state(void);

		virtual int32_t initialize_encoder(void * config);
		virtual int32_t release_encoder(void);

		virtual int32_t encode(video_encoder::entity_t * input, video_encoder::entity_t * bitstream);
		virtual int32_t encode(video_encoder::entity_t * input);
		virtual int32_t get_queued_data(video_encoder::entity_t * input);
		virtual void after_encoding_callback(uint8_t * bistream, size_t size) = 0;
	};

	class EXP_CLASS video_renderer : public video_base
	{
	public:
		typedef struct EXP_CLASS _configuration_t : public video_base::configuration_t
		{
			int32_t width;
			int32_t height;
			HWND hwnd_full;
			HWND hwnd;
			bool stretch;
			bool full_window;
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t & operator=(const _configuration_t & clone);
		} configuration_t;

		video_renderer(void);
		virtual ~video_renderer(void);

		virtual int32_t initialize_renderer(void * config);
		virtual int32_t release_renderer(void);
		virtual int32_t render(video_renderer::entity_t * decoded);
	};
};







#endif