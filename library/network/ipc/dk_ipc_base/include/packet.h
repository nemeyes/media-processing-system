#ifndef _ABTRACT_PACKET_H_
#define _ABTRACT_PACKET_H_

#include <platform.h>
/*#if defined(WITH_WORKING_AS_SERVER)
#include <rpc.h>
#include "abstract_socket_server.h"
#else
#include "abstract_socket_client.h"
#endif*/

#define FLAG_PKT_BEGIN          0x0001
#define FLAG_PKT_PLAY           0x0002
#define FLAG_PKT_END            0x0004

namespace ic
{
	typedef struct _packet_header_t
	{
		char dst[64];
		char src[64];
		int32_t seed;
		int32_t seq;
		int32_t flag;
		int32_t command;
		int32_t length;
	} packet_header_t;


	typedef struct _packet_queue_t
	{
		packet_header_t header;
		char * msg;
	} packet_queue_t;

};









#endif