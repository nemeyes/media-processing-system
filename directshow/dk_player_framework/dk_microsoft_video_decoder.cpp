#include "dk_microsoft_video_decoder.h"

#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <codecapi.h>


dk_microsoft_video_decoder::dk_microsoft_video_decoder(void)
{

}

dk_microsoft_video_decoder::~dk_microsoft_video_decoder(void)
{
	safe_release(&_decoder);
}

CComPtr<IBaseFilter> dk_microsoft_video_decoder::get_filter(void)
{
	return _decoder;
}

CComPtr<IPin> dk_microsoft_video_decoder::get_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_decoder, L"Video Output 1", &outpin);
	return outpin;
}

CComPtr<IPin> dk_microsoft_video_decoder::get_input_pin(void)
{
	CComPtr<IPin> inpin;
	dk_dshow_helper::get_pin(_decoder, L"Video Input", &inpin);
	return inpin;
}

HRESULT dk_microsoft_video_decoder::add_to_graph(CComPtr<IGraphBuilder> graph)
{
	CComPtr<IBaseFilter> decoder;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Video Decoder", CLSID_Microsoft_DTV_DVD_VideoDecoder, &decoder);
	if (FAILED(hr))
		return hr;

	{
		VARIANT variant;
		variant.vt = VT_UI4;
		variant.uiVal = (UINT32)true;
		CComQIPtr<ICodecAPI> codec_api = decoder;
		if (codec_api)
		{
			hr = codec_api->SetValue(&CODECAPI_AVDecVideoAcceleration_H264, &variant);
			if (FAILED(hr))
				return hr;
		}
		else
		{
			return E_FAIL;
		}
	}
	_decoder = decoder;
	return hr;
}