#include "mpeg2ts_muxer.h"

mpeg2ts_muxer::mpeg2ts_muxer(dk_ff_mpeg2ts_muxer * front)
	: _front(front)
	, _avio_ctx(nullptr)
	, _avio_buffer(nullptr)
	, _avio_buffer_size(1024*1024*2)
	, _state(dk_ff_mpeg2ts_muxer::STATE_NONE)
{
	_avio_buffer = static_cast<uint8_t*>(av_malloc(_avio_buffer_size));
}

mpeg2ts_muxer::~mpeg2ts_muxer(void)
{
	if (_avio_buffer)
	{
		av_free(_avio_buffer);
		_avio_buffer = nullptr;
	}
	_avio_buffer_size = 0;
}

dk_ff_mpeg2ts_muxer::ERR_CODE mpeg2ts_muxer::initialize(dk_ff_mpeg2ts_muxer::configuration_t * config)
{
	_config = config;

	_ofmt = av_guess_format("mpegts", NULL, NULL);
	if (!_ofmt)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;
	_ofmt->video_codec = AV_CODEC_ID_H264;

	_format_ctx = avformat_alloc_context();
	if (!_format_ctx)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;
	
	_format_ctx->oformat = _ofmt;
#if 0
	sprintf_s(_format_ctx->filename, "%s", "test.ts");
	// Open the output container file
	if (avio_open(&_format_ctx->pb, _format_ctx->filename, AVIO_FLAG_WRITE) < 0)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;
#else
	_avio_ctx = avio_alloc_context(_avio_buffer, _avio_buffer_size, 1, this, NULL, mpeg2ts_muxer::on_write_packet, NULL);
	_format_ctx->pb = _avio_ctx;
#endif

	if (_ofmt->video_codec == AV_CODEC_ID_NONE)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

	_vstream = avformat_new_stream(_format_ctx, NULL);
	if (!_vstream)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

	AVCodecContext * c = _vstream->codec;
	c->codec_id = _ofmt->video_codec;
	c->codec_type = AVMEDIA_TYPE_VIDEO;
	c->bit_rate = _config->vconfig.bitrate;
	c->width = _config->vconfig.width;
	c->height = _config->vconfig.height;
	c->time_base.den = _config->vconfig.fps; //time base : this is the fundamental unit of time(in seconds) in terms of which frame timestamps are represented. for fixed - fps content,
	c->time_base.num = 1;					//timebase should be 1/framerate and timestamp increments should be identically 1.
	if (_format_ctx->oformat->flags & AVFMT_GLOBALHEADER) // Some formats want stream headers to be separate
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	c->extradata = _config->vconfig.extradata; // Codec "extradata" conveys the H.264 stream SPS and PPS info (MPEG2: sequence header is housed in SPS buffer, PPS buffer is empty)
	c->extradata_size = _config->vconfig.extradata_size;

#ifdef ENCODE_AUDIO
	if (m_pFmt->audio_codec != AV_CODEC_ID_NONE) 
	{
		if (!add_audio_stream(m_pFormatCtx, m_pFmt->audio_codec))
			return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

		if (!open_audio())
			return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;
	}
#endif

	// Write container header
	if (avformat_write_header(_format_ctx, NULL))
		dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

	_nframes = 0;
	_state = dk_ff_mpeg2ts_muxer::STATE_INITIALIZED;
	return dk_ff_mpeg2ts_muxer::ERR_CODE_SUCCESS;
}

dk_ff_mpeg2ts_muxer::ERR_CODE mpeg2ts_muxer::release(void)
{
	if (_is_initialized)
	{
#ifdef ENCODE_AUDIO
		flush_audio(m_pFormatCtx);
#endif
		// Write the trailer, if any. 
		//   The trailer must be written before you close the CodecContexts open when you wrote the
		//   header; otherwise write_trailer may try to use memory that was freed on av_codec_close()
		av_write_trailer(_format_ctx);

#ifdef ENCODE_AUDIO
		close_audio();
#endif
		// Free the streams
		for (unsigned int i = 0; i < _format_ctx->nb_streams; i++)
		{
			av_freep(&_format_ctx->streams[i]->codec);
			av_freep(&_format_ctx->streams[i]);
		}

		avio_close(_format_ctx->pb);
		av_free(_format_ctx);
	}

	_nframes = 0;
	_state = dk_ff_mpeg2ts_muxer::STATE_NONE;
	return dk_ff_mpeg2ts_muxer::ERR_CODE_SUCCESS;
}

dk_ff_mpeg2ts_muxer::STATE mpeg2ts_muxer::state(void)
{
	return _state;
}

dk_ff_mpeg2ts_muxer::ERR_CODE mpeg2ts_muxer::put_video_stream(uint8_t * buffer, size_t nb, int64_t ts, bool keyframe)
{
	// Note for H.264 :
	//    At the moment the SPS/PPS will be written to container again for the first frame here
	//    To eliminate this we would have to search to first slice (or right after PPS)
	++_nframes;

	AVPacket pkt;
	av_init_packet(&pkt);

	AVCodecContext * c = _vstream->codec;

	//m_pVideoStream->pts.val = m_nProcessedFramesNum;
	pkt.pts = av_rescale_q(_nframes/*m_pVideoStream->pts.val*/, c->time_base, _vstream->time_base);
	pkt.dts = AV_NOPTS_VALUE;
	pkt.stream_index = _vstream->index;
	pkt.data = buffer;
	pkt.size = nb;

	//TODO
	//if (pMfxBitstream->FrameType == (MFX_FRAMETYPE_I | MFX_FRAMETYPE_REF | MFX_FRAMETYPE_IDR))
	if (keyframe)
		pkt.flags |= AV_PKT_FLAG_KEY;

	// Write the compressed frame in the media file
	int ret = av_interleaved_write_frame(_format_ctx, &pkt);
	if (ret<0)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

#ifdef ENCODE_AUDIO
	// Note that video + audio muxing timestamp handling in this sample is very rudimentary
	//  - Audio stream length is adjusted to same as video steram length 
	float real_video_	 = (float)pkt.pts * m_pVideoStream->time_base.num / m_pVideoStream->time_base.den;
	write_audio_frame(m_pFormatCtx, real_video_pts);
#endif

	return dk_ff_mpeg2ts_muxer::ERR_CODE_SUCCESS;
}

int32_t mpeg2ts_muxer::on_write_packet(void * opaque, uint8_t * buffer, int32_t size)
{
	mpeg2ts_muxer * self = reinterpret_cast<mpeg2ts_muxer*>(opaque);

	if (self && self->_front)
		self->_front->recv_ts_stream_callback(buffer, size);
	return size;
}