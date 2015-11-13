#ifndef _DK_CELT_ENCODER_H_
#define _DK_CELT_ENCODER_H_

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

class celt_encoder;
class EXP_DLL dk_celt_encoder
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
		int32_t bytes_per_packet;
		_configuration_t(void)
			: samplerate(441000)
			, channels(2)
			, framesize(0)
			, bytes_per_packet(0)
		{
		}

		_configuration_t(const _configuration_t & clone)
		{
			samplerate = clone.samplerate;
			channels = clone.channels;
			framesize = clone.framesize;
			bytes_per_packet = clone.bytes_per_packet;
		}

		_configuration_t operator=(const _configuration_t & clone)
		{
			samplerate = clone.samplerate;
			channels = clone.channels;
			framesize = clone.framesize;
			bytes_per_packet = clone.bytes_per_packet;
			return (*this);
		}
	} configuration_t;

	dk_celt_encoder(void);
	~dk_celt_encoder(void);

	dk_celt_encoder::ERR_CODE initialize(dk_celt_encoder::configuration_t * config);
	dk_celt_encoder::ERR_CODE encode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize);
	dk_celt_encoder::ERR_CODE release(void);

private:
	celt_encoder * _core;
};



#endif