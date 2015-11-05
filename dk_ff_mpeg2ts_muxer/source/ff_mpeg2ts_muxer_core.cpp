#include "ff_mpeg2ts_muxer_core.h"

ff_mpeg2ts_muxer_core::ff_mpeg2ts_muxer_core(void)
{

}

ff_mpeg2ts_muxer_core::~ff_mpeg2ts_muxer_core(void)
{

}

dk_ff_mpeg2ts_muxer::ERR_CODE ff_mpeg2ts_muxer_core::initialize(dk_ff_mpeg2ts_muxer::configuration_t * config)
{
	_config = *config;

	_ofmt = av_guess_format("mpegts", NULL, NULL);
	if (!_ofmt)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

	_ofmt->video_codec = AV_CODEC_ID_H264;

	_format_ctx = avformat_alloc_context();
	if (!_format_ctx)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;
	
	_format_ctx->oformat = _ofmt;
	sprintf_s(_format_ctx->filename, "%s", "test.ts");

	if (_ofmt->video_codec == AV_CODEC_ID_NONE)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

	_vstream = avformat_new_stream(_format_ctx, NULL);
	if (!_vstream)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

	AVCodecContext * c = _vstream->codec;
	c->codec_id = _ofmt->video_codec;
	c->codec_type = AVMEDIA_TYPE_VIDEO;
	c->bit_rate = _config.bitrate * 1000;
	c->width = _config.width;
	c->height = _config.height;

	// time base: this is the fundamental unit of time (in seconds) in terms of which frame timestamps are represented. for fixed-fps content,
	//            timebase should be 1/framerate and timestamp increments should be identically 1.
	c->time_base.den = _config.fps;
	c->time_base.num = 1;

	// Some formats want stream headers to be separate
	if (_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;

#ifdef ENCODE_AUDIO
	if (m_pFmt->audio_codec != AV_CODEC_ID_NONE) 
	{
		if (!add_audio_stream(m_pFormatCtx, m_pFmt->audio_codec))
			return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

		if (!open_audio())
			return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;
	}
#endif

	// Open the output container file
	if (avio_open(&_format_ctx->pb, _format_ctx->filename, AVIO_FLAG_WRITE) < 0)
		return dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

	/*m_pExtDataBuffer = (mfxU8*)av_malloc(SPSbufsize + PPSbufsize);
	if (!m_pExtDataBuffer) {
		_tcprintf(_T("FFMPEG: could not allocate required buffer\n"));
		return MFX_ERR_UNKNOWN;
	}*/


	// Codec "extradata" conveys the H.264 stream SPS and PPS info (MPEG2: sequence header is housed in SPS buffer, PPS buffer is empty)
	c->extradata = _config.extradata;
	c->extradata_size = _config.extradata_size;

	// Write container header
	if (avformat_write_header(_format_ctx, NULL))
		dk_ff_mpeg2ts_muxer::ERR_CODE_FAILED;

	m_nProcessedFramesNum = 0;
	m_bInited = true;
	return dk_ff_mpeg2ts_muxer::ERR_CODE_SUCCESS;
}

dk_ff_mpeg2ts_muxer::ERR_CODE ff_mpeg2ts_muxer_core::release(void)
{
	if (m_bInited)
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

	m_bInited = false;
	m_nProcessedFramesNum = 0;
	return dk_ff_mpeg2ts_muxer::ERR_CODE_SUCCESS;
}

dk_ff_mpeg2ts_muxer::ERR_CODE ff_mpeg2ts_muxer_core::put_video_stream(uint8_t * buffer, size_t nb, int64_t pts, bool keyframe)
{
	// Note for H.264 :
	//    At the moment the SPS/PPS will be written to container again for the first frame here
	//    To eliminate this we would have to search to first slice (or right after PPS)
	++m_nProcessedFramesNum;

	AVPacket pkt;
	av_init_packet(&pkt);

	AVCodecContext * c = _vstream->codec;

	//m_pVideoStream->pts.val = m_nProcessedFramesNum;
	pkt.pts = av_rescale_q(m_nProcessedFramesNum/*m_pVideoStream->pts.val*/, c->time_base, _vstream->time_base);
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

/*int ff_mpeg2ts_muxer_core::initialize(const char* outputName)
{
	av_register_all();
	_format_ctx = avformat_alloc_context();
	_fmt = av_guess_format("ts", NULL, NULL);
	_format_ctx->oformat = _fmt;
	strcpy(_format_ctx->filename, outputName);
	return 0;
}

int ff_mpeg2ts_muxer_core::add_video_stream(int stream_index, int width, int height, int bitrate, int fps)
{
	_fps = fps;
	int timebaseDen = 90000;
	int timebaseNum = 1;

	_video_stream = avformat_new_stream(_format_ctx, stream_index);
	avcodec_get_context_defaults2(_video_stream->codec, CODEC_TYPE_VIDEO);
	_video_stream->stream_copy = 1;
	_video_stream->avg_frame_rate.den = timebaseNum;
	_video_stream->avg_frame_rate.num = timebaseDen;
	_video_stream->time_base.den = timebaseDen;
	_video_stream->time_base.num = timebaseNum;
	//videoStream->first_dts = 0;
	//videoStream->start_time = 0;
	_video_stream->r_frame_rate.den = timebaseNum;
	_video_stream->r_frame_rate.num = timebaseDen;
	AVCodecContext * codec_ctx = _video_stream->codec;
	codec_ctx->codec_id = CODEC_ID_H264;
	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;

	codec_ctx->time_base.den = timebaseDen;
	codec_ctx->time_base.num = timebaseNum;
	codec_ctx->width = width;
	codec_ctx->height = height;
	codec_ctx->bit_rate = bitrate;
	return 0;
}

int ff_mpeg2ts_muxer_core::begin_mux(void)
{
	int ret = av_set_parameters(_format_ctx, NULL);
	dump_format(formatCtx, 0, formatCtx->filename, 1);

	if (!(fmt->flags & AVFMT_NOFILE))
	{

		url_open_dyn_buf(&ioctx);
		formatCtx->pb = ioctx;

		rtpCtx = av_alloc_format_context();
		rtpCtx->oformat = guess_format("rtp", 0, 0);
		AVStream *fakeVideo = 0;
		fakeVideo = av_new_stream(rtpCtx, 0);
		avcodec_get_context_defaults(fakeVideo->codec);
		fakeVideo->codec->codec_id = CODEC_ID_MPEG2TS;
		rtpCtx->audio_codec_id = CODEC_ID_NONE;
		rtpCtx->video_codec_id = CODEC_ID_MPEG2TS;
		av_set_parameters(rtpCtx, 0);

		char *url = new char[1024];
		sprintf(url, "rtp://%s:%d", rtpOutputAddr, rtpOutputPort);
		printf("will output to url:%s\n", url);
		ret = url_fopen(&rtpCtx->pb, url, URL_WRONLY);
		ret = av_write_header(rtpCtx);
		delete url;

		//if (url_fopen(&formatCtx->pb, formatCtx->filename, URL_WRONLY) < 0)
		//{
		//fprintf(stderr, "Could not open '%s'\n", formatCtx->filename);
		//return -1;
		//}
	}
	ret = av_write_header(formatCtx);
	return ret;
}

int ff_mpeg2ts_muxer_core::add_video_frame(void* encodedData, int encodedDataSize, int64_t pts, bool isKeyFrame)
{
	int streamIdx = 0;      //hardcode
	AVPacket pkt;
	av_init_packet(&pkt);

	static int64_t dts = 0;
	static int duration = 90000 / videoFPS;
	//pkt.pts = pts;
	pkt.pts = AV_NOPTS_VALUE;
	pkt.dts = dts;
	dts += duration;
	pkt.duration = duration;
	//printf("muxer: pts=%lld\n",pts);
	if (pts != AV_NOPTS_VALUE)
	pkt.pts= av_rescale_q(pts, videoCodecTimebase, videoStream->time_base);
	if (isKeyFrame)
		pkt.flags |= AV_PKT_FLAG_KEY;
	pkt.stream_index = streamIdx;
	pkt.data = (uint8_t*)encodedData;
	pkt.size = encodedDataSize;

	int ret = av_interleaved_write_frame(formatCtx, &pkt);
	int len = url_close_dyn_buf(ioctx, &mpegtsOutputBuf);
	AVPacket tspkt;
	av_init_packet(&tspkt);
	tspkt.size = len;
	tspkt.data = mpegtsOutputBuf;
	ret = av_interleaved_write_frame(rtpCtx, &tspkt);

	av_free(mpegtsOutputBuf);
	url_open_dyn_buf(&ioctx);
	formatCtx->pb = ioctx;
	printf("write rtp packet: %d bytes\n", len);
	return ret;
}

int ff_mpeg2ts_muxer_core::end_mux()
{
	av_write_trailer(_format_ctx);
	//int len = url_close_dyn_buf(&ioctx,&mpegtsOutputBuf);
	//av_write_trailer(rtpCtx);
	return 0;

}*/