// cu_h264_encoder2_test2.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <dshow.h>
#include <atlbase.h>
#include <initguid.h>
#include <dvdmedia.h>

BOOL hrcheck(HRESULT hr, TCHAR * errtext)
{
	if (hr >= S_OK)
		return FALSE;

	TCHAR szErr[MAX_ERROR_TEXT_LEN];
	DWORD res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
	if (res)
		printf("Error %x: %s\n%s\n", hr, errtext, szErr);
	else
		printf("Error %x: %s\n", hr, errtext);
}

#define CHECK_HR(hr, msg) if(hrcheck(hr,msg)) return hr;

CComPtr<IBaseFilter> CreateFilterByName(const WCHAR* filterName, const GUID& category)
{
	HRESULT hr = S_OK;
	CComPtr<ICreateDevEnum> pSysDevEnum;
	hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (hrcheck(hr, L"Can't create System Device Enumerator"))
		return NULL;

	CComPtr<IEnumMoniker> pEnumCat;
	hr = pSysDevEnum->CreateClassEnumerator(category, &pEnumCat, 0);

	if (hr == S_OK)
	{
		CComPtr<IMoniker> pMoniker;
		ULONG cFetched;
		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			CComPtr<IPropertyBag> pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
			if (SUCCEEDED(hr))
			{
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					if (wcscmp(filterName, varName.bstrVal) == 0)
					{
						CComPtr<IBaseFilter> pFilter;
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
						if (hrcheck(hr, L"Can,t bind moniker to filter object"))
							return NULL;
						return pFilter;
					}
				}
				VariantClear(&varName);
			}
			pMoniker.Release();
		}
	}
	return NULL;
}

CComPtr<IPin> GetPin(IBaseFilter * pFilter, LPCOLESTR pinname)
{
	CComPtr<IEnumPins> pEnum;
	CComPtr<IPin> pPin;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (hrcheck(hr, L"Can't enumerate pins."))
		return NULL;

	while (pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_INFO pinfo;
		pPin->QueryPinInfo(&pinfo);
		BOOL found = !_wcsicmp(pinname, pinfo.achName);
		wprintf(L"pinname %s\n", pinfo.achName);
		if (pinfo.pFilter) pinfo.pFilter->Release();
		if (found)
			return pPin;
		pPin.Release();
	}
	printf("Pin not found!\n");
	return NULL;
}

//DEFINE_GUID(CLSID_VideoCaptureSources, 
//	0x860BB310, 0x5D01, 0x11D0, 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86);

DEFINE_GUID(CLSID_DKScreenCaptureFilter,
	0xdf7c90a9, 0xc202, 0x4506, 0xa4, 0xee, 0x6, 0x56, 0xc0, 0xc0, 0x29, 0x23);

DEFINE_GUID(CLSID_DK_IMAGE_SOURCE_FILTER,
	0x47eb2952, 0xfbf7, 0x4d27, 0xbe, 0xa9, 0x8b, 0xe6, 0xa4, 0x4b, 0xba, 0xa2);

DEFINE_GUID(CLSID_DK_YUVSOURCE_FILTER,
	0xb46bc4e, 0x729e, 0x4496, 0xa7, 0xe0, 0x82, 0x35, 0x3d, 0x1b, 0x5f, 0xc0);

DEFINE_GUID(CLSID_DK_COLORSPACE_CONVERT_FILTER,
	0x96143a18, 0xcbd3, 0x406d, 0x8d, 0xd0, 0x31, 0x9e, 0x8e, 0x2f, 0xcd, 0x9d);

DEFINE_GUID(CLSID_DK_X264_ENCODE_FILTER,
	0x8331675f, 0xe5f4, 0x4b37, 0xa8, 0xe9, 0x46, 0x36, 0x5b, 0x6e, 0x61, 0x9f);

DEFINE_GUID(CLSID_Microsoft_DTV_DVD_VideoDecoder,
	0x212690FB, 0x83E5, 0x4526, 0x8F, 0xD7, 0x74, 0x47, 0x8B, 0x79, 0x39, 0xCD);

DEFINE_GUID(CLSID_VideoMixingRenderer9,
	0x51B4ABF3, 0x748F, 0x4E3B, 0xA2, 0x76, 0xC8, 0x28, 0x33, 0x0E, 0x92, 0x6A);

DEFINE_GUID(CLSID_NullRenderer,
	0xC1F400A4, 0x3F08, 0x11D3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37);

DEFINE_GUID(CLSID_VideoRenderer_Test,
	0x6BC1CFFA, 0x8FC1, 0x4261, 0xAC, 0x22, 0xCF, 0xB4, 0xCC, 0x38, 0xDB, 0x50);


