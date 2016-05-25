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

#define BITDEPTH_8BIT	0
#define BITDEPTH_16BIT	1

#define SOUND_MONO		0
#define SOUND_STEREO	1

#define DK_VERSION_STRING "DebuggerKing ver 1.1"
#define SAVC(x)    static const AVal av_##x = AVC(#x)
SAVC(onMetaData);
SAVC(duration);
SAVC(width);
SAVC(height);
SAVC(videocodecid);
SAVC(videodatarate);
SAVC(framerate);
SAVC(audiocodecid);
SAVC(audiodatarate);
SAVC(audiosamplerate);
SAVC(audiosamplesize);
SAVC(audiochannels);
SAVC(stereo);
SAVC(encoder);
SAVC(fileSize);
static const AVal av_setDataFrame = AVC("@setDataFrame");
SAVC(avc1);
SAVC(mp4a);
static const AVal av_DKVersion = AVC(DK_VERSION_STRING);

#define UNKNOWN_VIDEO_TYPE					-1
#define VIDEO_TYPE_JPEG						1
#define VIDEO_TYPE_SORENSON_H263			2
#define VIDEO_TYPE_SCREEN_VIDEO				3
#define VIDEO_TYPE_VP6						4
#define VIDEO_TYPE_VP_WITH_ALPHA_CHANNEL	5
#define VIDEO_TYPE_SCREEN_VIDEO_VERSION2	6
#define VIDEO_TYPE_AVC						7

#define UNKNOWN_AUDIO_TYPE					-1
#define AUDIO_TYPE_LINEAR_PCM_PE			0
#define AUDIO_TYPE_ADPCM					1
#define AUDIO_TYPE_MP3						2
#define AUDIO_TYPE_LINEAR_PCM_LE			3
#define AUDIO_TYPE_NELLYMOSER_16KHZ			4
#define AUDIO_TYPE_NELLYMOSER_8KHZ			5
#define AUDIO_TYPE_NELLYMOSER				6
#define AUDIO_TYPE_ALAW						7
#define AUDIO_TYPE_MLAW						8
#define AUDIO_TYPE_AAC						10
#define AUDIO_TYPE_SPEEX					11
#define AUDIO_TYPE_MP3_8KHZ					12

/*
typedef enum _vsubmedia_type
{
unknown_video_type = -1,
vsubmedia_type_jpeg = 1,
vsubmedia_type_sorenson_h263,
vsubmedia_type_screen_video,
vsubmedia_type_vp6,
vsubmedia_type_vp_with_alpha_channel,
vsubmedia_type_screen_video_version2,
vsubmedia_type_avc,
} vsubmedia_type;

typedef enum _asubmedia_type
{
unknown_audio_type = -1,
asubmedia_type_linear_pcm_pe = 0, //platform endian
asubmedia_type_adpcm,
asubmedia_type_mp3,
asubmedia_type_linear_pcm_le, //little endian
asubmedia_type_nellymoser_16khz,
asubmedia_type_nellymoser_8khz,
asubmedia_type_nellymoser,
asubmedia_type_alaw,
asubmedia_type_mlaw,
asubmedia_type_aac = 10,
asubmedia_type_speex,
asubmedia_type_mp3_8khz,
} asubmedia_type;
*/


#if defined(WIN32) && defined(_DEBUG)
FILE *netstackdump = 0;
FILE *netstackdump_read = 0;
#endif

debuggerking::rtmp_core::rtmp_core(rtmp_client * front)
	: _front(front)
	, _state(rtmp_client::state_stopped)
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

debuggerking::rtmp_core::~rtmp_core(void)
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

