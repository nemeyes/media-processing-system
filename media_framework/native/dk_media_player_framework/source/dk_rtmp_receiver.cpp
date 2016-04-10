#include "dk_rtmp_receiver.h"

dk_rtmp_receiver::dk_rtmp_receiver(void)
	: _frame_count(0)
{

}

dk_rtmp_receiver::~dk_rtmp_receiver(void)
{

}

void dk_rtmp_receiver::play(const char * url, const char * username, const char * password, int32_t recv_option, HWND hwnd)
{
	_hwnd = hwnd;
	dk_rtmp_client::subscribe_begin(url, username, password, recv_option, true);
}

void dk_rtmp_receiver::stop(void)
{
	dk_rtmp_client::subscribe_end();

	if (_video_decoder)
	{
		_video_decoder->release_decoder();
		delete _video_decoder;
		_video_decoder = nullptr;
	}
	if (_video_decoder_config)
	{
		delete _video_decoder_config;
		_video_decoder_config = nullptr;
	}

	if (_video_renderer)
	{
		_video_renderer->release_renderer();
		delete _video_renderer;
		_video_renderer = nullptr;
	}
	if (_video_renderer_config)
	{
		delete _video_renderer_config;
		_video_renderer_config = nullptr;
	}

	if (_audio_decoder)
	{
		_audio_decoder->release_decoder();
		delete _audio_decoder;
		_audio_decoder = nullptr;
	}
	if (_audio_decoder_config)
	{
		delete _audio_decoder_config;
		_audio_decoder_config = nullptr;
	}

	if (_audio_renderer)
	{
		_audio_renderer->release_renderer();
		delete _audio_renderer;
		_audio_renderer = nullptr;
	}
	if (_audio_renderer_config)
	{
		delete _audio_renderer_config;
		_audio_renderer_config = nullptr;
	}
}

