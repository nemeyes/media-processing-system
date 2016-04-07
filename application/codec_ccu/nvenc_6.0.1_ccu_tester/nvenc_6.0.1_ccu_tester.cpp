// cu_h264_encoder2_test2.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <dshow.h>
#include <atlbase.h>
#include <initguid.h>
#include <dvdmedia.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <mfidl.h>	
#include <vmr9.h>
#include <evr.h>

#ifdef __cplusplus
extern "C" {
#endif

	// {263768C3-5933-4D6B-B20C-2FD1FBE62AA7}
	DEFINE_GUID(IID_IYUVSource,
		0x263768c3, 0x5933, 0x4d6b, 0xb2, 0xc, 0x2f, 0xd1, 0xfb, 0xe6, 0x2a, 0xa7);

	DECLARE_INTERFACE_IID_(IYUVSource, IUnknown, "263768C3-5933-4D6B-B20C-2FD1FBE62AA7")
	{
		STDMETHOD(SetWidth)(UINT option) PURE;
		STDMETHOD(SetHeight)(UINT option) PURE;
		STDMETHOD(SetFPS)(UINT option) PURE;
	};

#ifdef __cplusplus
}
#endif

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

DEFINE_GUID(CLSID_DKScreenCaptureFilter,
	0xdf7c90a9, 0xc202, 0x4506, 0xa4, 0xee, 0x6, 0x56, 0xc0, 0xc0, 0x29, 0x23);

DEFINE_GUID(CLSID_DK_IMAGE_SOURCE_FILTER,
	0x47eb2952, 0xfbf7, 0x4d27, 0xbe, 0xa9, 0x8b, 0xe6, 0xa4, 0x4b, 0xba, 0xa2);

DEFINE_GUID(CLSID_DK_YUVSOURCE_FILTER,
	0xb46bc4e, 0x729e, 0x4496, 0xa7, 0xe0, 0x82, 0x35, 0x3d, 0x1b, 0x5f, 0xc0);

DEFINE_GUID(CLSID_DK_COLORSPACE_CONVERT_FILTER,
	0x96143a18, 0xcbd3, 0x406d, 0x8d, 0xd0, 0x31, 0x9e, 0x8e, 0x2f, 0xcd, 0x9d);

DEFINE_GUID(CLSID_DK_NVENC_ENCODE_FILTER,
	0x41f2b1d9, 0x12c9, 0x4178, 0xa6, 0xba, 0x37, 0x89, 0x69, 0x67, 0xf3, 0x35);

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

#define MODE_DISPLAY	0
#define MODE_TRANSCODE	1
#define MODE_ENCODE		2

#define DECODER_FFDSHOW	0
#define DECODER_DXVA	1

#define RENDER_VMR9		0
#define RENDER_EVR		1