DEFINE_GUID(CLSID_DKCUDAH264DecodeFilter,
	0x0A1954E8, 0xC6FF, 0x4337, 0x82, 0x04, 0x57, 0xE5, 0x54, 0x52, 0x93, 0xEC);

DEFINE_GUID(CLSID_ffdshowVideoDecoder,
	0x04FE9017, 0xF873, 0x410E, 0x87, 0x1E, 0xAB, 0x91, 0x66, 0x1A, 0x4E, 0xF7);

DEFINE_GUID(CLSID_DKCUDAH264EncodeFilter,
	0x05187DB69, 0x7F4B, 0x4EC6, 0xB0, 0x30, 0xF6, 0xF8, 0x07, 0x99, 0x94, 0x2E);


HRESULT BuildGraph(IGraphBuilder *pGraph, int option=1)
{
	HRESULT hr = S_OK;

	if (option == 0)
	{
		CComPtr<ICaptureGraphBuilder2> pBuilder;
		hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
		CHECK_HR(hr, L"Can't create Capture Graph Builder");
		hr = pBuilder->SetFiltergraph(pGraph);
		CHECK_HR(hr, L"Can't SetFiltergraph");

		CComPtr<IBaseFilter> pCaptureFilter;
		hr = pCaptureFilter.CoCreateInstance(CLSID_DK_YUVSOURCE_FILTER);
		pGraph->AddFilter(pCaptureFilter, L"Capture Filter");
		CHECK_HR(hr, L"Can't Add Capture Filter To Graph");

#if 0
		CComPtr<IBaseFilter> pColorSpaceConverter;
		hr = pColorSpaceConverter.CoCreateInstance(CLSID_DK_COLORSPACE_CONVERT_FILTER);
		CHECK_HR(hr, L"Can't Create ColorSpace Converter");
		hr = pGraph->AddFilter(pColorSpaceConverter, L"ColorSpace Converter");
		CHECK_HR(hr, L"Can't Add ColorSpace Converter To Graph");

		CComPtr<IBaseFilter> pVideoRenderer;
		hr = pVideoRenderer.CoCreateInstance(CLSID_VideoMixingRenderer9);
		CHECK_HR(hr, L"Can't Create Video Renderer");
		hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
		CHECK_HR(hr, L"Can't Add Video Renderer To Graph");

		hr = pGraph->ConnectDirect(GetPin(pCaptureFilter, L"out"), GetPin(pColorSpaceConverter, L"XForm In"), NULL);
		CHECK_HR(hr, L"Can't Connect Capture Filter and ColorSpace Converter");

		hr = pGraph->ConnectDirect(GetPin(pColorSpaceConverter, L"XForm Out"), GetPin(pVideoRenderer, L"VMR Input0"), NULL);
		CHECK_HR(hr, L"Can't Connect ColorSpace Converter and Video Renderer");
#else
		CComPtr<IBaseFilter> pVideoRenderer;
		hr = pVideoRenderer.CoCreateInstance(CLSID_VideoMixingRenderer9);
		CHECK_HR(hr, L"Can't Create Video Renderer");
		hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
		CHECK_HR(hr, L"Can't Add Video Renderer To Graph");

		hr = pGraph->ConnectDirect(GetPin(pCaptureFilter, L"out"), GetPin(pVideoRenderer, L"VMR Input0"), NULL);
		CHECK_HR(hr, L"Can't Connect Capture Filter and ideo Renderer");
#endif
	}
	else if (option==1)
	{
		CComPtr<ICaptureGraphBuilder2> pBuilder;
		hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
		CHECK_HR(hr, L"Can't create Capture Graph Builder");
		hr = pBuilder->SetFiltergraph(pGraph);
		CHECK_HR(hr, L"Can't SetFiltergraph");

		CComPtr<IBaseFilter> pCaptureFilter;
		hr = pCaptureFilter.CoCreateInstance(CLSID_DK_YUVSOURCE_FILTER);
		pGraph->AddFilter(pCaptureFilter, L"Capture Filter");
		CHECK_HR(hr, L"Can't Add Capture Filter To Graph");


#if 0
		CComPtr<IBaseFilter> pColorSpaceConverter;
		hr = pColorSpaceConverter.CoCreateInstance(CLSID_DK_COLORSPACE_CONVERT_FILTER);
		CHECK_HR(hr, L"Can't Create ColorSpace Converter");
		hr = pGraph->AddFilter(pColorSpaceConverter, L"ColorSpace Converter");
		CHECK_HR(hr, L"Can't Add ColorSpace Converter To Graph");

		CComPtr<IBaseFilter> pEncodeFilter;
		hr = pEncodeFilter.CoCreateInstance(CLSID_DK_X264_ENCODE_FILTER);
		CHECK_HR(hr, L"Can't Create Encode Filter");
		hr = pGraph->AddFilter(pEncodeFilter, L"Encode Filter");
		CHECK_HR(hr, L"Can't Add Encode Filter To Graph");

		CComPtr<IBaseFilter> pDecodeFilter;
		hr = pDecodeFilter.CoCreateInstance(CLSID_ffdshowVideoDecoder);
		CHECK_HR(hr, L"Can't Create Decode Filter");
		hr = pGraph->AddFilter(pDecodeFilter, L"Decode Filter");
		CHECK_HR(hr, L"Can't Add Decode Filter To Graph");


		CComPtr<IBaseFilter> pVideoRenderer;
		hr = pVideoRenderer.CoCreateInstance(CLSID_VideoMixingRenderer9);
		CHECK_HR(hr, L"Can't Create Video Renderer");
		hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
		CHECK_HR(hr, L"Can't Add Video Renderer To Graph");

		hr = pGraph->ConnectDirect(GetPin(pCaptureFilter, L"out"), GetPin(pColorSpaceConverter, L"XForm In"), NULL);
		CHECK_HR(hr, L"Can't Connect Capture Filter and ColorSpace Converter");

		hr = pGraph->ConnectDirect(GetPin(pColorSpaceConverter, L"XForm Out"), GetPin(pEncodeFilter, L"XForm In"), NULL);
		CHECK_HR(hr, L"Can't Connect ColorSpace Converter and Encode Filter");

		hr = pGraph->ConnectDirect(GetPin(pEncodeFilter, L"XForm Out"), GetPin(pDecodeFilter, L"In"), NULL);
		CHECK_HR(hr, L"Can't Connect Encode Filter and Decode Filter");

		hr = pGraph->ConnectDirect(GetPin(pDecodeFilter, L"Out"), GetPin(pVideoRenderer, L"VMR Input0"), NULL);
		CHECK_HR(hr, L"Can't Connect Decode Filter and Video Renderer");
#else
		CComPtr<IBaseFilter> pEncodeFilter;
		hr = pEncodeFilter.CoCreateInstance(CLSID_DK_X264_ENCODE_FILTER);
		CHECK_HR(hr, L"Can't Create Encode Filter");
		hr = pGraph->AddFilter(pEncodeFilter, L"Encode Filter");
		CHECK_HR(hr, L"Can't Add Encode Filter To Graph");

		CComPtr<IBaseFilter> pDecodeFilter;
		hr = pDecodeFilter.CoCreateInstance(CLSID_ffdshowVideoDecoder);
		CHECK_HR(hr, L"Can't Create Decode Filter");
		hr = pGraph->AddFilter(pDecodeFilter, L"Decode Filter");
		CHECK_HR(hr, L"Can't Add Decode Filter To Graph");

		CComPtr<IBaseFilter> pVideoRenderer;
		hr = pVideoRenderer.CoCreateInstance(CLSID_VideoMixingRenderer9);
		CHECK_HR(hr, L"Can't Create Video Renderer");
		hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
		CHECK_HR(hr, L"Can't Add Video Renderer To Graph");

		hr = pGraph->ConnectDirect(GetPin(pCaptureFilter, L"out"), GetPin(pEncodeFilter, L"XForm In"), NULL);
		CHECK_HR(hr, L"Can't Connect YUV Source Filter and Encode Filter");

		hr = pGraph->ConnectDirect(GetPin(pEncodeFilter, L"XForm Out"), GetPin(pDecodeFilter, L"In"), NULL);
		CHECK_HR(hr, L"Can't Connect Encode Filter and Decode Filter");

		hr = pGraph->ConnectDirect(GetPin(pDecodeFilter, L"Out"), GetPin(pVideoRenderer, L"VMR Input0"), NULL);
		CHECK_HR(hr, L"Can't Connect Decode Filter and Video Renderer");
#endif
	}
	else if (option == 2)
	{
		CComPtr<ICaptureGraphBuilder2> pBuilder;
		hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
		CHECK_HR(hr, L"Can't create Capture Graph Builder");
		hr = pBuilder->SetFiltergraph(pGraph);
		CHECK_HR(hr, L"Can't SetFiltergraph");

		CComPtr<IBaseFilter> pCaptureFilter;
		hr = pCaptureFilter.CoCreateInstance(CLSID_DK_YUVSOURCE_FILTER);
		pGraph->AddFilter(pCaptureFilter, L"Capture Filter");
		CHECK_HR(hr, L"Can't Add Capture Filter To Graph");

#if 0
		CComPtr<IBaseFilter> pColorSpaceConverter;
		hr = pColorSpaceConverter.CoCreateInstance(CLSID_DK_COLORSPACE_CONVERT_FILTER);
		CHECK_HR(hr, L"Can't Create ColorSpace Converter");
		hr = pGraph->AddFilter(pColorSpaceConverter, L"ColorSpace Converter");
		CHECK_HR(hr, L"Can't Add ColorSpace Converter To Graph");

		CComPtr<IBaseFilter> pEncodeFilter;
		hr = pEncodeFilter.CoCreateInstance(CLSID_DK_X264_ENCODE_FILTER);
		CHECK_HR(hr, L"Can't Create Encode Filter");
		hr = pGraph->AddFilter(pEncodeFilter, L"Encode Filter");
		CHECK_HR(hr, L"Can't Add Encode Filter To Graph");

		CComPtr<IBaseFilter> pNullRenderer;
		hr = pNullRenderer.CoCreateInstance(CLSID_NullRenderer);
		CHECK_HR(hr, L"Can't Create Null Renderer");
		hr = pGraph->AddFilter(pNullRenderer, L"Null Renderer");
		CHECK_HR(hr, L"Can't Add Null Renderer To Graph");

		hr = pGraph->ConnectDirect(GetPin(pCaptureFilter, L"out"), GetPin(pColorSpaceConverter, L"XForm In"), NULL);
		CHECK_HR(hr, L"Can't Connect Capture Filter and ColorSpace Converter");

		hr = pGraph->ConnectDirect(GetPin(pColorSpaceConverter, L"XForm Out"), GetPin(pEncodeFilter, L"XForm In"), NULL);
		CHECK_HR(hr, L"Can't Connect ColorSpace Converter and Encode Filter");

		hr = pGraph->ConnectDirect(GetPin(pEncodeFilter, L"XForm Out"), GetPin(pNullRenderer, L"In"), NULL);
		CHECK_HR(hr, L"Can't Connect Decode Filter and Video Renderer");
#else
		CComPtr<IBaseFilter> pEncodeFilter;
		hr = pEncodeFilter.CoCreateInstance(CLSID_DK_X264_ENCODE_FILTER);
		CHECK_HR(hr, L"Can't Create Encode Filter");
		hr = pGraph->AddFilter(pEncodeFilter, L"Encode Filter");
		CHECK_HR(hr, L"Can't Add Encode Filter To Graph");

		CComPtr<IBaseFilter> pNullRenderer;
		hr = pNullRenderer.CoCreateInstance(CLSID_NullRenderer);
		CHECK_HR(hr, L"Can't Create Null Renderer");
		hr = pGraph->AddFilter(pNullRenderer, L"Null Renderer");
		CHECK_HR(hr, L"Can't Add Null Renderer To Graph");

		hr = pGraph->ConnectDirect(GetPin(pCaptureFilter, L"out"), GetPin(pEncodeFilter, L"XForm In"), NULL);
		CHECK_HR(hr, L"Can't Connect YUV Source Filter and Encode Filter");

		hr = pGraph->ConnectDirect(GetPin(pEncodeFilter, L"XForm Out"), GetPin(pNullRenderer, L"In"), NULL);
		CHECK_HR(hr, L"Can't Connect Decode Filter and Video Renderer");
#endif
	}
	return S_OK;
}



