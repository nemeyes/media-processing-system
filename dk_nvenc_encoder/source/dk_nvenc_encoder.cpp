#include <windows.h>
#include <tchar.h>
#include "nvenc_encoder.h"
#include "dk_nvenc_encoder.h"
#include "nvEncodeAPI.h"
#include <cstdio>
#include <cstdlib>
#include <stdio.h>

typedef NVENCSTATUS(NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);

dk_nvenc_encoder::dk_nvenc_encoder(void)
{
	_core = new nvenc_encoder();
}

dk_nvenc_encoder::~dk_nvenc_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::initialize_encoder(void * config)
{
	return _core->initialize(static_cast<dk_nvenc_encoder::configuration_t*>(config));
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::release_encoder(void)
{
	return _core->release();
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input, dk_nvenc_encoder::dk_video_entity_t * bitstream)
{
	return _core->encode(input, bitstream);
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input)
{
	return _core->encode(input);
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::get_queued_data(dk_nvenc_encoder::dk_video_entity_t * bitstream)
{
	return _core->get_queued_data(bitstream);
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::encode_async(dk_nvenc_encoder::dk_video_entity_t * input)
{
	return _core->encode_async(input);
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::check_encoding_flnish(void)
{
	return _core->check_encoding_flnish();
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::initialize(dk_nvenc_encoder::configuration_t config, unsigned int * pitch)
{
	return _core->initialize(config, pitch);
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::release(void)
{
	return _core->release();
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, PIC_TYPE & pic_type, bool flush)
{
	NV_ENC_PIC_TYPE enc_pic_type;
	dk_nvenc_encoder::ERR_CODE result = _core->encode(input, isize, output, osize, enc_pic_type);
	switch (enc_pic_type)
	{
	case NV_ENC_PIC_TYPE_P :
		pic_type = PIC_TYPE_P;
		break;
	case NV_ENC_PIC_TYPE_B:
		pic_type = PIC_TYPE_B;
		break;
	case NV_ENC_PIC_TYPE_I:
		pic_type = PIC_TYPE_I;
		break;
	case NV_ENC_PIC_TYPE_IDR:
		pic_type = PIC_TYPE_IDR;
		break;
	case NV_ENC_PIC_TYPE_BI:
		pic_type = PIC_TYPE_BI;
		break;
	case NV_ENC_PIC_TYPE_SKIPPED:
		pic_type = PIC_TYPE_SKIPPED;
		break;
	case NV_ENC_PIC_TYPE_INTRA_REFRESH:
		pic_type = PIC_TYPE_INTRA_REFRESH;
		break;
	case NV_ENC_PIC_TYPE_UNKNOWN:
		pic_type = PIC_TYPE_UNKNOWN;
		break;
	}

	return result;
}
