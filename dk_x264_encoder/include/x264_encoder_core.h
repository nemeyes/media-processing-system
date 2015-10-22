#ifndef _X264_ENCODER_CORE_H_
#define _X264_ENCODER_CORE_H_

#include "dk_x264_encoder.h"
#include <stdint.h>
extern "C"
{
	#include <x264.h>
}

class x264_encoder_core
{
public:
	x264_encoder_core(void);
	~x264_encoder_core(void);

	dk_x264_encoder::ERR_CODE initialize(dk_x264_encoder::configuration_t conf);
	dk_x264_encoder::ERR_CODE release(void);
	dk_x264_encoder::ERR_CODE encode(uint8_t * input, size_t & isize, uint8_t * output, size_t & osize, dk_x264_encoder::PIC_TYPE & pic_type, bool flush = false);

private:
	dk_x264_encoder::configuration_t _config;

	x264_param_t _dk_param;
	x264_picture_t _dk_pic_in;
	x264_picture_t _dk_pic_out;
	x264_t * _dk_encoder;
	int _dk_frame_count;
};











#endif