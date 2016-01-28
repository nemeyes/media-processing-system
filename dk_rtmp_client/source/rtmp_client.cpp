#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#endif
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <dk_auto_lock.h>

#include "rtmp_sys.h"
#include "log.h"
#include "amf.h"
#include "rtmp.h"
#include "stream_parser.h"
#include "rtmp_client.h"


#define DEFAULT_TIMEOUT	30	/* seconds */
#define DEFAULT_BUFTIME	500//(10 * 60 * 60 * 1000)	/* 10 hours default */
#define DEFAULT_SKIPFRM	0

#define DEFAULT_METADATA_BUFFER_SIZE 2048

#define MAX_VIDEO_SIZE 1024*1024/*2*/

#define MAX_CHANNELS	8
#define MAX_SAMPLE_RATE	48000
#define MAX_BIT_DEPTH	16
#define MAX_AUDIO_SIZE	MAX_CHANNELS*MAX_SAMPLE_RATE*MAX_BIT_DEPTH*sizeof(int16_t) / 8


#define AVC_SEQUENCE_HEADER	0
#define AVC_NALU			1
#define AVC_ENC_OF_SEQUENCE	2

#define AAC_SEQUENCE_HEADER	0
#define AAC_RAW				1


#define SR_5_kHz	0
#define SR_11_kHZ	1
#define SR_22_kHz	2
#define SR_44_kHz	3


#if defined(WIN32) && defined(_DEBUG)
FILE *netstackdump = 0;
FILE *netstackdump_read = 0;
#endif

rtmp_client::rtmp_client(dk_rtmp_client * front)
	: _front(front)
	, _state(dk_rtmp_client::STATE_STOPPED)
	, _rtmp(nullptr)
{
	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);

	//_recv_buffer_size = 1024 * 1024 * 2;//2MB
	//_recv_buffer = (char*)malloc(_recv_buffer_size);

	_video_send_buffer_size = 1024 * 1024 * 2;//2MB
	_video_send_buffer = (char*)malloc(_video_send_buffer_size);

	_audio_send_buffer_size = 44100 * 2 * 8;//samplerate * bitdepth * channels
	_audio_send_buffer = (char*)malloc(_audio_send_buffer_size);

	::InitializeCriticalSection(&_video_mutex);
	::InitializeCriticalSection(&_audio_mutex);
}

