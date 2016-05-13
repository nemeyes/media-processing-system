#ifndef _DK_AAC_ENCODER_H_
#define _DK_AAC_ENCODER_H_

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
namespace debuggerking
{
	class faac_encoder;
	class EXP_CLASS aac_encoder : public audio_encoder
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

		typedef struct EXP_CLASS _configuration_t : public audio_encoder::configuration_t
		{
			aac_encoder::mpeg_version_type mpeg_version;
			aac_encoder::aac_object_type object_type;
			int32_t allow_midside; // Allow mid/side coding
			int32_t use_lfe; // Use one of the channels as LFE channel
			int32_t use_tns; // Use Temporal Noise Shaping
			int32_t bitdepth;
			unsigned long framesize;
			unsigned long ob;
			int32_t bandwidth; // AAC file frequency bandwidth
			unsigned long quantization_quality; // Quantizer quality
			aac_encoder::block_type shortctl;
			aac_encoder::input_format_type input_format;
			aac_encoder::output_format_type output_format; // Bitstream output format (0 = Raw; 1 = ADTS)
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t operator=(const _configuration_t & clone);
		} configuration_t;


		aac_encoder(void);
		virtual ~aac_encoder(void);

		int32_t initialize_encoder(void * config);
		int32_t release_encoder(void);

		int32_t encode(aac_encoder::entity_t * pcm, aac_encoder::entity_t * encoded);
		int32_t encode(aac_encoder::entity_t * pcm);
		int32_t get_queued_data(aac_encoder::entity_t * encoded);
		virtual void after_encoding_callback(uint8_t * bistream, size_t size);

	private:
		faac_encoder * _core;
	};
};



#endif