uint8_t * debuggerking::rtmp_core::get_sps(size_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

uint8_t * debuggerking::rtmp_core::get_pps(size_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

void debuggerking::rtmp_core::set_sps(uint8_t * sps, size_t sps_size)
{
	memset(_sps, 0x00, sizeof(_sps));
	memcpy(_sps, sps, sps_size);
	_sps_size = sps_size;
}

void debuggerking::rtmp_core::set_pps(uint8_t * pps, size_t pps_size)
{
	memset(_pps, 0x00, sizeof(_pps));
	memcpy(_pps, pps, pps_size);
	_pps_size = pps_size;
}

void debuggerking::rtmp_core::clear_sps(void)
{
	memset(_sps, 0x00, sizeof(_sps));
	_sps_size = 0;
}

void debuggerking::rtmp_core::clear_pps(void)
{
	memset(_pps, 0x00, sizeof(_pps));
	_pps_size = 0;
}

uint8_t * debuggerking::rtmp_core::get_configstr(size_t & configstr_size)
{
	configstr_size = _configstr_size;
	return _configstr;
}

void debuggerking::rtmp_core::set_configstr(uint8_t * configstr, size_t configstr_size)
{
	memset(_configstr, 0x00, sizeof(_configstr));
	memcpy(_configstr, configstr, configstr_size);
	_configstr_size = configstr_size;
}

debuggerking::rtmp_client::rtmp_state debuggerking::rtmp_core::state(void)
{
	return _state;
}

int32_t debuggerking::rtmp_core::subscribe_begin(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat)
{
	if ((_state == rtmp_client::state_subscribing) || (_state == rtmp_client::state_begin_publishing) || (_state == rtmp_client::state_publishing))
		return rtmp_client::err_code_t::success;

	if (!url || strlen(url)<1)
		return rtmp_client::err_code_t::fail;

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
	_sb_worker = (HANDLE)::_beginthreadex(0, 0, rtmp_core::sb_process_cb, this, 0, &thread_id);
#else
	pthread_create(&_sb_worker, 0, &rtmp_client::sb_process_cb, this);
#endif

	_state = rtmp_client::state_subscribing;
	return rtmp_client::err_code_t::success;
}

int32_t debuggerking::rtmp_core::subscribe_end(void)
{
	if (_state == rtmp_client::state_stopped)
		return rtmp_client::err_code_t::success;

	_repeat = false;
	RTMP_ctrlC = true;
#if defined(WIN32)
	::WaitForSingleObject(_sb_worker, INFINITE);
#else
	pthread_join(_worker, 0);
#endif
	_state = rtmp_client::state_stopped;
	return rtmp_client::err_code_t::success;
}

int32_t debuggerking::rtmp_core::publish_begin(int32_t vsmt, int32_t asmt, const char * url, const char * username, const char * password)
{
	if ((_state == rtmp_client::state_subscribing) || (_state == rtmp_client::state_begin_publishing) || (_state == rtmp_client::state_publishing))
		return rtmp_client::err_code_t::success;

	if (!url || strlen(url)<1)
		return rtmp_client::err_code_t::fail;

	memset(_url, 0x00, sizeof(_url));
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	if (strlen(url)>0)
		strcpy_s(_url, url);
	if (username && strlen(username)>0)
		strcpy_s(_username, username);
	if (password && strlen(password)>0)
		strcpy_s(_password, password);


	_vsmt = vsmt;
	_asmt = asmt;
#if defined(WIN32)
	unsigned int thread_id;
	_pb_worker = (HANDLE)::_beginthreadex(0, 0, rtmp_core::pb_process_cb, this, 0, &thread_id);
#else
	pthread_create(&_pb_worker, 0, &rtmp_client::pb_process_cb, this);
#endif
	_state = rtmp_client::state_begin_publishing;
	return rtmp_client::err_code_t::success;
}

int32_t debuggerking::rtmp_core::publish_end(void)
{
	if (_state == rtmp_client::state_stopped)
		return rtmp_client::err_code_t::success;

	RTMP_ctrlC = true;
#if defined(WIN32)
	::WaitForSingleObject(_pb_worker, INFINITE);
#else
	pthread_join(_worker, 0);
#endif
	_state = rtmp_client::state_stopped;
	return rtmp_client::err_code_t::success;
}

int32_t debuggerking::rtmp_core::publish_video(uint8_t * bitstream, size_t nb, long long timestamp)
{
	if (!_rtmp)
		return rtmp_client::err_code_t::fail;
	if (_state != rtmp_client::state_publishing)
		return rtmp_client::err_code_t::fail;
#if 0
	return push_video_send_packet(bitstream, nb);
#else
	RTMPPacket packet;
	packet.m_nChannel = 0x4;
	packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet.m_nTimeStamp = 0;
	packet.m_nInfoField2 = _rtmp->m_stream_id;
	packet.m_hasAbsTimestamp = TRUE;

	//memmove(_video_send_buffer + RTMP_MAX_HEADER_SIZE, bitstream, nb);

	memset(_video_send_buffer, 0x00, _video_send_buffer_size);
	packet.m_body = (char*)_video_send_buffer + RTMP_MAX_HEADER_SIZE;

	size_t video_tag_size = 0;
	char * video_tag = packet.m_body;

	if (((bitstream[4] & 0x1F) == 0x07) || ((bitstream[4] & 0x1F) == 0x08) || ((bitstream[4] & 0x1F) == 0x05))
		*(video_tag + video_tag_size) |= (0x01 << 4);
	else
		*(video_tag + video_tag_size) |= (0x02 << 4);
	*(video_tag + video_tag_size) |= VIDEO_TYPE_AVC;
	video_tag_size++;

	if (bitstream && (nb > 0) && (((bitstream[4] & 0x1F) == 0x07) || ((bitstream[4] & 0x1F) == 0x08))) //pps always comes after the sps
	{
		*(video_tag + video_tag_size) = AVC_SEQUENCE_HEADER;
		video_tag_size++;//increate avc packet type index
		//TODO : composition time 3bytes
		video_tag_size = video_tag_size + 3; //increase  compostion time index
		if (((bitstream[4] & 0x1F) == 0x07))
		{
			set_sps(bitstream + 4, nb - 4); //sps and pps should be sent in one time
			return rtmp_client::err_code_t::success;
		}
		else
		{
			size_t sps_size = 0, pps_size = 0;
			uint8_t * sps = nullptr, *pps = nullptr;

			set_pps(bitstream + 4, nb - 4);

			sps = get_sps(sps_size);
			pps = get_pps(pps_size);

			if (sps && sps_size > 0)
			{
				*(video_tag + video_tag_size) = 0x01; //configurationVersion
				video_tag_size++;//increase configurationVersion index
				video_tag_size = video_tag_size + 4;

				///// SPS /////
				*(video_tag + video_tag_size) = 0xE1;//number of sps
				video_tag_size++;
				*(video_tag + video_tag_size) = (uint16_t(sps_size) & 0xFF00) >> 8;
				video_tag_size++;
				*(video_tag + video_tag_size) = (uint16_t(sps_size) & 0x00FF);
				video_tag_size++;
				memmove(video_tag + video_tag_size, sps, sps_size);
				video_tag_size = video_tag_size + sps_size;


				///// PPS /////
				*(video_tag + video_tag_size) = 0x01; //number of pps
				video_tag_size++;
				*(video_tag + video_tag_size) = (uint16_t(pps_size) & 0xFF00) >> 8;
				video_tag_size++;
				*(video_tag + video_tag_size) = (uint16_t(pps_size) & 0x00FF);
				video_tag_size++;
				memmove(video_tag + video_tag_size, pps, pps_size);
				video_tag_size = video_tag_size + pps_size;
			}
			else
			{
				return rtmp_client::err_code_t::fail;
			}
		}
	}
	else if (bitstream && (nb > 0))
	{
		*(video_tag + video_tag_size) = AVC_NALU;
		video_tag_size++;

		//TODO : composition time 3bytes
		video_tag_size = video_tag_size + 3; //increase  compostion time index

		memmove(video_tag + video_tag_size, bitstream, nb);
		set_bitstream_size(video_tag + video_tag_size, nb - 4);
		video_tag_size = video_tag_size + nb;
	}
	else
	{
		*(video_tag + video_tag_size) = AVC_ENC_OF_SEQUENCE;
		video_tag_size++;
	}
	packet.m_nBodySize = video_tag_size;

	if (!RTMP_SendPacket(_rtmp, &packet, FALSE))
		return rtmp_client::err_code_t::fail;
	else
		return rtmp_client::err_code_t::success;
#endif
}

int32_t debuggerking::rtmp_core::publish_audio(uint8_t * bitstream, size_t nb, bool configstr, long long timestamp)
{
	if (!_rtmp)
		return rtmp_client::err_code_t::fail;
	if (_state != rtmp_client::state_publishing)
		return rtmp_client::err_code_t::fail;

	RTMPPacket packet;
	packet.m_nChannel = 0x5;
	packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;
	packet.m_nTimeStamp = 0;
	packet.m_nInfoField2 = _rtmp->m_stream_id;
	packet.m_hasAbsTimestamp = FALSE;

	memset(_audio_send_buffer, 0x00, _audio_send_buffer_size);
	packet.m_body = (char*)_audio_send_buffer + RTMP_MAX_HEADER_SIZE;

	size_t audio_tag_size = 0;
	char * audio_tag = packet.m_body;

	*(audio_tag + audio_tag_size) |= (0x0F & AUDIO_TYPE_AAC) << 4; //UB[4] Format of SoundData
	*(audio_tag + audio_tag_size) |= (0x03 & SR_44_kHz) << 2;
	*(audio_tag + audio_tag_size) |= (0x01 & SOUND_STEREO) << 1;
	*(audio_tag + audio_tag_size) |= (0x01 & BITDEPTH_16BIT);
	audio_tag_size++;

	if (configstr)
	{
		*(audio_tag + audio_tag_size) = AAC_SEQUENCE_HEADER;
		audio_tag_size++;

		memmove(audio_tag + audio_tag_size, bitstream, nb);
		set_bitstream_size(audio_tag + audio_tag_size, nb);
		audio_tag_size = audio_tag_size + nb;
	}
	else
	{
		*(audio_tag + audio_tag_size) = AAC_RAW;
		audio_tag_size++;

		memmove(audio_tag + audio_tag_size, bitstream, nb);
		set_bitstream_size(audio_tag + audio_tag_size, nb);
		audio_tag_size = audio_tag_size + nb;
	}
	packet.m_nBodySize = audio_tag_size;

	if (!RTMP_SendPacket(_rtmp, &packet, FALSE))
		return rtmp_client::err_code_t::fail;
	else
		return rtmp_client::err_code_t::success;
}

void debuggerking::rtmp_core::sb_process_video(const RTMPPacket * packet)
{
	if (!(_recv_option & rtmp_client::recv_option_t::video))
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

	if ((video_data[0] & 0x0F) == VIDEO_TYPE_AVC) //CodecID
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
				bool is_idr = stream_parser::is_idr(rtmp_client::video_submedia_type_t::avc, nalu[4] & 0x1F);
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
								_front->on_begin_video(rtmp_client::video_submedia_type_t::avc, saved_sps, saved_sps_size, saved_pps, saved_pps_size, nalu, nalu_size, presentation_time);
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
						if (_front)
							_front->on_recv_video(rtmp_client::video_submedia_type_t::avc, nalu, nalu_size, presentation_time);
					}
				}
				remained -= nalu_size;//nalu size itself
				data += nalu_size;
			}
		}
		else //end of sequence
		{
			//_repeat = false;
			RTMP_ctrlC = true;
		}
	}
}

