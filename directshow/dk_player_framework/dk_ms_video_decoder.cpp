#include "dk_ms_video_decoder.h"

#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <codecapi.h>


dk_ms_video_decoder::dk_ms_video_decoder(void)
{

}

dk_ms_video_decoder::~dk_ms_video_decoder(void)
{

}

HRESULT dk_ms_video_decoder::add_to_graph(CComPtr<IGraphBuilder> graph, bool dxva2)
{
	CComPtr<IBaseFilter> decoder;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Decoder", CLSID_Microsoft_DTV_DVD_VideoDecoder, &decoder);
	if (FAILED(hr))
		return hr;

	if (dxva2)
	{
		VARIANT variant;
		variant.vt = VT_UI4;
		variant.uiVal = (UINT32)true;
		CComQIPtr<ICodecAPI> codec_api = decoder;
		hr = codec_api->SetValue(&CODECAPI_AVDecVideoAcceleration_H264, &variant);
		if (FAILED(hr))
			return hr;
	}
	_decoder = decoder;
	return hr;
}