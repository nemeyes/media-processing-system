#include "dk_rtmp_receiver.h"

debuggerking::rtmp_receiver::rtmp_receiver(void)
	: _frame_count(0)
{

}

debuggerking::rtmp_receiver::~rtmp_receiver(void)
{

}

int32_t debuggerking::rtmp_receiver::play(const char * url, const char * username, const char * password, int32_t recv_option, HWND hwnd)
{
	_hwnd = hwnd;
	return subscribe_begin(url, username, password, recv_option, true);
}

int32_t debuggerking::rtmp_receiver::stop(void)
{
	int32_t status = subscribe_end();

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
	return status;
}

void debuggerking::rtmp_receiver::on_begin_video(int32_t smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp)
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


	_video_decoder = new ff_video_decoder();
	_video_decoder_config = new ff_video_decoder::configuration_t();
	_video_renderer = new directdraw_renderer();
	_video_renderer_config = new directdraw_renderer::configuration_t();

	ff_video_decoder * video_decoder = static_cast<ff_video_decoder*>(_video_decoder);
	ff_video_decoder::configuration_t * video_decoder_config = static_cast<ff_video_decoder::configuration_t*>(_video_decoder_config);

	directdraw_renderer * video_renderer = static_cast<directdraw_renderer*>(_video_renderer);
	directdraw_renderer::configuration_t * video_renderer_config = static_cast<directdraw_renderer::configuration_t*>(_video_renderer_config);


	do
	{
		if (parse_sps((BYTE*)(sps), spssize, &video_decoder_config->iwidth, &video_decoder_config->iheight, &video_decoder_config->sarwidth, &video_decoder_config->sarheight) > 0)
		{
			video_decoder_config->owidth = video_decoder_config->iwidth;
			video_decoder_config->oheight = video_decoder_config->iheight;
			video_decoder_config->codec = ff_video_decoder::video_submedia_type_t::h264;
			video_decoder_config->cs = ff_video_decoder::video_submedia_type_t::rgb32;

			video_renderer_config->hwnd = _hwnd;
			video_renderer_config->width = video_decoder_config->owidth;
			video_renderer_config->height = video_decoder_config->oheight;

			int32_t decode_err = video_decoder->initialize_decoder(video_decoder_config);
			int32_t render_err = video_renderer->initialize_renderer(video_renderer_config);


			if (decode_err == video_decoder::err_code_t::success)
			{
				video_decoder::entity_t encoded;
				encoded.mem_type = video_decoder::video_memory_type_t::host;

				video_decoder::entity_t decoded; //= { dk_ff_video_decoder::MEMORY_TYPE_HOST, nullptr, _video_buffer, 0, VIDEO_BUFFER_SIZE, dk_ff_video_decoder::PICTURE_TYPE_NONE };
				decoded.mem_type = video_decoder::video_memory_type_t::host;//, nullptr, nullptr, 0, 0, dk_ff_video_decoder::PICTURE_TYPE_NONE };
				decoded.data = _video_buffer;
				decoded.data_capacity = VIDEO_BUFFER_SIZE;

				//sps
				encoded.data = (uint8_t*)sps;
				encoded.data_size = spssize;
				decode_err = video_decoder->decode(&encoded, &decoded);
				if ((decode_err == video_decoder::err_code_t::success) && (decoded.data_size > 0))
				{
					if (render_err == video_renderer::err_code_t::success)
					{
						video_renderer::entity_t render;//= { dk_ff_video_decoder::MEMORY_TYPE_HOST, nullptr, nullptr, 0, 0, dk_ff_video_decoder::PICTURE_TYPE_NONE };
						render.mem_type = video_renderer::video_memory_type_t::host;
						render.data = decoded.data;
						render.data_size = decoded.data_size;
						video_renderer->render(&render);
					}
				}

				//pps
				encoded.data = (uint8_t*)pps;
				encoded.data_size = ppssize;
				decode_err = video_decoder->decode(&encoded, &decoded);
				if ((decode_err == video_decoder::err_code_t::success) && (decoded.data_size > 0))
				{
					if (render_err == video_renderer::err_code_t::success)
					{
						video_renderer::entity_t render;
						render.mem_type = video_decoder::video_memory_type_t::host;
						render.data = decoded.data;
						render.data_size = decoded.data_size;
						video_renderer->render(&render);
					}
				}

				//idr
				encoded.data = (uint8_t*)data;
				encoded.data_size = data_size;
				decode_err = video_decoder->decode(&encoded, &decoded);
				if ((decode_err == video_decoder::err_code_t::success) && (decoded.data_size > 0))
				{
					if (render_err == video_renderer::err_code_t::success)
					{
						video_renderer::entity_t render;
						render.mem_type = video_renderer::video_memory_type_t::host;
						render.data = decoded.data;
						render.data_size = decoded.data_size;
						video_renderer->render(&render);
					}
				}
			}
		}
	} while (0);
}

