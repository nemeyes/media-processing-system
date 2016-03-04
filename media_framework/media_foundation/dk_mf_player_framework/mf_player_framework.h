#pragma once
#include "dk_mf_player_framework.h"

#include "mf_defines.h"

class mf_player_framework : public IMFAsyncCallback
{
public:
	mf_player_framework(void);
	virtual ~mf_player_framework(void);

	// Playback control
	dk_mf_player_framework::ERR_CODE seek(int position);
	dk_mf_player_framework::ERR_CODE slowfoward_rate(float rate);
	dk_mf_player_framework::ERR_CODE fastforward_rate(float rate);


	dk_mf_player_framework::ERR_CODE open_file(const wchar_t * file, HWND hwnd);
	dk_mf_player_framework::ERR_CODE play(void);
	dk_mf_player_framework::ERR_CODE pause(void);
	dk_mf_player_framework::ERR_CODE stop(void);
	dk_mf_player_framework::STATE state(void) const;


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


	dk_mf_player_framework::STATE _state;
	ATL::CComPtr<IMFMediaSession> _session;
	ATL::CComPtr<IMFRateControl> _rate_control;
	ATL::CComPtr<IMFClock> _presentation_clock;

	ATL::CComQIPtr<IMFTopology> _topology;     // the topology itself
	ATL::CComQIPtr<IMFMediaSource> _media_source;       // the MF source

	ATL::CComPtr<IUnknown> _device_manager;

	long long _duration;

	ATL::CComPtr<IMFVideoDisplayControl> _video_display;
	//HWND _hwnd;





	// event fied when session close is complete
	HANDLE _close_completion_event;
};