void debuggerking::rtmp_core::sb_process_audio(const RTMPPacket * packet)
{
	if (!(_recv_option & rtmp_client::recv_option_t::audio))
		return;

	uint8_t extradata[50] = { 0 };
	size_t extradata_size = 0;

	long long presentation_time = 0;
	char * audio_data = packet->m_body;
	uint32_t audio_data_length = packet->m_nBodySize;

	if (((audio_data[0] & 0xF0) >> 4) == AUDIO_TYPE_MP3)
	{
		int32_t samplerate = 44100;
		int32_t samplerate_index = ((audio_data[0] & 0x0C) >> 2);
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
				_front->on_begin_audio(rtmp_client::audio_submedia_type_t::mp3, nullptr, 0, samplerate, bitdepth, channels, mp3_packet, mp3_packet_size, presentation_time);
			_rcv_first_audio = true;
		}
		else
		{
			if (_front)
				_front->on_recv_audio(rtmp_client::audio_submedia_type_t::mp3, mp3_packet, mp3_packet_size, presentation_time);
		}
	}
	else if (((audio_data[0] & 0xF0) >> 4) == AUDIO_TYPE_AAC) //CodecID
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
			_front->on_begin_audio(cap_rtmp_client::SUBMEDIA_TYPE_AAC, data, data_size, samplerate, bitdepth, channels, presentation_time);*/
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
					_front->on_begin_audio(rtmp_client::audio_submedia_type_t::aac, configstr, configstr_size, samplerate, bitdepth, channels, data, data_size, presentation_time);
				_rcv_first_audio = true;
			}
			else
			{
				if (_front)
					_front->on_recv_audio(rtmp_client::audio_submedia_type_t::aac, data, data_size, presentation_time);
			}
		}
	}
}

