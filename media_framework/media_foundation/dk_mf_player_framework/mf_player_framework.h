#pragma once
#include "dk_mf_player_framework.h"

#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <atlbase.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <evr.h>
#include <wmcontainer.h>
#include <initguid.h>
#include <mferror.h>

#define BREAK_ON_FAIL(value)            if(FAILED(value)) break;
#define BREAK_ON_NULL(value, newHr)     if(value == NULL) { hr = newHr; break; }
#define RETURN_ON_FAIL(value) if(FAILED(value)) return value;

const IID BORROWED_IID_IMFDXGIDeviceManager = { 0xeb533d5d, 0x2db6, 0x40f8, { 0x97, 0xa9, 0x49, 0x46, 0x92, 0x01, 0x4f, 0x07 } };
const GUID BORROWED_MF_SA_D3D11_AWARE = { 0x206b4fc8, 0xfcf9, 0x4c51, { 0xaf, 0xe3, 0x97, 0x64, 0x36, 0x9e, 0x33, 0xa0 } };

class mf_player_framework : public IMFAsyncCallback
{
public:
	mf_player_framework(void);
	virtual ~mf_player_framework(void);

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

	//create custom video renderer
	HRESULT create_dx11_video_renderer_activate(HWND hwnd, IMFActivate ** activate);
	HRESULT create_video_decoder_node(IMFMediaType * media_type, IMFTopologyNode ** decoder, ULONG_PTR device_manager, IMFTransform ** transform);
	HRESULT find_video_decoder(REFCLSID subtype, uint32_t width, uint32_t height, IMFTransform ** decoder);

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
	HWND _hwnd;





	// event fied when session close is complete
	HANDLE _close_completion_event;
};