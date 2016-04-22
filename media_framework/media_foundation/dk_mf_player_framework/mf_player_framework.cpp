#include <windows.h>
#include "mf_player_framework.h"
//#include <mferror.h>
#include "mf_topology_builder.h"

mf_player_framework::mf_player_framework(void)
	: _ref_count(1)
	, _state(dk_mf_player_framework::player_state_closed)
	, _session(NULL)
	, _rate_control(NULL)
	, _presentation_clock(NULL)
	, _duration(0)
	, _video_display(NULL)
{
	MFStartup(MF_VERSION);
	_close_completion_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

mf_player_framework::~mf_player_framework(void)
{
	stop();
	close_session();
	shutdown_source();
	if (_topology)
	{
		_topology.Release();
		//_topology = NULL;
	}
	if (_video_display)
	{
		_video_display.Release();
		_video_display = NULL;
	}
	MFShutdown();
	::CloseHandle(_close_completion_event);
}

// Playback control
dk_mf_player_framework::err_code mf_player_framework::seek(int position)
{
	return dk_mf_player_framework::err_code_success;
}

dk_mf_player_framework::err_code mf_player_framework::slowfoward_rate(float rate)
{
	HRESULT hr = S_OK;
	BOOL thin = FALSE;
	do
	{
		if (_state != dk_mf_player_framework::player_state_started)
		{
			hr = MF_E_INVALIDREQUEST;
			break;
		}
		BREAK_ON_NULL(_rate_control, E_UNEXPECTED);

		// set the rate
		hr = _rate_control->SetRate(thin, -rate);
	} while (0);

	if (FAILED(hr))
		return dk_mf_player_framework::err_code_failed;
	else
		return dk_mf_player_framework::err_code_success;
}

dk_mf_player_framework::err_code mf_player_framework::fastforward_rate(float rate)
{
	HRESULT hr = S_OK;
	BOOL thin = FALSE;
	do
	{
		if (_state != dk_mf_player_framework::player_state_started)
		{
			hr = MF_E_INVALIDREQUEST;
			break;
		}
		BREAK_ON_NULL(_rate_control, E_UNEXPECTED);

		// set the rate
		hr = _rate_control->SetRate(thin, rate);
	} while (0);

	if (FAILED(hr))
		return dk_mf_player_framework::err_code_failed;
	else
		return dk_mf_player_framework::err_code_success;
}

dk_mf_player_framework::err_code mf_player_framework::open_file(const wchar_t * file, uint32_t gpu_index, HWND hwnd)
{
	HRESULT hr = S_OK;
	if (hwnd == NULL)
		return dk_mf_player_framework::err_code_failed;

	do
	{
		// Step 1: create a media session if one doesn't exist already
		// close the session if one is already created
		stop();
		close_session();
		shutdown_source();

		_topology.Release();
		_device_manager.Release();

		hr = create_session();
		BREAK_ON_FAIL(hr);

		// Step 2 : create a media source for specified URL string, The URL can be a path to a stream or it can be a path to a local file
		hr = mf_topology_builder::create_source(file, &_media_source);
		BREAK_ON_FAIL(hr);

		// Step 3: build the topology
		CComQIPtr<IMFPresentationDescriptor> present_descriptor;
		DWORD number_of_streams = 0;
		do
		{
			
			hr = MFCreateTopology(&_topology);
			BREAK_ON_FAIL(hr);

			hr = _media_source->CreatePresentationDescriptor(&present_descriptor);
			BREAK_ON_FAIL(hr);

			hr = present_descriptor->GetStreamDescriptorCount(&number_of_streams);
			BREAK_ON_FAIL(hr);

			for (DWORD index = 0; index < number_of_streams; index++)
			{
				mf_topology_builder::add_branch_to_partial_topology(_topology, _media_source, index, present_descriptor, hwnd, gpu_index, &_device_manager, &_key_event);
			}
		} while (0);
		BREAK_ON_FAIL(hr);

		// Step 4: add the topology to the internal queue of topologies associated with this
		hr = _topology->SetUINT32(MF_TOPOLOGY_HARDWARE_MODE, MFTOPOLOGY_HWMODE_USE_HARDWARE);
		BREAK_ON_FAIL(hr);

		hr = _topology->SetUINT32(MF_TOPOLOGY_DXVA_MODE, MFTOPOLOGY_DXVA_FULL);
		BREAK_ON_FAIL(hr);

		// Step 5: add the topology to the internal queue of topologies associated with this
		// media session
		hr = _session->SetTopology(0, _topology);
		BREAK_ON_FAIL(hr);

		// If we've just initialized a brand new topology in step 1, set the player state 
		// to "open pending" - not playing yet, but ready to begin.
		if (_state == dk_mf_player_framework::player_state_ready)
		{
			_state = dk_mf_player_framework::player_state_open_pending;
		}
	} while (0);

	if (FAILED(hr))
	{
		_state = dk_mf_player_framework::player_state_closed;
		return dk_mf_player_framework::err_code_failed;
	}
	return dk_mf_player_framework::err_code_success;
}

dk_mf_player_framework::err_code mf_player_framework::play(void)
{
	if (_state != dk_mf_player_framework::player_state_paused && _state != dk_mf_player_framework::player_state_stopped)
		return dk_mf_player_framework::err_code_failed;

	if (_session == NULL)
		return dk_mf_player_framework::err_code_failed;

	HRESULT hr = start_playback();
	if (SUCCEEDED(hr))
		return dk_mf_player_framework::err_code_success;
	else
		return dk_mf_player_framework::err_code_failed;
}

dk_mf_player_framework::err_code mf_player_framework::pause(void)
{
	if (_state != dk_mf_player_framework::player_state_started)
		return dk_mf_player_framework::err_code_failed;

	if (_session == NULL)
		return dk_mf_player_framework::err_code_failed;

	HRESULT hr = _session->Pause();
	if (SUCCEEDED(hr))
	{
		_state = dk_mf_player_framework::player_state_paused;
		return dk_mf_player_framework::err_code_success;
	}
	else
		return dk_mf_player_framework::err_code_failed;
}

dk_mf_player_framework::err_code mf_player_framework::stop(void)
{
	HRESULT hr = S_OK;
	do
	{
		if (_state != dk_mf_player_framework::player_state_started)
		{
			hr = MF_E_INVALIDREQUEST;
			break;
		}

		BREAK_ON_NULL(_session, E_UNEXPECTED);

		hr = _session->Stop();
		BREAK_ON_FAIL(hr);

		_state = dk_mf_player_framework::player_state_stopped;
	} while (0);

	if (SUCCEEDED(hr))
		return dk_mf_player_framework::err_code_success;
	else
		return dk_mf_player_framework::err_code_failed;
}

dk_mf_player_framework::player_state mf_player_framework::state(void) const 
{ 
	return _state; 
}

void mf_player_framework::on_keydown_right(void)
{
	if (_key_event)
		_key_event->OnKeyDownRight();
}

void mf_player_framework::on_keyup_right(void)
{
	if (_key_event)
		_key_event->OnKeyUpRight();
}

void mf_player_framework::on_keydown_left(void)
{
	if (_key_event)
		_key_event->OnKeyDownLeft();
}

void mf_player_framework::on_keyup_left(void)
{
	if (_key_event)
		_key_event->OnKeyUpLeft();
}

void mf_player_framework::on_keydown_up(void)
{
	if (_key_event)
		_key_event->OnKeyDownUp();
}

void mf_player_framework::on_keyup_up(void)
{
	if (_key_event)
		_key_event->OnKeyUpUp();
}

void mf_player_framework::on_keydown_down(void)
{
	if (_key_event)
		_key_event->OnKeyDownDown();
}

void mf_player_framework::on_keyup_down(void)
{
	if (_key_event)
		_key_event->OnKeyUpDown();
}

HRESULT mf_player_framework::QueryInterface(REFIID riid, void ** ppv)
{
	HRESULT hr = S_OK;

	if (ppv == NULL)
	{
		return E_POINTER;
	}

	if (riid == IID_IMFAsyncCallback)
	{
		*ppv = static_cast<IMFAsyncCallback*>(this);
	}
	else if (riid == IID_IUnknown)
	{
		*ppv = static_cast<IUnknown*>(this);
	}
	else
	{
		*ppv = NULL;
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr))
		AddRef();

	return hr;
}