void debuggerking::rtmp_receiver::on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp)
{
	ff_video_decoder * video_decoder = static_cast<ff_video_decoder*>(_video_decoder);
	ff_video_decoder::configuration_t * video_decoder_config = static_cast<ff_video_decoder::configuration_t*>(_video_decoder_config);

	directdraw_renderer * video_renderer = static_cast<directdraw_renderer*>(_video_renderer);


	video_decoder::entity_t encoded;
	encoded.mem_type = video_decoder::video_memory_type_t::host;
	video_decoder::entity_t decoded;
	decoded.mem_type = video_decoder::video_memory_type_t::host;

	encoded.data = (uint8_t*)data;
	encoded.data_size = data_size;

	decoded.data = _video_buffer;
	decoded.data_capacity = VIDEO_BUFFER_SIZE;

	int32_t decode_err = video_decoder->decode(&encoded, &decoded);
	if ((decode_err == video_decoder::err_code_t::success) && (decoded.data_size > 0))
	{
		video_renderer::entity_t render;// = { dk_ff_video_decoder::MEMORY_TYPE_HOST, nullptr, nullptr, 0, 0, dk_ff_video_decoder::PICTURE_TYPE_NONE };
		render.mem_type = video_renderer::video_memory_type_t::host;
		render.data = decoded.data;
		render.data_size = decoded.data_size;
		video_renderer->render(&render);
	}
}

void debuggerking::rtmp_receiver::on_begin_audio(int32_t smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp)
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

	_audio_renderer = new mmwave_renderer();
	_audio_renderer_config = new mmwave_renderer::configuration_t();
	mmwave_renderer * audio_renderer = static_cast<mmwave_renderer*>(_audio_renderer);
	mmwave_renderer::configuration_t * audio_renderer_config = static_cast<mmwave_renderer::configuration_t*>(_audio_renderer_config);

	switch (smt)
	{
		case rtmp_client::audio_submedia_type_t::aac:
		{
			_audio_decoder = new aac_decoder();
			_audio_decoder_config = new aac_decoder::configuration_t();

			aac_decoder * audio_decoder = static_cast<aac_decoder*>(_audio_decoder);
			aac_decoder::configuration_t * audio_decoder_config = static_cast<aac_decoder::configuration_t*>(_audio_decoder_config);

			audio_decoder_config->extradata_size = config_size;
			memcpy(audio_decoder_config->extradata, config, audio_decoder_config->extradata_size);
			audio_decoder_config->samplerate = samplerate;
			audio_decoder_config->bitdepth = bitdepth;
			audio_decoder_config->channels = channels;

			audio_renderer_config->samplerate = samplerate;
			audio_renderer_config->bitdepth = bitdepth;
			audio_renderer_config->channels = 2;

			int32_t decode_err = audio_decoder->initialize_decoder(audio_decoder_config);
			int32_t render_err = audio_renderer->initialize_renderer(audio_renderer_config);

			if (decode_err == audio_decoder::err_code_t::success)
			{
				audio_decoder::entity_t encoded;
				encoded.data = (uint8_t*)data;
				encoded.data_size = data_size;

				audio_decoder::entity_t pcm;
				pcm.data = _audio_buffer;
				pcm.data_size = 0;
				pcm.data_capacity = audio_decoder::max_media_value_t::max_audio_size;

				decode_err = audio_decoder->decode(&encoded, &pcm);
				if ((decode_err == audio_decoder::err_code_t::success) && (pcm.data_size > 0))
				{
					audio_renderer::entity_t render;
					render.data = pcm.data;
					render.data_size = pcm.data_size;
					audio_renderer->render(&render);
				}
			}
			break;
		}
		case rtmp_client::audio_submedia_type_t::mp3:
		{
			_audio_decoder = new ff_mp3_decoder();
			_audio_decoder_config = new ff_mp3_decoder::configuration_t();

			ff_mp3_decoder * audio_decoder = static_cast<ff_mp3_decoder*>(_audio_decoder);
			ff_mp3_decoder::configuration_t * audio_decoder_config = static_cast<ff_mp3_decoder::configuration_t*>(_audio_decoder_config);

			audio_decoder_config->samplerate = samplerate;
			audio_decoder_config->bitdepth = bitdepth;
			audio_decoder_config->channels = channels;

			audio_renderer_config->samplerate = samplerate;
			audio_renderer_config->bitdepth = bitdepth;
			audio_renderer_config->channels = 2;

			int32_t decode_err = audio_decoder->initialize_decoder(audio_decoder_config);
			int32_t render_err = audio_renderer->initialize_renderer(audio_renderer_config);

			if (decode_err == audio_decoder::err_code_t::success)
			{
				audio_decoder::entity_t encoded;
				encoded.data = (uint8_t*)data;
				encoded.data_size = data_size;

				audio_decoder::entity_t pcm;
				pcm.data = _audio_buffer;
				pcm.data_size = 0;
				pcm.data_capacity = AUDIO_BUFFER_SIZE;

				decode_err = audio_decoder->decode(&encoded, &pcm);
				if ((decode_err == audio_decoder::err_code_t::success) && (pcm.data_size > 0))
				{
					audio_renderer::entity_t render;
					render.data = pcm.data;
					render.data_size = pcm.data_size;
					audio_renderer->render(&render);
				}
			}
			break;
		}
	}
}

