#include <windows.h>
#include <tchar.h>
#include "cu_enc_core.h"
#include "dk_nvenc_encoder.h"
#include "nvEncodeAPI.h"
#include "cu_enc_core.h"
#include <cstdio>
#include <cstdlib>
#include <stdio.h>

typedef NVENCSTATUS(NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);

dk_nvenc_encoder::dk_nvenc_encoder(void)
{
	_core = new cu_enc_core();
	//_cu_device = NULL;

	//_enc_buffer_count = 0;
	//memset(&_enc_config, 0, sizeof(_enc_config));
	//memset(&_enc_buffer, 0, sizeof(_enc_buffer));
	//memset(&_enc_eos_output_buffer, 0, sizeof(_enc_eos_output_buffer));
}

dk_nvenc_encoder::~dk_nvenc_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = NULL;
	}
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
