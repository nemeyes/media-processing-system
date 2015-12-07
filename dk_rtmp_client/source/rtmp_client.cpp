#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#endif
#include "rtmp_client.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "rtmp_sys.h"
#include "log.h"
#include "amf.h"
#include "rtmp.h"
#include "stream_parser.h"
#include "rtmp_client.h"


#define DEFAULT_TIMEOUT	30	/* seconds */
#define DEFAULT_BUFTIME	(10 * 60 * 60 * 1000)	/* 10 hours default */
#define DEFAULT_SKIPFRM	0


#define MAX_VIDEO_SIZE 1024*1024*2

#define MAX_CHANNELS	8
#define MAX_SAMPLE_RATE	48000
#define MAX_BIT_DEPTH	16
#define MAX_AUDIO_SIZE	MAX_CHANNELS*MAX_SAMPLE_RATE*MAX_BIT_DEPTH*sizeof(int16_t) / 8


#if defined(WIN32) && defined(_DEBUG)
FILE *netstackdump = 0;
FILE *netstackdump_read = 0;
#endif

class dk_rtmp_client_auto_lock
{
public:
	dk_rtmp_client_auto_lock(CRITICAL_SECTION * lock)
		: _lock(lock)
	{
		::EnterCriticalSection(_lock);
	}

	~dk_rtmp_client_auto_lock(void)
	{
		::LeaveCriticalSection(_lock);
	}
private:
	CRITICAL_SECTION * _lock;
};


rtmp_client::rtmp_client(dk_rtmp_client * front)
	: _front(front)
{
	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);

	_buffer_size = 1024 * 1024 * 2;//2MB
	_buffer = (char*)malloc(_buffer_size);

	::InitializeCriticalSection(&_video_mutex);
	::InitializeCriticalSection(&_audio_mutex);
}

rtmp_client::~rtmp_client(void)
{
	::DeleteCriticalSection(&_video_mutex);
	::DeleteCriticalSection(&_audio_mutex);

	free(_buffer);
	WSACleanup();
}