ULONG mf_player_framework::AddRef(void)
{
	return InterlockedIncrement(&_ref_count);
}

ULONG mf_player_framework::Release(void)
{
	ULONG uCount = InterlockedDecrement(&_ref_count);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}

//
// IMFAsyncCallback::Invoke implementation.  This is the function called by media session
// whenever anything of note happens or an asynchronous operation is complete.
//
// async_result - a pointer to the asynchronous result object which references the event 
// itself in the IMFMediaEventGenerator's event queue.  (The media session is the object
// that implements the IMFMediaEventGenerator interface.)
HRESULT mf_player_framework::Invoke(IMFAsyncResult * async_result)
{
	ATL::CComPtr<IMFMediaEvent> media_event;
	HRESULT hr = S_OK;
	MediaEventType media_event_type;

	do
	{
		// Get the event from the event queue.
		hr = _session->EndGetEvent(async_result, &media_event);
		BREAK_ON_FAIL(hr);

		// Get the event type.
		hr = media_event->GetType(&media_event_type);
		BREAK_ON_FAIL(hr);

		// MESessionClosed event is guaranteed to be the last event fired by the session. 
		// Fire the m_closeCompleteEvent to let the player know that it can safely shut 
		// down the session and release resources associated with the session.
		if (media_event_type == MESessionClosed)
		{
			SetEvent(_close_completion_event);
		}
		else
		{
			// If this is not the final event, tell the Media Session that this player is 
			// the object that will handle the next event in the queue.
			hr = _session->BeginGetEvent(this, NULL);
			BREAK_ON_FAIL(hr);
		}

		// If we are in a normal state, handle the event by passing it to the HandleEvent()
		// function.  Otherwise, if we are in the closing state, do nothing with the event.
		if (_state != dk_mf_player_framework::player_state_closing)
		{
			process_event(media_event);
		}
	} while (0);

	return S_OK;
}

