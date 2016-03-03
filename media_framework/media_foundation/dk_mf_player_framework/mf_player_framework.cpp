#include <windows.h>
#include "mf_player_framework.h"
#include <mferror.h>

mf_player_framework::mf_player_framework(void)
	: _ref_count(1)
	, _state(dk_mf_player_framework::STATE_CLOSED)
	, _session(NULL)
	, _rate_control(NULL)
	, _presentation_clock(NULL)
	, _duration(0)
	, _video_display(NULL)
	, _hwnd(NULL)
{
	MFStartup(MF_VERSION);
	_close_completion_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

mf_player_framework::~mf_player_framework(void)
{
	shutdown_source();
	close_session();
	MFShutdown();
	::CloseHandle(_close_completion_event);
}

dk_mf_player_framework::ERR_CODE mf_player_framework::initialize(HWND hwnd, bool aspect_ratio, bool use_clock, bool enable_audio)
{
	_hwnd = hwnd;
	return dk_mf_player_framework::ERR_CODE_SUCCESS;
}

dk_mf_player_framework::ERR_CODE mf_player_framework::release(void)
{
	return dk_mf_player_framework::ERR_CODE_SUCCESS;
}

// Playback control
dk_mf_player_framework::ERR_CODE mf_player_framework::seek(int position)
{
	return dk_mf_player_framework::ERR_CODE_SUCCESS;
}

dk_mf_player_framework::ERR_CODE mf_player_framework::slowfoward_rate(float rate)
{
	HRESULT hr = S_OK;
	BOOL thin = FALSE;
	do
	{
		if (_state != dk_mf_player_framework::STATE_STARTED)
		{
			hr = MF_E_INVALIDREQUEST;
			break;
		}
		BREAK_ON_NULL(_rate_control, E_UNEXPECTED);

		// set the rate
		hr = _rate_control->SetRate(thin, -rate);
	} while (0);

	if (FAILED(hr))
		return dk_mf_player_framework::ERR_CODE_FAILED;
	else
		return dk_mf_player_framework::ERR_CODE_SUCCESS;
}

dk_mf_player_framework::ERR_CODE mf_player_framework::fastforward_rate(float rate)
{
	HRESULT hr = S_OK;
	BOOL thin = FALSE;
	do
	{
		if (_state != dk_mf_player_framework::STATE_STARTED)
		{
			hr = MF_E_INVALIDREQUEST;
			break;
		}
		BREAK_ON_NULL(_rate_control, E_UNEXPECTED);

		// set the rate
		hr = _rate_control->SetRate(thin, rate);
	} while (0);

	if (FAILED(hr))
		return dk_mf_player_framework::ERR_CODE_FAILED;
	else
		return dk_mf_player_framework::ERR_CODE_SUCCESS;
}

dk_mf_player_framework::ERR_CODE mf_player_framework::open_file(const wchar_t * file)
{
	HRESULT hr = S_OK;
	if (_hwnd == NULL)
		return dk_mf_player_framework::ERR_CODE_FAILED;

	do
	{
		// Step 1: create a media session if one doesn't exist already
		if (_session == NULL)
		{
			hr = create_session();
			BREAK_ON_FAIL(hr);
		}


		// Step 2 : create a media source for specified URL string, The URL can be a path to a stream or it can be a path to a local file
		MF_OBJECT_TYPE obj_type = MF_OBJECT_INVALID;
		ATL::CComPtr<IMFSourceResolver> src_resolver = NULL;
		ATL::CComPtr<IUnknown> source;
		//ATL::CComPtr<IMFMediaSource> media_source;
		do
		{
			hr = MFCreateSourceResolver(&src_resolver);
			BREAK_ON_FAIL(hr);

			hr = src_resolver->CreateObjectFromURL(file, MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE, NULL, &obj_type, &source);
			BREAK_ON_FAIL(hr);

			hr = source->QueryInterface(IID_PPV_ARGS(&_media_source));
			BREAK_ON_NULL(_media_source, E_NOINTERFACE);
		} while (0);

		if (FAILED(hr))
			return dk_mf_player_framework::ERR_CODE_FAILED;

		// Step 3: build the topology
		CComQIPtr<IMFPresentationDescriptor> present_descriptor;
		DWORD number_of_streams = 0;
		do
		{
			_topology.Release();
			hr = MFCreateTopology(&_topology);
			BREAK_ON_FAIL(hr);

			hr = _media_source->CreatePresentationDescriptor(&present_descriptor);
			BREAK_ON_FAIL(hr);

			hr = present_descriptor->GetStreamDescriptorCount(&number_of_streams);
			BREAK_ON_FAIL(hr);

			for (DWORD index = 0; index < number_of_streams; index++)
			{
				ATL::CComPtr<IMFStreamDescriptor> stream_descriptor;
				ATL::CComPtr<IMFTopologyNode> source_node = NULL;
				ATL::CComPtr<IMFTopologyNode> transform_node = NULL;
				ATL::CComPtr<IMFTopologyNode> sink_node = NULL;
				BOOL stream_selected = FALSE;

				do
				{
					BREAK_ON_NULL(_topology, E_UNEXPECTED);

					// get the stream descriptor for this stream(information about stream).
					hr = present_descriptor->GetStreamDescriptorByIndex(index, &stream_selected, &stream_descriptor);
					BREAK_ON_FAIL(hr);

					if (stream_selected)
					{

						/////////////////create a source node for this stream///////////////////
						do
						{
							// create a source node for this stream
							hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &source_node);
							BREAK_ON_FAIL(hr);

							// associate the node with the souce by passing in a pointer to the media source and indicating that it is the source
							hr = source_node->SetUnknown(MF_TOPONODE_SOURCE, _media_source);
							BREAK_ON_FAIL(hr);

							// set the node presentation descriptor attribute of the node by passing in a pointer to the presentation descriptor
							hr = source_node->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, present_descriptor);
							BREAK_ON_FAIL(hr);

							// set the node stream descriptor attribute by passing in a pointer to the stream descriptor
							hr = source_node->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, stream_descriptor);
							BREAK_ON_FAIL(hr);

							hr = source_node->SetUINT32(MF_TOPONODE_CONNECT_METHOD, MF_CONNECT_ALLOW_DECODER);
							BREAK_ON_FAIL(hr);

						} while (0);
						BREAK_ON_FAIL(hr);

						/////////////////create a output node for renderer///////////////////
						ATL::CComPtr<IMFMediaTypeHandler> media_type_handler = NULL;
						ATL::CComPtr<IMFMediaType> media_type;
						ATL::CComPtr<IMFActivate> renderer_activate = NULL;
						GUID major_type = GUID_NULL;

						do
						{
							if (_hwnd!=NULL)
							{
								hr = stream_descriptor->GetMediaTypeHandler(&media_type_handler);
								BREAK_ON_FAIL(hr);

								hr = media_type_handler->GetCurrentMediaType(&media_type);
								BREAK_ON_FAIL(hr);

								hr = media_type->GetMajorType(&major_type);
								BREAK_ON_FAIL(hr);

								// Create an IMFActivate controller object for the renderer, based on the media type.
								if (major_type == MFMediaType_Audio)
								{
									hr = MFCreateAudioRendererActivate(&renderer_activate);
									// create the node which will represent the renderer
									hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &sink_node);
									BREAK_ON_FAIL(hr);

									// store the IActivate object in the sink node - it will be extracted later by the media session during the topology render phase.
									hr = sink_node->SetObject(renderer_activate);
									BREAK_ON_FAIL(hr);
								}
								else if (major_type == MFMediaType_Video)
								{
#if 0
									hr = MFCreateVideoRendererActivate(_hwnd, &renderer_activate);
									BREAK_ON_FAIL(hr);

									// create the node which will represent the renderer
									hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &sink_node);
									BREAK_ON_FAIL(hr);

									// store the IActivate object in the sink node - it will be extracted later by the media session during the topology render phase.
									hr = sink_node->SetObject(renderer_activate);
									BREAK_ON_FAIL(hr);
#else
									hr = create_dx11_video_renderer_activate(_hwnd, &renderer_activate);
									BREAK_ON_FAIL(hr);

									ATL::CComPtr<IMFMediaSink> media_sink;
									hr = renderer_activate->ActivateObject(IID_PPV_ARGS(&media_sink));
									BREAK_ON_FAIL(hr);


									ATL::CComPtr<IMFGetService> get_service;
									hr = media_sink->QueryInterface(IID_PPV_ARGS(&get_service));
									BREAK_ON_FAIL(hr);


									ATL::CComPtr<IMFStreamSink> stream_sink;
									DWORD stream_sink_count = 0;
									hr = media_sink->GetStreamSinkCount(&stream_sink_count);
									BREAK_ON_FAIL(hr);
									hr = media_sink->GetStreamSinkByIndex((stream_sink_count-1), &stream_sink);
									BREAK_ON_FAIL(hr);

									CLSID devuce_manager_iid = IID_IMFDXGIDeviceManager;
									hr = get_service->GetService(MR_VIDEO_ACCELERATION_SERVICE, devuce_manager_iid, (void**)&_device_manager);
									BREAK_ON_FAIL(hr);

									LONG_PTR device_manage_ptr = reinterpret_cast<ULONG_PTR>(_device_manager.p);

									hr = create_video_decoder_node(media_type, &transform_node, device_manage_ptr, NULL);
									BREAK_ON_FAIL(hr);


									hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &sink_node);
									BREAK_ON_FAIL(hr);

									hr = sink_node->SetObject(stream_sink);
									BREAK_ON_FAIL(hr);

									hr = sink_node->SetUINT32(MF_TOPONODE_STREAMID, index+1);
									BREAK_ON_FAIL(hr);

									hr = sink_node->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
									BREAK_ON_FAIL(hr);
#endif

								}
								else
								{
									hr = E_FAIL;
								}
								BREAK_ON_FAIL(hr);
							}
						} while (0);
						BREAK_ON_FAIL(hr);

						if (source_node)
						{
							hr = _topology->AddNode(source_node);
							BREAK_ON_FAIL(hr);
						}

						if (transform_node)
						{
							hr = _topology->AddNode(transform_node);
							BREAK_ON_FAIL(hr);
						}

						if (sink_node)
						{
							hr = _topology->AddNode(sink_node);
							BREAK_ON_FAIL(hr);
						}

						// Connect the source node to the output node.  The topology will find the
						// intermediate nodes needed to convert media types.
						if (source_node && transform_node && sink_node)
						{
							hr = source_node->ConnectOutput(0, transform_node, 0);
							hr = transform_node->ConnectOutput(0, sink_node, 0);
						}
						else if (source_node && sink_node)
						{
							hr = source_node->ConnectOutput(0, sink_node, 0);
						}
					}
				} while (0);

				if (FAILED(hr))
				{
					hr = present_descriptor->DeselectStream(index);
					BREAK_ON_FAIL(hr);
				}
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
		if (_state == dk_mf_player_framework::STATE_READY)
		{
			_state = dk_mf_player_framework::STATE_OPEN_PENDING;
		}
	} while (0);

	if (FAILED(hr))
	{
		_state = dk_mf_player_framework::STATE_CLOSED;
		return dk_mf_player_framework::ERR_CODE_FAILED;
	}
	return dk_mf_player_framework::ERR_CODE_SUCCESS;
}

