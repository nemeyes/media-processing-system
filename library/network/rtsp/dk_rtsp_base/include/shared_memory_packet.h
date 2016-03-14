#ifndef _SHARED_MEMORY_MEDIA_PACKET_H_
#define _SHARED_MEMORY_MEDIA_PACKET_H_

#include <winsock2.h>
#include <windows.h>
#include <cstdint>

#define FLAG_PKT_BEGIN          0x0001
#define FLAG_PKT_PLAY           0x0002
#define FLAG_PKT_END            0x0004

namespace shared_memory
{
	typedef struct _packet_header_t
	{
		int16_t type;
		int16_t seed;
		int16_t seq;
		int16_t flag;
		int16_t length;
	} packet_header_t;
};
#endif