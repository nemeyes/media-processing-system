#include "buffer_sink.h"
#include <GroupsockHelper.hh>

buffer_sink::buffer_sink(debuggerking::rtsp_client * front, int32_t mt, int32_t smt, UsageEnvironment & env, unsigned buffer_size)
    : MediaSink(env)
	, _front(front)
    , _buffer_size(buffer_size)
    , _same_presentation_time_counter(0)
	, _mt(mt)
	, _change_sps(false)
	, _change_pps(false)
	, _recv_idr(false)
{
	if (_mt == debuggerking::rtsp_client::media_type_t::video)
		_vsmt = smt;
	if (_mt == debuggerking::rtsp_client::media_type_t::audio)
		_asmt = smt;

    _buffer = new unsigned char[buffer_size];
    _prev_presentation_time.tv_sec = ~0;
    _prev_presentation_time.tv_usec = ~0;
}

buffer_sink::~buffer_sink(void)
{
	if (_buffer)
	{
		delete[] _buffer;
		_buffer = 0;
	}
}

buffer_sink* buffer_sink::createNew(debuggerking::rtsp_client * front, int32_t mt, int32_t smt, UsageEnvironment & env, unsigned buffer_size)
{
	return new buffer_sink(front, mt, smt, env, buffer_size);
}

Boolean buffer_sink::continuePlaying(void)
{
    if( !fSource )
        return False;

    fSource->getNextFrame(_buffer, _buffer_size, after_getting_frame, this, onSourceClosure, this);
    return True;
}

void buffer_sink::after_getting_frame(void * param, unsigned frame_size, unsigned truncated_bytes, struct timeval timestamp, unsigned /*duration_msec*/)
{
    buffer_sink * sink = static_cast<buffer_sink*>(param);
	sink->after_getting_frame(frame_size, truncated_bytes, timestamp);
}

void buffer_sink::add_data(unsigned char * data, unsigned data_size, struct timeval timestamp)
{
    //put data to output
	if(_front)
	{
		if (_mt == debuggerking::rtsp_client::media_type_t::video)
		{
			if (_vsmt == debuggerking::rtsp_client::video_submedia_type_t::h264)
			{
				size_t saved_sps_size = 0;
				unsigned char * saved_sps = _front->get_sps(saved_sps_size);
				size_t saved_pps_size = 0;
				unsigned char * saved_pps = _front->get_pps(saved_pps_size);

				bool is_sps = debuggerking::rtsp_client::is_sps(_vsmt, data[4] & 0x1F);
				if (is_sps)
				{
					if (saved_sps_size < 1 || !saved_sps)
					{
						_front->set_sps(data, data_size);
						_change_sps = true;
					}
					else
					{
						if (memcmp(saved_sps, data, saved_sps_size))
						{
							_front->set_sps(data, data_size);
							_change_sps = true;
						}
					}
				}

				bool is_pps = debuggerking::rtsp_client::is_pps(_vsmt, data[4] & 0x1F);
				if (is_pps)
				{
					if (saved_pps_size < 1 || !saved_pps)
					{
						_front->set_pps(data, data_size);
						_change_pps = true;
					}
					else
					{
						if (memcmp(saved_pps, data, saved_pps_size))
						{
							_front->set_pps(data, data_size);
							_change_pps = true;
						}
					}
				}

				bool is_idr = debuggerking::rtsp_client::is_idr(_vsmt, data[4] & 0x1F);
				if (_change_sps || _change_pps)
				{
					saved_sps = _front->get_sps(saved_sps_size);
					saved_pps = _front->get_pps(saved_pps_size);
					if ((saved_sps_size > 0) && (saved_pps_size > 0))
					{
						if (is_idr && !_recv_idr)
						{
							_recv_idr = true;
							_front->on_begin_video(_vsmt, nullptr, 0, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size, 0);
							_change_sps = false;
							_change_pps = false;
						}
					}
				}
				else if (saved_sps && saved_sps_size>0 && saved_pps && saved_pps_size>0)
				{
					if (is_idr && !_recv_idr)
					{
						_recv_idr = true;
						_front->on_begin_video(_vsmt, nullptr, 0, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size, 0);
					}
				}

				if (!is_sps && !is_pps && _recv_idr)
				{
					saved_sps = _front->get_sps(saved_sps_size);
					saved_pps = _front->get_pps(saved_pps_size);
					if (saved_sps_size > 0 && saved_pps_size > 0)
					{
						_front->on_recv_video(_vsmt, data, data_size, 0);
					}
				}
			}
			else if (_vsmt == debuggerking::rtsp_client::video_submedia_type_t::hevc)
			{

			}
		}		
		else if (_mt == debuggerking::rtsp_client::media_type_t::audio)
		{
			if (_asmt == debuggerking::rtsp_client::audio_submedia_type_t::aac)
			{

			}
		}
	}
}

void buffer_sink::after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval timestamp)
{
	if (_front)
	{
		if (_mt == debuggerking::rtsp_client::media_type_t::video)
		{
			if (_vsmt == debuggerking::rtsp_client::video_submedia_type_t::h264)
			{
				const unsigned char start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
				//if ((_buffer[0] == start_code[0]) && (_buffer[1] == start_code[1]) && (_buffer[2] == start_code[2]) && (_buffer[3] == start_code[3]))
				//{
				//	add_data(_buffer, frame_size, timestamp);
				//}
				//else
				{
					if (truncated_bytes > 0)
					{
						printf("the input frame data was too large for out buffer size\n");
						memmove(_buffer + 4, _buffer, frame_size - 4);
					}
					else
					{
						truncated_bytes = (frame_size + 4) - _buffer_size;
						if (truncated_bytes > 0 && (frame_size + 4) > _buffer_size)
							memmove(_buffer + 4, _buffer, frame_size - truncated_bytes);
						else
							memmove(_buffer + 4, _buffer, frame_size);
					}
					memmove(_buffer, start_code, sizeof(start_code));

					add_data(_buffer, frame_size + sizeof(start_code), timestamp);
				}
			}
		}
		else if (_mt == debuggerking::rtsp_client::media_type_t::audio)
		{
			add_data(_buffer, frame_size, timestamp);
		}
	}
    continuePlaying();
}