HRESULT BuildGraph(IGraphBuilder *pGraph, wchar_t * filepath, int width, int height, int fps, int mode, int decoder, int renderer)
{
	HRESULT hr = S_OK;

	if (mode == MODE_DISPLAY)
	{
		CComPtr<ICaptureGraphBuilder2> pBuilder;
		hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
		CHECK_HR(hr, L"Can't create Capture Graph Builder");
		hr = pBuilder->SetFiltergraph(pGraph);
		CHECK_HR(hr, L"Can't SetFiltergraph");

		////////// YUV Source Filter////////////////////
		CComPtr<IBaseFilter> pCaptureFilter;
		hr = pCaptureFilter.CoCreateInstance(CLSID_DK_YUVSOURCE_FILTER);
		pGraph->AddFilter(pCaptureFilter, L"Capture Filter");
		CHECK_HR(hr, L"Can't Add Capture Filter To Graph");

		//CComQIPtr<IYUVSource, &IID_IYUVSource> pYuvFilter(pCaptureFilter);
		CComQIPtr<IYUVSource> pYuvFilter(pCaptureFilter);
		pYuvFilter->SetWidth(width);
		pYuvFilter->SetHeight(height);
		pYuvFilter->SetFPS(fps);

		CComQIPtr<IFileSourceFilter> pFileSourceFilter(pCaptureFilter);
		pFileSourceFilter->Load(filepath, 0);
		////////// YUV Source Filter////////////////////

		CComPtr<IBaseFilter> pVideoRenderer;
		if (renderer==RENDER_VMR9)
		{
			hr = pVideoRenderer.CoCreateInstance(CLSID_VideoMixingRenderer9);
			CHECK_HR(hr, L"Can't Create Video Renderer");
			hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
			CHECK_HR(hr, L"Can't Add Video Renderer To Graph");

			hr = pGraph->ConnectDirect(GetPin(pCaptureFilter, L"out"), GetPin(pVideoRenderer, L"VMR Input0"), NULL);
			CHECK_HR(hr, L"Can't Connect Capture Filter and ideo Renderer");
		}
		else if (renderer == RENDER_EVR)
		{
			hr = pVideoRenderer.CoCreateInstance(CLSID_EnhancedVideoRenderer);
			CHECK_HR(hr, L"Can't Create Video Renderer");
			hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
			CHECK_HR(hr, L"Can't Add Video Renderer To Graph");

			CComQIPtr<IMFGetService> mfgs(pVideoRenderer);
			CComPtr<IMFVideoDisplayControl> display;
			if (mfgs)
				hr = mfgs->GetService(MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&display));
			else
				return E_FAIL;

			if (FAILED(hr))
				return hr;

			// Set the clipping window.
			//hr = display->SetVideoWindow(hwnd);
			//if (FAILED(hr))
			//	return hr;

			hr = display->SetAspectRatioMode(MFVideoARMode_PreservePicture);
			if (FAILED(hr))
				return hr;

			hr = pGraph->ConnectDirect(GetPin(pCaptureFilter, L"out"), GetPin(pVideoRenderer, L"EVR Input0"), NULL);
			CHECK_HR(hr, L"Can't Connect Capture Filter and ideo Renderer");
		}
	}
	else if (mode == MODE_TRANSCODE)
	{
		CComPtr<ICaptureGraphBuilder2> pBuilder;
		hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
		CHECK_HR(hr, L"Can't create Capture Graph Builder");
		hr = pBuilder->SetFiltergraph(pGraph);
		CHECK_HR(hr, L"Can't SetFiltergraph");

		////////// YUV Source Filter////////////////////
		CComPtr<IBaseFilter> pCaptureFilter;
		hr = pCaptureFilter.CoCreateInstance(CLSID_DK_YUVSOURCE_FILTER);
		pGraph->AddFilter(pCaptureFilter, L"Capture Filter");
		CHECK_HR(hr, L"Can't Add Capture Filter To Graph");

		CComQIPtr<IYUVSource> pYuvFilter(pCaptureFilter);
		pYuvFilter->SetWidth(width);
		pYuvFilter->SetHeight(height);
		pYuvFilter->SetFPS(fps);

		CComQIPtr<IFileSourceFilter> pFileSourceFilter(pCaptureFilter);
		pFileSourceFilter->Load(filepath, 0);
		////////// YUV Source Filter////////////////////

		CComPtr<IBaseFilter> pEncodeFilter;
		hr = pEncodeFilter.CoCreateInstance(CLSID_DK_NVENC_ENCODE_FILTER);
		CHECK_HR(hr, L"Can't Create Encode Filter");
		hr = pGraph->AddFilter(pEncodeFilter, L"Encode Filter");
		CHECK_HR(hr, L"Can't Add Encode Filter To Graph");

		CComPtr<IBaseFilter> pDecodeFilter;
		if (decoder==DECODER_FFDSHOW)
		{
			hr = pDecodeFilter.CoCreateInstance(CLSID_ffdshowVideoDecoder);
			CHECK_HR(hr, L"Can't Create Decode Filter");
			hr = pGraph->AddFilter(pDecodeFilter, L"Decode Filter");
			CHECK_HR(hr, L"Can't Add Decode Filter To Graph");
		}
		else if (decoder == DECODER_DXVA)
		{
			hr = pDecodeFilter.CoCreateInstance(CLSID_Microsoft_DTV_DVD_VideoDecoder);
			CHECK_HR(hr, L"Can't Create Decode Filter");
			hr = pGraph->AddFilter(pDecodeFilter, L"Decode Filter");
			CHECK_HR(hr, L"Can't Add Decode Filter To Graph");
		}

		CComPtr<IBaseFilter> pVideoRenderer;
		if (renderer == RENDER_VMR9)
		{
			hr = pVideoRenderer.CoCreateInstance(CLSID_VideoMixingRenderer9);
			CHECK_HR(hr, L"Can't Create Video Renderer");
			hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
		}
		else if (renderer == RENDER_EVR)
		{
			hr = pVideoRenderer.CoCreateInstance(CLSID_EnhancedVideoRenderer);
			CHECK_HR(hr, L"Can't Create Video Renderer");
			hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
		}
		CHECK_HR(hr, L"Can't Add Video Renderer To Graph");

		hr = pGraph->ConnectDirect(GetPin(pCaptureFilter, L"out"), GetPin(pEncodeFilter, L"XForm In"), NULL);
		CHECK_HR(hr, L"Can't Connect YUV Source Filter and Encode Filter");

		if (decoder == DECODER_FFDSHOW)
		{
			hr = pGraph->ConnectDirect(GetPin(pEncodeFilter, L"XForm Out"), GetPin(pDecodeFilter, L"In"), NULL);
		}
		else if (decoder == DECODER_DXVA)
		{
			hr = pGraph->ConnectDirect(GetPin(pEncodeFilter, L"XForm Out"), GetPin(pDecodeFilter, L"Video Input"), NULL);
		}
		CHECK_HR(hr, L"Can't Connect Encode Filter and Decode Filter");

		if (renderer == RENDER_VMR9)
		{
			if (decoder == DECODER_FFDSHOW)
			{
				hr = pGraph->ConnectDirect(GetPin(pDecodeFilter, L"Out"), GetPin(pVideoRenderer, L"VMR Input0"), NULL);
			}
			else if (decoder == DECODER_DXVA)
			{
				hr = pGraph->ConnectDirect(GetPin(pDecodeFilter, L"Video Output 1"), GetPin(pVideoRenderer, L"VMR Input0"), NULL);
			}
			CHECK_HR(hr, L"Can't Connect Decode Filter and Video Renderer");
		}
		else if (renderer == RENDER_EVR)
		{
			CComQIPtr<IMFGetService> mfgs(pVideoRenderer);
			CComPtr<IMFVideoDisplayControl> display;
			if (mfgs)
				hr = mfgs->GetService(MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&display));
			else
				return E_FAIL;

			if (FAILED(hr))
				return hr;

			//display->SetVideoPosition()
			// Set the clipping window.
			//hr = display->SetVideoWindow(hwnd);
			//if (FAILED(hr))
			//	return hr;

			hr = display->SetAspectRatioMode(MFVideoARMode_PreservePicture);
			if (FAILED(hr))
				return hr;

			if (decoder == DECODER_FFDSHOW)
			{
				hr = pGraph->ConnectDirect(GetPin(pDecodeFilter, L"Out"), GetPin(pVideoRenderer, L"EVR Input0"), NULL);
			}
			else if (decoder == DECODER_DXVA)
			{
				hr = pGraph->ConnectDirect(GetPin(pDecodeFilter, L"Video Output 1"), GetPin(pVideoRenderer, L"EVR Input0"), NULL);
			}
			CHECK_HR(hr, L"Can't Connect Capture Filter and ideo Renderer");
		}
	}
	else if (mode == MODE_ENCODE)
	{
		CComPtr<ICaptureGraphBuilder2> pBuilder;
		hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
		CHECK_HR(hr, L"Can't create Capture Graph Builder");
		hr = pBuilder->SetFiltergraph(pGraph);
		CHECK_HR(hr, L"Can't SetFiltergraph");

		////////// YUV Source Filter////////////////////
		CComPtr<IBaseFilter> pCaptureFilter;
		hr = pCaptureFilter.CoCreateInstance(CLSID_DK_YUVSOURCE_FILTER);
		pGraph->AddFilter(pCaptureFilter, L"Capture Filter");
		CHECK_HR(hr, L"Can't Add Capture Filter To Graph");

		CComQIPtr<IYUVSource> pYuvFilter(pCaptureFilter);
		pYuvFilter->SetWidth(width);
		pYuvFilter->SetHeight(height);
		pYuvFilter->SetFPS(fps);

		CComQIPtr<IFileSourceFilter> pFileSourceFilter(pCaptureFilter);
		pFileSourceFilter->Load(filepath, 0);
		////////// YUV Source Filter////////////////////

		CComPtr<IBaseFilter> pEncodeFilter;
		hr = pEncodeFilter.CoCreateInstance(CLSID_DK_NVENC_ENCODE_FILTER);
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
	}
	return S_OK;
}