rtmp_client::~rtmp_client(void)
{
	if (!RTMP_ctrlC)
		subscribe_end();

	::DeleteCriticalSection(&_video_mutex);
	::DeleteCriticalSection(&_audio_mutex);

	free(_audio_send_buffer);
	_audio_send_buffer = nullptr;
	_audio_send_buffer_size = 0;

	free(_video_send_buffer);
	_video_send_buffer = nullptr;
	_video_send_buffer_size = 0;

	//free(_recv_buffer);
	//_recv_buffer = nullptr;
	//_recv_buffer_size = 0;

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

uint8_t * rtmp_client::get_configstr(size_t & configstr_size)
{
	configstr_size = _configstr_size;
	return _configstr;
}

void rtmp_client::set_configstr(uint8_t * configstr, size_t configstr_size)
{
	memset(_configstr, 0x00, sizeof(_configstr));
	memcpy(_configstr, configstr, configstr_size);
	_configstr_size = configstr_size;
}

dk_rtmp_client::STATE_T rtmp_client::state(void)
{
	return _state;
}

dk_rtmp_client::ERR_CODE rtmp_client::subscribe_begin(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat)
{
	if ((_state == dk_rtmp_client::STATE_SUBSCRIBING) || (_state == dk_rtmp_client::STATE_PUBLISHING))
		return dk_rtmp_client::ERR_CODE_SUCCESS;

	if (!url || strlen(url)<1)
		return dk_rtmp_client::ERR_CODE_FAIL;

	memset(_url, 0x00, sizeof(_url));
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	if (strlen(url)>0)
		strcpy_s(_url, url);
	if (username && strlen(username)>0)
		strcpy_s(_username, username);
	if (password && strlen(password)>0)
		strcpy_s(_password, password);
	_recv_option = recv_option;
	_repeat = repeat;

#if defined(WIN32)
	unsigned int thread_id;
	_sb_worker = (HANDLE)::_beginthreadex(0, 0, rtmp_client::sb_process_cb, this, 0, &thread_id);
#else
	pthread_create(&_sb_worker, 0, &rtmp_client::sb_process_cb, this);
#endif

	_state = dk_rtmp_client::STATE_SUBSCRIBING;
	return dk_rtmp_client::ERR_CODE_SUCCESS;
}

dk_rtmp_client::ERR_CODE rtmp_client::subscribe_end(void)
{
	if (_state == dk_rtmp_client::STATE_STOPPED)
		return dk_rtmp_client::ERR_CODE_SUCCESS;

	_repeat = false;
	RTMP_ctrlC = true;
#if defined(WIN32)
	while (WaitForSingleObject(_sb_worker, 100) != WAIT_OBJECT_0)
		::Sleep(10);
#else
	pthread_join(_worker, 0);
#endif
	_state = dk_rtmp_client::STATE_STOPPED;
	return dk_rtmp_client::ERR_CODE_SUCCESS;
}

dk_rtmp_client::ERR_CODE rtmp_client::publish_begin(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T vsmt, dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T asmt, const char * url, const char * username, const char * password)
{
	if ((_state == dk_rtmp_client::STATE_SUBSCRIBING) || (_state == dk_rtmp_client::STATE_PUBLISHING))
		return dk_rtmp_client::ERR_CODE_SUCCESS;

	_vsmt = vsmt;
	_asmt = asmt;
#if defined(WIN32)
	unsigned int thread_id;
	_pb_worker = (HANDLE)::_beginthreadex(0, 0, rtmp_client::pb_process_cb, this, 0, &thread_id);
#else
	pthread_create(&_pb_worker, 0, &rtmp_client::pb_process_cb, this);
#endif
	_state = dk_rtmp_client::STATE_PUBLISHING;
	return dk_rtmp_client::ERR_CODE_SUCCESS;
}

dk_rtmp_client::ERR_CODE rtmp_client::publish_end(void)
{
	if (_state == dk_rtmp_client::STATE_STOPPED)
		return dk_rtmp_client::ERR_CODE_SUCCESS;

	RTMP_ctrlC = true;
#if defined(WIN32)
	::WaitForSingleObject(_pb_worker, INFINITE);
#else
	pthread_join(_worker, 0);
#endif
	_state = dk_rtmp_client::STATE_STOPPED;
	return dk_rtmp_client::ERR_CODE_SUCCESS;
}

dk_rtmp_client::ERR_CODE rtmp_client::publish_video(uint8_t * bitstream, size_t nb)
{
	if (!_rtmp)
		return dk_rtmp_client::ERR_CODE_FAIL;
#if 0
	return push_video_send_packet(bitstream, nb);
#else
	RTMPPacket packet;
	packet.m_nChannel = 0x4;
	packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet.m_nTimeStamp = 0;
	packet.m_nInfoField2 = _rtmp->m_stream_id;
	packet.m_hasAbsTimestamp = FALSE;

	memmove(_video_send_buffer + RTMP_MAX_HEADER_SIZE, bitstream, nb);

	packet.m_nBodySize = nb;
	packet.m_body = (char*)_video_send_buffer + RTMP_MAX_HEADER_SIZE;

	if (!RTMP_SendPacket(_rtmp, &packet, FALSE))
		return dk_rtmp_client::ERR_CODE_FAIL;
	else
		return dk_rtmp_client::ERR_CODE_SUCCESS;
#endif
}

dk_rtmp_client::ERR_CODE rtmp_client::publish_audio(uint8_t * bitstream, size_t nb)
{
	if (!_rtmp)
		return dk_rtmp_client::ERR_CODE_FAIL;
#if 0
	return push_audio_send_packet(bitstream, nb);
#else


	return dk_rtmp_client::ERR_CODE_SUCCESS;
#endif
}

void rtmp_client::sb_process_video(const RTMPPacket * packet)
{
	if (!(_recv_option & dk_rtmp_client::RECV_OPTION_VIDEO))
		return;

	uint8_t extradata[100] = { 0 };
	size_t extradata_size = 0;
	long long  presentation_time = 0;
	uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };

	char * video_data = packet->m_body;
	uint32_t video_data_length = packet->m_nBodySize;

	//bool keyframe = false;
	//if (((video_data[0] & 0xF0) >> 4) == 0x01) //FrameType Key Frame
	//	keyframe = true;
	//if (((video_data[0] & 0xF0) >> 4) == 0x02) //FrameType Inter Frame
	//	keyframe = true;
	//if (((video_data[0] & 0xF0) >> 4) == 0x03) //FrameType Disposable Inter Frame
	//	keyframe = true;
	//if (((video_data[0] & 0xF0) >> 4) == 0x04) //FrameType Generated Key Frame
	//	keyframe = true;
	//if (((video_data[0] & 0xF0) >> 4) == 0x05) //FrameType Generated Video Info/Command Frame
	//	keyframe = true;

	if ((video_data[0] & 0x0F) == dk_rtmp_client::SUBMEDIA_TYPE_AVC) //CodecID
	{
		uint8_t * avc_video_packet = (uint8_t*)&video_data[1];
		uint8_t avc_packet_type = avc_video_packet[0]; //0:AVC Sequence header, 1:AVC NALU, 2:AVC end of sequence
		//avc_video_packet[1], avc_video_packet[2], avc_video_packet[3]
		if (avc_packet_type == AVC_SEQUENCE_HEADER)
		{
			//uint8_t configuration_version = avc_video_packet[4];
			//uint8_t avc_profile_indication = avc_video_packet[5];
			//uint8_t profile_compatibility = avc_video_packet[6];
			//uint8_t avc_level_indication = avc_video_packet[7];
			//uint8_t length_size_minus_one = avc_video_packet[8] & 0x03;
			uint8_t num_sps = avc_video_packet[9] & 0x1F;

			uint16_t sps_size = 0;
			uint16_t pps_size = 0;
			for (uint8_t index = 0; index < num_sps; index++)
			{
				size_t saved_sps_size = 0;
				uint8_t * saved_sps = get_sps(saved_sps_size);
				sps_size = avc_video_packet[10] << 8 | avc_video_packet[11];
				uint8_t * sps = (uint8_t*)&avc_video_packet[12];
				if (saved_sps_size < 1 || !saved_sps)
				{
					memcpy(extradata, start_code, sizeof(start_code));
					memcpy(extradata + sizeof(start_code), sps, sps_size);
					set_sps(extradata, sps_size + sizeof(start_code));
					_change_sps = true;
				}
				else
				{
					if (memcmp(saved_sps + 4, sps, sps_size))
					{
						memcpy(extradata, start_code, sizeof(start_code));
						memcpy(extradata + sizeof(start_code), sps, sps_size);
						set_sps(extradata, sps_size + sizeof(start_code));
						_change_sps = true;
					}
				}
			}

			uint8_t num_pps = avc_video_packet[12 + sps_size];
			for (uint8_t index = 0; index < num_pps; index++)
			{
				size_t saved_pps_size = 0;
				unsigned char * saved_pps = get_pps(saved_pps_size);

				pps_size = avc_video_packet[13 + sps_size] << 8 | avc_video_packet[14 + sps_size];
				uint8_t * pps = (uint8_t*)&avc_video_packet[15 + sps_size];

				if (saved_pps_size < 1 || !saved_pps)
				{
					memcpy(extradata, start_code, sizeof(start_code));
					memcpy(extradata + sizeof(start_code), pps, pps_size);
					set_pps(extradata, pps_size + sizeof(start_code));
					_change_pps = true;
				}
				else
				{
					if (memcmp(saved_pps + 4, pps, pps_size))
					{
						memcpy(extradata, start_code, sizeof(start_code));
						memcpy(extradata + sizeof(start_code), pps, pps_size);
						set_pps(extradata, pps_size + sizeof(start_code));
						_change_pps = true;
					}
				}
			}
		}
		else if (avc_packet_type == AVC_NALU)
		{
			int32_t remained = video_data_length - 5; //FrameType(UB4)+CodecID(UB4)+AVCPacketType(UI8)+CompositionTime(SI24)
			uint8_t * data = (uint8_t*)&avc_video_packet[4];//AVCPacketType(UI8)+CompositionTime(SI24)
			while (remained>0)
			{
				uint32_t nalu_size = (data[0] & 0xff) << 24 | (data[1] & 0xff) << 16 | (data[2] & 0xff) << 8 | (data[3] & 0xff);
				nalu_size += sizeof(start_code);
				uint8_t * nalu = data;
				memcpy(nalu, start_code, sizeof(start_code));

				size_t saved_sps_size = 0;
				size_t saved_pps_size = 0;
				uint8_t * saved_sps = nullptr;
				uint8_t * saved_pps = nullptr;
				bool is_idr = stream_parser::is_idr(dk_rtmp_client::SUBMEDIA_TYPE_AVC, nalu[4] & 0x1F);
				if (!_rcv_first_idr)
				{
					if (is_idr && (_change_sps || _change_pps))
					{
						saved_sps = get_sps(saved_sps_size);
						saved_pps = get_pps(saved_pps_size);
						if ((saved_sps_size > 0) && (saved_pps_size > 0))
						{
							_rcv_first_idr = true;
							if (_front)
								_front->on_begin_video(dk_rtmp_client::SUBMEDIA_TYPE_AVC, saved_sps, saved_sps_size, saved_pps, saved_pps_size, nalu, nalu_size, presentation_time);
							_change_sps = false;
							_change_pps = false;
						}
					}
				}
				else
				{
					saved_sps = get_sps(saved_sps_size);
					saved_pps = get_pps(saved_pps_size);
					if (saved_sps_size > 0 && saved_pps_size > 0)
					{
						if (is_idr)
						{
							if (_front)
								_front->on_recv_video(dk_rtmp_client::SUBMEDIA_TYPE_AVC, nalu, nalu_size, presentation_time);
						}
						else
						{
							if (_front)
								_front->on_recv_video(dk_rtmp_client::SUBMEDIA_TYPE_AVC, nalu, nalu_size, presentation_time);
						}
					}
				}
				remained -= nalu_size;
				data += nalu_size;
			}
		}
		else //end of sequence
		{
			_repeat = false;
			RTMP_ctrlC = true;
		}
	}
}

