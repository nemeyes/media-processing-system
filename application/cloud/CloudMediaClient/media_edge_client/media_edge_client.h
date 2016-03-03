#ifndef _MEDIA_CLIENT_H_
#define _MEDIA_CLIENT_H_

#include <dk_ipc_client.h>

class CCloudMediaClientDlg;
namespace ic
{
	class media_edge_client : public ic::dk_ipc_client
	{
	public:
		media_edge_client(CCloudMediaClientDlg * front);
		virtual ~media_edge_client(void);

		void assoc_completion_callback(void);
		void leave_completion_callback(void);

	private:
		CCloudMediaClientDlg * _front;
	};

};


#endif