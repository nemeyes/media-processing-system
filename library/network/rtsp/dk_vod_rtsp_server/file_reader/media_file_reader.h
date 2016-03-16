#ifndef _MEDIA_FILE_READER_H_
#define _MEDIA_FILE_READER_H_

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavutil/opt.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/time.h>
}

class media_file_reader
{
public:
	typedef struct _packet_queue_t
	{
		AVPacketList *first_pkt, *last_pkt;
		int32_t nb_packets;
		int32_t size;
		HANDLE lock;
	} packet_queue_t;

	typedef enum _media_type
	{
		media_type_video = 0,
		media_type_audio
	} media_type;

	typedef enum _video_submedia_type
	{
		unknown_video_type = -1,
		vsubmedia_type_jpeg = 0,
		vsubmedia_type_mpeg4,
		vsubmedia_type_h264,
		vsubmedia_type_hevc,
	} vsubmedia_type;

	typedef enum _asubmedia_type
	{
		unknown_audio_type = -1,
		asubmedia_type_mp3 = 0,
		asubmedia_type_aac,
	} asubmedia_type;

	typedef enum _err_code
	{
		err_code_success = 0,
		err_code_fail
	} err_code;


	media_file_reader(void);
	virtual ~media_file_reader(void);

	bool open(const char * stream_name, long long timestamp, media_file_reader::vsubmedia_type & vsubmedia_type, media_file_reader::asubmedia_type & asubmedia_type);
	bool close(void);
	bool read(media_file_reader::media_type mt, uint8_t * data, size_t data_capacity, size_t & data_size, long long & timestamp);

	void packet_queue_init(media_file_reader::packet_queue_t * q);
	int32_t packet_queue_push(media_file_reader::packet_queue_t * q, AVPacket * pkt);
	int32_t packet_queue_pop(media_file_reader::packet_queue_t * q, AVPacket * pkt);

	void set_vps(uint8_t * vps, size_t size);
	void set_sps(uint8_t * sps, size_t size);
	void set_pps(uint8_t * pps, size_t size);
	const uint8_t * get_vps(size_t & size);
	const uint8_t * get_sps(size_t & size);
	const uint8_t * get_pps(size_t & size);



private:
	unsigned static __stdcall process_cb(void * param);
	void process(void);

	static bool is_vps(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type);
	static bool is_sps(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type);
	static bool is_pps(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type);
	static bool is_idr(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type);
	static bool is_vlc(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type);
	static const int32_t find_nal_unit(uint8_t * bitstream, size_t size, int32_t * nal_start, int32_t * nal_end);
	static const uint8_t * find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state);

private:
	char _stream_name[250];

	AVFormatContext * _format_ctx;

	AVCodecContext * _video_ctx;
	int32_t _video_stream_index;
	AVStream * _video_stream;

	//double _video_timer;
	double _video_last_dts;
	//double _video_last_delay;
	//double _video_current_dts;
	//double _video_current_dts_time;
	//double _video_clock; //dts of last bitstream

	bool _video_recv_keyframe;
	uint8_t _video_extradata[100];
	size_t _video_extradata_size;


	int32_t _audio_buffer_size;
	int32_t _audio_buffer_index;


	AVCodecContext * _audio_ctx;
	int32_t _audio_stream_index;
	AVStream * _audio_stream;
	/*
	double _audio_timer;
	double _audio_last_dts;
	double _audio_last_delay;
	double _audio_current_dts;
	double _audio_current_dts_time;
	double _audio_clock; //dts of last bitstream

	bool _audio_recv_sample;
	uint8_t _audio_extradata[20];
	size_t _audio_extradata_size;
	*/

	media_file_reader::vsubmedia_type _vsubmedia_type;
	media_file_reader::asubmedia_type _asubmedia_type;

	media_file_reader::packet_queue_t _video_packet_queue;
	media_file_reader::packet_queue_t _audio_packet_queue;

	uint8_t _vps[200];
	uint8_t _sps[200];
	uint8_t _pps[200];
	size_t _vps_size;
	size_t _sps_size;
	size_t _pps_size;

	uint8_t * _video_buffer;
	uint8_t * _audio_buffer;

	bool _run;
	bool _run_video;
	bool _run_audio;
	HANDLE _thread;
};











#endif