void dk_rtmp_receiver::on_begin_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp)
{
	if (_video_decoder)
	{
		_video_decoder->release_decoder();
		delete _video_decoder;
		_video_decoder = nullptr;
	}
	if (_video_decoder_config)
	{
		delete _video_decoder_config;
		_video_decoder_config = nullptr;
	}

	if (_video_renderer)
	{
		_video_renderer->release_renderer();
		delete _video_renderer;
		_video_renderer = nullptr;
	}
	if (_video_renderer_config)
	{
		delete _video_renderer_config;
		_video_renderer_config = nullptr;
	}


	_video_decoder = new dk_ff_video_decoder();
	_video_decoder_config = new dk_ff_video_decoder::configuration_t();
	_video_renderer = new dk_directdraw_renderer();
	_video_renderer_config = new dk_directdraw_renderer::configuration_t();

	dk_ff_video_decoder * video_decoder = static_cast<dk_ff_video_decoder*>(_video_decoder);
	dk_ff_video_decoder::configuration_t * video_decoder_config = static_cast<dk_ff_video_decoder::configuration_t*>(_video_decoder_config);

	dk_directdraw_renderer * video_renderer = static_cast<dk_directdraw_renderer*>(_video_renderer);
	dk_directdraw_renderer::configuration_t * video_renderer_config = static_cast<dk_directdraw_renderer::configuration_t*>(_video_renderer_config);


	do
	{
		if (parse_sps((BYTE*)(sps), spssize, &video_decoder_config->iwidth, &video_decoder_config->iheight, &video_decoder_config->sarwidth, &video_decoder_config->sarheight) > 0)
		{
			video_decoder_config->owidth = video_decoder_config->iwidth;
			video_decoder_config->oheight = video_decoder_config->iheight;
			video_decoder_config->codec = dk_ff_video_decoder::submedia_type_t::submedia_type_h264;
			video_decoder_config->cs = dk_ff_video_decoder::submedia_type_t::submedia_type_rgb32;

			video_renderer_config->hwnd = _hwnd;
			video_renderer_config->width = video_decoder_config->owidth;
			video_renderer_config->height = video_decoder_config->oheight;

			dk_video_decoder::err_code decode_err = video_decoder->initialize_decoder(video_decoder_config);
			dk_video_renderer::err_code render_err = video_renderer->initialize_renderer(video_renderer_config);


			if (decode_err == dk_video_decoder::err_code_success)
			{
				dk_video_decoder::dk_video_entity_t encoded;
				encoded.mem_type = dk_video_decoder::memory_type_host;

				dk_video_decoder::dk_video_entity_t decoded; //= { dk_ff_video_decoder::MEMORY_TYPE_HOST, nullptr, _video_buffer, 0, VIDEO_BUFFER_SIZE, dk_ff_video_decoder::PICTURE_TYPE_NONE };
				decoded.mem_type = dk_video_decoder::memory_type_host;//, nullptr, nullptr, 0, 0, dk_ff_video_decoder::PICTURE_TYPE_NONE };
				decoded.data = _video_buffer;
				decoded.data_capacity = VIDEO_BUFFER_SIZE;

				//sps
				encoded.data = (uint8_t*)sps;
				encoded.data_size = spssize;
				decode_err = video_decoder->decode(&encoded, &decoded);
				if ((decode_err == dk_video_decoder::err_code_success) && (decoded.data_size > 0))
				{
					if (render_err == dk_video_renderer::err_code_success)
					{
						dk_video_renderer::dk_video_entity_t render;//= { dk_ff_video_decoder::MEMORY_TYPE_HOST, nullptr, nullptr, 0, 0, dk_ff_video_decoder::PICTURE_TYPE_NONE };
						render.mem_type = dk_video_renderer::memory_type_host;
						render.data = decoded.data;
						render.data_size = decoded.data_size;
						video_renderer->render(&render);
					}
				}

				//pps
				encoded.data = (uint8_t*)pps;
				encoded.data_size = ppssize;
				decode_err = video_decoder->decode(&encoded, &decoded);
				if ((decode_err == dk_video_decoder::err_code_success) && (decoded.data_size > 0))
				{
					if (render_err == dk_video_renderer::err_code_success)
					{
						dk_video_renderer::dk_video_entity_t render;
						render.mem_type = dk_video_decoder::memory_type_host;
						render.data = decoded.data;
						render.data_size = decoded.data_size;
						video_renderer->render(&render);
					}
				}

				//idr
				encoded.data = (uint8_t*)data;
				encoded.data_size = data_size;
				decode_err = video_decoder->decode(&encoded, &decoded);
				if ((decode_err == dk_video_decoder::err_code_success) && (decoded.data_size > 0))
				{
					if (render_err == dk_video_renderer::err_code_success)
					{
						dk_video_renderer::dk_video_entity_t render;
						render.mem_type = dk_video_renderer::memory_type_host;
						render.data = decoded.data;
						render.data_size = decoded.data_size;
						video_renderer->render(&render);
					}
				}
			}
		}
	} while (0);
}

void dk_rtmp_receiver::on_recv_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long timestamp)
{
	dk_ff_video_decoder * video_decoder = static_cast<dk_ff_video_decoder*>(_video_decoder);
	dk_ff_video_decoder::configuration_t * video_decoder_config = static_cast<dk_ff_video_decoder::configuration_t*>(_video_decoder_config);

	dk_directdraw_renderer * video_renderer = static_cast<dk_directdraw_renderer*>(_video_renderer);


	dk_video_decoder::dk_video_entity_t encoded;
	encoded.mem_type = dk_video_decoder::memory_type_host;
	dk_video_decoder::dk_video_entity_t decoded;
	decoded.mem_type = dk_video_decoder::memory_type_host;

	encoded.data = (uint8_t*)data;
	encoded.data_size = data_size;

	decoded.data = _video_buffer;
	decoded.data_capacity = VIDEO_BUFFER_SIZE;

	dk_video_decoder::err_code decode_err = video_decoder->decode(&encoded, &decoded);
	if ((decode_err == dk_video_decoder::err_code_success) && (decoded.data_size > 0))
	{
		dk_video_renderer::dk_video_entity_t render;// = { dk_ff_video_decoder::MEMORY_TYPE_HOST, nullptr, nullptr, 0, 0, dk_ff_video_decoder::PICTURE_TYPE_NONE };
		render.mem_type = dk_video_renderer::memory_type_host;
		render.data = decoded.data;
		render.data_size = decoded.data_size;
		video_renderer->render(&render);
	}
}

