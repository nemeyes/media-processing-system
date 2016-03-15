#include <windows.h>
#include <process.h>
#include "media_file_reader.h"
#include <dk_multiple_media_buffering.h>

#define VIDEO_BUFFER_SIZE 1024 * 1024 * 2
#define AUDIO_BUFFER_SIZE 48000 * 2 * 8 //48000hz * 16bitdetph * 8 channels ex) for 2channel 192000

#define MAX_AUDIO_FRAME_SIZE 192000

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

#define AV_RB32(x)  \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) | \
               (((const uint8_t*)(x))[1] << 16) | \
               (((const uint8_t*)(x))[2] <<  8) | \
                ((const uint8_t*)(x))[3])

#define MIN(a,b) ((a) > (b) ? (b) : (a))

media_file_reader::media_file_reader(void)
{
	_video_buffer = static_cast<uint8_t*>(malloc(VIDEO_BUFFER_SIZE));
	_audio_buffer = static_cast<uint8_t*>(malloc((MAX_AUDIO_FRAME_SIZE * 3) / 2)); //48000hz * 16bitdetph * 8 channels
}

media_file_reader::~media_file_reader(void)
{
	if (_video_buffer)
		free(_video_buffer);
	if (_audio_buffer)
		free(_audio_buffer);
}

bool media_file_reader::open(const char * stream_name, long long timestamp, media_file_reader::vsubmedia_type & vsubmedia_type, media_file_reader::asubmedia_type & asubmedia_type)
{
	if (!stream_name || strlen(stream_name) < 1)
		return media_file_reader::err_code_fail;

	strncpy(_stream_name, stream_name, sizeof(_stream_name));
	_format_ctx = nullptr;

	_video_ctx = nullptr;
	_video_stream_index = -1;
	_video_stream = nullptr;
	_video_timer = (double)av_gettime() / 1000000.0;
	_video_last_dts = 0;
	_video_last_delay = 40e-3;
	_video_current_dts = 0;
	_video_current_dts_time = av_gettime();
	_video_clock = 0;
	_video_recv_keyframe = false;

	//open video file
	if (avformat_open_input(&_format_ctx, stream_name, NULL, NULL) != 0)
		return false; //couldn't open file

	//retrieve stream information
	if (avformat_find_stream_info(_format_ctx, NULL) < 0)
		return false; //couldn't find stream information

	_video_stream_index = -1;
	_audio_stream_index = -1;
	for (uint32_t index = 0; index<_format_ctx->nb_streams; index++)
	{
		if (_format_ctx->streams[index]->codec->codec_type == AVMEDIA_TYPE_VIDEO && _video_stream_index < 0)
			_video_stream_index = index;
		if (_format_ctx->streams[index]->codec->codec_type == AVMEDIA_TYPE_AUDIO && _audio_stream_index < 0)
			_audio_stream_index = index;
	}

	if (_video_stream_index == -1)
		return false;
	if (_audio_stream_index == -1)
		return false;

	_video_stream = _format_ctx->streams[_video_stream_index];
	_video_ctx = _video_stream->codec;
	switch (_video_ctx->codec_id)
	{
	case AV_CODEC_ID_MJPEG:
		_vsubmedia_type = vsubmedia_type_jpeg;
		break;
	case AV_CODEC_ID_MPEG4:
		_vsubmedia_type = vsubmedia_type_mpeg4;
		break;
	case AV_CODEC_ID_H264:
		_vsubmedia_type = vsubmedia_type_h264;
		break;
	default:
		_vsubmedia_type = unknown_video_type;
		break;
	}

	_audio_stream = _format_ctx->streams[_audio_stream_index];
	_audio_ctx = _audio_stream->codec;
	switch (_audio_ctx->codec_id)
	{
	case AV_CODEC_ID_MP3:
		_asubmedia_type = asubmedia_type_mp3;
		break;
	case AV_CODEC_ID_AAC:
		_asubmedia_type = asubmedia_type_aac;
		break;
	default:
		_asubmedia_type = unknown_audio_type;
		break;
	}

	vsubmedia_type = _vsubmedia_type;
	asubmedia_type = _asubmedia_type;

	buffering::err_code code = dk_multiple_media_buffering::instance().create(_stream_name);
	if (code == buffering::err_code_success)
	{
		unsigned thrdaddr = 0;
		_thread = (HANDLE)::_beginthreadex(NULL, 0, media_file_reader::process_cb, this, 0, &thrdaddr);
		return true;
	}
	else
	{
		close();
		return false;
	}
	
}