HRESULT mf_player_framework::create_dx11_video_renderer_activate(HWND hwnd, IMFActivate ** activate)
{
	if (activate == nullptr)
		return E_POINTER;

	HMODULE renderer_dll = ::LoadLibrary(L"dk_mf_dx11_video_renderer.dll");
	if (renderer_dll == NULL)
		return E_FAIL;

	LPCSTR fn_name = "CreateDX11VideoRendererActivate";
	FARPROC create_dx11_video_renderer_activate_far_proc = ::GetProcAddress(renderer_dll, fn_name);
	if (create_dx11_video_renderer_activate_far_proc == nullptr)
		return E_FAIL;

	typedef HRESULT(STDAPICALLTYPE* LPCreateDX11VideoRendererActivate)(HWND, IMFActivate**);
	LPCreateDX11VideoRendererActivate create_dx11_video_renderer_activate = reinterpret_cast<LPCreateDX11VideoRendererActivate> (create_dx11_video_renderer_activate_far_proc);

	HRESULT hr = create_dx11_video_renderer_activate(hwnd, activate);

	//::FreeLibrary(renderer_dll);

	return hr;
}

HRESULT mf_player_framework::create_video_decoder_node(IMFMediaType * media_type, IMFTopologyNode ** decoder, ULONG_PTR device_manager, IMFTransform ** transform)
{
	HRESULT hr;

	GUID subtype;
	hr = media_type->GetGUID(MF_MT_SUBTYPE, &subtype);
	RETURN_ON_FAIL(hr);

	ATL::CComPtr<IMFTransform> decoder_transform;

	uint32_t input_image_width = 0;
	uint32_t input_image_height = 0;
	hr = MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE, &input_image_width, &input_image_height);
	RETURN_ON_FAIL(hr);

	hr = find_video_decoder(subtype, input_image_width, input_image_height, &decoder_transform);
	RETURN_ON_FAIL(hr);

	ATL::CComPtr<IMFAttributes> decoder_attribute;
	hr = decoder_transform->GetAttributes(&decoder_attribute);
	RETURN_ON_FAIL(hr);

	uint32_t transform_async = 0;
	hr = decoder_attribute->GetUINT32(MF_TRANSFORM_ASYNC, &transform_async);
	if (SUCCEEDED(hr) && transform_async == TRUE)
	{
		hr = decoder_attribute->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
		RETURN_ON_FAIL(hr);
	}

	if (device_manager != NULL)
	{
		ATL::CComPtr<IUnknown> device_manager_unknown = reinterpret_cast<IUnknown*>(device_manager);
		ATL::CComPtr<IUnknown> dxgi_device_manager;
		CLSID d3d_aware_attribute;
		hr = device_manager_unknown->QueryInterface(BORROWED_IID_IMFDXGIDeviceManager, (void**)(&dxgi_device_manager));
		if (SUCCEEDED(hr))
			d3d_aware_attribute = BORROWED_MF_SA_D3D11_AWARE;
		else
			d3d_aware_attribute = MF_SA_D3D_AWARE;

		uint32_t d3d_aware;
		hr = decoder_attribute->GetUINT32(d3d_aware_attribute, &d3d_aware);
		if (SUCCEEDED(hr) && d3d_aware != 0)
		{
			hr = decoder_transform->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, device_manager);
			RETURN_ON_FAIL(hr);
		}
	}

	hr = decoder_transform->SetInputType(0, media_type, 0);
	RETURN_ON_FAIL(hr);

	CComPtr<IMFTopologyNode> node;
	hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &node);
	RETURN_ON_FAIL(hr);

	hr = node->SetObject(decoder_transform);
	RETURN_ON_FAIL(hr);

	hr = node->SetUINT32(MF_TOPONODE_CONNECT_METHOD, MF_CONNECT_ALLOW_CONVERTER);
	RETURN_ON_FAIL(hr);

	*decoder = node.Detach();
	if (transform != NULL)
	{
		*transform = decoder_transform.Detach();
	}
	return S_OK;
}

