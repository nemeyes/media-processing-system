#pragma once
#include "dk_mf_player_framework.h"
#include "mf_defines.h"

namespace debuggerking
{
	class mf_player_core : public IMFAsyncCallback
	{
	public:
		mf_player_core(void);
		virtual ~mf_player_core(void);

		// Playback control
		int32_t seek(int position);
		int32_t slowfoward_rate(float rate);
		int32_t fastforward_rate(float rate);


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


		//IUnKnown Methods
		STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		//IMFAsyncCallback Methods
		// Skip the optional GetParameters() function - it is used only in advanced players.
		// Returning the E_NOTIMPL error code causes the system to use default parameters.
		STDMETHODIMP GetParameters(DWORD * flags, DWORD * queue)   { return E_NOTIMPL; }
		// Main MF event handling function
		STDMETHODIMP Invoke(IMFAsyncResult * async_result);


	private:
		HRESULT create_session(void);
		HRESULT close_session(void);
		HRESULT start_playback(void);
		HRESULT determine_duration(void);

		HRESULT process_event(CComPtr<IMFMediaEvent> & media_event);

		// Media event handlers
		HRESULT topology_ready_callback(void);
		HRESULT presentation_ended_callback(void);

		HRESULT shutdown_source(void);

	private:
		volatile long _ref_count;

		mf_player_framework::configuration_t * _config;
		mf_player_framework::player_state _state;
		ATL::CComPtr<IMFMediaSession> _session;
		ATL::CComPtr<IMFRateControl> _rate_control;
		ATL::CComPtr<IMFClock> _presentation_clock;

		ATL::CComQIPtr<IMFTopology> _topology;     // the topology itself
		ATL::CComQIPtr<IMFMediaSource> _media_source;       // the MF source
		ATL::CComPtr<IKeyEvent> _key_event;

		ATL::CComPtr<IUnknown> _device_manager;

		long long _duration;

		ATL::CComPtr<IMFVideoDisplayControl> _video_display;
		//HWND _hwnd;
		// event fied when session close is complete
		HANDLE _close_completion_event;
	};
};
