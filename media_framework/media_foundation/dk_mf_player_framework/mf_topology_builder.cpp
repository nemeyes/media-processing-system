#include "mf_topology_builder.h"

HRESULT mf_topology_builder::create_source(const wchar_t * filepath, IMFMediaSource ** media_source)
{
	HRESULT hr = S_OK;
	MF_OBJECT_TYPE obj_type = MF_OBJECT_INVALID;
	ATL::CComPtr<IMFSourceResolver> src_resolver = NULL;
	ATL::CComPtr<IUnknown> source;
	do
	{
		hr = MFCreateSourceResolver(&src_resolver);
		BREAK_ON_FAIL(hr);

		hr = src_resolver->CreateObjectFromURL(filepath, MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE, NULL, &obj_type, &source);
		BREAK_ON_FAIL(hr);

		hr = source->QueryInterface(IID_PPV_ARGS(media_source));
		BREAK_ON_NULL(media_source, E_NOINTERFACE);
	} while (0);
	return hr;
}

HRESULT mf_topology_builder::add_branch_to_partial_topology(IMFTopology * topology, IMFMediaSource * media_source, DWORD stream_index, IMFPresentationDescriptor * present_descriptor, HWND hwnd, UINT gpu_index, IUnknown ** device_manager, IKeyEvent ** keyevent)
{
	HRESULT hr = S_OK;
	ATL::CComPtr<IMFStreamDescriptor> stream_descriptor = NULL;
	ATL::CComPtr<IMFTopologyNode> source_node = NULL;
	ATL::CComPtr<IMFTopologyNode> transform_node = NULL;
	ATL::CComPtr<IMFTopologyNode> sink_node = NULL;
	BOOL stream_selected = FALSE;

	do
	{
		BREAK_ON_NULL(topology, E_UNEXPECTED);

		// get the stream descriptor for this stream(information about stream).
		hr = present_descriptor->GetStreamDescriptorByIndex(stream_index, &stream_selected, &stream_descriptor);
		BREAK_ON_FAIL(hr);

		if (stream_selected)
		{

			/////////////////create a source node for this stream///////////////////
			hr = mf_topology_builder::create_stream_source_node(media_source, present_descriptor, stream_descriptor, &source_node);
			BREAK_ON_FAIL(hr);

			/////////////////create a output node for renderer///////////////////
			ATL::CComPtr<IMFMediaTypeHandler> media_type_handler = NULL;
			ATL::CComPtr<IMFMediaType> media_type;
			ATL::CComPtr<IMFActivate> renderer_activate = NULL;
			GUID major_type = GUID_NULL;

			do
			{
				if (hwnd != NULL)
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
						hr = mf_topology_builder::create_dx11_video_renderer_activate(hwnd, &renderer_activate);
						BREAK_ON_FAIL(hr);

						ATL::CComQIPtr<IGPUSelector> gpu_selector(renderer_activate);
						if (!gpu_selector)
							break;
						hr = gpu_selector->SetGPUIndex(gpu_index);
						BREAK_ON_FAIL(hr);

						ATL::CComPtr<IMFMediaSink> media_sink;
						hr = renderer_activate->ActivateObject(IID_PPV_ARGS(&media_sink));
						BREAK_ON_FAIL(hr);

						ATL::CComPtr<IMFGetService> get_service;
						hr = media_sink->QueryInterface(IID_PPV_ARGS(&get_service));
						BREAK_ON_FAIL(hr);

						ATL::CComQIPtr<IKeyEvent> key_event(media_sink);
						if (!key_event)
							break;

						ATL::CComPtr<IMFStreamSink> stream_sink;
						DWORD stream_sink_count = 0;
						hr = media_sink->GetStreamSinkCount(&stream_sink_count);
						BREAK_ON_FAIL(hr);
						hr = media_sink->GetStreamSinkByIndex((stream_sink_count - 1), &stream_sink);
						BREAK_ON_FAIL(hr);

						CLSID devuce_manager_iid = IID_IMFDXGIDeviceManager;
						hr = get_service->GetService(MR_VIDEO_ACCELERATION_SERVICE, devuce_manager_iid, (void**)device_manager);
						BREAK_ON_FAIL(hr);

						LONG_PTR device_manager_ptr = reinterpret_cast<ULONG_PTR>(*device_manager);

						hr = mf_topology_builder::create_video_decoder_node(media_type, device_manager_ptr, &transform_node);
						BREAK_ON_FAIL(hr);

						hr = mf_topology_builder::create_stream_sink_node(stream_sink, stream_index + 1, &sink_node);
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
				hr = topology->AddNode(source_node);
				BREAK_ON_FAIL(hr);
			}

			if (transform_node)
			{
				hr = topology->AddNode(transform_node);
				BREAK_ON_FAIL(hr);
			}

			if (sink_node)
			{
				hr = topology->AddNode(sink_node);
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
		hr = present_descriptor->DeselectStream(stream_index);
	}

	return hr;
}

HRESULT mf_topology_builder::create_stream_source_node(IMFMediaSource * media_source, IMFPresentationDescriptor * present_descriptor, IMFStreamDescriptor * stream_descriptor, IMFTopologyNode ** node)
{
	HRESULT hr = S_OK;
	do
	{
		// create a source node for this stream
		hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, node);
		BREAK_ON_FAIL(hr);

		// associate the node with the souce by passing in a pointer to the media source and indicating that it is the source
		hr = (*node)->SetUnknown(MF_TOPONODE_SOURCE, media_source);
		BREAK_ON_FAIL(hr);

		// set the node presentation descriptor attribute of the node by passing in a pointer to the presentation descriptor
		hr = (*node)->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, present_descriptor);
		BREAK_ON_FAIL(hr);

		// set the node stream descriptor attribute by passing in a pointer to the stream descriptor
		hr = (*node)->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, stream_descriptor);
		BREAK_ON_FAIL(hr);

		hr = (*node)->SetUINT32(MF_TOPONODE_CONNECT_METHOD, MF_CONNECT_ALLOW_DECODER);
		BREAK_ON_FAIL(hr);

	} while (0);

	return hr;
}