int _tmain(int argc, _TCHAR* argv[])
{
	if (argc == 1) 
	{
		fputs("option parameter is needed...\n", stderr);
		exit(1);
	}



	CoInitialize(NULL);
	CComPtr<IGraphBuilder> graph;
	graph.CoCreateInstance(CLSID_FilterGraph);

	printf("Building graph....\n");
	HRESULT hr = E_FAIL;
	if (!wcscmp(argv[1], L"display"))
	{
		hr = BuildGraph(graph, 0);
	}
	else if (!wcscmp(argv[1], L"transcode"))
	{
		hr = BuildGraph(graph, 1);
	}
	else if (!wcscmp(argv[1], L"encode"))
	{
		hr = BuildGraph(graph, 2);
	}
	
	if (hr == S_OK)
	{
		printf("Running");
		CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl(graph);
		hr = mediaControl->Run();
		CHECK_HR(hr, L"Can't run the graph");
		CComQIPtr<IMediaEvent, &IID_IMediaEvent> mediaEvent(graph);
		BOOL stop = FALSE;
		MSG msg;
		while (!stop)
		{
			long ev = 0, p1 = 0, p2 = 0;
			Sleep(500);
			printf(".");
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				DispatchMessage(&msg);
			while (mediaEvent->GetEvent(&ev, &p1, &p2, 0) == S_OK)
			{
				if (ev == EC_COMPLETE || ev == EC_USERABORT)
				{
					printf("Done!\n");
					stop = TRUE;
				}
				else
				{
					if (ev == EC_ERRORABORT)
					{
						printf("An error occured : HRESULT=%x\n", p1);
						mediaControl->Stop();
						stop = TRUE;
					}
				}
				mediaEvent->FreeEventParams(ev, p1, p2);
			}
		}
	}
	CoUninitialize();
	::getchar();
	return 0;
}

