#ifndef _DK_AAC_DECODER_H_
#define _DK_AAC_DECODER_H_


#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LIB)
#  define EXP_DLL __declspec(dllexport)
# else
#  define EXP_DLL __declspec(dllimport)
# endif
#else
# define EXP_DLL
#endif

class faad2_decoder;
class EXP_DLL dk_aac_decoder
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED,
	} ERR_CODE;

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



	typedef struct EXP_DLL _configuration_t
	{
		dk_aac_decoder::AAC_OBJECT_TYPE object_type;
		unsigned long samplerate;
		unsigned char channels;
		unsigned int bitpersamples;
		unsigned int bitrate;
		dk_aac_decoder::INPUT_FORMAT_TYPE input_format; // Bitstream output format (0 = Raw; 1 = ADTS)
		dk_aac_decoder::OUTPUT_FORMAT_TYPE output_format; // Bitstream output format (0 = Raw; 1 = ADTS)
		bool mix_down;
		_configuration_t(void)
			: object_type(dk_aac_decoder::AAC_OBJECT_TYPE_SSR)
			, samplerate(0)
			, channels(2)
			, bitpersamples(16)
			, bitrate(0)
			, input_format(dk_aac_decoder::FORMAT_TYPE_RAW)
			, output_format(dk_aac_decoder::FORMAT_TYPE_16BIT)
			, mix_down(false)
		{
		}

		_configuration_t(const _configuration_t & clone)
		{
			object_type = clone.object_type;
			samplerate = clone.samplerate;
			channels = clone.channels;
			bitpersamples = clone.bitpersamples;
			input_format = clone.input_format;
			output_format = clone.output_format;
		}

		_configuration_t operator=(const _configuration_t & clone)
		{
			object_type = clone.object_type;
			samplerate = clone.samplerate;
			channels = clone.channels;
			bitpersamples = clone.bitpersamples;
			input_format = clone.input_format;
			output_format = clone.output_format;
			return (*this);
		}
	} configuration_t;


	dk_aac_decoder(void);
	~dk_aac_decoder(void);

	dk_aac_decoder::ERR_CODE initialize(dk_aac_decoder::configuration_t config, unsigned char * extra_data, int extra_data_size, int & samplerate, int & channels);
	dk_aac_decoder::ERR_CODE release(void);
	dk_aac_decoder::ERR_CODE decode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int & osize);

private:
	faad2_decoder * _core;

};









#endif