void debuggerking::rtmp_receiver::on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp)
{
	switch (smt)
	{
		case rtmp_client::audio_submedia_type_t::aac:
		{
			aac_decoder * audio_decoder = static_cast<aac_decoder*>(_audio_decoder);
			aac_decoder::configuration_t * audio_decoder_config = static_cast<aac_decoder::configuration_t*>(_audio_decoder_config);

			aac_decoder * aac_audio_decoder = static_cast<aac_decoder*>(_audio_decoder);

			audio_decoder::entity_t encoded;
			encoded.data = (uint8_t*)data;
			encoded.data_size = data_size;

			audio_decoder::entity_t pcm;
			pcm.data = _audio_buffer;
			pcm.data_size = 0;
			pcm.data_capacity = AUDIO_BUFFER_SIZE;

			int32_t decode_err = aac_audio_decoder->decode(&encoded, &pcm);
			if ((decode_err == audio_decoder::err_code_t::success) && (pcm.data_size > 0))
			{
				audio_renderer::entity_t render;
				render.data = pcm.data;
				render.data_size = pcm.data_size;
				_audio_renderer->render(&render);
			}
			break;
		}
		case rtmp_client::audio_submedia_type_t::mp3:
		{
			ff_mp3_decoder * audio_decoder = static_cast<ff_mp3_decoder*>(_audio_decoder);
			ff_mp3_decoder::configuration_t * audio_decoder_config = static_cast<ff_mp3_decoder::configuration_t*>(_audio_decoder_config);

			audio_decoder::entity_t encoded;
			encoded.data = (uint8_t*)data;
			encoded.data_size = data_size;

			audio_decoder::entity_t pcm;
			pcm.data = _audio_buffer;
			pcm.data_size = 0;
			pcm.data_capacity = AUDIO_BUFFER_SIZE;

			int32_t decode_err = audio_decoder->decode(&encoded, &pcm);
			if ((decode_err == audio_decoder::err_code_t::success) && (pcm.data_size > 0))
			{
				audio_renderer::entity_t render;
				render.data = pcm.data;
				render.data_size = pcm.data_size;
				_audio_renderer->render(&render);
			}
			break;
		}
	}
}