void rtmp_client::sb_process_audio(const RTMPPacket * packet)
{
	if (!(_recv_option & dk_rtmp_client::RECV_OPTION_AUDIO))
		return;

	uint8_t extradata[50] = { 0 };
	size_t extradata_size = 0;

	long long presentation_time = 0;
	char * audio_data = packet->m_body;
	uint32_t audio_data_length = packet->m_nBodySize;

	if (((audio_data[0] & 0xF0) >> 4) == dk_rtmp_client::SUBMEDIA_TYPE_MP3)
	{
		int32_t samplerate = 44100;
		int32_t samplerate_index = ((audio_data[0] & 0x0C)>>2);
		if (samplerate_index == SR_5_kHz)
			samplerate = 5500;
		else if (samplerate_index == SR_11_kHZ)
			samplerate = 11000;
		else if (samplerate_index == SR_22_kHz)
			samplerate = 22000;
		else if (samplerate_index == SR_44_kHz)
			samplerate = 44100;

		int32_t bitdepth = (audio_data[0] & 0x02) ? 16 : 8;
		int32_t channels = (audio_data[0] & 0x01) ? 1 : 2;

		uint8_t * mp3_packet = (uint8_t*)&audio_data[1];
		size_t mp3_packet_size = audio_data_length - 1;
		if (!_rcv_first_audio)
		{
			if (_front)
				_front->on_begin_audio(dk_rtmp_client::SUBMEDIA_TYPE_MP3, nullptr, 0, samplerate, bitdepth, channels, mp3_packet, mp3_packet_size, presentation_time);
			_rcv_first_audio = true;
		}
		else
		{
			if (_front)
				_front->on_recv_audio(dk_rtmp_client::SUBMEDIA_TYPE_MP3, mp3_packet, mp3_packet_size, presentation_time);
		}
	}
	else if (((audio_data[0] & 0xF0) >> 4) == dk_rtmp_client::SUBMEDIA_TYPE_AAC) //CodecID
	{
		int32_t samplerate = 44100; //AAC always 44.1kHz in video_file_format_spec_v10.pdf
		int32_t bitdepth = (audio_data[0] & 0x02) ? 16 : 8;
		int32_t channels = 2; //AAC always 2 channels : audio_data[0] & 0x01) ? 1 : 2;
		
		uint8_t * aac_packet = (uint8_t*)&audio_data[1];
		uint8_t aac_packet_type = aac_packet[0]; //0:AVC Sequence header, 1:AVC NALU, 2:AVC end of sequence
		if (aac_packet_type == AAC_SEQUENCE_HEADER)
		{
			size_t data_size = audio_data_length - 2; //SoundFormat(UB4)+SoundRate(UB2)+SoundSize(UB1)+SoundType(UB1)+AACPacketType(UI8)
			uint8_t * data = (uint8_t*)&aac_packet[1];
			set_configstr(data, data_size);
			/*if (_front)
				_front->on_begin_audio(dk_rtmp_client::SUBMEDIA_TYPE_AAC, data, data_size, samplerate, bitdepth, channels, presentation_time);*/
		}
		else if (aac_packet_type == AAC_RAW)
		{
			size_t data_size = audio_data_length - 2; //SoundFormat(UB4)+SoundRate(UB2)+SoundSize(UB1)+SoundType(UB1)+AACPacketType(UI8)
			uint8_t * data = (uint8_t*)&aac_packet[1];

			if (!_rcv_first_audio)
			{
				size_t configstr_size = 0;
				uint8_t * configstr = get_configstr(configstr_size);
				if (_front)
					_front->on_begin_audio(dk_rtmp_client::SUBMEDIA_TYPE_AAC, configstr, configstr_size, samplerate, bitdepth, channels, data, data_size, presentation_time);
				_rcv_first_audio = true;
			}
			else
			{
				if (_front)
					_front->on_recv_audio(dk_rtmp_client::SUBMEDIA_TYPE_AAC, data, data_size, presentation_time);
			}
		}
	}
}