void debuggerking::rtmp_core::sb_process(void)
{
	do
	{
#ifdef _DEBUG
		netstackdump = fopen("netstackdump", "wb");
		netstackdump_read = fopen("netstackdump_read", "wb");
#endif

		int read_buffer_size = MAX_VIDEO_SIZE;
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

		long int timeout = 1;// DEFAULT_TIMEOUT;
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
		do
		{
			nb_read = RTMP_Read(_rtmp, read_buffer, read_buffer_size);

			double duration = RTMP_GetDuration(_rtmp);
			//::Sleep(1);
			//Sleep(duration / 1000);

		} while (!RTMP_ctrlC /*&& (nb_read>-1)*/ && RTMP_IsConnected(_rtmp) && !RTMP_IsTimedout(_rtmp));

		RTMP_Close(_rtmp);
		RTMP_Free(_rtmp);
		_rtmp = nullptr;

		if (read_buffer)
			free(read_buffer);


		/*
		if (host.av_len > 0)
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

	_state = rtmp_client::state_stopped;
}

void debuggerking::rtmp_core::pb_process(void)
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

		/*char * enc = _video_send_buffer + RTMP_MAX_HEADER_SIZE;
		char * pend = _video_send_buffer + _video_send_buffer_size;
		enc = AMF_EncodeString(enc, pend, &av_setDataFrame);
		enc = AMF_EncodeString(enc, pend, &av_onMetaData);

		*enc++ = AMF_OBJECT;
		enc = AMF_EncodeNamedNumber(enc, pend, &av_duration, 0.0);
		enc = AMF_EncodeNamedNumber(enc, pend, &av_fileSize, 0.0);
		enc = AMF_EncodeNamedNumber(enc, pend, &av_width, double(1280));
		enc = AMF_EncodeNamedNumber(enc, pend, &av_height, double(720));
		enc = AMF_EncodeNamedString(enc, pend, &av_videocodecid, &av_avc1);//7.0);//
		enc = AMF_EncodeNamedNumber(enc, pend, &av_videodatarate, double(6000));
		enc = AMF_EncodeNamedNumber(enc, pend, &av_framerate, double(60));*/

		/*enc = AMF_EncodeNamedString(enc, pend, &av_audiocodecid, &av_mp4a);
		enc = AMF_EncodeNamedNumber(enc, pend, &av_audiodatarate, double(128)); //ex. 128kb\s
		enc = AMF_EncodeNamedNumber(enc, pend, &av_audiosamplerate, double(44.1));
		enc = AMF_EncodeNamedNumber(enc, pend, &av_audiosamplesize, 16.0);
		enc = AMF_EncodeNamedNumber(enc, pend, &av_audiochannels, double(2));
		enc = AMF_EncodeNamedBoolean(enc, pend, &av_stereo, true);*/

		/*enc = AMF_EncodeNamedString(enc, pend, &av_encoder, &av_DKVersion);
		*enc++ = 0;
		*enc++ = 0;
		*enc++ = AMF_OBJECT_END;

		RTMPPacket packet;
		packet.m_nChannel = 0x03;     // control channel (invoke)
		packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
		packet.m_packetType = RTMP_PACKET_TYPE_INFO;
		packet.m_nTimeStamp = 0;
		packet.m_nInfoField2 = _rtmp->m_stream_id;
		packet.m_hasAbsTimestamp = TRUE;
		packet.m_body = _video_send_buffer + RTMP_MAX_HEADER_SIZE;
		packet.m_nBodySize = enc - packet.m_body;
		if (!RTMP_SendPacket(_rtmp, &packet, FALSE))
		break;*/

		_state = rtmp_client::state_publishing;

		RTMP_ctrlC = false;
		int32_t nb_read = 0;
		do
		{
			nb_read = RTMP_Read(_rtmp, read_buffer, read_buffer_size);
			double duration = RTMP_GetDuration(_rtmp);
		} while (!RTMP_ctrlC /*&& (nb_read>-1)*/ && RTMP_IsConnected(_rtmp) /*&& !RTMP_IsTimedout(_rtmp)*/);

		RTMP_DeleteStream(_rtmp);
		RTMP_Close(_rtmp);
		RTMP_Free(_rtmp);
		_rtmp = nullptr;

		if (read_buffer)
			free(read_buffer);

	} while (0);
	_state = rtmp_client::state_stopped;
}

