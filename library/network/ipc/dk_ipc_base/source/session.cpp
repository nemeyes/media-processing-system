#include <session.h>
#include <scoped_lock.h>
#if defined(WITH_WORKING_AS_SERVER)
#include "abstract_ipc_server.h"
#else
#include "abstract_ipc_client.h"
#endif

#if defined(WITH_WORKING_AS_SERVER)
ic::session::session(abstract_ipc_server * processor, SOCKET fd)
#else
ic::session::session(abstract_ipc_client * processor, SOCKET fd)
#endif
	: _processor(processor)
	, _recv_buffer_index(0)
	, _max_send_queue_size(50)
	, _max_recv_queue_size(50)
	, _mtu(1500)
	, _fd(fd)
	, _disconnect(false)
	, _connected(false)
	, _associated(false)
	, _hb_retry_count(3)
{
	memset(_send_buffer, 0x00, MAX_SEND_BUFFER_SIZE);
	memset(_recv_buffer, 0x00, MAX_RECV_BUFFER_SIZE);
	_recv_context = allocate_io_context();
	_send_context = allocate_io_context();

	memset(_ip, 0x00, sizeof(_ip));
	memset(_uuid, 0x00, sizeof(_uuid));
	memset(_name, 0x00, sizeof(_name));

	clear_send_queue();
	clear_recv_queue();

	_send_lock = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	_recv_lock = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	::SetEvent(_send_lock);
	::SetEvent(_recv_lock);

	update_hb_start_time();
	update_hb_end_time();
}

ic::session::~session(void)
{
	::CloseHandle(_send_lock);
	::CloseHandle(_recv_lock);
	shutdown_fd();
}

bool ic::session::shutdown_fd(void)
{
#if defined(WIN32)
	if (_fd == INVALID_SOCKET) /* Already clsoed */
		return false;

	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 0;
	setsockopt(_fd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

	if (closesocket(_fd) == 0)
	{
		clear_send_queue();
		clear_recv_queue();
		_fd = INVALID_SOCKET;
		return true;
	}
	else
	{
		int32_t code = WSAGetLastError();
		clear_send_queue();
		clear_recv_queue();
		_fd = INVALID_SOCKET;
		return false;
	}
#else
	int fd = _fd;
	if (fd <= 0) /* Already clsoed */
		return 0;

	shutdown(_fd, SHUT_RDWR);
	if (close(_fd) != 0)
	{
		//printf( "socket close failed [%d].\n", fd );
		LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("system"), "socket close failed [" << fd << "]");
		clear_send_queue();
		clear_recv_queue();
		_fd = -1;
		return -1;
	}
	else
	{
		//printf( "socket close success [%d].\n", fd );
		LOG4CPLUS_TRACE(log4cplus::Logger::getInstance("system"), "socket close success [" << fd << "]");
		clear_send_queue();
		clear_recv_queue();
		_fd = -1;
		return 0;
	}
#endif
}

