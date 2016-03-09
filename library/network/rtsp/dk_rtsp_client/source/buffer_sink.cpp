#include "buffer_sink.h"
#include <GroupsockHelper.hh>
#include "stream_parser.h"

buffer_sink::buffer_sink(dk_rtsp_client * front, dk_rtsp_client::MEDIA_TYPE_T mt, int32_t smt, UsageEnvironment & env, unsigned buffer_size)
    : MediaSink(env)
	, _front(front)
    , _buffer_size(buffer_size)
    , _same_presentation_time_counter(0)
	, _mt(mt)
	, _change_sps(false)
	, _change_pps(false)
	, _recv_idr(false)
{
	if (_mt == dk_rtsp_client::MEDIA_TYPE_VIDEO)
		_vsmt = dk_rtsp_client::VIDEO_SUBMEDIA_TYPE_T(smt);
	if (_mt == dk_rtsp_client::MEDIA_TYPE_AUDIO)
		_asmt = dk_rtsp_client::AUDIO_SUBMEDIA_TYPE_T(smt);

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

buffer_sink* buffer_sink::createNew(dk_rtsp_client * front, dk_rtsp_client::MEDIA_TYPE_T mt, int32_t smt, UsageEnvironment & env, unsigned buffer_size)
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

void buffer_sink::after_getting_frame(void * param, unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned /*duration_msec*/)
{
    buffer_sink * sink = static_cast<buffer_sink*>(param);
    sink->after_getting_frame(frame_size, truncated_bytes, presentation_time);
}

void buffer_sink::add_data(unsigned char * data, unsigned data_size, struct timeval presentation_time)
{
    //put data to output
	if(_front)
	{
#if 1
		//nal_unit_type specifies the type of RBSP data structure contained in
		//the NAL unit as specified in Table 7-1. VCL NAL units
		//are specified as those NAL units having nal_unit_type
		//equal to 1 to 5, inclusive. All remaining NAL units
		//are called non-VCL NAL units:

		//0  Unspecified
		//1  Coded slice of a non-IDR picture slice_layer_without_partitioning_rbsp( )
		//2  Coded slice data partition A slice_data_partition_a_layer_rbsp( )
		//3  Coded slice data partition B slice_data_partition_b_layer_rbsp( )
		//4  Coded slice data partition C slice_data_partition_c_layer_rbsp( )
		//5  Coded slice of an IDR picture slice_layer_without_partitioning_rbsp( )
		//6  Supplemental enhancement information (SEI) 5 sei_rbsp( )
		//7  Sequence parameter set (SPS) seq_parameter_set_rbsp( )
		//8  Picture parameter set pic_parameter_set_rbsp( )
		//9  Access unit delimiter access_unit_delimiter_rbsp( )
		//10 End of sequence end_of_seq_rbsp( )
		//11 End of stream end_of_stream_rbsp( )
		if ((_mt == dk_rtsp_client::MEDIA_TYPE_VIDEO) && (_vsmt == dk_rtsp_client::SUBMEDIA_TYPE_H264))
		{
			size_t saved_sps_size = 0;
			unsigned char * saved_sps = _front->get_sps(saved_sps_size);
			size_t saved_pps_size = 0;
			unsigned char * saved_pps = _front->get_pps(saved_pps_size);

			bool is_sps = stream_parser::is_sps(_vsmt, data[4] & 0x1F);
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

			bool is_pps = stream_parser::is_pps(_vsmt, data[4] & 0x1F);
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

			bool is_idr = stream_parser::is_idr(_vsmt, data[4] & 0x1F);

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

			//if (_sps_not_changed && _pps_not_changed && is_idr && !_is_first_idr_rcvd)
			//	_is_first_idr_rcvd = true;
			
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
		else if ((_mt == dk_rtsp_client::MEDIA_TYPE_VIDEO) && (_vsmt == dk_rtsp_client::SUBMEDIA_TYPE_HEVC))
		{

		}
#else
		_front->on_recv_media(_mt, _smt, data, data_size, presentation_time);
#endif
	}
}

void buffer_sink::after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval timestamp)
{
	const unsigned char start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
	if (truncated_bytes>0)
	{
		printf("the input frame data was too large for out buffer size\n");
		memmove(_buffer+4, _buffer, frame_size - 4);
	}
	else
	{
		truncated_bytes = (frame_size + 4) - _buffer_size;
		if (truncated_bytes>0 && (frame_size + 4)>_buffer_size)
			memmove(_buffer+4, _buffer, frame_size - truncated_bytes);
		else
			memmove(_buffer+4, _buffer, frame_size);
	}
	memmove(_buffer, start_code, sizeof(start_code));
	add_data(_buffer, frame_size + sizeof(start_code), timestamp);

    continuePlaying();
}