#if defined(WIN32)
unsigned __stdcall debuggerking::rtmp_core::sb_process_cb(void * param)
{
	rtmp_core * self = static_cast<rtmp_core*>(param);
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
unsigned __stdcall debuggerking::rtmp_core::pb_process_cb(void * param)
{
	rtmp_core * self = static_cast<rtmp_core*>(param);
	self->pb_process();
	return 0;
}
#else
void* debuggerking::rtmp_core::pb_process_cb(void * param)
{
	rtmp_core * self = static_cast<rtmp_core*>(param);
	self->pb_process();
	return 0;
}
#endif

int32_t debuggerking::rtmp_core::push_video_send_packet(uint8_t * bs, size_t size)
{
	int32_t status = rtmp_client::err_code_t::success;
	auto_lock lock(&_video_mutex);
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
		int32_t result = circular_buffer_t::write(_video_queue, bs, vbuffer->amount);
		if (result == -1)
		{
			if (vbuffer->prev)
				vbuffer->prev->next = nullptr;
			free(vbuffer);
			vbuffer = nullptr;
			status = rtmp_client::err_code_t::fail;
		}
	}
	return status;
}

int32_t debuggerking::rtmp_core::pop_video_send_packet(uint8_t * bs, size_t & size)
{
	int32_t status = rtmp_client::err_code_t::success;
	size = 0;
	auto_lock lock(&_video_mutex);
	pb_buffer_t * vbuffer = _video_root->next;
	if (vbuffer)
	{
		_video_root->next = vbuffer->next;
		if (_video_root->next)
			_video_root->next->prev = _video_root;

		int32_t result = circular_buffer_t::read(_video_queue, bs, vbuffer->amount);
		if (result == -1)
			status = rtmp_client::err_code_t::fail;

		size = vbuffer->amount;
		free(vbuffer);
	}
	return status;
}

