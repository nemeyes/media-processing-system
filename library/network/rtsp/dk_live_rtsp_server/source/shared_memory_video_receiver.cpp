#include "shared_memory_video_receiver.h"
#include <cstdint>
#include <cstdio>

#define MAX_VIDEO_BUFFER_SIZE 1024*1024*2

shared_memory_video_receiver::shared_memory_video_receiver(void)
	: _type(shared_memory::unknown_video_type)
	, _vps_size(0)
	, _sps_size(0)
	, _pps_size(0)
	, _buffer(nullptr)
	, _buffer_size(0)
	, _buffer_index(0)
	, _prev_media_packet_seq(-1)
{
	memset(_vps, 0x00, sizeof(_vps));
	memset(_sps, 0x00, sizeof(_sps));
	memset(_pps, 0x00, sizeof(_pps));

	_buffer_size = MAX_VIDEO_BUFFER_SIZE;
	_buffer = static_cast<uint8_t*>(malloc(_buffer_size));
	memset(_buffer, 0x00, _buffer_size);
	_buffer_index = 0;

	_client = new ic::dk_shared_memory_client();
}

shared_memory_video_receiver::~shared_memory_video_receiver(void)
{
	if (_client)
	{
		_client->disconnect_shared_memory();
		delete _client;
		_client = nullptr;
	}
}

bool shared_memory_video_receiver::connect(const char * uuid)
{
	if (!uuid || strlen(uuid) < 1)
		return false;

	disconnect();

	char id[MAX_PATH];
	_snprintf_s(id, sizeof(uuid), "%s_video", uuid);
	return _client->connect_shared_memory(id);
}

bool shared_memory_video_receiver::disconnect(void)
{
	bool ret =  _client->disconnect_shared_memory();

	_type = dk_live_rtsp_server::unknown_video_type;

	_vps_size = 0;
	_sps_size = 0;
	_pps_size = 0;
	memset(_vps, 0x00, sizeof(_vps));
	memset(_sps, 0x00, sizeof(_sps));
	memset(_pps, 0x00, sizeof(_pps));

	memset(_buffer, 0x00, _buffer_size);
	_buffer_index = 0;
	_prev_media_packet_seq = -1;

	return ret;
}

void shared_memory_video_receiver::read(uint8_t * data, size_t data_capacity, size_t & data_size)
{
	uint8_t sm_data[SM_BLOCK_SIZE] = { 0 };
	long sm_data_size = _client->read(sm_data, SM_BLOCK_SIZE);
	if (sm_data_size > 0)
	{
		int32_t header_size = sizeof(shared_memory::packet_header_t);
		shared_memory::packet_header_t header;
		memcpy(&header, sm_data, header_size);
		uint8_t * payload = sm_data + header_size;

		if (header.seq == 0)
		{
			_type = dk_live_rtsp_server::vsubmedia_type_t(header.type);
			_buffer_index = 0;
			_prev_media_packet_seq = -1;
		}

		if ((_prev_media_packet_seq + 1) == header.seq)
		{
			memmove(_buffer + _buffer_index, payload, header.length);
			_buffer_index += header.length;

			if (header.flag & FLAG_PKT_END)
			{
				data_size = data_capacity > _buffer_index ? _buffer_index : data_capacity;
				memmove(data, _buffer, data_size);

				if (_type == dk_live_rtsp_server::vsubmedia_type_hevc)
				{
					bool is_vps = dk_live_rtsp_server::is_vps(_type, data[4]);
					if (is_vps)
					{
						_vps_size = data_size;
						memmove(_vps, data, _vps_size);
					}
				}
				if ((_type == dk_live_rtsp_server::vsubmedia_type_hevc) || (_type == dk_live_rtsp_server::vsubmedia_type_h264))
				{
					bool is_sps = dk_live_rtsp_server::is_sps(_type, data[4]);
					if (is_sps)
					{
						_sps_size = data_size;
						memmove(_sps, data, _sps_size);
					}
					bool is_pps = dk_live_rtsp_server::is_pps(_type, data[4]);
					if (is_pps)
					{
						_pps_size = data_size;
						memmove(_pps, data, _pps_size);
					}
				}

				_buffer_index = 0;
				_prev_media_packet_seq = -1;
			}
		}
	}
}

dk_live_rtsp_server::vsubmedia_type_t shared_memory_video_receiver::type(void)
{
	return _type;
}

const uint8_t * shared_memory_video_receiver::vps(size_t & vps_size) const
{
	vps_size = _vps_size;
	return _vps;
}

const uint8_t * shared_memory_video_receiver::sps(size_t & sps_size) const
{
	sps_size = _sps_size;
	return _sps;
}

const uint8_t * shared_memory_video_receiver::pps(size_t & pps_size) const
{
	pps_size = _pps_size;
	return _pps;
}