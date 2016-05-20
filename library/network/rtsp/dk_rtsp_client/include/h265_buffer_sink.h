#ifndef _H265_BUFFER_SINK_H_
#define _H265_BUFFER_SINK_H_

#include "h2645_buffer_sink.h"
#include "dk_rtsp_client.h"

class h265_buffer_sink : public h2645_buffer_sink
{
public:
	static h265_buffer_sink * createNew(debuggerking::rtsp_client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size);

protected:
	h265_buffer_sink(debuggerking::rtsp_client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size);
    virtual ~h265_buffer_sink(void);
};

#endif // H265_BUFFER_SINK_H

