#ifndef _DK_AAC_DECODER_H_
#define _DK_AAC_DECODER_H_

#include <dk_audio_base.h>

namespace debuggerking
{
	class faad2_decoder;
	class EXP_CLASS aac_decoder : public audio_decoder
	{
	public:
		typedef enum _aac_object_type
		{
			aac_object_type_main,
			aac_object_type_low,
			aac_object_type_ssr,
			aac_object_type_ltp,
			aac_object_type_he_aac,
			aac_object_type_er_lc,
			aac_object_type_er_ltp,
			aac_object_type_ld,
			aac_object_type_drm_er_lc
		} aac_object_type;

		typedef enum _input_format_type
		{
			format_type_raw,
			format_type_adif,
			format_type_adts,
			format_type_latm
		} input_format_type;

		typedef enum _sbr_signalling
		{
			sbr_signalling_no_sbr,
			sbr_signalling_sbr_upsampled,
			sbr_signalling_sbr_downsampled,
			sbr_signalling_no_sbr_upsampled
		} sbr_signalling;

		typedef enum _output_format_type
		{
			format_type_16bit,
			format_type_24bit,
			format_type_32bit,
			format_type_float,
			format_type_fixed,
			format_type_double
		} output_format_type;

		typedef struct EXP_CLASS _configuration_t : public audio_decoder::configuration_t
		{
			aac_decoder::aac_object_type object_type;
			aac_decoder::input_format_type input_format; // Bitstream output format (0 = Raw; 1 = ADTS)
			aac_decoder::output_format_type output_format; // Bitstream output format (0 = Raw; 1 = ADTS)
			bool mix_down;
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t operator=(const _configuration_t & clone);
		} configuration_t;


		aac_decoder(void);
		virtual ~aac_decoder(void);

		int32_t initialize_decoder(void* config);
		int32_t release_decoder(void);
		int32_t decode(aac_decoder::entity_t * encoded, aac_decoder::entity_t * pcm);
		int32_t decode(audio_decoder::entity_t * encoded);
		int32_t get_queued_data(audio_decoder::entity_t * pcm);
		virtual void after_decoding_callback(uint8_t * decoded, size_t size);

	private:
		faad2_decoder * _core;

	};
};








#endif