
#include "h2645_buffer_sink.h"
#include <H264VideoRTPSource.hh>

h2645_buffer_sink::h2645_buffer_sink(dk_rtsp_client * front, dk_rtsp_client::SUBMEDIA_TYPE_T smt, UsageEnvironment & env, unsigned buffer_size, const char * vps, const char * sps, const char * pps)
	: buffer_sink(front, dk_rtsp_client::MEDIA_TYPE_VIDEO, smt, env, buffer_size)
	, _receive_first_frame(false)
{
    _vspps[0] = vps;
    _vspps[1] = sps;
    _vspps[2] = pps;
}

h2645_buffer_sink::~h2645_buffer_sink(void)
{
}

void h2645_buffer_sink::after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time)
{
    //const unsigned char start_code[4] = {0x00, 0x00, 0x00, 0x01};
    
	if (!_front->ignore_sdp())
	{
		if (!_receive_first_frame)
		{
			//if we have NAL units encoded in "sprop parameter string",
			for (unsigned i = 0; i < 3; ++i)
			{
				unsigned number_of_spspps_records;
				SPropRecord * spspps_records = parseSPropParameterSets(_vspps[i], number_of_spspps_records);
				for (unsigned j = 0; j < number_of_spspps_records; ++j)
				{
					//add_data(start_code, 4, presentation_time);
					add_data(spspps_records[j].sPropBytes, spspps_records[j].sPropLength, presentation_time);
				}
				delete[] spspps_records;
			}
			_receive_first_frame = true;
		}
    }

    //add_data( start_code, 4, presentation_time );
    buffer_sink::after_getting_frame( frame_size, truncated_bytes, presentation_time );
}
