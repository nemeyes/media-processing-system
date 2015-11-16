#ifndef _DK_CELT_DECODER_H_
#define _DK_CELT_DECODER_H_

#include <cstdint>

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

class celt_decoder;
class EXP_DLL dk_celt_decoder
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED,
	} ERR_CODE;

	typedef struct EXP_DLL _configuration_t
	{
		int32_t samplerate;
		int32_t channels;
		int32_t framesize;
		int32_t bitdepth;
		_configuration_t(void)
			: samplerate(48000)
			, channels(2)
			, framesize(960)
			, bitdepth(16)
		{
		}

		_configuration_t(const _configuration_t & clone)
		{
			samplerate = clone.samplerate;
			channels = clone.channels;
			framesize = clone.framesize;
			bitdepth = clone.bitdepth;
		}

		_configuration_t operator=(const _configuration_t & clone)
		{
			samplerate = clone.samplerate;
			channels = clone.channels;
			framesize = clone.framesize;
			bitdepth = clone.bitdepth;
			return (*this);
		}
	} configuration_t;

	dk_celt_decoder(void);
	~dk_celt_decoder(void);

	dk_celt_decoder::ERR_CODE initialize(dk_celt_decoder::configuration_t * config);
	dk_celt_decoder::ERR_CODE decode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize);
	dk_celt_decoder::ERR_CODE release(void);

private:
	celt_decoder * _core;
};



#endif