bool media_file_reader::close(void)
{
	_run = false;
	if (_thread != INVALID_HANDLE_VALUE)
	{
		::WaitForSingleObject(_thread, INFINITE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}

	if (_format_ctx)
	{
		avformat_close_input(&_format_ctx);
		_format_ctx = nullptr;
	}

	buffering::err_code code = dk_multiple_media_buffering::instance().destroy(_stream_name);
	if (code == buffering::err_code_success)
		return true;
	else
		return false;
}

unsigned media_file_reader::process_cb(void * param)
{
	media_file_reader * self = static_cast<media_file_reader*>(param);
	self->process();
	return 0;
}

void media_file_reader::process(void)
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;
	_run = true;

	double dts = 0.f;
	double delay = 0.f;
	double actual_delay = 0.f;

	for (; _run && av_read_frame(_format_ctx, &packet) >= 0;)
	{
		if (packet.stream_index == _video_stream_index)
		{
			if (packet.dts != AV_NOPTS_VALUE)
				dts = packet.dts;
			dts *= av_q2d(_video_stream->time_base);
			if (dts != 0)
				_video_clock = dts;
			else
				dts = _video_clock;

			delay = av_q2d(_video_ctx->time_base);
			/* if we are repeating a frame, adjust clock accordingly */
			//delay += src_frame->repeat_pict * (delay * 0.5);
			_video_clock += delay;

			_video_current_dts = dts;
			_video_current_dts_time = av_gettime();
			delay = _video_current_dts - _video_last_dts;
			if (delay <= 0 || delay >= 1.0)
			{
				delay = _video_last_delay;
			}
			_video_last_delay = delay;
			_video_last_dts = _video_current_dts;

			_video_timer += delay;
			actual_delay = _video_timer - (av_gettime() / 1000000.0);
			if (actual_delay < 0.010)
			{
				/* Really it should skip the picture instead */
				actual_delay = 0.010;
			}
			actual_delay = actual_delay * 1000 + 0.5;

			if (_vsubmedia_type == vsubmedia_type_h264)
			{
				uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
				uint8_t * video_data = _video_buffer;
				int32_t video_data_size = 0;
				int32_t index = 0;
				int32_t packet_size = packet.size;

				do
				{
					if (packet_size < (index + 4))
						break;
					video_data_size = (packet.data[index] << 24) + (packet.data[index + 1] << 16) + (packet.data[index + 2] << 8) + packet.data[index + 3];
					memmove(video_data, start_code, sizeof(start_code));
					index += sizeof(start_code);

					if (packet_size < (index + video_data_size) || (UINT32_MAX - index) < video_data_size)
						break;

					memcpy(video_data + index, &packet.data[index], video_data_size);

					bool is_idr = media_file_reader::is_idr(_vsubmedia_type, video_data[4] & 0x1F);
					if (is_idr && !_video_recv_keyframe)
					{
						uint8_t * sps = _video_ctx->extradata + 8;
						size_t sps_size = sps[-1];
						if (_video_ctx->extradata_size < (8 + sps_size))
							break; // We don't have a complete SPS
						else
						{
							memmove(_video_extradata, start_code, sizeof(start_code));
							memmove(_video_extradata + sizeof(start_code), sps, sps_size);
							dk_multiple_media_buffering::instance().set_sps(_stream_name, _video_extradata, sizeof(start_code) + sps_size);
							_video_extradata_size = sizeof(start_code) + sps_size;
						}

						uint8_t * pps = _video_ctx->extradata + 8 + sps_size + 3;
						size_t pps_size = pps[-1];
						if (_video_ctx->extradata_size < (8 + sps_size + 3 + pps_size))
							break;
						else
						{
							memmove(_video_extradata + sizeof(start_code) + sps_size, start_code, sizeof(start_code));
							memmove(_video_extradata + sizeof(start_code) + sps_size + sizeof(start_code), pps, pps_size);
							dk_multiple_media_buffering::instance().set_pps(_stream_name, _video_extradata + sizeof(start_code) + sps_size, sizeof(start_code) + pps_size);
							_video_extradata_size = _video_extradata_size + sizeof(start_code) + pps_size;
						}
						_video_recv_keyframe = true;
						dk_multiple_media_buffering::instance().push_video(_stream_name, video_data, video_data_size, 0);
					}

					if (_video_recv_keyframe)
						dk_multiple_media_buffering::instance().push_video(_stream_name, video_data, video_data_size, 0);

					index += video_data_size;
				} while (index < packet_size);
			}
		}
		else if (packet.stream_index == _audio_stream_index)
		{
			
		}
		av_free_packet(&packet);
		::Sleep(10);
	}
}

