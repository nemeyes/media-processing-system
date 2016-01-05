#include "dk_dmo_mpeg4_decoder.h"

#include <uuids.h>
#include <initguid.h>
#include <dk_dshow_helper.h>
#include <dk_submedia_type.h>
#include <dmo.h>
#include <dmodshow.h>


dk_dmo_mpeg4_decoder::dk_dmo_mpeg4_decoder(void)
{

}

dk_dmo_mpeg4_decoder::~dk_dmo_mpeg4_decoder(void)
{
	//safe_release(&_decoder);
}

CComPtr<IBaseFilter> dk_dmo_mpeg4_decoder::get_filter(void)
{
	return _decoder;
}

CComPtr<IPin> dk_dmo_mpeg4_decoder::get_output_pin(void)
{
	CComPtr<IPin> outpin;
	dk_dshow_helper::get_pin(_decoder, L"out0", &outpin);
	return outpin;
}

CComPtr<IPin> dk_dmo_mpeg4_decoder::get_input_pin(void)
{
	CComPtr<IPin> inpin;
	dk_dshow_helper::get_pin(_decoder, L"in0", &inpin);
	return inpin;
}

HRESULT dk_dmo_mpeg4_decoder::add_to_graph(CComPtr<IGraphBuilder> graph)
{
	CComPtr<IBaseFilter> decoder;
	enum_decoder_dmo();
	if (_found)
	{
		HRESULT hr = CoCreateInstance(CLSID_DMOWrapperFilter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&decoder));
		if (SUCCEEDED(hr))
		{
			CComPtr<IDMOWrapperFilter> dmo_wrapper;
			hr = decoder->QueryInterface(IID_IDMOWrapperFilter, reinterpret_cast<void**>(&dmo_wrapper));
			if (SUCCEEDED(hr))
			{
				hr = dmo_wrapper->Init(_clsidDMO, DMOCATEGORY_VIDEO_DECODER);

				if (SUCCEEDED(hr))
				{
					hr = graph->AddFilter(decoder, L"Mpeg4 Decoder DMO");
					//hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Video Decoder", CLSID_DMOWrapperFilter, &decoder);
				}
			}
		}

		if (SUCCEEDED(hr))
			_decoder = decoder;
		return hr;
	}
	else
	{
		return E_FAIL;
	}
}

void dk_dmo_mpeg4_decoder::enum_decoder_dmo(void)
{
	_found = FALSE;
	IEnumDMO* pEnum = NULL;
	HRESULT hr = DMOEnum(DMOCATEGORY_VIDEO_DECODER, DMO_ENUMF_INCLUDE_KEYED,  0, NULL, 0, NULL, &pEnum);
	if (SUCCEEDED(hr))
	{
		CLSID clsidDMO;
		WCHAR* wszName;
		do
		{
			hr = pEnum->Next(1, &clsidDMO, &wszName, NULL);
			if (hr == S_OK)
			{
				// Now wszName holds the friendly name of the DMO, 
				// and clsidDMO holds the CLSID. 
				// wprintf(L"DMO Name: %s\n", wszName);
				if (wcscmp(wszName, L"Mpeg4 Decoder DMO") == 0)
				{
					_clsidDMO = clsidDMO;
					_found = TRUE;
				}

				// Remember to release wszName!
				CoTaskMemFree(wszName);
			}
		} while (hr == S_OK);
		pEnum->Release();
	}
}
