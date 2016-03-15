#ifndef _BUFFER_SINK_H_
#define _BUFFER_SINK_H_

#include <MediaSink.hh>
#include <UsageEnvironment.hh>
#include "dk_live_rtsp_client.h"

class buffer_sink : public MediaSink
{
public:
	static buffer_sink * createNew(dk_live_rtsp_client * front, dk_live_rtsp_client::media_type mt, int32_t smt, UsageEnvironment & env, unsigned buffer_size);

    virtual void add_data(unsigned char * data, unsigned size, struct timeval presentation_time);

protected:
	buffer_sink(dk_live_rtsp_client * front, dk_live_rtsp_client::media_type mt, int32_t smt, UsageEnvironment & env, unsigned buffer_size);
    virtual ~buffer_sink(void);

protected: //redefined virtual functions
    virtual Boolean continuePlaying(void);

protected:
    static void after_getting_frame(void * param, unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec);
	virtual void after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval timestamp);


	dk_live_rtsp_client * _front;
    unsigned char * _buffer;
    unsigned _buffer_size;

	bool _change_sps;
	bool _change_pps;
	bool _recv_idr;
	unsigned char _video_extradata[400];
	size_t _video_extradata_size;


    struct timeval _prev_presentation_time;
    unsigned _same_presentation_time_counter;

	dk_live_rtsp_client::media_type _mt;
	dk_live_rtsp_client::vsubmedia_type _vsmt;
	dk_live_rtsp_client::asubmedia_type _asmt;
};

#endif // BUFFER_SINK_H

