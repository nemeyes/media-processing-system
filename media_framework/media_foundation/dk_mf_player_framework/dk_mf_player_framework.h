#pragma once

#include <dk_basic_type.h>

#if defined(EXPORT_MF_PLAYER_FRAMEWORK)
#define EXP_MF_PLAYER_FRAMEWORK_CLASS __declspec(dllexport)
#else
#define EXP_MF_PLAYER_FRAMEWORK_CLASS __declspec(dllimport)
#endif

#include <vector>
#include <string>

namespace debuggerking
{
	class mf_player_core;
	class EXP_MF_PLAYER_FRAMEWORK_CLASS mf_player_framework : public foundation
	{
	public:
		typedef enum _player_state
		{
			state_closed,
			state_ready,
			state_open_pending,
			state_started,
			state_paused,
			state_stopped,
			state_closing
		} player_state;

		typedef struct _configuration_t
		{
			wchar_t uuid[MAX_PATH];
			wchar_t address[MAX_PATH];
			int32_t port_number;
			int32_t reconnect;
			wchar_t filepath[MAX_PATH];
			int32_t protocol;
			int32_t video_codec;
			int32_t video_width;
			int32_t video_height;
			int32_t video_bitrate;
			int32_t video_fps;
			int32_t video_keyframe_interval;
			int32_t gpu_index;
			int32_t enable_present;
			HWND hwnd;
			int32_t audio_codec;
			int32_t audio_bitrate;
			int32_t enable_repeat;
			_configuration_t(void)
				: port_number(15000)
				, reconnect(true)
				, protocol(mf_player_framework::protocol_type_t::rtmp)
				, video_codec(mf_player_framework::video_submedia_type_t::avc)
				, video_width(0)
				, video_height(0)
				, video_bitrate(0)
				, video_fps(0)
				, video_keyframe_interval(0)
				, gpu_index(0)
				, enable_present(false)
				, hwnd(NULL)
				, audio_codec(mf_player_framework::audio_submedia_type_t::aac)
				, audio_bitrate(0)
				, enable_repeat(true)
			{}
		} configuration_t;

		typedef struct _gpu_desc_t
		{
			char description[128];
			int32_t adaptor_index;
			uint32_t vendor_id;
			uint32_t device_id;
			uint32_t subsys_id;
			uint32_t revision;
			int32_t coord_left;
			int32_t coord_top;
			int32_t coord_right;
			int32_t coord_bottom;
			char luid[64];
			_gpu_desc_t(void)
				: adaptor_index(-1)
				, vendor_id(0)
				, device_id(0)
				, subsys_id(0)
				, revision(0)
				, coord_left(0)
				, coord_top(0)
				, coord_right(0)
				, coord_bottom(0)
			{
				memset(description, 0x00, sizeof(description));
				memset(luid, 0x00, sizeof(luid));
			}

			_gpu_desc_t(const _gpu_desc_t & clone)
			{
				strncpy_s(description, clone.description, sizeof(description));
				adaptor_index = clone.adaptor_index;
				vendor_id = clone.vendor_id;
				device_id = clone.device_id;
				subsys_id = clone.subsys_id;
				revision = clone.subsys_id;
				strncpy_s(luid, clone.luid, sizeof(luid));
				coord_left = clone.coord_left;
				coord_top = clone.coord_top;
				coord_right = clone.coord_right;
				coord_bottom = clone.coord_bottom;
			}

			_gpu_desc_t operator=(const _gpu_desc_t & clone)
			{
				strncpy_s(description, clone.description, sizeof(description));
				adaptor_index = clone.adaptor_index;
				vendor_id = clone.vendor_id;
				device_id = clone.device_id;
				subsys_id = clone.subsys_id;
				revision = clone.subsys_id;
				strncpy_s(luid, clone.luid, sizeof(luid));
				coord_left = clone.coord_left;
				coord_top = clone.coord_top;
				coord_right = clone.coord_right;
				coord_bottom = clone.coord_bottom;
				return (*this);
			}

		} gpu_desc_t;

		mf_player_framework(void);
		virtual ~mf_player_framework(void);

		static void retreieve_gpus(std::vector<gpu_desc_t>  & adapters);

		// Playback control
		int32_t open_file(mf_player_framework::configuration_t * config);
		int32_t play(void);
		int32_t pause(void);
		int32_t stop(void);
		mf_player_framework::player_state state(void) const;

		void on_keydown_right(void);
		void on_keyup_right(void);
		void on_keydown_left(void);
		void on_keyup_left(void);
		void on_keydown_up(void);
		void on_keyup_up(void);
		void on_keydown_down(void);
		void on_keyup_down(void);


	private:
		mf_player_core * _core;
	};
};
