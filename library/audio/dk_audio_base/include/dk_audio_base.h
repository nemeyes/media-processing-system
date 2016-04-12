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
		long long pts;
		size_t amount;
		_abuffer_t * prev;
		_abuffer_t * next;
	} abuffer_t;

	typedef enum _err_code
	{
		err_code_success,
		err_code_fail,
		err_code_not_implemented,
	} err_code;

	typedef struct _dk_audio_entity_t
	{
		long long pts;
		void * data;
		size_t data_size;
		size_t data_capacity;
		_dk_audio_entity_t(void)
			: pts(0)
			, data(nullptr)
			, data_size(0)
			, data_capacity(0)
		{}
		_dk_audio_entity_t(const _dk_audio_entity_t & clone)
		{
			pts = clone.pts;
			data = clone.data;
			data_size = clone.data_size;
			data_capacity = clone.data_capacity;
		}
		_dk_audio_entity_t & operator=(const _dk_audio_entity_t & clone)
		{
			pts = clone.pts;
			data = clone.data;
			data_size = clone.data_size;
			data_capacity = clone.data_capacity;
			return (*this);
		}
	} dk_audio_entity_t;

	dk_audio_base(void);
	virtual ~dk_audio_base(void);

	dk_audio_base::err_code push(uint8_t * bs, size_t size, long long pts);
	dk_audio_base::err_code pop(uint8_t * bs, size_t & size, long long & pts);
	dk_audio_base::err_code init(abuffer_t * buffer);

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
	typedef struct EXP_CLASS _configuration_t
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

	dk_audio_decoder(void);
	virtual ~dk_audio_decoder(void);

	virtual dk_audio_decoder::err_code initialize_decoder(void * config);
	virtual dk_audio_decoder::err_code release_decoder(void);
	virtual dk_audio_decoder::err_code decode(dk_audio_decoder::dk_audio_entity_t * encoded, dk_audio_decoder::dk_audio_entity_t * pcm);
};

class EXP_CLASS dk_audio_encoder : public dk_audio_base
{
public:
	typedef struct EXP_CLASS _configuration_t
	{
		int32_t samplerate;
		int32_t channels;
		int32_t bitrate;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;

	dk_audio_encoder(void);
	virtual ~dk_audio_encoder(void);

	virtual dk_audio_encoder::err_code initialize_encoder(void * config);
	virtual dk_audio_encoder::err_code release_encoder(void);
	virtual dk_audio_encoder::err_code encode(dk_audio_encoder::dk_audio_entity_t * pcm, dk_audio_encoder::dk_audio_entity_t * encoded);
	virtual dk_audio_encoder::err_code encode(dk_audio_encoder::dk_audio_entity_t * pcm);
	virtual dk_audio_encoder::err_code get_queued_data(dk_audio_encoder::dk_audio_entity_t * encoded);

	virtual uint8_t * extradata(void);
	virtual size_t extradata_size(void);
};

class EXP_CLASS dk_audio_renderer : public dk_audio_base
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

	dk_audio_renderer(void);
	virtual ~dk_audio_renderer(void);

	virtual dk_audio_renderer::err_code initialize_renderer(void * config);
	virtual dk_audio_renderer::err_code release_renderer(void);
	virtual dk_audio_renderer::err_code render(dk_audio_renderer::dk_audio_entity_t * pcm);
};






#endif