HRESULT mf_player_framework::process_event(ATL::CComPtr<IMFMediaEvent> & media_event)
{
	HRESULT hr_status = S_OK;            // Event status
	HRESULT hr = S_OK;
	MF_TOPOSTATUS topo_status = MF_TOPOSTATUS_INVALID;
	MediaEventType media_event_type;

	do
	{
		BREAK_ON_NULL(media_event, E_POINTER);

		// Get the event type.
		hr = media_event->GetType(&media_event_type);
		BREAK_ON_FAIL(hr);

		// Get the event status. If the operation that triggered the event did
		// not succeed, the status is a failure code.
		hr = media_event->GetStatus(&hr_status);
		BREAK_ON_FAIL(hr);

		//Check if the async operation succeeded.
		if (FAILED(hr_status))
		{
			hr = hr_status;
			break;
		}

		// Switch on the event type. Update the internal state of the CPlayer as needed.
		if (media_event_type == MESessionTopologyStatus)
		{
			// Get the status code.
			hr = media_event->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&topo_status);
			BREAK_ON_FAIL(hr);

			if (topo_status == MF_TOPOSTATUS_READY)
			{
				hr = topology_ready_callback();
			}
		}
		else if (media_event_type == MEEndOfPresentation)
		{
			hr = presentation_ended_callback();
		}
	} while (0);
	return hr;
}

HRESULT mf_player_framework::topology_ready_callback(void)
{
	HRESULT hr = S_OK;

	// release any previous instance of the m_pVideoDisplay interface
	_video_display.Release();
	// Ask the session for the IMFVideoDisplayControl interface. 
	MFGetService(_session, MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&_video_display));

	// since the topology is ready, start playback
	hr = start_playback();

	_presentation_clock = NULL;
	_session->GetClock(&_presentation_clock);

	determine_duration();

	// get the rate control service that can be used to change the playback rate of the service
	_rate_control = NULL;
	MFGetService(_session, MF_RATE_CONTROL_SERVICE, IID_IMFRateControl, (void**)&_rate_control);

	return hr;
}

HRESULT mf_player_framework::presentation_ended_callback(void)
{
	// The session puts itself into the stopped state automatically.
	_state = dk_mf_player_framework::player_state_stopped;
	_session->Stop();
	_session->Shutdown();
	return S_OK;
}

