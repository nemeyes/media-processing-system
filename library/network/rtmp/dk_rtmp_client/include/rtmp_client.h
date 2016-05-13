#ifndef _RTMP_CLIENT_H_
#define _RTMP_CLIENT_H_

#include "dk_rtmp_client.h"
#include "dk_circular_buffer.h"

typedef struct RTMPPacket RTMPPacket;
typedef struct RTMP RTMP;
namespace debuggerking
{
	class rtmp_core
	{
	public:
		typedef struct _pb_buffer_t
		{
			size_t amount;
			_pb_buffer_t * prev;
			_pb_buffer_t * next;
		} pb_buffer_t;

		rtmp_core(rtmp_client * front);
		~rtmp_core(void);

		uint8_t * get_sps(size_t & sps_size);
		uint8_t * get_pps(size_t & pps_size);
		void set_sps(uint8_t * sps, size_t sps_size);
		void set_pps(uint8_t * pps, size_t pps_size);
		void clear_sps(void);
		void clear_pps(void);

		uint8_t * get_configstr(size_t & configstr_size);
		void set_configstr(uint8_t * configstr, size_t configstr_size);

		rtmp_client::rtmp_state state(void);

		int32_t subscribe_begin(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat = true);
		int32_t subscribe_end(void);
		//int32_t pause(void);
		void sb_process_video(const RTMPPacket * packet);
		void sb_process_audio(const RTMPPacket * packet);

		int32_t publish_begin(int32_t vsmt, int32_t asmt, const char * url, const char * username, const char * password);
		int32_t publish_video(uint8_t * bitstream, size_t nb, long long timestamp);
		int32_t publish_audio(uint8_t * bitstream, size_t nb, bool configstr, long long timestamp);
		int32_t publish_end(void);

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

		static void set_bitstream_size(char * nalu, int32_t nalu_size);
		static void get_bitstream_size(char * nalu, int32_t & nalu_size);

		int32_t push_video_send_packet(uint8_t * bs, size_t size);
		int32_t pop_video_send_packet(uint8_t * bs, size_t & size);

		int32_t push_audio_send_packet(uint8_t * bs, size_t size);
		int32_t pop_audio_send_packet(uint8_t * bs, size_t & size);

		int32_t init_pb_buffer(pb_buffer_t * buffer);


	private:
		rtmp_client * _front;
		rtmp_client::rtmp_state _state;

		RTMP * _rtmp;

		char _url[MAX_PATH];
		char _username[MAX_PATH];
		char _password[MAX_PATH];
		int32_t _recv_option;
		bool _repeat;

		uint8_t _sps[100];
		uint8_t _pps[100];
		int32_t _sps_size;
		int32_t _pps_size;

		//char * _recv_buffer;
		//int32_t _recv_buffer_size;

		char * _video_send_buffer;
		char * _audio_send_buffer;
		int32_t _video_send_buffer_size;
		int32_t _audio_send_buffer_size;

		bool _rcv_first_idr;
		bool _change_sps;
		bool _change_pps;


		uint8_t _configstr[100];
		int32_t _configstr_size;
		bool _rcv_first_audio;

		int32_t _vsmt;
		int32_t _asmt;
		pb_buffer_t * _audio_root;
		circular_buffer_t * _audio_queue;
		CRITICAL_SECTION _audio_mutex;

		pb_buffer_t * _video_root;
		circular_buffer_t * _video_queue;
		CRITICAL_SECTION _video_mutex;
	};
};

#endif // _DK_RTMP_CLIENT_H_
