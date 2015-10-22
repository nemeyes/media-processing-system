#ifndef _DK_AAC_ENCODER_H_
#define _DK_AAC_ENCODER_H_

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

class aac_enc_core;
class EXP_DLL dk_aac_encoder
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED,
	} ERR_CODE;

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

	typedef struct EXP_DLL _configuration_t
	{
		dk_aac_encoder::MPEG_VERSION_TYPE mpeg_version;
		dk_aac_encoder::AAC_OBJECT_TYPE object_type;
		unsigned int allow_midside; // Allow mid/side coding
		unsigned int use_lfe; // Use one of the channels as LFE channel
		unsigned int use_tns; // Use Temporal Noise Shaping
		unsigned long bitrate; // bitrate / channel of AAC file
		unsigned int sample_rate;
		unsigned int channels;
		unsigned int bitpersamples;
		unsigned long input_samples;
		unsigned long max_output_bytes;
		unsigned int bandwidth; // AAC file frequency bandwidth
		unsigned long quantization_quality; // Quantizer quality
		dk_aac_encoder::BLOCK_TYPE shortctl;
		dk_aac_encoder::INPUT_FORMAT_TYPE input_format;
		dk_aac_encoder::OUTPUT_FORMAT_TYPE output_format; // Bitstream output format (0 = Raw; 1 = ADTS)

		/*
			32000,
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
			256000
		*/
		_configuration_t(void)
			: mpeg_version(dk_aac_encoder::VERSION_TYPE_MPEG4)
			, object_type(dk_aac_encoder::AAC_OBJECT_TYPE_LOW)
			, allow_midside(1)
			, use_lfe(0)
			, use_tns(1)
			, bitrate(128000)
			, sample_rate(0)
			, channels(2)
			, bitpersamples(32)
			, input_samples(0)
			, max_output_bytes(0)
			, bandwidth(0)
			, quantization_quality(100)
			, shortctl(BLOCK_TYPE_NOSHORT)
			, input_format(dk_aac_encoder::FORMAT_TYPE_16BIT)
			, output_format(dk_aac_encoder::FORMAT_TYPE_RAW)
		{
		}

		_configuration_t(const _configuration_t & clone)
		{
			mpeg_version = clone.mpeg_version;
			object_type = clone.object_type;
			allow_midside = clone.allow_midside;
			use_lfe = clone.use_lfe;
			use_tns = clone.use_tns;
			bitrate = clone.bitrate;
			sample_rate = clone.sample_rate;
			channels = clone.channels;
			bitpersamples = clone.bitpersamples;
			input_samples = clone.input_samples;
			max_output_bytes = clone.max_output_bytes;
			bandwidth = clone.bandwidth;
			quantization_quality = clone.quantization_quality;
			shortctl = clone.shortctl;
			input_format = clone.input_format;
			output_format = clone.output_format;
		}

		_configuration_t operator=(const _configuration_t & clone)
		{
			mpeg_version = clone.mpeg_version;
			object_type = clone.object_type;
			allow_midside = clone.allow_midside;
			use_lfe = clone.use_lfe;
			use_tns = clone.use_tns;
			bitrate = clone.bitrate;
			sample_rate = clone.sample_rate;
			channels = clone.channels;
			bitpersamples = clone.bitpersamples;
			input_samples = clone.input_samples;
			max_output_bytes = clone.max_output_bytes;
			bandwidth = clone.bandwidth;
			quantization_quality = clone.quantization_quality;
			shortctl = clone.shortctl;
			input_format = clone.input_format;
			output_format = clone.output_format;
			return (*this);
		}
	} configuration_t;


	dk_aac_encoder(void);
	~dk_aac_encoder(void);

	dk_aac_encoder::ERR_CODE initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, unsigned char * extra_data, unsigned long & extra_data_size);
	dk_aac_encoder::ERR_CODE release(void);
	dk_aac_encoder::ERR_CODE encode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int &osize);

private:
	aac_enc_core * _core;

};




#endif