int32_t debuggerking::rtmp_core::push_audio_send_packet(uint8_t * bs, size_t size)
{
	int32_t status = rtmp_client::err_code_t::success;
	auto_lock lock(&_audio_mutex);
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
		int32_t result = circular_buffer_t::write(_audio_queue, bs, abuffer->amount);
		if (result == -1)
		{
			if (abuffer->prev)
				abuffer->prev->next = nullptr;
			free(abuffer);
			abuffer = nullptr;
			status = rtmp_client::err_code_t::fail;
		}
	}
	return status;
}

int32_t debuggerking::rtmp_core::pop_audio_send_packet(uint8_t * bs, size_t & size)
{
	int32_t status = rtmp_client::err_code_t::success;
	size = 0;
	auto_lock lock(&_audio_mutex);
	pb_buffer_t * abuffer = _audio_root->next;
	if (abuffer)
	{
		_audio_root->next = abuffer->next;
		if (_audio_root->next)
			_audio_root->next->prev = _audio_root;

		int32_t result = circular_buffer_t::read(_audio_queue, bs, abuffer->amount);
		if (result == -1)
			status = rtmp_client::err_code_t::fail;

		size = abuffer->amount;
		free(abuffer);
	}
	return status;
}

int32_t debuggerking::rtmp_core::init_pb_buffer(pb_buffer_t * buffer)
{
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return rtmp_client::err_code_t::success;
}

void debuggerking::rtmp_core::set_bitstream_size(char * nalu, int32_t nalu_size)
{
	nalu[0] = (nalu_size & 0xFF000000) >> 24;
	nalu[1] = (nalu_size & 0x00FF0000) >> 16;
	nalu[2] = (nalu_size & 0x0000FF00) >> 8;
	nalu[3] = (nalu_size & 0x000000FF);
}

void debuggerking::rtmp_core::get_bitstream_size(char * nalu, int32_t & nalu_size)
{
	int32_t size = 0;
	size = ((nalu[0] & 0xFF) << 24) | ((nalu[1] & 0xFF) << 16) | ((nalu[2] & 0xFF) << 8) | ((nalu[3] & 0xFF));
	nalu_size = size;
}