void rtmp_client::sb_process(void)
{
	_state = dk_rtmp_client::STATE_START_SUBSCRIBING;
	do
	{
#ifdef _DEBUG
		netstackdump = fopen("netstackdump", "wb");
		netstackdump_read = fopen("netstackdump_read", "wb");
#endif

		int read_buffer_size = MAX_VIDEO_SIZE;// 1024 * 1024;
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

		_rtmp = RTMP_Alloc();
		RTMP_debuglevel = RTMP_LOGINFO;
		RTMP_Init(_rtmp);
		_rtmp->rtmp_client_wrapper = this;

		if (RTMP_ParseURL((char*)_url, &protocol, &host, &port, &playpath, &app))
		{
			if (tc_url.av_len == 0)
			{
				char str[512] = { 0 };
				tc_url.av_len = snprintf(str, 511, "%s://%.*s:%d/%.*s", RTMPProtocolStringsLower[protocol], host.av_len, host.av_val, port, app.av_len, app.av_val);
				tc_url.av_val = (char *)malloc(tc_url.av_len + 1);
				strcpy(tc_url.av_val, str);
			}
			RTMP_SetupStream(_rtmp, protocol, &host, port, &sock_host, &playpath, &tc_url, &swf_url, &page_url, &app, &auth, &swf_hash, swf_size, &flash_version, &subscribe_path, 0, 0, true, timeout);
		}

		clear_sps();
		clear_pps();
		_rcv_first_idr = false;
		_change_sps = false;
		_change_pps = false;

		_rcv_first_audio = false;

		RTMP_SetBufferMS(_rtmp, buffer_time);
		if (!RTMP_Connect(_rtmp, NULL))
			break;

		if (!RTMP_ConnectStream(_rtmp, seek))
			break;

		RTMP_ctrlC = false;
		int32_t nb_read = 0;

#if 0
		RTMPPacket packet;
		while (!RTMP_ctrlC && RTMP_IsConnected(_rtmp) &/*& RTMP_ReadPacket(_rtmp, &packet)*/)
		{
			if (RTMPPacket_IsReady(&packet))
			{
				if (!packet.m_nBodySize)
					continue;
				RTMP_ClientPacket(_rtmp, &packet); //This takes care of handling ping/other messages
				//RTMPPacket_Free(&packet);
			}
		}

#else
		do
		{
			nb_read = RTMP_Read(_rtmp, read_buffer, read_buffer_size);
			double duration = RTMP_GetDuration(_rtmp);
			//::Sleep(1);
			//Sleep(duration / 1000);

		} while (!RTMP_ctrlC && /*(nb_read>-1) &&*/ RTMP_IsConnected(_rtmp) && !RTMP_IsTimedout(_rtmp));
#endif

		RTMP_Close(_rtmp);
		RTMP_Free(_rtmp);
		_rtmp = nullptr;

		if (read_buffer)
		{
			free(read_buffer);
			read_buffer = nullptr;
		}
		read_buffer_size = 0;


	/*	if (host.av_len > 0)
			free(host.av_val);*/
		if (playpath.av_len > 0)
			free(playpath.av_val);
		/*if (app.av_len > 0)
			free(app.av_val);*/
		if (tc_url.av_len > 0)
			free(tc_url.av_val);
		//if (swf_url.av_len > 0)
		//	free(swf_url.av_val);
		//if (page_url.av_len > 0)
		//	free(page_url.av_val);
		//if (auth.av_len > 0)
		//	free(auth.av_val);
		//if (swf_hash.av_len > 0)
		//	free(swf_hash.av_val);
		//if (flash_version.av_len > 0)
		//	free(flash_version.av_val);
		//if (sock_host.av_len > 0)
		//	free(sock_host.av_val);
		//if (subscribe_path.av_len > 0)
		//	free(subscribe_path.av_val);

#ifdef _DEBUG
		if (netstackdump != 0)
			fclose(netstackdump);
		if (netstackdump_read != 0)
			fclose(netstackdump_read);
#endif

	} while (_repeat);

	_state = dk_rtmp_client::STATE_STOPPED;
}

