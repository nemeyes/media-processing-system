#pragma once
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>

#include "dk_base_filter.h"

class dk_microsoft_video_decoder : public dk_base_video_decode_filter
{
public:
	dk_microsoft_video_decoder(void);
	virtual ~dk_microsoft_video_decoder(void);

	CComPtr<IBaseFilter> get_filter(void);
	CComPtr<IPin> get_output_pin(void);
	CComPtr<IPin> get_input_pin(void);

	HRESULT add_to_graph(CComPtr<IGraphBuilder> graph);
	//HRESULT connect_to_upstream_filter()

private:
	CComPtr<IBaseFilter> _decoder;
};