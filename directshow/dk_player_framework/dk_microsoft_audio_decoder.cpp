#include "dk_microsoft_audio_decoder.h"

#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <codecapi.h>


dk_microsoft_audio_decoder::dk_microsoft_audio_decoder(void)
{

}

dk_microsoft_audio_decoder::~dk_microsoft_audio_decoder(void)
{
	//safe_release(&_decoder);
}

CComPtr<IBaseFilter> dk_microsoft_audio_decoder::get_filter(void)
{
	return _decoder;
}

CComPtr<IPin> dk_microsoft_audio_decoder::get_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_decoder, L"XFrom Out", &outpin);
	return outpin;
}

CComPtr<IPin> dk_microsoft_audio_decoder::get_input_pin(void)
{
	CComPtr<IPin> inpin;
	dk_dshow_helper::get_pin(_decoder, L"XForm In", &inpin);
	return inpin;
}

HRESULT dk_microsoft_audio_decoder::add_to_graph(CComPtr<IGraphBuilder> graph)
{
	CComPtr<IBaseFilter> decoder;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Audio Decoder", CLSID_Microsoft_DTV_DVD_AudioDecoder, &decoder);
	if (FAILED(hr))
		return hr;

	_decoder = decoder;
	return hr;
}