uint8_t * rtmp_client::get_sps(size_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

uint8_t * rtmp_client::get_pps(size_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

void rtmp_client::set_sps(uint8_t * sps, size_t sps_size)
{
	memset(_sps, 0x00, sizeof(_sps));
	memcpy(_sps, sps, sps_size);
	_sps_size = sps_size;
}

void rtmp_client::set_pps(uint8_t * pps, size_t pps_size)
{
	memset(_pps, 0x00, sizeof(_pps));
	memcpy(_pps, pps, pps_size);
	_pps_size = pps_size;
}

void rtmp_client::clear_sps(void)
{
	memset(_sps, 0x00, sizeof(_sps));
	_sps_size = 0;
}

void rtmp_client::clear_pps(void)
{
	memset(_pps, 0x00, sizeof(_pps));
	_pps_size = 0;
}

dk_rtmp_client::ERR_CODE rtmp_client::subscribe_begin(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat)
{
	if (!url || strlen(url)<1)
		return dk_rtmp_client::ERR_CODE_FAIL;

	memset(_url, 0x00, sizeof(_url));
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	if (strlen(url)>0)
		strcpy_s(_url, url);
	if (strlen(username)>0)
		strcpy_s(_username, username);
	if (strlen(password)>0)
		strcpy_s(_password, password);
	_recv_option = recv_option;
	_repeat = repeat;

#if defined(WIN32)
	unsigned int thread_id;
	_sb_worker = (HANDLE)::_beginthreadex(0, 0, rtmp_client::sb_process_cb, this, 0, &thread_id);
#else
	pthread_create(&_sb_worker, 0, &rtmp_client::sb_process_cb, this);
#endif
	return dk_rtmp_client::ERR_CODE_SUCCESS;
}

dk_rtmp_client::ERR_CODE rtmp_client::subscribe_end(void)
{
	RTMP_ctrlC = true;
#if defined(WIN32)
	::WaitForSingleObject(_sb_worker, INFINITE);
#else
	pthread_join(_worker, 0);
#endif
	return dk_rtmp_client::ERR_CODE_SUCCESS;
}

//dk_rtmp_client::ERR_CODE rtmp_client::pause(void)
//{
//	return dk_rtmp_client::ERR_CODE_SUCCESS;
//}

dk_rtmp_client::ERR_CODE rtmp_client::publish_begin(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T vsmt, dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T asmt, const char * url, const char * username, const char * password)
{
	_vsmt = vsmt;
	_asmt = asmt;
#if defined(WIN32)
	unsigned int thread_id;
	_pb_worker = (HANDLE)::_beginthreadex(0, 0, rtmp_client::pb_process_cb, this, 0, &thread_id);
#else
	pthread_create(&_pb_worker, 0, &rtmp_client::pb_process_cb, this);
#endif
	return dk_rtmp_client::ERR_CODE_SUCCESS;
}

dk_rtmp_client::ERR_CODE rtmp_client::publish_video(uint8_t * bitstream, size_t nb)
{
	return push_video_send_packet(bitstream, nb);
}

dk_rtmp_client::ERR_CODE rtmp_client::publish_audio(uint8_t * bitstream, size_t nb)
{
	return push_audio_send_packet(bitstream, nb);
}

dk_rtmp_client::ERR_CODE rtmp_client::publish_end(void)
{
	RTMP_ctrlC = true;
#if defined(WIN32)
	::WaitForSingleObject(_pb_worker, INFINITE);
#else
	pthread_join(_worker, 0);
#endif
	return dk_rtmp_client::ERR_CODE_SUCCESS;
}

void rtmp_client::sb_process_video(const RTMPPacket * packet)
{
	uint8_t extradata[200] = { 0 };
	size_t extradata_size = 0;
	struct timeval presentation_time = { 0, 0 };
	uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };

	char * packet_body = packet->m_body;
	uint32_t packet_length = packet->m_nBodySize;

	bool keyframe = false;
	if (((packet_body[0] & 0xF0) >> 4) == 0x01)
		keyframe = true;

	if ((packet_body[0] & 0x0F) == 0x07) //AVC
	{
		uint8_t avc_packet_type = packet_body[1]; //0:AVC Sequence header, 1:AVC NALU, 2:AVC end of sequence
		if (avc_packet_type == 0)
		{
			//uint8_t configuration_version = packet_body[5];
			//uint8_t avc_profile_indication = packet_body[6];
			//uint8_t profile_compatibility = packet_body[7];
			//uint8_t avc_level_indication = packet_body[8];
			//uint8_t length_size_minus_one = packet_body[9] & 0x03;
			uint8_t num_sps = packet_body[10] & 0x1F;

			uint16_t sps_size = 0;
			uint16_t pps_size = 0;
			for (uint8_t index = 0; index < num_sps; index++)
			{
				size_t saved_sps_size = 0;
				unsigned char * saved_sps = get_sps(saved_sps_size);

				sps_size = packet_body[11] << 8 | packet_body[12];
				uint8_t * sps = (uint8_t*)&packet_body[13];

				if (saved_sps_size < 1 || !saved_sps)
				{
					memcpy(extradata, start_code, sizeof(start_code));
					memcpy(extradata + sizeof(start_code), sps, sps_size);
					set_sps(extradata, sps_size + 4);
					_change_sps = true;
				}
				else
				{
					if (memcmp(saved_sps, sps, saved_sps_size))
					{
						memcpy(extradata, start_code, sizeof(start_code));
						memcpy(extradata + sizeof(start_code), sps, sps_size);
						set_sps(extradata, sps_size + 4);
						_change_sps = true;
					}
				}
			}

			uint8_t num_pps = packet_body[13 + sps_size];
			for (uint8_t index = 0; index < num_pps; index++)
			{
				size_t saved_pps_size = 0;
				unsigned char * saved_pps = get_pps(saved_pps_size);

				pps_size = packet_body[14 + sps_size] << 8 | packet_body[15 + sps_size];
				uint8_t * pps = (uint8_t*)&packet_body[16 + sps_size];

				if (saved_pps_size < 1 || !saved_pps)
				{
					memcpy(extradata, start_code, sizeof(start_code));
					memcpy(extradata + sizeof(start_code), pps, pps_size);
					set_pps(extradata, pps_size + 4);
					_change_pps = true;
				}
				else
				{
					if (memcmp(saved_pps, pps, saved_pps_size))
					{
						memcpy(extradata, start_code, sizeof(start_code));
						memcpy(extradata + sizeof(start_code), pps, pps_size);
						set_pps(extradata, pps_size + 4);
						_change_pps = true;
					}
				}
			}
		}
		else if (avc_packet_type == 1)
		{
			int32_t remained = packet_length - 5;
			uint8_t * vpacket = (uint8_t*)&packet_body[5];
			while (remained>0)
			{
				uint32_t nalu_size = (vpacket[0] & 0xff) << 24 | (vpacket[1] & 0xff) << 16 | (vpacket[2] & 0xff) << 8 | (vpacket[3] & 0xff);
				uint8_t * nalu = (uint8_t*)&vpacket[4];

				if (!_rcv_first_idr)
				{
					bool is_idr = stream_parser::is_idr(dk_rtmp_client::SUBMEDIA_TYPE_AVC, *nalu & 0x1F);
					if (is_idr && (_change_sps || _change_pps))
					{
						size_t saved_sps_size = 0;
						size_t saved_pps_size = 0;
						uint8_t * saved_sps = get_sps(saved_sps_size);
						uint8_t * saved_pps = get_pps(saved_pps_size);
						if ((saved_sps_size > 0) && (saved_pps_size > 0))
						{
							memcpy(extradata, saved_sps, saved_sps_size);
							memcpy(extradata + saved_sps_size, saved_pps, saved_pps_size);
							extradata_size = saved_sps_size + saved_pps_size;

							memmove(_buffer, extradata, extradata_size);
							memmove(_buffer + extradata_size, start_code, sizeof(start_code));
							memmove(_buffer + extradata_size + sizeof(start_code), nalu, nalu_size);

							_rcv_first_idr = true;
							if (_front)
								_front->on_begin_media(dk_rtmp_client::MEDIA_TYPE_VIDEO, dk_rtmp_client::SUBMEDIA_TYPE_AVC, saved_sps, saved_sps_size, saved_pps, saved_pps_size, (uint8_t*)_buffer, extradata_size + sizeof(start_code) + nalu_size, presentation_time);
							_change_sps = false;
							_change_pps = false;
						}
					}
				}
				else
				{
					size_t saved_sps_size = 0;
					size_t saved_pps_size = 0;
					uint8_t * saved_sps = get_sps(saved_sps_size);
					uint8_t * saved_pps = get_pps(saved_pps_size);
					if (saved_sps_size > 0 && saved_pps_size > 0)
					{
						memcpy(_buffer, start_code, sizeof(start_code));
						memcpy(_buffer + sizeof(start_code), nalu, nalu_size);
						if (_front)
							_front->on_recv_media(dk_rtmp_client::MEDIA_TYPE_VIDEO, dk_rtmp_client::SUBMEDIA_TYPE_AVC, (uint8_t*)_buffer, sizeof(start_code) + nalu_size, presentation_time);
					}
				}

				remained -= 4;//4byte for calcualting nalu_size
				remained -= nalu_size;//nalu size itself
				vpacket += 4;
				vpacket += nalu_size;
			}
		}
		else //end of sequence
		{

		}
	}
}

void rtmp_client::sb_process_audio(const RTMPPacket * packet)
{

}

void rtmp_client::sb_process(void)
{
	do
	{
#ifdef _DEBUG
		netstackdump = fopen("netstackdump", "wb");
		netstackdump_read = fopen("netstackdump_read", "wb");
#endif

		int read_buffer_size = 64 * 1024;
		char * read_buffer = (char *)malloc(read_buffer_size);

		int32_t protocol = RTMP_PROTOCOL_UNDEFINED;
		AVal host = { 0, 0 };
		uint32_t port = 1935;
		AVal playpath = { 0, 0 };
		AVal app = { 0, 0 };
		AVal tc_url = { 0, 0 };
		AVal swf_url = { 0, 0 };
		AVal page_url = { 0, 0 };
		AVal auth = { 0, 0 };
		AVal swf_hash = { 0, 0 };
		int32_t swf_size = 0;
		AVal flash_version = { 0, 0 };
		AVal sock_host = { 0, 0 };
		AVal subscribe_path = { 0, 0 };

		long int timeout = DEFAULT_TIMEOUT;
		int32_t seek = 0;	 // seek position in resume mode, 0 otherwise
		double duration = 0.0;
		uint32_t buffer_time = DEFAULT_BUFTIME; // 10 hours as default

		RTMP rtmp = { 0 };
		RTMP_debuglevel = RTMP_LOGINFO;
		RTMP_Init(&rtmp);
		rtmp.rtmp_client_wrapper = this;

		if (RTMP_ParseURL((char*)_url, &protocol, &host, &port, &playpath, &app))
		{
			if (tc_url.av_len == 0)
			{
				char str[512] = { 0 };
				tc_url.av_len = snprintf(str, 511, "%s://%.*s:%d/%.*s", RTMPProtocolStringsLower[protocol], host.av_len, host.av_val, port, app.av_len, app.av_val);
				tc_url.av_val = (char *)malloc(tc_url.av_len + 1);
				strcpy(tc_url.av_val, str);
			}
			RTMP_SetupStream(&rtmp, protocol, &host, port, &sock_host, &playpath, &tc_url, &swf_url, &page_url, &app, &auth, &swf_hash, swf_size, &flash_version, &subscribe_path, 0, 0, true, timeout);
		}

		clear_sps();
		clear_pps();
		_rcv_first_idr = false;
		_change_sps = false;
		_change_pps = false;

		RTMP_SetBufferMS(&rtmp, buffer_time);
		if (!RTMP_Connect(&rtmp, NULL))
			break;

		if (!RTMP_ConnectStream(&rtmp, seek))
			break;

		RTMP_ctrlC = false;
		int32_t nb_read = 0;
		do
		{
			nb_read = RTMP_Read(&rtmp, read_buffer, read_buffer_size);

			double duration = RTMP_GetDuration(&rtmp);
			//Sleep(duration / 1000.f);

		} while (!RTMP_ctrlC && (nb_read>-1) && RTMP_IsConnected(&rtmp) && !RTMP_IsTimedout(&rtmp));

		RTMP_Close(&rtmp);

		if (read_buffer)
			free(read_buffer);

		if (tc_url.av_len > 0)
			free(tc_url.av_val);

#ifdef _DEBUG
		if (netstackdump != 0)
			fclose(netstackdump);
		if (netstackdump_read != 0)
			fclose(netstackdump_read);
#endif

	} while (0/*_repeat*/);
}

void rtmp_client::pb_process(void)
{
	do
	{
#ifdef _DEBUG
		netstackdump = fopen("netstackdump", "wb");
		netstackdump_read = fopen("netstackdump_read", "wb");
#endif

		char * pb_video_buffer = static_cast<char*>(malloc(MAX_VIDEO_SIZE));
		char * pb_audio_buffer = static_cast<char*>(malloc(MAX_AUDIO_SIZE));

		int32_t protocol = RTMP_PROTOCOL_UNDEFINED;
		AVal host = { 0, 0 };
		uint32_t port = 1935;
		AVal playpath = { 0, 0 };
		AVal app = { 0, 0 };
		AVal tc_url = { 0, 0 };
		AVal swf_url = { 0, 0 };
		AVal page_url = { 0, 0 };
		AVal auth = { 0, 0 };
		AVal swf_hash = { 0, 0 };
		int32_t swf_size = 0;
		AVal flash_version = { 0, 0 };
		AVal sock_host = { 0, 0 };
		AVal subscribe_path = { 0, 0 };

		long int timeout = DEFAULT_TIMEOUT;
		int32_t seek = 0;	 // seek position in resume mode, 0 otherwise
		double duration = 0.0;
		uint32_t buffer_time = DEFAULT_BUFTIME; // 10 hours as default

		RTMP rtmp = { 0 };
		RTMP_debuglevel = RTMP_LOGINFO;
		RTMP_Init(&rtmp);
		rtmp.rtmp_client_wrapper = this;
		RTMP_EnableWrite(&rtmp);

		if (RTMP_ParseURL((char*)_url, &protocol, &host, &port, &playpath, &app))
		{
			if (tc_url.av_len == 0)
			{
				char str[512] = { 0 };
				tc_url.av_len = snprintf(str, 511, "%s://%.*s:%d/%.*s", RTMPProtocolStringsLower[protocol], host.av_len, host.av_val, port, app.av_len, app.av_val);
				tc_url.av_val = (char *)malloc(tc_url.av_len + 1);
				strcpy(tc_url.av_val, str);
			}
			RTMP_SetupStream(&rtmp, protocol, &host, port, &sock_host, &playpath, &tc_url, &swf_url, &page_url, &app, &auth, &swf_hash, swf_size, &flash_version, &subscribe_path, 0, 0, true, timeout);
		}

		RTMP_SetBufferMS(&rtmp, buffer_time);
		if (!RTMP_Connect(&rtmp, NULL))
			break;

		if (!RTMP_ConnectStream(&rtmp, NULL))
			break;

		RTMP_ctrlC = false;
		


		RTMPPacket packet;


		if (_vsmt!=dk_rtmp_client::UNKNOWN_SUBMEDIA_TYPE)
		{
			packet.m_nChannel = 0x04; //source channel
			packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
			packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
			packet.m_body = pb_video_buffer + RTMP_MAX_HEADER_SIZE;


		}

		do
		{
			//nb_read = RTMP_Read(&rtmp, read_buffer, read_buffer_size);

			double duration = RTMP_GetDuration(&rtmp);
			Sleep(duration / 1000.f);

		} while (!RTMP_ctrlC /*&& (nb_read>-1)*/ && RTMP_IsConnected(&rtmp) && !RTMP_IsTimedout(&rtmp));

		RTMP_Close(&rtmp);

		if (tc_url.av_len > 0)
			free(tc_url.av_val);

		if (pb_video_buffer)
			free(pb_video_buffer);
		if (pb_audio_buffer)
			free(pb_audio_buffer);
#ifdef _DEBUG
		if (netstackdump != 0)
			fclose(netstackdump);
		if (netstackdump_read != 0)
			fclose(netstackdump_read);
#endif

	} while (0/*_repeat*/);
}

#if defined(WIN32)
unsigned __stdcall rtmp_client::sb_process_cb(void * param)
{
	rtmp_client * self = static_cast<rtmp_client*>(param);
	self->sb_process();
	return 0;
}
#else
void* rtmp_client::sb_process_cb(void * param)
{
	rtmp_client * self = static_cast<rtmp_client*>(param);
	self->process();
	return 0;
}
#endif

#if defined(WIN32)
unsigned __stdcall rtmp_client::pb_process_cb(void * param)
{
	rtmp_client * self = static_cast<rtmp_client*>(param);
	self->pb_process();
	return 0;
}
#else
void* rtmp_client::pb_process_cb(void * param)
{
	rtmp_client * self = static_cast<rtmp_client*>(param);
	self->pb_process();
	return 0;
}
#endif

dk_rtmp_client::ERR_CODE rtmp_client::push_video_send_packet(uint8_t * bs, size_t size)
{
	dk_rtmp_client::ERR_CODE status = dk_rtmp_client::ERR_CODE_SUCCESS;
	dk_rtmp_client_auto_lock lock(&_video_mutex);
	if (bs && size > 0)
	{
		pb_buffer_t * vbuffer = _video_root;
		vbuffer->amount = MAX_VIDEO_SIZE;
		//move to tail
		do
		{
			if (!vbuffer->next)
				break;
			vbuffer = vbuffer->next;
		} while (1);

		vbuffer->next = static_cast<pb_buffer_t*>(malloc(sizeof(pb_buffer_t)));
		init_pb_buffer(vbuffer->next);
		vbuffer->next->prev = vbuffer;
		vbuffer = vbuffer->next;

		vbuffer->amount = size;
		int32_t result = dk_circular_buffer_write(_video_queue, bs, vbuffer->amount);
		if (result == -1)
		{
			if (vbuffer->prev)
				vbuffer->prev->next = nullptr;
			free(vbuffer);
			vbuffer = nullptr;
			status = dk_rtmp_client::ERR_CODE_FAIL;
		}
	}
	return status;
}

dk_rtmp_client::ERR_CODE rtmp_client::pop_video_send_packet(uint8_t * bs, size_t & size)
{
	dk_rtmp_client::ERR_CODE status = dk_rtmp_client::ERR_CODE_SUCCESS;
	size = 0;
	dk_rtmp_client_auto_lock lock(&_video_mutex);
	pb_buffer_t * vbuffer = _video_root->next;
	if (vbuffer)
	{
		_video_root->next = vbuffer->next;
		if (_video_root->next)
			_video_root->next->prev = _video_root;

		int32_t result = dk_circular_buffer_read(_video_queue, bs, vbuffer->amount);
		if (result == -1)
			status = dk_rtmp_client::ERR_CODE_FAIL;

		size = vbuffer->amount;
		free(vbuffer);
	}
	return status;
}

dk_rtmp_client::ERR_CODE rtmp_client::push_audio_send_packet(uint8_t * bs, size_t size)
{
	dk_rtmp_client::ERR_CODE status = dk_rtmp_client::ERR_CODE_SUCCESS;
	dk_rtmp_client_auto_lock lock(&_audio_mutex);
	if (bs && size > 0)
	{
		pb_buffer_t * abuffer = _audio_root;
		abuffer->amount = MAX_AUDIO_SIZE;
		//move to tail
		do
		{
			if (!abuffer->next)
				break;
			abuffer = abuffer->next;
		} while (1);

		abuffer->next = static_cast<pb_buffer_t*>(malloc(sizeof(pb_buffer_t)));
		init_pb_buffer(abuffer->next);
		abuffer->next->prev = abuffer;
		abuffer = abuffer->next;

		abuffer->amount = size;
		int32_t result = dk_circular_buffer_write(_audio_queue, bs, abuffer->amount);
		if (result == -1)
		{
			if (abuffer->prev)
				abuffer->prev->next = nullptr;
			free(abuffer);
			abuffer = nullptr;
			status = dk_rtmp_client::ERR_CODE_FAIL;
		}
	}
	return status;
}

dk_rtmp_client::ERR_CODE rtmp_client::pop_audio_send_packet(uint8_t * bs, size_t & size)
{
	dk_rtmp_client::ERR_CODE status = dk_rtmp_client::ERR_CODE_SUCCESS;
	size = 0;
	dk_rtmp_client_auto_lock lock(&_audio_mutex);
	pb_buffer_t * abuffer = _audio_root->next;
	if (abuffer)
	{
		_audio_root->next = abuffer->next;
		if (_audio_root->next)
			_audio_root->next->prev = _audio_root;

		int32_t result = dk_circular_buffer_read(_audio_queue, bs, abuffer->amount);
		if (result == -1)
			status = dk_rtmp_client::ERR_CODE_FAIL;

		size = abuffer->amount;
		free(abuffer);
	}
	return status;
}

dk_rtmp_client::ERR_CODE rtmp_client::init_pb_buffer(pb_buffer_t * buffer)
{
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return dk_rtmp_client::ERR_CODE_SUCCESS;
}
