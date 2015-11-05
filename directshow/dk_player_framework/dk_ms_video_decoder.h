#pragma once
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>

class dk_ms_video_decoder
{
public:
	dk_ms_video_decoder(void);
	~dk_ms_video_decoder(void);


	HRESULT add_to_graph(CComPtr<IGraphBuilder> graph, bool dxva2);
	//HRESULT connect_to_upstream_filter()

private:
	CComPtr<IBaseFilter> _decoder;
};