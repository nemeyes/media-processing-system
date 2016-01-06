#ifndef _DK_AAC_DECODER_H_
#define _DK_AAC_DECODER_H_

#include <cstdint>

#include <dk_audio_base.h>

class faad2_decoder;
class EXP_CLASS dk_aac_decoder : public dk_audio_decoder
{
public:
	typedef enum _AAC_OBJECT_TYPE
	{
		AAC_OBJECT_TYPE_MAIN,
		AAC_OBJECT_TYPE_LOW,
		AAC_OBJECT_TYPE_SSR,
		AAC_OBJECT_TYPE_LTP,
		AAC_OBJECT_TYPE_HE_AAC,
		AAC_OBJECT_TYPE_ER_LC,
		AAC_OBJECT_TYPE_ER_LTP,
		AAC_OBJECT_TYPE_LD,
		AAC_OBJECT_TYPE_DRM_ER_LC
	} AAC_OBJECT_TYPE;

	typedef enum _INPUT_FORMAT_TYPE
	{
		FORMAT_TYPE_RAW,
		FORMAT_TYPE_ADIF,
		FORMAT_TYPE_ADTS,
		FORMAT_TYPE_LATM
	} INPUT_FORMAT_TYPE;

	typedef enum _SBR_SIGNALLING
	{
		SBR_SIGNALLING_NO_SBR,
		SBR_SIGNALLING_SBR_UPSAMPLED,
		SBR_SIGNALLING_SBR_DOWNSAMPLED,
		SBR_SIGNALLING_NO_SBR_UPSAMPLED
	} SBR_SIGNALLING;

	typedef enum _OUTPUT_FORMAT_TYPE
	{
		FORMAT_TYPE_16BIT,
		FORMAT_TYPE_24BIT,
		FORMAT_TYPE_32BIT,
		FORMAT_TYPE_FLOAT,
		FORMAT_TYPE_FIXED,
		FORMAT_TYPE_DOUBLE
	} OUTPUT_FORMAT_TYPE;

	typedef struct EXP_CLASS _configuration_t
	{
		dk_aac_decoder::AAC_OBJECT_TYPE object_type;
		unsigned long samplerate;
		unsigned char channels;
		unsigned int bitdepth;
		unsigned int bitrate;
		dk_aac_decoder::INPUT_FORMAT_TYPE input_format; // Bitstream output format (0 = Raw; 1 = ADTS)
		dk_aac_decoder::OUTPUT_FORMAT_TYPE output_format; // Bitstream output format (0 = Raw; 1 = ADTS)
		uint8_t extradata[100];
		size_t extradata_size;
		bool mix_down;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;


	dk_aac_decoder(void);
	virtual ~dk_aac_decoder(void);


	dk_aac_decoder::ERR_CODE initialize_decoder(void* config);
	dk_aac_decoder::ERR_CODE release_decoder(void);

	dk_aac_decoder::ERR_CODE decode(dk_aac_decoder::dk_audio_entity_t * encoded, dk_aac_decoder::dk_audio_entity_t * pcm);

	/*dk_aac_decoder::ERR_CODE initialize(dk_aac_decoder::configuration_t config, unsigned char * extra_data, int extra_data_size, int & samplerate, int & channels);
	dk_aac_decoder::ERR_CODE release(void);
	dk_aac_decoder::ERR_CODE decode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int & osize);*/

private:
	faad2_decoder * _core;

};









#endif