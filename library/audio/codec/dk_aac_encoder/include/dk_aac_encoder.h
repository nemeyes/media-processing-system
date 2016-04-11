#ifndef _DK_AAC_ENCODER_H_
#define _DK_AAC_ENCODER_H_

#include <cstdint>

#include <dk_audio_base.h>

/* TODO
1. verify various input format(PCM 8bit, 24bit, 32bit)
2. verify various output format(ADTS, LATM)
3. verify various bitrate (32000,
			40000,
			48000,
			56000,
			64000,
			72000,
			80000,
			88000,
			96000,
			104000,
			112000,
			120000,
			128000,
			140000,
			160000,
			192000,
			224000,
			256000)
*/

class faac_encoder;
class EXP_CLASS dk_aac_encoder : public dk_audio_encoder
{
public:
	typedef enum _mpeg_version_type
	{
		version_type_mpeg4,
		version_type_mpeg2
	} mpeg_version_type;

	typedef enum _aac_object_type
	{
		aac_object_type_main,
		aac_object_type_low,
		aac_object_type_ssr,
		aac_object_type_ltp
	} aac_object_type;

	typedef enum _output_format_type
	{
		format_type_raw,
		format_type_adts,
		format_type_latm
	} output_format_type;

	typedef enum _input_format_type
	{
		format_type_16bit,
		format_type_24bit,
		format_type_32bit,
		format_type_float
	} input_format_type;

	typedef enum _block_type
	{
		block_type_normal,
		block_type_noshort,
		block_type_nolong
	} block_type;

	typedef struct EXP_CLASS _configuration_t : public dk_audio_encoder::configuration_t
	{
		dk_aac_encoder::mpeg_version_type mpeg_version;
		dk_aac_encoder::aac_object_type object_type;
		int32_t allow_midside; // Allow mid/side coding
		int32_t use_lfe; // Use one of the channels as LFE channel
		int32_t use_tns; // Use Temporal Noise Shaping
		int32_t bitdepth;
		unsigned long framesize;
		unsigned long ob;
		int32_t bandwidth; // AAC file frequency bandwidth
		unsigned long quantization_quality; // Quantizer quality
		dk_aac_encoder::block_type shortctl;
		dk_aac_encoder::input_format_type input_format;
		dk_aac_encoder::output_format_type output_format; // Bitstream output format (0 = Raw; 1 = ADTS)
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;


	dk_aac_encoder(void);
	virtual ~dk_aac_encoder(void);

	//dk_aac_encoder::ERR_CODE initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, uint8_t * extra_data, size_t & extra_data_size);

	dk_aac_encoder::err_code initialize_encoder(void * config);
	dk_aac_encoder::err_code release_encoder(void);

	dk_aac_encoder::err_code encode(dk_aac_encoder::dk_audio_entity_t * pcm, dk_aac_encoder::dk_audio_entity_t * encoded);
	dk_aac_encoder::err_code encode(dk_aac_encoder::dk_audio_entity_t * pcm);
	dk_aac_encoder::err_code get_queued_data(dk_aac_encoder::dk_audio_entity_t * encoded);

	virtual uint8_t * extradata(void);
	virtual size_t extradata_size(void);

	//dk_aac_encoder::ERR_CODE encode(int32_t * input, size_t isize, uint8_t * output, size_t osize, size_t & bytes_written);
	//dk_aac_encoder::ERR_CODE encode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize);

private:
	faac_encoder * _core;
};




#endif