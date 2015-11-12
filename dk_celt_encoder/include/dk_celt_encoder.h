#ifndef _DK_CELT_ENCODER_H_
#define _DK_CELT_ENCODER_H_

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

class EXP_DLL dk_celt_encoder
{
public:
	dk_celt_encoder(void);
	~dk_celt_encoder(void);



};



#endif