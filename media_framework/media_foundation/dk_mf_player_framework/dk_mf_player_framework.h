#pragma once

#include <cstdint>

#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif

#include <vector>

class mf_player_framework;
class EXP_CLASS dk_mf_player_framework
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED
	} ERR_CODE;

	typedef enum _STATE
	{
		STATE_CLOSED,
		STATE_READY,
		STATE_OPEN_PENDING,
		STATE_STARTED,
		STATE_PAUSED,
		STATE_STOPPED,
		STATE_CLOSING
	} STATE;

	dk_mf_player_framework(void);
	virtual ~dk_mf_player_framework(void);

	dk_mf_player_framework::ERR_CODE initialize(HWND hwnd, bool aspect_ratio, bool use_clock, bool enable_audio);
	dk_mf_player_framework::ERR_CODE release(void);


	// Playback control
	dk_mf_player_framework::ERR_CODE seek(int position);
	dk_mf_player_framework::ERR_CODE slowfoward_rate(float rate);
	dk_mf_player_framework::ERR_CODE fastforward_rate(float rate);


	dk_mf_player_framework::ERR_CODE open_file(const wchar_t * file);
	dk_mf_player_framework::ERR_CODE play(void);
	dk_mf_player_framework::ERR_CODE pause(void);
	dk_mf_player_framework::ERR_CODE stop(void);
	dk_mf_player_framework::STATE state(void) const;


private:
	mf_player_framework * _core;
};