HRESULT mf_topology_builder::create_dx11_video_renderer_activate(HWND hwnd, IMFActivate ** activate)
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

HRESULT mf_topology_builder::create_video_decoder_node(IMFMediaType * media_type, ULONG_PTR device_manager_ptr, IMFTopologyNode ** node)
{
	HRESULT hr;

	GUID subtype;
	hr = media_type->GetGUID(MF_MT_SUBTYPE, &subtype);
	RETURN_ON_FAIL(hr);

	ATL::CComPtr<IMFTransform> decoder_transform;

	UINT32 input_image_width = 0;
	UINT32 input_image_height = 0;
	hr = MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE, &input_image_width, &input_image_height);
	RETURN_ON_FAIL(hr);

	hr = find_video_decoder(subtype, input_image_width, input_image_height, &decoder_transform);
	RETURN_ON_FAIL(hr);

	ATL::CComPtr<IMFAttributes> decoder_attribute;
	hr = decoder_transform->GetAttributes(&decoder_attribute);
	RETURN_ON_FAIL(hr);

	UINT32 transform_async = 0;
	hr = decoder_attribute->GetUINT32(MF_TRANSFORM_ASYNC, &transform_async);
	if (SUCCEEDED(hr) && transform_async == TRUE)
	{
		hr = decoder_attribute->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
		RETURN_ON_FAIL(hr);
	}

	if (device_manager_ptr != NULL)
	{
		ATL::CComPtr<IUnknown> device_manager_unknown = reinterpret_cast<IUnknown*>(device_manager_ptr);
		ATL::CComPtr<IUnknown> dxgi_device_manager;
		CLSID d3d_aware_attribute;
		hr = device_manager_unknown->QueryInterface(BORROWED_IID_IMFDXGIDeviceManager, (void**)(&dxgi_device_manager));
		if (SUCCEEDED(hr))
			d3d_aware_attribute = BORROWED_MF_SA_D3D11_AWARE;
		else
			d3d_aware_attribute = MF_SA_D3D_AWARE;

		UINT32 d3d_aware;
		hr = decoder_attribute->GetUINT32(d3d_aware_attribute, &d3d_aware);
		if (SUCCEEDED(hr) && d3d_aware != 0)
		{
			hr = decoder_transform->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, device_manager_ptr);
			RETURN_ON_FAIL(hr);
		}
	}

	hr = decoder_transform->SetInputType(0, media_type, 0);
	RETURN_ON_FAIL(hr);

	hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, node);
	RETURN_ON_FAIL(hr);

	hr = (*node)->SetObject(decoder_transform);
	RETURN_ON_FAIL(hr);

	hr = (*node)->SetUINT32(MF_TOPONODE_CONNECT_METHOD, MF_CONNECT_ALLOW_CONVERTER);
	RETURN_ON_FAIL(hr);

	return hr;
}
#
HRESULT mf_topology_builder::create_stream_sink_node(IUnknown * stream_sink, DWORD stream_number, IMFTopologyNode ** node)
{
	HRESULT hr;
	hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, node);
	RETURN_ON_FAIL(hr);

	hr = (*node)->SetObject(stream_sink);
	RETURN_ON_FAIL(hr);

	hr = (*node)->SetUINT32(MF_TOPONODE_STREAMID, stream_number);
	RETURN_ON_FAIL(hr);

	hr = (*node)->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
	RETURN_ON_FAIL(hr);

	return hr;
}

HRESULT mf_topology_builder::find_video_decoder(REFCLSID subtype, UINT32 width, UINT32 height, IMFTransform ** decoder)
{
	HRESULT hr;
	UINT32 flags = MFT_ENUM_FLAG_SORTANDFILTER;

	MFT_REGISTER_TYPE_INFO input_register_type_info = { MFMediaType_Video, subtype };
	flags |= MFT_ENUM_FLAG_SYNCMFT;
	flags |= MFT_ENUM_FLAG_HARDWARE;


	IMFActivate ** activate = NULL;
	UINT32 registered_decoders_number = 0;

	const CLSID supported_output_subtypes[] = { MFVideoFormat_NV12, MFVideoFormat_YUY2 };
	bool found_decoder = false;
	for (UINT32 x = 0; !found_decoder && x < ARRAYSIZE(supported_output_subtypes); x++)
	{
		MFT_REGISTER_TYPE_INFO output_register_type_info = { MFMediaType_Video, supported_output_subtypes[x] };
		hr = MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER, flags, &input_register_type_info, &output_register_type_info, &activate, &registered_decoders_number);
		RETURN_ON_FAIL(hr);

		if (SUCCEEDED(hr) && (registered_decoders_number == 0))
			hr = MF_E_TOPO_CODEC_NOT_FOUND;

		if (SUCCEEDED(hr))
		{
			hr = activate[0]->ActivateObject(IID_PPV_ARGS(decoder));
			found_decoder = true;
		}
		for (UINT32 y = 0; y < registered_decoders_number; y++)
		{
			activate[y]->Release();
		}
		CoTaskMemFree(activate);
	}

	if (!found_decoder)
		return E_FAIL;

	return S_OK;
}