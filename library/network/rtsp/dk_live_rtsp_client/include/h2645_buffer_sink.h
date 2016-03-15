#ifndef _H2645_BUFFER_SINK_H_
#define _H2645_BUFFER_SINK_H_

#include "buffer_sink.h"
#include "dk_live_rtsp_client.h"

class h2645_buffer_sink : public buffer_sink
{
public:
	h2645_buffer_sink(dk_live_rtsp_client * front, dk_live_rtsp_client::vsubmedia_type smt, UsageEnvironment & env, unsigned buffer_size, const char * vps, const char * sps = 0, const char * pps = 0);
    virtual ~h2645_buffer_sink(void);

protected:
    virtual void after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time);

private:
    const char * _vspps[3];
    bool _receive_first_frame;

};

#endif // H264_BUFFER_SINK_H

