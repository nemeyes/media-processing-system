#pragma once

#include <windows.h>
#include <cstdlib>
#include <cstdint>
#include <map>

namespace buffering
{
	typedef enum _err_code
	{
		err_code_success,
		err_code_failed
	} err_code;

	typedef enum _vsubmedia_type
	{
		unknown_video_type = -1,
		vsubmedia_type_avc,
		vsubmedia_type_hevc,
		vsubmedia_type_mpeg4,
		vsubmedia_type_jpeg,
	} vsubmedia_type;

	typedef enum _asubmedia_type
	{
		unknown_audio_type = -1,
		audio_submedia_type_aac,
		audio_submedia_type_mp3,
		audio_submedia_type_ac3,
		audio_submedia_type_celt,
	} asubmedia_type;
}