HRESULT mf_player_framework::find_video_decoder(REFCLSID subtype, uint32_t width, uint32_t height, IMFTransform ** decoder)
{
	HRESULT hr;
	UINT32 flags = MFT_ENUM_FLAG_SORTANDFILTER;

	MFT_REGISTER_TYPE_INFO input_register_type_info = { MFMediaType_Video, subtype };
	flags |= MFT_ENUM_FLAG_SYNCMFT;
	flags |= MFT_ENUM_FLAG_HARDWARE;


	IMFActivate ** activate = NULL;
	uint32_t registered_decoders_number = 0;

	const CLSID supported_output_subtypes[] = { MFVideoFormat_NV12, MFVideoFormat_YUY2 };
	bool found_decoder = false;
	for (int32_t index = 0; !found_decoder && index < ARRAYSIZE(supported_output_subtypes); index++)
	{
		MFT_REGISTER_TYPE_INFO output_register_type_info = { MFMediaType_Video, supported_output_subtypes[index] };
		hr = MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER, flags, &input_register_type_info, &output_register_type_info, &activate, &registered_decoders_number);
		RETURN_ON_FAIL(hr);

		if (SUCCEEDED(hr) && (registered_decoders_number == 0))
			hr = MF_E_TOPO_CODEC_NOT_FOUND;

		if (SUCCEEDED(hr))
		{
			hr = activate[0]->ActivateObject(IID_PPV_ARGS(decoder));
			found_decoder = true;
		}
		for (int32_t x = 0; x < registered_decoders_number; x++)
		{
			activate[x]->Release();
		}
		CoTaskMemFree(activate);
	}

	if (!found_decoder)
		return E_FAIL;

	return S_OK;
}