HRESULT mf_player_framework::create_session(void)
{
	// Close the old session, if any.
	HRESULT hr = S_OK;
	CComQIPtr<IMFMediaEvent> media_event;

	do
	{
		assert(_state == dk_mf_player_framework::player_state_closed);

		// Create the media session.
		hr = MFCreateMediaSession(NULL, &_session);
		BREAK_ON_FAIL(hr);

		_state = dk_mf_player_framework::player_state_ready;

		// designate this class as the one that will be handling events from the media 
		hr = _session->BeginGetEvent((IMFAsyncCallback*)this, NULL);
		BREAK_ON_FAIL(hr);
	} while (0);

	return hr;
}

HRESULT mf_player_framework::close_session(void)
{
	HRESULT hr = S_OK;
	DWORD wait_result = 0;

	_state = dk_mf_player_framework::player_state_closing;

	// release the video display object
	_video_display.Release();
	_video_display = NULL;

	// Call the asynchronous Close() method and then wait for the close
	// operation to complete on another thread
	if (_session != NULL)
	{
		_state = dk_mf_player_framework::player_state_closing;
		hr = _session->Close();
		if (SUCCEEDED(hr))
		{
			// Begin waiting for the Win32 close event, fired in CPlayer::Invoke(). The 
			// close event will indicate that the close operation is finished, and the 
			// session can be shut down.
			wait_result = WaitForSingleObject(_close_completion_event, INFINITE);
			if (wait_result == WAIT_TIMEOUT)
			{
				assert(FALSE);
			}
		}
	}

	// Shut down the media session. (Synchronous operation, no events.)  Releases all of the
	// internal session resources.
	if (_session != NULL)
	{
		_session->Shutdown();
	}

	_session = NULL;
	_state = dk_mf_player_framework::player_state_closed;
	return hr;
}

HRESULT mf_player_framework::start_playback(void)
{
	assert(_session != NULL);
	PROPVARIANT var_play;
	PropVariantInit(&var_play);

	var_play.vt = VT_EMPTY;

	// If Start fails later, we will get an MESessionStarted event with an error code, 
	// and will update our state. Passing in GUID_NULL and VT_EMPTY indicates that
	// playback should start from the current position.
	HRESULT hr = _session->Start(&GUID_NULL, &var_play);
	if (SUCCEEDED(hr))
	{
		_state = dk_mf_player_framework::player_state_started;
	}

	PropVariantClear(&var_play);
	return hr;
}

HRESULT mf_player_framework::determine_duration(void)
{
	HRESULT hr = S_OK;
	CComPtr<IMFTopology> topology;
	CComPtr<IMFTopologyNode> toponode;
	CComPtr<IMFMediaSource> source;
	CComPtr<IMFPresentationDescriptor> presentation_descriptor;
	WORD node_index = 0;
	MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;

	do
	{
		BREAK_ON_NULL(_session, E_UNEXPECTED);

		hr = _session->GetFullTopology(MFSESSION_GETFULLTOPOLOGY_CURRENT, 0, &topology);
		BREAK_ON_FAIL(hr);

		while (1)
		{
			hr = topology->GetNode(node_index++, &toponode);
			BREAK_ON_FAIL(hr);

			hr = toponode->GetNodeType(&node_type);
			BREAK_ON_FAIL(hr);

			if (node_type == MF_TOPOLOGY_SOURCESTREAM_NODE)
			{
				hr = toponode->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**)&source);
				BREAK_ON_FAIL(hr);

				hr = source->CreatePresentationDescriptor(&presentation_descriptor);
				BREAK_ON_FAIL(hr);

				hr = presentation_descriptor->GetUINT64(MF_PD_DURATION, (UINT64*)&_duration);
				BREAK_ON_FAIL(hr);

				break;
			}
		}
	} while (0);

	return hr;
}

HRESULT mf_player_framework::shutdown_source(void)
{
	HRESULT hr = S_OK;
	if (_media_source)
	{
		// shut down the source
		hr = _media_source->Shutdown();
		// release the source, since all subsequent calls to it will fail
		_media_source.Release();
	}
	else
	{
		hr = E_UNEXPECTED;
	}
	return hr;
}