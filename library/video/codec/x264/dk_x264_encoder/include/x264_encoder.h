#ifndef _X264_ENCODER_H_
#define _X264_ENCODER_H_

#include "dk_x264_encoder.h"
#include <stdint.h>
extern "C"
{
	#include <x264.h>
}

class x264_encoder
{
public:
	x264_encoder(dk_x264_encoder * front);
	~x264_encoder(void);

	dk_x264_encoder::ENCODER_STATE state(void);

	dk_x264_encoder::ERR_CODE initialize_encoder(dk_x264_encoder::configuration_t * config);
	dk_x264_encoder::ERR_CODE release_encoder(void);

	dk_x264_encoder::ERR_CODE encode(dk_video_encoder::dk_video_entity_t * input, dk_video_encoder::dk_video_entity_t * bitstream);
	dk_x264_encoder::ERR_CODE encode(dk_video_encoder::dk_video_entity_t * input);
	dk_x264_encoder::ERR_CODE get_queued_data(dk_video_encoder::dk_video_entity_t * input);

	dk_x264_encoder::ERR_CODE encode_async(dk_video_encoder::dk_video_entity_t * input);
	dk_x264_encoder::ERR_CODE check_encoding_finish(void);


	dk_x264_encoder::ERR_CODE encode(uint8_t * input, size_t & isize, uint8_t * output, size_t & osize, dk_x264_encoder::PIC_TYPE & pic_type, bool flush = false);

private:
	dk_x264_encoder::ENCODER_STATE _state;
	dk_x264_encoder::configuration_t * _config;
	dk_x264_encoder * _front;

	x264_param_t _param;
	x264_picture_t _pic_in;
	x264_picture_t _pic_out;
	x264_t * _encoder;
	int _frame_count;
};











#endif