void ic::session::push_send_packet(int32_t cmd, char * msg, int32_t length, bool post_send)
{
	if (_send_queue.size()>size_t(_max_send_queue_size))
	{
		//ipc_elog("maximum send buffer size[%d] is exceeded[%zd].\n", _max_send_queue_size, _send_queue.size());
		if (post_send)
			_processor->data_request(shared_from_this());
		return;
	}

	char * pkt_payload = msg;
	int32_t pkt_header_size = sizeof(ic::packet_header_t);
	int32_t pkt_payload_size = _mtu - pkt_header_size;
	int32_t pkt_size = _mtu;
	int32_t pkt_count = length / (_mtu - pkt_header_size);
	int32_t last_pkt_size = length % (_mtu - pkt_header_size);
	if (last_pkt_size>0)
		pkt_count++;
	if (pkt_count<1)
		return;

	int pkt_seed = rand();
	for (int i = 0; i<pkt_count; i++)
	{
		ic::packet_queue_t * queue_pkt = static_cast<ic::packet_queue_t*>(malloc(sizeof(ic::packet_queue_t)));
		memset(queue_pkt, 0x00, sizeof(ic::packet_queue_t));

		if (i == (pkt_count - 1))//last packet			
		{
			pkt_payload_size = last_pkt_size;
			queue_pkt->header.seed = pkt_seed;
			queue_pkt->header.seq = i;
			queue_pkt->header.flag = FLAG_PKT_END;
			queue_pkt->header.command = cmd;
			queue_pkt->header.length = pkt_payload_size;
			queue_pkt->msg = static_cast<char*>(malloc(queue_pkt->header.length));
			memcpy(queue_pkt->msg, pkt_payload, queue_pkt->header.length);
		}
		else
		{
			queue_pkt->header.seed = pkt_seed;
			queue_pkt->header.seq = i;
			if (i == 0)
				queue_pkt->header.flag = FLAG_PKT_BEGIN;
			else
				queue_pkt->header.flag = FLAG_PKT_PLAY;
			queue_pkt->header.command = cmd;
			queue_pkt->header.length = pkt_payload_size;
			queue_pkt->msg = static_cast<char*>(malloc(queue_pkt->header.length));
			memcpy(queue_pkt->msg, pkt_payload, queue_pkt->header.length);
			pkt_payload += pkt_payload_size;
		}
		if (queue_pkt && queue_pkt->header.length>0)
		{
			scoped_lock lock(_send_lock);
			_send_queue.push_back(queue_pkt);
		}
	}
	if (post_send)
		_processor->data_request(shared_from_this());
}

//use for fragmented packet only
void ic::session::push_front_send_packet(char * msg, int32_t length)
{
	//message's original source
	/*
	ic::packet_queue_t * queue_pkt = _send_queue.front();
	int32_t seed = queue_pkt->seed;
	int32_t seq = queue_pkt->seq;
	int32_t flag = queue_pkt->flag;
	int32_t cmd = queue_pkt->command;
	pop_front_send_packet();

	int32_t pkt_header_size = sizeof(ic::packet_header_t);
	queue_pkt = static_cast<ic::packet_queue_t*>(malloc(sizeof(ic::packet_queue_t)));
	queue_pkt->seed = seed;
	queue_pkt->seq = seq;
	queue_pkt->flag = flag;
	queue_pkt->command = cmd;
	queue_pkt->length = size;
	int32_t msg_length = pkt_header_size + queue_pkt->length;
	queue_pkt->msg = static_cast<char*>(malloc(msg_length));
	memcpy(queue_pkt->msg, &header, pkt_header_size);
	memcpy(queue_pkt->msg + pkt_header_size, msg, msg_length);

	_send_queue.push_front(queue_pkt);
	*/
}

void ic::session::front_send_packet(char * msg, int32_t & length)
{
	length = 0;
	scoped_lock lock(_send_lock);
	if (_send_queue.size()>0)
	{
		ic::packet_queue_t * queue_pkt = _send_queue.front();
		if (queue_pkt)
		{
			int32_t packet_header_size = sizeof(ic::packet_header_t);
			memcpy(msg, &(queue_pkt->header), packet_header_size);
			memcpy(msg + packet_header_size, queue_pkt->msg, queue_pkt->header.length);
			length = packet_header_size + queue_pkt->header.length;
		}
	}
}

void ic::session::pop_front_send_packet(void)
{
	scoped_lock lock(_send_lock);
	if (_send_queue.size()>0)
	{
		ic::packet_queue_t * queue_pkt = _send_queue.front();
		if (queue_pkt)
		{
			if (queue_pkt->msg)
			{
				free(queue_pkt->msg);
				queue_pkt->msg = nullptr;
			}
			free(queue_pkt);
			queue_pkt = nullptr;
		}
		_send_queue.pop_front();
	}
}