bool media_file_reader::read(media_file_reader::media_type mt, uint8_t * data, size_t data_capacity, size_t & data_size, long long & timestamp)
{
	buffering::err_code code = buffering::err_code_failed;
	if (mt == media_file_reader::media_type_video)
	{
		code = dk_multiple_media_buffering::instance().pop_video(_stream_name, data, data_size, timestamp);
	}
	else if (mt == media_file_reader::media_type_audio)
	{
		code = dk_multiple_media_buffering::instance().pop_audio(_stream_name, data, data_size, timestamp);
	}

	if (code == buffering::err_code_failed)
		return false;
	else
		return true;
}

bool media_file_reader::is_vps(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_file_reader::vsubmedia_type_hevc && nal_unit_type == 32;
}

bool media_file_reader::is_sps(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_file_reader::vsubmedia_type_h264 ? nal_unit_type == 7 : nal_unit_type == 33;
}

bool media_file_reader::is_pps(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_file_reader::vsubmedia_type_h264 ? nal_unit_type == 8 : nal_unit_type == 34;
}

bool media_file_reader::is_idr(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_file_reader::vsubmedia_type_h264 ? nal_unit_type == 5 : nal_unit_type == 34;
}

bool media_file_reader::is_vlc(media_file_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_file_reader::vsubmedia_type_h264 ? (nal_unit_type <= 5 && nal_unit_type > 0) : (nal_unit_type <= 31);
}

const int32_t media_file_reader::find_nal_unit(uint8_t * bitstream, size_t size, int32_t * nal_start, int32_t * nal_end)
{
	int i;
	// find start
	*nal_start = 0;
	*nal_end = 0;

	i = 0;
	//( next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0 || bitstream[i + 3] != 0x01))
	{
		i++; // skip leading zero
		if (i + 4 >= size)
		{
			return 0;
		} // did not find nal start
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) // ( next_bits( 24 ) != 0x000001 )
	{
		i++;
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
	{
		/* error, should never happen */
		return 0;
	}

	i += 3;
	*nal_start = i;

	//( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01))
	{
		i++;
		// FIXME the next line fails when reading a nal that ends exactly at the end of the data
		if (i + 3 >= size)
		{
			*nal_end = size;
			return -1;
		} // did not find nal end, stream ended first
	}

	*nal_end = i;
	return (*nal_end - *nal_start);
}

const uint8_t * media_file_reader::find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state)
{
	int i;
	if (begin >= end)
		return end;

	for (i = 0; i < 3; i++)
	{
		uint32_t tmp = *state << 8;
		*state = tmp + *(begin++);
		if (tmp == 0x100 || begin == end)
			return begin;
	}

	while (begin < end)
	{
		if (begin[-1] > 1)
			begin += 3;
		else if (begin[-2])
			begin += 2;
		else if (begin[-3] | (begin[-1] - 1))
			begin++;
		else
		{
			begin++;
			break;
		}
	}

	begin = MIN(begin, end) - 4;
	*state = AV_RB32(begin);
	return begin + 4;
}