#include "buffer_sink.h"
#include <GroupsockHelper.hh>
#include "stream_parser.h"

buffer_sink::buffer_sink(dk_rtsp_client * front, dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, UsageEnvironment & env, unsigned buffer_size)
    : MediaSink(env)
	, _front(front)
    , _buffer_size(buffer_size)
    , _same_presentation_time_counter(0)
	, _mt(mt)
	, _smt(smt)
	, _change_sps(false)
	, _change_pps(false)
	, _sps_not_changed(false)
	, _pps_not_changed(false)
	, _is_first_idr_rcvd(false)
{
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

buffer_sink* buffer_sink::createNew(dk_rtsp_client * front, dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, UsageEnvironment & env, unsigned buffer_size)
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
		if ((_mt == dk_rtsp_client::MEDIA_TYPE_VIDEO) && (_smt == dk_rtsp_client::SUBMEDIA_TYPE_H264))
		{
			size_t saved_sps_size = 0;
			unsigned char * saved_sps = _front->get_sps(saved_sps_size);
			size_t saved_pps_size = 0;
			unsigned char * saved_pps = _front->get_pps(saved_pps_size);

			bool is_sps = stream_parser::is_sps(_smt, data[4] & 0x1F);
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
					else
					{
						_sps_not_changed = true;
					}
				}
			}

			bool is_pps = stream_parser::is_pps(_smt, data[4] & 0x1F);
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
					else
					{
						_pps_not_changed = true;
					}
				}
			}

			bool is_idr = stream_parser::is_idr(_smt, data[4] & 0x1F);

			if (_change_sps || _change_pps)
			{
				saved_sps = _front->get_sps(saved_sps_size);
				saved_pps = _front->get_pps(saved_pps_size);
				if ((saved_sps_size > 0) && (saved_pps_size > 0))
				{
					memcpy(_extra_data, saved_sps, saved_sps_size);
					memcpy(_extra_data + saved_sps_size, saved_pps, saved_pps_size);
					_extra_data_size = saved_sps_size + saved_pps_size;

					if (is_idr && !_is_first_idr_rcvd)
					{
						memmove(data + _extra_data_size, data, data_size);
						memmove(data, _extra_data, _extra_data_size);
						_is_first_idr_rcvd = true;

						_front->on_begin_media(_mt, _smt, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size + _extra_data_size, presentation_time);
						_change_sps = false;
						_change_pps = false;
					}
				}
			}

			//if (_sps_not_changed && _pps_not_changed && is_idr && !_is_first_idr_rcvd)
			//	_is_first_idr_rcvd = true;
			
			if (!is_sps && !is_pps && _is_first_idr_rcvd)
			{
				saved_sps = _front->get_sps(saved_sps_size);
				saved_pps = _front->get_pps(saved_pps_size);
				if (saved_sps_size > 0 && saved_pps_size > 0)
				{
					_front->on_recv_media(_mt, _smt, data, data_size, presentation_time);
				}
			}

			/*size_t saved_sps_size = 0;
			unsigned char * saved_sps = _front->get_sps(saved_sps_size);
			size_t saved_pps_size = 0;
			unsigned char * saved_pps = _front->get_pps(saved_pps_size);

			bool change_sps = false;
			bool change_pps = false;

			uint8_t * begin = data;
			uint8_t * end = data;
			uint32_t remained_size = data_size;

			bool exit_loop = false;

			while (begin < data + data_size)
			{
				int nal_begin, nal_end;
				int nal_size = stream_parser::find_nal_unit(begin, remained_size, &nal_begin, &nal_end);
				if (nal_size == 0)
				{
					return; //could not find any nal unit
				}
				else if (nal_size < 0)
				{
					begin += nal_begin;
					end += nal_end;
					exit_loop = true;
				}
				else
				{
					begin += nal_begin;
					end += nal_end;
					remained_size -= nal_size;
				}

				bool is_sps = stream_parser::is_sps(_smt, begin[0] & 0x1F);
				if (is_sps)
				{
					const uint8_t * sps_begin = begin - 4;
					const uint8_t * sps_end = end;
					if (saved_sps_size < 1 || !saved_sps)
					{
						_front->set_sps((unsigned char*)sps_begin, sps_end - sps_begin);
						change_sps = true;
					}
					else
					{
						if (memcmp(saved_sps, sps_begin, saved_sps_size))
						{
							_front->set_sps((unsigned char*)sps_begin, sps_end - sps_begin);
							change_sps = true;
						}
					}
				}

				bool is_pps = stream_parser::is_pps(_smt, begin[0] & 0x1F);
				if (is_pps)
				{
					const uint8_t * pps_begin = begin - 4;
					const uint8_t * pps_end = end;
					if (saved_pps_size < 1 || !saved_pps)
					{
						_front->set_pps((unsigned char*)pps_begin, pps_end - pps_begin);
						change_pps = true;
					}
					else
					{
						if (memcmp(saved_pps, pps_begin, saved_pps_size))
						{
							_front->set_pps((unsigned char*)pps_begin, pps_end - pps_begin);
							change_pps = true;
						}
					}
				}

				if (change_sps || change_pps)
				{
					saved_sps = _front->get_sps(saved_sps_size);
					saved_pps = _front->get_pps(saved_pps_size);
					if ((saved_sps_size > 0) && (saved_pps_size > 0))
					{
						memcpy(_extra_data, saved_sps, saved_sps_size);
						memcpy(_extra_data + saved_sps_size, saved_pps, saved_pps_size);
						_extra_data_size = saved_sps_size + saved_pps_size;
						_front->on_begin_media(_mt, _smt, _extra_data, _extra_data_size, presentation_time);
					}
				}
				else
				{
					if (!is_sps && !is_pps)
					{
						saved_sps = _front->get_sps(saved_sps_size);
						saved_pps = _front->get_pps(saved_pps_size);
						if (saved_sps_size > 0 && saved_pps_size > 0)
						{
							const uint8_t * nalu_begin = begin - 4;
							const uint8_t * nalu_end = end;
							_front->on_recv_media(_mt, _smt, nalu_begin, nalu_end - nalu_begin, presentation_time);
						}
					}
				}

				if (exit_loop)
					break;

				begin = end;
			}*/
		}
		else if ((_mt == dk_rtsp_client::MEDIA_TYPE_VIDEO) && (_smt == dk_rtsp_client::SUBMEDIA_TYPE_H265))
		{

		}
#else
		_front->on_recv_media(_mt, _smt, data, data_size, presentation_time);
#endif
	}
}

void buffer_sink::after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time)
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
	//memcpy(_buffer, start_code, sizeof(start_code));
    add_data(_buffer, frame_size+4, presentation_time);

    continuePlaying();
}