typedef struct _configuration_t
{
	wchar_t mode[100];
	wchar_t filepath[260];
	wchar_t decoder[260];
	wchar_t renderer[260];
	int width;
	int height;
	int fps;
} configuration_t;

bool parse_argument(configuration_t * configuration, int argc, wchar_t * argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (_wcsicmp(argv[i], L"-yuvFilePath") == 0)
		{
			if (++i >= argc)
			{
				wprintf(L"invalid parameter for %s\n", argv[i - 1]);
				return false;
			}
			wcscpy_s(configuration->filepath, argv[i]);
		}
		else if (_wcsicmp(argv[i], L"-size") == 0)
		{
			if (++i >= argc || swscanf_s(argv[i], L"%d", &configuration->width) != 1)
			{
				wprintf(L"invalid parameter for %s\n", argv[i - 1]);
				return false;
			}

			if (++i >= argc || swscanf_s(argv[i], L"%d", &configuration->height) != 1)
			{
				wprintf(L"invalid parameter for %s\n", argv[i - 2]);
				return false;
			}
		}
		else if (_wcsicmp(argv[i], L"-fps") == 0)
		{
			if (++i >= argc || swscanf_s(argv[i], L"%d", &configuration->fps) != 1)
			{
				wprintf(L"invalid parameter for %s\n", argv[i - 1]);
				return false;
			}
		}
		else if (_wcsicmp(argv[i], L"-mode") == 0)
		{
			if (++i >= argc)
			{
				wprintf(L"invalid parameter for %s\n", argv[i - 1]);
				return false;
			}
			wcscpy_s(configuration->mode, argv[i]);
		}
		else if (_wcsicmp(argv[i], L"-decoder") == 0)
		{
			if (++i >= argc)
			{
				wprintf(L"invalid parameter for %s\n", argv[i - 1]);
				return false;
			}
			wcscpy_s(configuration->decoder, argv[i]);
		}
		else if (_wcsicmp(argv[i], L"-renderer") == 0)
		{
			if (++i >= argc)
			{
				wprintf(L"invalid parameter for %s\n", argv[i - 1]);
				return false;
			}
			wcscpy_s(configuration->renderer, argv[i]);
		}
		else if (_wcsicmp(argv[i], L"-help") == 0)
		{
			return false;
		}
		else
		{
			wprintf(L"invalid parameter  %s\n", argv[i++]);
			return false;
		}
	}
	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	configuration_t configuration;
	memset(&configuration, 0x00, sizeof(configuration_t));
	wcscpy_s(configuration.decoder, L"ffdshow");
	wcscpy_s(configuration.renderer, L"vmr9");

	bool result = parse_argument(&configuration, argc, argv);
	if (!result)
	{
		fputs("option parameter is needed...\n", stderr);
		exit(1);
	}

	wprintf(L"----------------------------------------------------\n");
	wprintf(L"\t mode : %s\n", configuration.mode);
	wprintf(L"\t input yuv file : %s\n", configuration.filepath);
	wprintf(L"\t width : %d\n", configuration.width);
	wprintf(L"\t height : %d\n", configuration.height);
	wprintf(L"\t fps : %d\n", configuration.fps);
	wprintf(L"\t decoder : %s\n", configuration.decoder);
	wprintf(L"\t renderer : %s\n", configuration.renderer);
	wprintf(L"----------------------------------------------------\n");

	CoInitialize(NULL);
	CComPtr<IGraphBuilder> graph;
	graph.CoCreateInstance(CLSID_FilterGraph);

	printf("Building graph....\n");
	HRESULT hr = E_FAIL;

	int mode = MODE_DISPLAY;
	if (!wcscmp(configuration.mode, L"display"))
		mode = MODE_DISPLAY;
	else if (!wcscmp(configuration.mode, L"transcode"))
		mode = MODE_TRANSCODE;
	else if (!wcscmp(configuration.mode, L"encode"))
		mode = MODE_ENCODE;

	int decoder = DECODER_FFDSHOW;
	if (!wcscmp(configuration.decoder, L"ffdshow"))
		decoder = DECODER_FFDSHOW;
	else if (!wcscmp(configuration.decoder, L"dxva"))
		decoder = DECODER_DXVA;

	int renderer = RENDER_VMR9;
	if (!wcscmp(configuration.renderer, L"vmr9"))
		renderer = RENDER_VMR9;
	else if (!wcscmp(configuration.renderer, L"evr"))
		renderer = RENDER_EVR;

	hr = BuildGraph(graph, configuration.filepath, configuration.width, configuration.height, configuration.fps, mode, decoder, renderer);
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

