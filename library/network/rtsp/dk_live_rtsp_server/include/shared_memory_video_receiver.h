#ifndef _SHARED_MEMORY_RECEIVER_H_
#define _SHARED_MEMORY_RECEIVER_H_

#include <dk_shared_memory.h>
#include <shared_memory_packet.h>
#include <dk_live_rtsp_server.h>

class shared_memory_video_receiver
{
public:
	shared_memory_video_receiver(void);
	virtual ~shared_memory_video_receiver(void);

	bool connect(const char * uuid);
	bool disconnect(void);

	void read(uint8_t * data, size_t data_capacity, size_t & data_size);

	dk_live_rtsp_server::vsubmedia_type_t type(void);
	const uint8_t * vps(size_t & vps_size) const;
	const uint8_t * sps(size_t & sps_size) const;
	const uint8_t * pps(size_t & pps_size) const;

private:
	ic::dk_shared_memory_client * _client;

	uint8_t * _buffer;
	size_t _buffer_size;
	size_t _buffer_index;
	int32_t _prev_media_packet_seq;

	dk_live_rtsp_server::vsubmedia_type_t _type;

	uint8_t _vps[100];
	uint8_t _sps[100];
	uint8_t _pps[100];

	size_t _vps_size;
	size_t _sps_size;
	size_t _pps_size;

};








#endif