#ifndef _LIVE_MEDIA_WRAPPER_H_
#define _LIVE_MEDIA_WRAPPER_H_

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <signal.h>
#include "dk_rtsp_client.h"

namespace debuggerking
{
	class rtsp_core : public RTSPClient
	{
	public:
		static rtsp_core * createNew(rtsp_client * front, UsageEnvironment & env, const char * url, const char * username, const char * password, int transport_option, int recv_option, int recv_timeout, float scale, unsigned int http_port_number, bool * kill_flag);

		static void continue_after_client_creation(RTSPClient * param);
		static void continue_after_options(RTSPClient * param, int result_code, char * result_string);
		static void continue_after_describe(RTSPClient * param, int result_code, char * result_string);
		static void continue_after_setup(RTSPClient * param, int result_code, char * result_string);
		static void continue_after_play(RTSPClient * param, int result_code, char * result_string);
		static void continue_after_teardown(RTSPClient * param, int result_code, char * result_string);

		void close(void);

	protected:
		rtsp_core(rtsp_client * front, UsageEnvironment & env, const char * url, const char * username, const char * password, int transport_option, int recv_option, int recv_timeout, float scale, unsigned int http_port_number, bool * kill_flag);
		~rtsp_core(void);

	private:



		//Medium * create_client( UsageEnvironment & env, const char * url, unsigned int http_port, const char * app_name );
		//void assign_client( Medium * client );
		void get_options(RTSPClient::responseHandler * after_func);
		void get_description(RTSPClient::responseHandler * after_func);
		void setup_media_subsession(MediaSubsession * media_subsession, bool rtp_over_tcp, bool force_multicast_unspecified, RTSPClient::responseHandler * after_func);
		void start_playing_session(MediaSession * media_session, double start, double end, float scale, RTSPClient::responseHandler * after_func);
		void start_playing_session(MediaSession * media_session, const char * abs_start_time, const char * abs_end_time, float scale, RTSPClient::responseHandler * after_func);
		void teardown_session(MediaSession * media_session, RTSPClient::responseHandler * after_func);
		void set_user_agent_string(const char * user_agent);

		void setup_streams(void);
		void shutdown(void);

		static void subsession_after_playing(void * param);
		static void subsession_bye_handler(void * param);
		static void session_after_playing(void * param = 0);


		static void session_timer_handler(void * param);
		static void check_packet_arrival(void * param);
		static void check_inter_packet_gaps(void * param);
		static void check_session_timeout_broken_server(void * param);

		static void kill_trigger(void * param);

	private:
		int _transport_option;
		int _recv_option;

		bool * _kill_flag;
		int	_kill_trigger;

		rtsp_client * _front;
		Authenticator * _auth;
		MediaSession * _media_session;

		MediaSubsessionIterator * _iter;

		TaskToken _session_timer_task;
		TaskToken _arrival_check_timer_task;
		TaskToken _inter_packet_gap_check_timer_task;
		TaskToken _session_timeout_broken_server_task;

		int _socket_input_buffer_size;
		bool _made_progress;
		unsigned _session_timeout_parameter;
		//bool _repeat;

		unsigned _inter_packet_gap_max_time;
		unsigned _total_packets_received;

		double _duration;
		double _duration_slot;
		double _init_seek_time;
		char * _init_abs_seek_time;
		float _scale;
		double _end_time;


		bool _shutting_down;
		bool _wait_teardown_response;
		bool _send_keepalives_to_broken_servers;

		char * _sps;
		char * _pps;

	};
};


#endif // LIVE_MEDIA_WRAPPER_H

