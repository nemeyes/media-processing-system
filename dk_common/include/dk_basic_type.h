#ifndef _DK_BASIC_TYPE_H_
#define _DK_BASIC_TYPE_H_

#include <limits.h>         // So we can set the bounds of our types
#include <stddef.h>         // For size_t
#include <string.h>         // for memcpy
//#include <stdint.h>
#include <cstdint>
#include <cstdlib>

#if defined(WIN32)
#define GG_LONGLONG(x) x##I64
#define GG_ULONGLONG(x) x##UI64
#if !defined(MAX_PATH)
#define MAX_PATH 260
#endif
#else
#define GG_LONGLONG(x) x##LL
#define GG_ULONGLONG(x) x##ULL
#endif

const uint8_t  uint8_max = ((uint8_t)0xFF);
const uint16_t uint16_max = ((uint16_t)0xFFFF);
const uint32_t uint32_max = ((uint32_t)0xFFFFFFFF);
const uint64_t uint64_max = ((uint64_t)GG_LONGLONG(0xFFFFFFFFFFFFFFFF));
const int8_t  int8_min = ((int8_t)0x80);
const int8_t  int8_max = ((int8_t)0x7F);
const int16_t int16_min = ((int16_t)0x8000);
const int16_t int16_max = ((int16_t)0x7FFF);
const int32_t int32_min = ((int32_t)0x80000000);
const int32_t int32_max = ((int32_t)0x7FFFFFFF);
const int64_t int64_min = ((int64_t)GG_LONGLONG(0x8000000000000000));
const int64_t int64_max = ((int64_t)GG_LONGLONG(0x7FFFFFFFFFFFFFFF));

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName();                                    \
  DISALLOW_COPY_AND_ASSIGN(TypeName)

// Use implicit_cast as a safe version of static_cast or const_cast
// for upcasting in the type hierarchy (i.e. casting a pointer to Foo
// to a pointer to SuperclassOfFoo or casting a pointer to Foo to
// a const pointer to Foo).
// When you use implicit_cast, the compiler checks that the cast is safe.
// Such explicit implicit_casts are necessary in surprisingly many
// situations where C++ demands an exact type match instead of an
// argument type convertable to a target type.
//
// The From type can be inferred, so the preferred syntax for using
// implicit_cast is the same as for static_cast etc.:
//
//   implicit_cast<ToType>(expr)
//
// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.
template<typename To, typename From>
inline To implicit_cast(From const &f) 
{
	return f;
}

namespace debuggerking
{
	class foundation
	{
	public:
		typedef struct _err_code_t
		{
			static const int32_t success = 0;
			static const int32_t fail = 1;
			static const int32_t not_implemented = 2;
			static const int32_t unsupported_function = 3;
			static const int32_t invalid_encoding_device = 4;
			static const int32_t encoding_under_processing = 5;
		} err_code_t;

		typedef struct _max_media_value_t
		{
			static const int32_t max_video_size = 1024 * 1024 * 16;
			static const int32_t max_audio_channels = 8;
			static const int32_t max_audio_samplerate = 48000;
			static const int32_t max_audio_bitdepth = 16;
			static const int32_t max_audio_size = max_audio_samplerate*(max_audio_bitdepth / 8)* max_audio_channels;//705600; // 44100 (Samples) * 2 (Bytes per Sample) * 8 (channels), 44100*2*8
		} max_media_value_t;

		typedef struct _protocol_type_t
		{
			static const int32_t casp = 0;
			static const int32_t rtmp = 1;
			static const int32_t rtsp = 2;
		} protocol_type_t;

		typedef struct _media_type_t
		{
			static const int32_t video = 0;
			static const int32_t audio = 1;
		} media_type_t;

		typedef struct _video_submedia_type_t
		{
			static const int32_t unknown = -1;
			static const int32_t sorenson_h263 = 0;
			static const int32_t screen_video = 1;
			static const int32_t vp6 = 2;
			static const int32_t vp_with_alpha_channel = 3;
			static const int32_t screen_video_version2 = 4;
			static const int32_t avc = 5;
			static const int32_t h264 = 6;
			static const int32_t h264_bp = 7;
			static const int32_t h264_hp = 8;
			static const int32_t h264_mp = 9;
			static const int32_t h264_ep = 10;
			static const int32_t mp4v = 11;
			static const int32_t mp4v_sp = 12;
			static const int32_t mp4v_asp = 13;
			static const int32_t jpeg = 14;
			static const int32_t hevc = 15;
			static const int32_t hevc_mp = 16;
			static const int32_t vc1 = 17;
			static const int32_t mvc = 18;
			static const int32_t vp8 = 19;
			static const int32_t rgb32 = 20;
			static const int32_t rgb24 = 21;
			static const int32_t yuy2 = 22;
			static const int32_t i420 = 23;
			static const int32_t yv12 = 24;
			static const int32_t nv12 = 25;
			static const int32_t boundary = nv12;
		} video_submedia_type_t;

		typedef struct _entropy_coding_mode_t
		{
			static const int32_t unknown = -1;
			static const int32_t cabac = 0;
			static const int32_t cavlc = 1;
		} entropy_coding_mode_t;

		typedef struct _video_memory_type_t
		{
			static const int32_t host = 0;
			static const int32_t d3d9 = 1;
			static const int32_t d3d10 = 2;
			static const int32_t d3d11 = 3;
			static const int32_t d3d12 = 4;
			static const int32_t opengl = 5;
		} video_memory_type_t;

		typedef struct _video_picture_type_t
		{
			static const int32_t unknown = 0;
			static const int32_t idr = 1;
			static const int32_t i = 2;
			static const int32_t p = 3;
			static const int32_t b = 4;
		} video_picture_type_t;

		typedef struct _audio_submedia_type_t
		{
			static const int32_t unknown = -1;
			static const int32_t linear_pcm_pe = 0; //platform endian
			static const int32_t adpcm = 1;
			static const int32_t mp3 = 2;
			static const int32_t linear_pcm_le = 3; //little endian
			static const int32_t nellymoser_16khz = 4;
			static const int32_t nellymoser_8khz = 5;
			static const int32_t nellymoser = 6;
			static const int32_t alaw = 7;
			static const int32_t mlaw = 8;
			static const int32_t aac = 9;
			static const int32_t speex = 10;
			static const int32_t mp3_8khz = 11;
			static const int32_t ac3 = 12;
			static const int32_t celt = 13;
		} audio_submedia_type_t;

		typedef struct _recv_option_t
		{
			static const int32_t none = 0x00;
			static const int32_t audio = 0x01;
			static const int32_t video = 0x02;
		} recv_option_t;

		typedef struct _focus_option_t
		{
			static const int32_t nothing = 0x00;
			static const int32_t audio = 0x01;
			static const int32_t video = 0x02;
		} focus_option_t;

	};
};

#endif