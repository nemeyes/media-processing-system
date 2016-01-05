#ifndef _RTMP_CLIENT_H_
#define _RTMP_CLIENT_H_

#include "dk_rtmp_client.h"
#include "dk_circular_buffer.h"

typedef struct RTMPPacket RTMPPacket;
class rtmp_client
{
public:
	typedef struct _pb_buffer_t
	{
		size_t amount;
		_pb_buffer_t * prev;
		_pb_buffer_t * next;
	} pb_buffer_t;

	rtmp_client(dk_rtmp_client * front);
	~rtmp_client(void);

	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);
	void set_sps(uint8_t * sps, size_t sps_size);
	void set_pps(uint8_t * pps, size_t pps_size);
	void clear_sps(void);
	void clear_pps(void);

	dk_rtmp_client::STATE_T state(void);

	dk_rtmp_client::ERR_CODE subscribe_begin(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat = true);
	dk_rtmp_client::ERR_CODE subscribe_end(void);
	//dk_rtmp_client::ERROR_CODE pause(void);
	void sb_process_video(const RTMPPacket * packet);
	void sb_process_audio(const RTMPPacket * packet);

	dk_rtmp_client::ERR_CODE publish_begin(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T vsmt, dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T asmt, const char * url, const char * username, const char * password);
	dk_rtmp_client::ERR_CODE publish_video(uint8_t * bitstream, size_t nb);
	dk_rtmp_client::ERR_CODE publish_audio(uint8_t * bitstream, size_t nb);
	dk_rtmp_client::ERR_CODE publish_end(void);

private:

	void sb_process(void);
#if !defined(WIN32)
	static void* sb_process_cb(void * param);
#else
	static unsigned __stdcall sb_process_cb(void * param);
#endif
#if !defined(WIN32)
	pthread_t _sb_worker;
#else
	void * _sb_worker;
#endif

	void pb_process(void);
#if !defined(WIN32)
	static void* pb_process_cb(void * param);
#else
	static unsigned __stdcall pb_process_cb(void * param);
#endif
#if !defined(WIN32)
	pthread_t _pb_worker;
#else
	void * _pb_worker;
#endif

	dk_rtmp_client::ERR_CODE push_video_send_packet(uint8_t * bs, size_t size);
	dk_rtmp_client::ERR_CODE pop_video_send_packet(uint8_t * bs, size_t & size);

	dk_rtmp_client::ERR_CODE push_audio_send_packet(uint8_t * bs, size_t size);
	dk_rtmp_client::ERR_CODE pop_audio_send_packet(uint8_t * bs, size_t & size);

	dk_rtmp_client::ERR_CODE init_pb_buffer(pb_buffer_t * buffer);
	

private:
	dk_rtmp_client * _front;
	dk_rtmp_client::STATE_T _state;

	char _url[260];
	char _username[260];
	char _password[260];
	int32_t _recv_option;
	bool _repeat;

	uint8_t _sps[100];
	uint8_t _pps[100];
	int32_t _sps_size;
	int32_t _pps_size;

	char * _buffer;
	int32_t _buffer_size;

	//bool _first;
	bool _rcv_first_idr;
	bool _change_sps;
	bool _change_pps;



	dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T _vsmt;
	dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T _asmt;
	pb_buffer_t * _audio_root;
	dk_circular_buffer_t * _audio_queue;
	CRITICAL_SECTION _audio_mutex;

	pb_buffer_t * _video_root;
	dk_circular_buffer_t * _video_queue;
	CRITICAL_SECTION _video_mutex;


};

#endif // _DK_RTMP_CLIENT_H_
