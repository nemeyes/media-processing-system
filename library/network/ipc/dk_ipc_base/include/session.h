#ifndef _ABSTRACT_SESSION_H_
#define _ABSTRACT_SESSION_H_

#include <packet.h>
#include <iocp_io_context.h>

#if defined(WITH_DELAYED_TASK)
//#include <delayed_task_queue.h>
#endif

namespace ic
{
#if defined(WITH_WORKING_AS_SERVER)
	class abstract_ipc_server;
#else
	class abstract_ipc_client;
#endif
	class session : public std::enable_shared_from_this<ic::session>
	{
	public:
#if defined(WITH_WORKING_AS_SERVER)
		session(abstract_ipc_server * processor, SOCKET fd);
#else
		session(abstract_ipc_client * processor, SOCKET fd);
#endif
		virtual ~session(void);

		bool shutdown_fd(void);

		void push_send_packet(const char * dst, const char * src, int32_t cmd, char * msg, int32_t length, bool post_send = true);
		void push_front_send_packet(char * msg, int32_t length);
		void front_send_packet(char * msg, int32_t & length);
		void pop_front_send_packet(void);
		void push_recv_packet(const char * msg, int32_t length);

		SOCKET fd(void);
		int32_t mtu(void);
		void fd(SOCKET fd);
		void mtu(int32_t mtu);

		std::shared_ptr<ic::PER_IO_CONTEXT_T> recv_context(void);
		std::shared_ptr<ic::PER_IO_CONTEXT_T> send_context(void);

		const char * ip(void);
		const char * uuid(void);
		const char * name(void);

		void ip(const char * ip);
		void uuid(const char * uuid);
		void name(const char * name);

		void clear_send_queue(void);
		void clear_recv_queue(void);

		bool disconnect_flag(void) const;
		void disconnect_flag(bool flag);
		bool connected_flag(void) const;
		void connected_flag(bool flag);
		bool assoc_flag(void) const;
		void assoc_flag(bool flag);



		int32_t get_hb_period_sec(void) const;
		void set_hb_period_sec(int32_t sec);


		void update_hb_start_time(void);
		void update_hb_end_time(void);
		bool check_hb(void);



	private:
		void data_indication_callback(const char * dst, const char * src, int32_t command_id, const char * msg, size_t length, std::shared_ptr<ic::session> session);
		std::shared_ptr<ic::PER_IO_CONTEXT_T> allocate_io_context(void);

	private:
		char _send_buffer[MAX_SEND_BUFFER_SIZE];
		char _recv_buffer[MAX_RECV_BUFFER_SIZE];
		int32_t _recv_buffer_index;

		std::deque<ic::packet_queue_t*> _send_queue;
		std::map<int32_t, ic::packet_queue_t*> _recv_queue;

		HANDLE _send_lock;
		HANDLE _recv_lock;

#if defined(WITH_WORKING_AS_SERVER)
		abstract_ipc_server * _processor;
#else
		abstract_ipc_client * _processor;
#endif

		int32_t _max_send_queue_size;
		int32_t _max_recv_queue_size;
		int32_t _mtu;

		bool _disconnect;
		bool _connected;
		bool _associated;

		char _ip[20];
		char _uuid[100];
		char _name[100];

		SOCKET _fd;
		std::shared_ptr<ic::PER_IO_CONTEXT_T> _recv_context;
		std::shared_ptr<ic::PER_IO_CONTEXT_T> _send_context;

#if defined(WIN32)
		DWORD _hb_start;
		DWORD _hb_end;
#else
		struct timeval _hb_start;
		struct timeval _hb_end;
		struct timeval _hb_block;
#endif
		int32_t _hb_period;
		int32_t _hb_retry_count;

#if defined(WITH_DELAYED_TASK)
		//delayed_task_queue _dt_queue;
#endif
	};

};





#endif