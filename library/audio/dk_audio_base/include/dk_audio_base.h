#ifndef _DK_AUDIO_BASE_H_
#define _DK_AUDIO_BASE_H_

#include <dk_basic_type.h>

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

namespace debuggerking
{
	typedef struct _circular_buffer_t circular_buffer_t;
	class EXP_CLASS audio_base : public foundation
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
			long long timestamp;
			void * data;
			size_t data_size;
			size_t data_capacity;
			_entity_t(void)
				: timestamp(0)
				, data(nullptr)
				, data_size(0)
				, data_capacity(0)
			{}
			_entity_t(const _entity_t & clone)
			{
				timestamp = clone.timestamp;
				data = clone.data;
				data_size = clone.data_size;
				data_capacity = clone.data_capacity;
			}
			_entity_t & operator=(const _entity_t & clone)
			{
				timestamp = clone.timestamp;
				data = clone.data;
				data_size = clone.data_size;
				data_capacity = clone.data_capacity;
				return (*this);
			}
		} entity_t;

		audio_base(void);
		virtual ~audio_base(void);

		virtual int32_t initialize(configuration_t * config);
		virtual int32_t release(void);

		int32_t push(uint8_t * bs, size_t size, long long timestamp);
		int32_t pop(uint8_t * bs, size_t & size, long long & timestamp);
		int32_t init(buffer_t * buffer);

		void set_extradata(uint8_t * extradata, size_t extradata_size);
		uint8_t * get_extradata(size_t & extradata_size);
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
		size_t _extradata_size;
	};

	class EXP_CLASS audio_decoder : public audio_base
	{
	public:
		typedef struct EXP_CLASS _configuration_t : public audio_base::configuration_t
		{
			unsigned long samplerate;
			unsigned char channels;
			unsigned int bitdepth;
			unsigned int bitrate;
			uint8_t extradata[100];
			size_t extradata_size;
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t operator=(const _configuration_t & clone);
		} configuration_t;

		audio_decoder(void);
		virtual ~audio_decoder(void);

		virtual int32_t initialize_decoder(void * config);
		virtual int32_t release_decoder(void);
		virtual int32_t decode(audio_decoder::entity_t * encoded, audio_decoder::entity_t * pcm);
		virtual int32_t decode(audio_decoder::entity_t * encoded);
		virtual int32_t get_queued_data(audio_decoder::entity_t * pcm);
		virtual void after_decoding_callback(uint8_t * pcm, size_t size) = 0;
	};

	class EXP_CLASS audio_encoder : public audio_base
	{
	public:
		typedef struct EXP_CLASS _configuration_t : public audio_base::configuration_t
		{
			int32_t samplerate;
			int32_t channels;
			int32_t bitrate;
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t operator=(const _configuration_t & clone);
		} configuration_t;

		audio_encoder(void);
		virtual ~audio_encoder(void);

		virtual int32_t initialize_encoder(void * config);
		virtual int32_t release_encoder(void);
		virtual int32_t encode(audio_encoder::entity_t * pcm, audio_encoder::entity_t * encoded);
		virtual int32_t encode(audio_encoder::entity_t * pcm);
		virtual int32_t get_queued_data(audio_encoder::entity_t * encoded);
		virtual void after_encoding_callback(uint8_t * bistream, size_t size) = 0;
	};

	class EXP_CLASS audio_renderer : public audio_base
	{
	public:
		typedef struct EXP_CLASS _configuration_t
		{
			int32_t samplerate;
			int32_t bitdepth;
			int32_t channels;
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t & operator=(const _configuration_t & clone);
		} configuration_t;

		audio_renderer(void);
		virtual ~audio_renderer(void);

		virtual int32_t initialize_renderer(void * config);
		virtual int32_t release_renderer(void);
		virtual int32_t render(audio_renderer::entity_t * pcm);
	};
};








#endif