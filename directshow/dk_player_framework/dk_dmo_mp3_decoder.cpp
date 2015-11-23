#include "dk_dmo_mp3_decoder.h"

#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <codecapi.h>


dk_dmo_mp3_decoder::dk_dmo_mp3_decoder(void)
{

}

dk_dmo_mp3_decoder::~dk_dmo_mp3_decoder(void)
{
	safe_release(&_decoder);
}

CComPtr<IBaseFilter> dk_dmo_mp3_decoder::get_filter(void)
{
	return _decoder;
}

CComPtr<IPin> dk_dmo_mp3_decoder::get_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_decoder, L"out0", &outpin);
	return outpin;
}

CComPtr<IPin> dk_dmo_mp3_decoder::get_input_pin(void)
{
	CComPtr<IPin> inpin;
	dk_dshow_helper::get_pin(_decoder, L"in0", &inpin);
	return inpin;
}

HRESULT dk_dmo_mp3_decoder::add_to_graph(CComPtr<IGraphBuilder> graph)
{
	CComPtr<IBaseFilter> decoder;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Audio Decoder", CLSID_Mp3DecoderDMO, &decoder);
	if (FAILED(hr))
		return hr;

	_decoder = decoder;
	return hr;
}