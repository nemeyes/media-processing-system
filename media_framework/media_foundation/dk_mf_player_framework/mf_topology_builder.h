#pragma once
#include "mf_defines.h"

class mf_topology_builder
{
public:
	static HRESULT create_source(const wchar_t * filepath, IMFMediaSource ** media_source);

	static HRESULT add_branch_to_partial_topology(IMFTopology * topology, IMFMediaSource * media_source, DWORD stream_index, IMFPresentationDescriptor * present_descriptor, HWND hwnd, IUnknown ** device_manager);

	static HRESULT create_stream_source_node(IMFMediaSource * media_source, IMFPresentationDescriptor * present_descriptor, IMFStreamDescriptor * stream_descriptor, IMFTopologyNode ** node);
	static HRESULT create_dx11_video_renderer_activate(HWND hwnd, IMFActivate ** activate);
	static HRESULT create_video_decoder_node(IMFMediaType * media_type, ULONG_PTR device_manager_ptr, IMFTopologyNode ** node);
	static HRESULT create_stream_sink_node(IUnknown * stream_sink, DWORD stream_number, IMFTopologyNode ** node);


private:
	static HRESULT find_video_decoder(REFCLSID subtype, UINT32 width, UINT32 height, IMFTransform ** decoder);



};