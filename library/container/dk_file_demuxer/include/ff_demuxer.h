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
#include <libavutil/time.h>

}

class ff_demuxer
{
	typedef struct _PACKET_QUEUE_T
	{
		AVPacketList *first_pkt, *last_pkt;
		int32_t nb_packets;
		int32_t size;
		HANDLE lock;
	} PACKET_QUEUE_T;

public:
	ff_demuxer(dk_file_demuxer * front);
	virtual ~ff_demuxer(void);

	dk_file_demuxer::ERR_CODE play(const char * filepath);
	dk_file_demuxer::ERR_CODE stop(void);

private:
	unsigned static __stdcall process_cb(void * param);
	unsigned static __stdcall process_video_cb(void * param);
	unsigned static __stdcall process_audio_cb(void * param);

	void process(void);
	void process_video(void);
	void process_audio(void);

	void packet_queue_init(PACKET_QUEUE_T * q);
	int32_t packet_queue_push(PACKET_QUEUE_T * q, AVPacket * pkt);
	int32_t packet_queue_pop(PACKET_QUEUE_T * q, AVPacket * pkt);

	double get_audio_clock(void);
	double get_video_clock(void);
	double get_master_clock(void);
private:
	dk_file_demuxer * _front;

	AVFormatContext * _format_ctx;

	AVCodecContext * _video_ctx;
	int32_t _video_stream_index;
	AVStream * _video_stream;

	double _video_timer;
	double _video_last_dts;
	double _video_last_delay;
	double _video_current_dts;
	double _video_current_dts_time;
	double _video_clock; //dts of last bitstream
	PACKET_QUEUE_T _video_packet_queue;

	bool _video_recv_keyframe;
	uint8_t _video_extradata[100];
	size_t _video_extradata_size;


	AVCodecContext * _audio_ctx;
	int32_t _audio_stream_index;
	AVStream * _audio_stream;


	int32_t _audio_buffer_size;
	int32_t _audio_buffer_index;

	double _audio_timer;
	double _audio_last_dts;
	double _audio_last_delay;
	double _audio_current_dts;
	double _audio_current_dts_time;
	double _audio_clock; //dts of last bitstream
	PACKET_QUEUE_T _audio_packet_queue;

	bool _audio_recv_sample;
	uint8_t _audio_extradata[20];
	size_t _audio_extradata_size;

	dk_file_demuxer::VIDEO_SUBMEDIA_TYPE_T _vsubmedia_type;
	dk_file_demuxer::AUDIO_SUBMEDIA_TYPE_T _asubmedia_type;

	bool _run;
	bool _run_video;
	bool _run_audio;
	HANDLE _thread;
	HANDLE _thread_video;
	HANDLE _thread_audio;


	uint8_t * _video_buffer;
	uint8_t * _audio_buffer;

};




















#endif
