#ifndef _MEDIA_EDGE_SERVER_H_
#define _MEDIA_EDGE_SERVER_H_

#include <dk_ipc_server.h>

namespace ic
{
	class media_edge_server : public ic::dk_ipc_server
	{
	public:
		media_edge_server(const char * uuid);
		virtual ~media_edge_server(void);

		void assoc_completion_callback(const char * uuid);
		void leave_completion_callback(const char * uuid);
	};

};


#endif