void dk_rtmp_receiver::on_begin_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp)
{
	if (_audio_decoder)
	{
		_audio_decoder->release_decoder();
		delete _audio_decoder;
		_audio_decoder = nullptr;
	}
	if (_audio_decoder_config)
	{
		delete _audio_decoder_config;
		_audio_decoder_config = nullptr;
	}

	if (_audio_renderer)
	{
		_audio_renderer->release_renderer();
		delete _audio_renderer;
		_audio_renderer = nullptr;
	}
	if (_audio_renderer_config)
	{
		delete _audio_renderer_config;
		_audio_renderer_config = nullptr;
	}

	_audio_renderer = new dk_mmwave_renderer();
	_audio_renderer_config = new dk_mmwave_renderer::configuration_t();
	dk_mmwave_renderer * audio_renderer = static_cast<dk_mmwave_renderer*>(_audio_renderer);
	dk_mmwave_renderer::configuration_t * audio_renderer_config = static_cast<dk_mmwave_renderer::configuration_t*>(_audio_renderer_config);

	switch (smt)
	{
	case dk_rtmp_client::SUBMEDIA_TYPE_AAC:
	{
		_audio_decoder = new dk_aac_decoder();
		_audio_decoder_config = new dk_aac_decoder::configuration_t();

		dk_aac_decoder * audio_decoder = static_cast<dk_aac_decoder*>(_audio_decoder);
		dk_aac_decoder::configuration_t * audio_decoder_config = static_cast<dk_aac_decoder::configuration_t*>(_audio_decoder_config);

		audio_decoder_config->extradata_size = config_size;
		memcpy(audio_decoder_config->extradata, config, audio_decoder_config->extradata_size);
		audio_decoder_config->samplerate = samplerate;
		audio_decoder_config->bitdepth = bitdepth;
		audio_decoder_config->channels = channels;

		audio_renderer_config->samplerate = samplerate;
		audio_renderer_config->bitdepth = bitdepth;
		audio_renderer_config->channels = 2;

		dk_audio_decoder::err_code decode_err = audio_decoder->initialize_decoder(audio_decoder_config);
		dk_audio_renderer::err_code render_err = audio_renderer->initialize_renderer(audio_renderer_config);

		if (decode_err == dk_audio_decoder::err_code_success)
		{
			dk_audio_decoder::dk_audio_entity_t encoded = { 0, 0, 0, 0 };
			dk_audio_decoder::dk_audio_entity_t pcm = { 0, _audio_buffer, 0, AUDIO_BUFFER_SIZE };
			encoded.data = (uint8_t*)data;
			encoded.data_size = data_size;
			decode_err = audio_decoder->decode(&encoded, &pcm);
			if ((decode_err == dk_audio_decoder::err_code_success) && (pcm.data_size > 0))
			{
				dk_audio_renderer::dk_audio_entity_t render = { 0, 0, 0, 0 };
				render.data = pcm.data;
				render.data_size = pcm.data_size;
				audio_renderer->render(&render);
			}
		}
		break;
	}
	case dk_rtmp_client::SUBMEDIA_TYPE_MP3:
	{
		_audio_decoder = new dk_ff_mp3_decoder();
		_audio_decoder_config = new dk_ff_mp3_decoder::configuration_t();

		dk_ff_mp3_decoder * audio_decoder = static_cast<dk_ff_mp3_decoder*>(_audio_decoder);
		dk_ff_mp3_decoder::configuration_t * audio_decoder_config = static_cast<dk_ff_mp3_decoder::configuration_t*>(_audio_decoder_config);

		audio_decoder_config->samplerate = samplerate;
		audio_decoder_config->bitdepth = bitdepth;
		audio_decoder_config->channels = channels;

		audio_renderer_config->samplerate = samplerate;
		audio_renderer_config->bitdepth = bitdepth;
		audio_renderer_config->channels = 2;

		dk_audio_decoder::err_code decode_err = audio_decoder->initialize_decoder(audio_decoder_config);
		dk_audio_renderer::err_code render_err = audio_renderer->initialize_renderer(audio_renderer_config);

		if (decode_err == dk_audio_decoder::err_code_success)
		{
			dk_audio_decoder::dk_audio_entity_t encoded = { 0, 0, 0, 0 };
			dk_audio_decoder::dk_audio_entity_t pcm = { 0, _audio_buffer, 0, AUDIO_BUFFER_SIZE };
			encoded.data = (uint8_t*)data;
			encoded.data_size = data_size;
			decode_err = audio_decoder->decode(&encoded, &pcm);
			if ((decode_err == dk_audio_decoder::err_code_success) && (pcm.data_size > 0))
			{
				dk_audio_renderer::dk_audio_entity_t render = { 0, 0, 0, 0 };
				render.data = pcm.data;
				render.data_size = pcm.data_size;
				audio_renderer->render(&render);
			}
		}
		break;
	}
	}
}

