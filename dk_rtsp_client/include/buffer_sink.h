#ifndef _BUFFER_SINK_H_
#define _BUFFER_SINK_H_

#include <MediaSink.hh>
#include <UsageEnvironment.hh>
#include "dk_rtsp_client.h"

class buffer_sink : public MediaSink
{
public:
	static buffer_sink * createNew(dk_rtsp_client * front, dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, UsageEnvironment & env, unsigned buffer_size);

    virtual void add_data(unsigned char * data, unsigned size, struct timeval presentation_time);

protected:
	buffer_sink(dk_rtsp_client * front, dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, UsageEnvironment & env, unsigned buffer_size);
    virtual ~buffer_sink(void);

protected: //redefined virtual functions
    virtual Boolean continuePlaying(void);

protected:
    static void after_getting_frame(void * param, unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec);
    virtual void after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time);


	dk_rtsp_client * _front;
    unsigned char * _buffer;
    unsigned _buffer_size;

	unsigned char _extra_data[400];
	size_t _extra_data_size;


    struct timeval _prev_presentation_time;
    unsigned _same_presentation_time_counter;

	dk_rtsp_client::MEDIA_TYPE_T _mt;
	dk_rtsp_client::SUBMEDIA_TYPE_T _smt;
};

#endif // BUFFER_SINK_H

