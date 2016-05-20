#ifndef _H264_BUFFER_SINK_H_
#define _H264_BUFFER_SINK_H_

#include "h2645_buffer_sink.h"
#include "dk_rtsp_client.h"

class h264_buffer_sink : public h2645_buffer_sink
{
public:
	static h264_buffer_sink * createNew(debuggerking::rtsp_client * front, UsageEnvironment & env, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size = 100000);

protected:
	h264_buffer_sink(debuggerking::rtsp_client * front, UsageEnvironment & env, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size);
    virtual ~h264_buffer_sink(void);
};


#endif // H264_BUFFER_SINK_H