void ic::session::push_recv_packet(const char * msg, int32_t length)
{
	memcpy(_recv_buffer + _recv_buffer_index, msg, length);
	_recv_buffer_index += length;

	int32_t pkt_header_size = sizeof(ic::packet_header_t);
	ic::packet_queue_t * queue_pkt = 0;
	do
	{
		if (_recv_buffer_index<pkt_header_size)
			return;

		ic::packet_header_t header;
		memcpy(&header, _recv_buffer, pkt_header_size);
		if (header.length>(_recv_buffer_index - pkt_header_size))
			return;

		if (header.length < 1)
			return;


		bool exist = false;
		ic::packet_queue_t * prev_queue_pkt = 0;
		{
			scoped_lock lock(_recv_lock);
			std::map<int32_t, ic::packet_queue_t*>::iterator iter = _recv_queue.find(header.seed);
			if (iter != _recv_queue.end())
			{
				prev_queue_pkt = iter->second;
				exist = true;
			}
			else
				exist = false;
		}
		if (exist)
		{
			int32_t prev_length = 0;
			queue_pkt = prev_queue_pkt;
			prev_length = queue_pkt->header.length;
			queue_pkt->header.flag = header.flag;
			queue_pkt->header.length += header.length;
			if (queue_pkt->header.length>0)
			{
				queue_pkt->msg = static_cast<char*>(realloc(queue_pkt->msg, queue_pkt->header.length));
				memmove(queue_pkt->msg + prev_length, _recv_buffer + pkt_header_size, header.length);

				_recv_buffer_index = _recv_buffer_index - (pkt_header_size + header.length);
				memmove(_recv_buffer, _recv_buffer + (pkt_header_size + header.length), _recv_buffer_index);
			}

			if (queue_pkt->header.flag & FLAG_PKT_END)
			{
				{
					scoped_lock lock(_recv_lock);
					_recv_queue.erase(header.seed);
				}

				data_indication_callback(queue_pkt->header.dst, queue_pkt->header.src, queue_pkt->header.command, queue_pkt->msg, queue_pkt->header.length, shared_from_this());

				if (queue_pkt)
				{
					if (queue_pkt->msg)
					{
						free(queue_pkt->msg);
						queue_pkt->msg = nullptr;
					}
					free(queue_pkt);
					queue_pkt = nullptr;
				}

			}
		}
		else
		{
			queue_pkt = static_cast<ic::packet_queue_t*>(malloc(sizeof(ic::packet_queue_t)));
			memset(queue_pkt, 0x00, sizeof(ic::packet_queue_t));
			queue_pkt->header.seed = header.seed;
			queue_pkt->header.seq = header.seq;
			queue_pkt->header.flag = header.flag;
			queue_pkt->header.command = header.command;
			queue_pkt->header.length = header.length;
			if (queue_pkt->header.length>0)
			{
				queue_pkt->msg = static_cast<char*>(malloc(queue_pkt->header.length));
				memmove(queue_pkt->msg, _recv_buffer + pkt_header_size, header.length);

				_recv_buffer_index = _recv_buffer_index - (pkt_header_size + header.length);
				memmove(_recv_buffer, _recv_buffer + (pkt_header_size + header.length), _recv_buffer_index);
			}

			if (queue_pkt->header.flag & FLAG_PKT_END)
			{
				data_indication_callback(queue_pkt->header.dst, queue_pkt->header.src, queue_pkt->header.command, queue_pkt->msg, queue_pkt->header.length, shared_from_this());

				if (queue_pkt)
				{
					if (queue_pkt->msg)
					{
						free(queue_pkt->msg);
						queue_pkt->msg = nullptr;
					}
					free(queue_pkt);
					queue_pkt = nullptr;
				}
			}
			else
			{
				scoped_lock lock(_recv_lock);
				_recv_queue.insert(std::make_pair(header.seed, queue_pkt));
			}
		}
	} while (_recv_buffer_index>0);
}

SOCKET ic::session::get_fd(void)
{
	return _fd;
}

void ic::session::set_fd(SOCKET fd)
{
	_fd = fd;
}

std::shared_ptr<ic::PER_IO_CONTEXT_T> ic::session::recv_context(void)
{
	return _recv_context;
}

std::shared_ptr<ic::PER_IO_CONTEXT_T> ic::session::send_context(void)
{
	return _send_context;
}