dk_mf_player_framework::ERR_CODE mf_player_framework::play(void)
{
	if (_state != dk_mf_player_framework::STATE_PAUSED && _state != dk_mf_player_framework::STATE_STOPPED)
		return dk_mf_player_framework::ERR_CODE_FAILED;

	if (_session == NULL)
		return dk_mf_player_framework::ERR_CODE_FAILED;

	HRESULT hr = start_playback();
	if (SUCCEEDED(hr))
		return dk_mf_player_framework::ERR_CODE_SUCCESS;
	else
		return dk_mf_player_framework::ERR_CODE_FAILED;
}

dk_mf_player_framework::ERR_CODE mf_player_framework::pause(void)
{
	if (_state != dk_mf_player_framework::STATE_STARTED)
		return dk_mf_player_framework::ERR_CODE_FAILED;

	if (_session == NULL)
		return dk_mf_player_framework::ERR_CODE_FAILED;

	HRESULT hr = _session->Pause();
	if (SUCCEEDED(hr))
	{
		_state = dk_mf_player_framework::STATE_PAUSED;
		return dk_mf_player_framework::ERR_CODE_SUCCESS;
	}
	else
		return dk_mf_player_framework::ERR_CODE_FAILED;
}

dk_mf_player_framework::ERR_CODE mf_player_framework::stop(void)
{
	HRESULT hr = S_OK;
	do
	{
		if (_state != dk_mf_player_framework::STATE_STARTED)
		{
			hr = MF_E_INVALIDREQUEST;
			break;
		}

		BREAK_ON_NULL(_session, E_UNEXPECTED);

		hr = _session->Stop();
		BREAK_ON_FAIL(hr);

		_state = dk_mf_player_framework::STATE_STOPPED;
	} while (0);

	if (SUCCEEDED(hr))
		return dk_mf_player_framework::ERR_CODE_SUCCESS;
	else
		return dk_mf_player_framework::ERR_CODE_FAILED;
}