void rtmp_client::pb_process(void)
{
	do
	{
#ifdef _DEBUG
		netstackdump = fopen("netstackdump", "wb");
		netstackdump_read = fopen("netstackdump_read", "wb");
#endif

		int read_buffer_size = MAX_VIDEO_SIZE;// 1024 * 1024;
		char * read_buffer = (char *)malloc(read_buffer_size);

		_rtmp = RTMP_Alloc();
		RTMP_Init(_rtmp);
		if (!RTMP_SetupURL(_rtmp, (char*)_url))
			break;

		RTMP_EnableWrite(_rtmp);
		_rtmp->Link.swfUrl.av_len = _rtmp->Link.tcUrl.av_len;
		_rtmp->Link.swfUrl.av_val = _rtmp->Link.tcUrl.av_val;
		/*rtmp->Link.pageUrl.av_len = rtmp->Link.tcUrl.av_len;
		rtmp->Link.pageUrl.av_val = rtmp->Link.tcUrl.av_val;*/
		_rtmp->Link.flashVer.av_val = "FMLE/3.0 (compatible; FMSc/1.0)";
		_rtmp->Link.flashVer.av_len = (int)strlen(_rtmp->Link.flashVer.av_val);
		_rtmp->m_outChunkSize = 4096;//RTMP_DEFAULT_CHUNKSIZE;

		if (!RTMP_Connect(_rtmp, NULL))
		{	
			break;
		}

		if (!RTMP_ConnectStream(_rtmp, 0))
		{
			break;
		}

		RTMP_ctrlC = false;
		do
		{
#if 1
			RTMP_Read(_rtmp, read_buffer, read_buffer_size);
			double duration = RTMP_GetDuration(_rtmp);
#else

#endif
		} while (!RTMP_ctrlC && RTMP_IsConnected(_rtmp) && !RTMP_IsTimedout(_rtmp));

		RTMP_DeleteStream(_rtmp);

		RTMP_Close(_rtmp);
		RTMP_Free(_rtmp);
		_rtmp = nullptr;

		if (read_buffer)
			free(read_buffer);

	} while (0);
	_state = dk_rtmp_client::STATE_STOPPED;
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
	dk_auto_lock lock(&_video_mutex);
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
	dk_auto_lock lock(&_video_mutex);
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
	dk_auto_lock lock(&_audio_mutex);
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
	dk_auto_lock lock(&_audio_mutex);
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