int32_t ic::session::get_mtu(void)
{
	return _mtu;
}

void ic::session::set_mtu(int32_t mtu)
{
	_mtu = mtu;
}

const char * ic::session::get_ip(void)
{
	return _ip;
}

const char * ic::session::get_uuid(void)
{
	return _uuid;
}

const char * ic::session::get_name(void)
{
	return _name;
}

void ic::session::set_ip(const char * ip)
{
	strncpy_s(_ip, ip, sizeof(_ip));
}

void ic::session::set_uuid(const char * uuid)
{
	strncpy_s(_uuid, uuid, sizeof(_uuid));
}

void ic::session::set_name(const char * name)
{
	strncpy_s(_name, name, sizeof(_name));
}

void ic::session::clear_send_queue(void)
{
	std::deque<ic::packet_queue_t*>::iterator iter;
	for (iter = _send_queue.begin(); iter != _send_queue.end(); iter++)
	{
		packet_queue_t * queue_pkt = *iter;
		if (queue_pkt)
		{
			if (queue_pkt->msg)
			{
				free(queue_pkt->msg);
				queue_pkt->msg = nullptr;
			}
			free(queue_pkt);
			queue_pkt = nullptr;
		}
	}
	_send_queue.clear();
}

void ic::session::clear_recv_queue(void)
{
	ic::packet_queue_t * queue_pkt = nullptr;
	std::map<int32_t, ic::packet_queue_t*>::iterator iter;
	for (iter = _recv_queue.begin(); iter != _recv_queue.end(); iter++)
	{
		queue_pkt = iter->second;
		if (queue_pkt)
		{
			if (queue_pkt->msg)
			{
				free(queue_pkt->msg);
				queue_pkt->msg = nullptr;
			}
			free(queue_pkt);
			queue_pkt = nullptr;
		}
	}
	_recv_queue.clear();
}

bool ic::session::get_disconnect_flag(void) const
{
	return _disconnect;
}

void ic::session::set_disconnect_flag(bool flag)
{
	_disconnect = flag;
}

bool ic::session::get_connected_flag(void) const
{
	return _connected;
}

void ic::session::set_connected_flag(bool flag)
{
	_connected = flag;
}

bool ic::session::get_assoc_flag(void) const
{
	return _associated;
}

void ic::session::set_assoc_flag(bool flag)
{
	_associated = flag;
}

int32_t ic::session::get_hb_period_sec(void) const
{
#if defined(WIN32)
	return _hb_period/1000;
#else
	return _hb_period;
#endif
}

void ic::session::set_hb_period_sec(int32_t sec)
{
#if defined(WIN32)
	_hb_period = sec*1000;
#else
	_hb_period = sec;
#endif
}

void ic::session::update_hb_start_time(void)
{
	_hb_start = GetTickCount();
}

void ic::session::update_hb_end_time(void)
{
	_hb_end = GetTickCount();
}

bool ic::session::check_hb(void)
{
#if defined(WIN32)
	DWORD hb_diff = _hb_end - _hb_start;
	if (hb_diff >= _hb_period)
		return true;
	else
		return false;

#else
	struct timeval hb_diff;
	struct timeval block_diff;
	timersub(&_hb_end, &_hb_start, &hb_diff);
	timersub(&_hb_end, &_hb_block, &block_diff);

	if (block_diff.tv_sec>5) //prevent more than 1packet
	{
		gettimeofday(&_hb_block, 0);
		if (hb_diff.tv_sec >= _hb_period)
			return true;
		else
			return false;
	}
	else
		return false;
#endif
}

void ic::session::data_indication_callback(const char * dst, const char * src, int32_t command_id, const char * msg, size_t length, std::shared_ptr<ic::session> session)
{
	_processor->data_indication_callback(dst, src, command_id, msg, length, session);
}

std::shared_ptr<ic::PER_IO_CONTEXT_T> ic::session::allocate_io_context(void)
{
	std::shared_ptr<ic::PER_IO_CONTEXT_T> io_context(new ic::PER_IO_CONTEXT_T);
	return io_context;
}
