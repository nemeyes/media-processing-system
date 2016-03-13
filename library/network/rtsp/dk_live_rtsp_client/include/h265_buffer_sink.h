#ifndef _H265_BUFFER_SINK_H_
#define _H265_BUFFER_SINK_H_

#include "h2645_buffer_sink.h"
#include "dk_live_rtsp_client.h"

class h265_buffer_sink : public h2645_buffer_sink
{
public:
	static h265_buffer_sink * createNew(dk_live_rtsp_client * front, UsageEnvironment & env, const char * vps, const char * sps, const char * pps, unsigned buffer_size);

protected:
	h265_buffer_sink(dk_live_rtsp_client * front, UsageEnvironment & env, const char * vps, const char * sps, const char * pps, unsigned buffer_size);
    virtual ~h265_buffer_sink(void);
};

#endif // H265_BUFFER_SINK_H