dk_mf_player_framework::STATE mf_player_framework::state(void) const 
{ 
	return _state; 
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
		if (_state != dk_mf_player_framework::STATE_CLOSING)
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
	_state = dk_mf_player_framework::STATE_STOPPED;
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
		// close the session if one is already created
		hr = close_session();
		BREAK_ON_FAIL(hr);

		assert(_state == dk_mf_player_framework::STATE_CLOSED);

		// Create the media session.
		hr = MFCreateMediaSession(NULL, &_session);
		BREAK_ON_FAIL(hr);

		_state = dk_mf_player_framework::STATE_READY;

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

	_state = dk_mf_player_framework::STATE_CLOSING;

	// release the video display object
	_video_display = NULL;

	// Call the asynchronous Close() method and then wait for the close
	// operation to complete on another thread
	if (_session != NULL)
	{
		_state = dk_mf_player_framework::STATE_CLOSING;
		hr = _session->Close();
		if (SUCCEEDED(hr))
		{
			// Begin waiting for the Win32 close event, fired in CPlayer::Invoke(). The 
			// close event will indicate that the close operation is finished, and the 
			// session can be shut down.
			wait_result = WaitForSingleObject(_close_completion_event, 5000);
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
	_state = dk_mf_player_framework::STATE_CLOSED;
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
		_state = dk_mf_player_framework::STATE_STARTED;
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