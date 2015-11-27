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
	typedef enum _MPEG_VERSION_TYPE
	{
		VERSION_TYPE_MPEG4,
		VERSION_TYPE_MPEG2
	} MPEG_VERSION_TYPE;

	typedef enum _AAC_OBJECT_TYPE
	{
		AAC_OBJECT_TYPE_MAIN,
		AAC_OBJECT_TYPE_LOW,
		AAC_OBJECT_TYPE_SSR,
		AAC_OBJECT_TYPE_LTP
	} AAC_OBJECT_TYPE;

	typedef enum _OUTPUT_FORMAT_TYPE
	{
		FORMAT_TYPE_RAW,
		FORMAT_TYPE_ADTS,
		FORMAT_TYPE_LATM
	} OUTPUT_FORMAT_TYPE;

	typedef enum _INPUT_FORMAT_TYPE
	{
		FORMAT_TYPE_16BIT,
		FORMAT_TYPE_24BIT,
		FORMAT_TYPE_32BIT,
		FORMAT_TYPE_FLOAT
	} INPUT_FORMAT_TYPE;

	typedef enum _BLOCK_TYPE
	{
		BLOCK_TYPE_NORMAL,
		BLOCK_TYPE_NOSHORT,
		BLOCK_TYPE_NOLONG
	} BLOCK_TYPE;

	typedef struct EXP_CLASS _configuration_t
	{
		dk_aac_encoder::MPEG_VERSION_TYPE mpeg_version;
		dk_aac_encoder::AAC_OBJECT_TYPE object_type;
		int32_t allow_midside; // Allow mid/side coding
		int32_t use_lfe; // Use one of the channels as LFE channel
		int32_t use_tns; // Use Temporal Noise Shaping
		int32_t bitrate; // bitrate / channel of AAC file
		int32_t samplerate;
		int32_t channels;
		int32_t bitdepth;
		unsigned long framesize;
		unsigned long ob;
		int32_t bandwidth; // AAC file frequency bandwidth
		unsigned long quantization_quality; // Quantizer quality
		dk_aac_encoder::BLOCK_TYPE shortctl;
		dk_aac_encoder::INPUT_FORMAT_TYPE input_format;
		dk_aac_encoder::OUTPUT_FORMAT_TYPE output_format; // Bitstream output format (0 = Raw; 1 = ADTS)
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;


	dk_aac_encoder(void);
	virtual ~dk_aac_encoder(void);

	//dk_aac_encoder::ERR_CODE initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, uint8_t * extra_data, size_t & extra_data_size);

	dk_aac_encoder::ERR_CODE initialize_encoder(void * config);
	dk_aac_encoder::ERR_CODE release_encoder(void);

	dk_aac_encoder::ERR_CODE encode(dk_audio_entity_t * pcm, dk_audio_entity_t * encoded);
	dk_aac_encoder::ERR_CODE encode(dk_audio_entity_t * pcm);
	dk_aac_encoder::ERR_CODE get_queued_data(dk_audio_entity_t * encoded);

	virtual uint8_t * extradata(void);
	virtual size_t extradata_size(void);

	//dk_aac_encoder::ERR_CODE encode(int32_t * input, size_t isize, uint8_t * output, size_t osize, size_t & bytes_written);
	//dk_aac_encoder::ERR_CODE encode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize);

private:
	faac_encoder * _core;
};




#endif