#pragma once

#include <cstdint>

#if defined(EXPORT_MF_PLAYER_FRAMEWORK)
#define EXP_MF_PLAYER_FRAMEWORK_CLASS __declspec(dllexport)
#else
#define EXP_MF_PLAYER_FRAMEWORK_CLASS __declspec(dllimport)
#endif

#include <vector>
#include <string>

class mf_player_framework;
class EXP_MF_PLAYER_FRAMEWORK_CLASS dk_mf_player_framework
{
public:
	typedef enum _err_code
	{
		err_code_success,
		err_code_failed
	} err_code;

	typedef enum _player_state
	{
		player_state_closed,
		player_state_ready,
		player_state_open_pending,
		player_state_started,
		player_state_paused,
		player_state_stopped,
		player_state_closing
	} player_state;

	typedef struct _gpu_desc_t
	{
		char description[128];
		uint32_t vendor_id;
		uint32_t device_id;
		uint32_t subsys_id;
		uint32_t revision;
		char luid[64];
		_gpu_desc_t(void)
			: vendor_id(0)
			, device_id(0)
			, subsys_id(0)
			, revision(0)
		{
			memset(description, 0x00, sizeof(description));
			memset(luid, 0x00, sizeof(luid));
		}

		_gpu_desc_t(const _gpu_desc_t & clone)
		{
			strncpy_s(description, clone.description, sizeof(description));
			vendor_id = clone.vendor_id;
			device_id = clone.device_id;
			subsys_id = clone.subsys_id;
			revision = clone.subsys_id;
			strncpy_s(luid, clone.luid, sizeof(luid));
		}

		_gpu_desc_t operator=(const _gpu_desc_t & clone)
		{
			strncpy_s(description, clone.description, sizeof(description));
			vendor_id = clone.vendor_id;
			device_id = clone.device_id;
			subsys_id = clone.subsys_id;
			revision = clone.subsys_id;
			strncpy_s(luid, clone.luid, sizeof(luid));
			return (*this);
		}
	} gpu_desc_t;

	dk_mf_player_framework(void);
	virtual ~dk_mf_player_framework(void);

	static void retreieve_gpus(std::vector<gpu_desc_t>  & adapters);

	// Playback control
	dk_mf_player_framework::err_code open_file(const wchar_t * file, uint32_t gpu_index, HWND hwnd);
	dk_mf_player_framework::err_code play(void);
	dk_mf_player_framework::err_code pause(void);
	dk_mf_player_framework::err_code stop(void);
	dk_mf_player_framework::player_state state(void) const;

	void on_keydown_right(void);
	void on_keyup_right(void);
	void on_keydown_left(void);
	void on_keyup_left(void);
	void on_keydown_up(void);
	void on_keyup_up(void);
	void on_keydown_down(void);
	void on_keyup_down(void);

private:
	mf_player_framework * _core;
};