void dk_rtmp_receiver::on_recv_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long timestamp)
{
	switch (smt)
	{
	case dk_rtmp_client::SUBMEDIA_TYPE_AAC:
	{
		dk_aac_decoder * audio_decoder = static_cast<dk_aac_decoder*>(_audio_decoder);
		dk_aac_decoder::configuration_t * audio_decoder_config = static_cast<dk_aac_decoder::configuration_t*>(_audio_decoder_config);

		dk_aac_decoder * aac_audio_decoder = static_cast<dk_aac_decoder*>(_audio_decoder);

		dk_audio_decoder::dk_audio_entity_t encoded = { 0, 0, 0, 0 };
		dk_audio_decoder::dk_audio_entity_t pcm = { 0, _audio_buffer, 0, AUDIO_BUFFER_SIZE };
		encoded.data = (uint8_t*)data;
		encoded.data_size = data_size;

		dk_audio_decoder::err_code decode_err = aac_audio_decoder->decode(&encoded, &pcm);
		if ((decode_err == dk_audio_decoder::err_code_success) && (pcm.data_size > 0))
		{
			dk_audio_renderer::dk_audio_entity_t render = { 0, 0, 0 };
			render.data = pcm.data;
			render.data_size = pcm.data_size;
			_audio_renderer->render(&render);
		}
		break;
	}
	case dk_rtmp_client::SUBMEDIA_TYPE_MP3:
	{
		dk_ff_mp3_decoder * audio_decoder = static_cast<dk_ff_mp3_decoder*>(_audio_decoder);
		dk_ff_mp3_decoder::configuration_t * audio_decoder_config = static_cast<dk_ff_mp3_decoder::configuration_t*>(_audio_decoder_config);

		dk_audio_decoder::dk_audio_entity_t encoded = { 0, 0, 0, 0 };
		dk_audio_decoder::dk_audio_entity_t pcm = { 0, _audio_buffer, 0, AUDIO_BUFFER_SIZE };
		encoded.data = (uint8_t*)data;
		encoded.data_size = data_size;

		dk_audio_decoder::err_code decode_err = audio_decoder->decode(&encoded, &pcm);
		if ((decode_err == dk_audio_decoder::err_code_success) && (pcm.data_size > 0))
		{
			dk_audio_renderer::dk_audio_entity_t render = { 0, 0, 0, 0 };
			render.data = pcm.data;
			render.data_size = pcm.data_size;
			_audio_renderer->render(&render);
		}
		break;
	}
	}
}