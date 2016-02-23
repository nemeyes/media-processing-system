#ifndef _FF_DEMUXER_H_
#define _FF_DEMUXER_H_

#include "dk_file_demuxer.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
}

class ff_demuxer
{
public:
	ff_demuxer(dk_file_demuxer * front);
	virtual ~ff_demuxer(void);

	dk_file_demuxer::ERR_CODE play(const char * filepath);
	dk_file_demuxer::ERR_CODE stop(void);

private:
	unsigned static __stdcall process_cb(void * param);
	void process(void);

private:
	dk_file_demuxer * _front;

	AVFormatContext * _format_ctx;

	AVCodecContext * _video_ctx;
	int32_t _video_stream_index;
	AVStream * _video_stream;
	int64_t _video_pts;
	double _video_clock; //pts of last decoded frame
	bool _change_sps;
	bool _change_pps;
	bool _is_first_idr_rcvd;
	uint8_t _video_extradata[400];
	size_t _video_extradata_size;



	int32_t _audio_stream_index;
	AVStream * _audio_stream;

	dk_file_demuxer::VIDEO_SUBMEDIA_TYPE_T _vsubmedia_type;
	dk_file_demuxer::AUDIO_SUBMEDIA_TYPE_T _asubmedia_type;



	bool _run;
	HANDLE _thread;

};




















#endif
