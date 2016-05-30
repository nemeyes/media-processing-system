#pragma once

#include "common.h"
#include "display.h"

namespace debuggerking
{
	class presenter : public IMFVideoDisplayControl,
					  public IMFGetService, 
					  private mf_base
	{
	public:
		presenter(void);
		virtual ~presenter(void);

		// IUnknown
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void ** ppv);
		STDMETHODIMP_(ULONG) Release(void);

		// IMFVideoDisplayControl
		STDMETHODIMP GetAspectRatioMode(__RPC__out DWORD* pdwAspectRatioMode) { return E_NOTIMPL; }
		STDMETHODIMP GetBorderColor(__RPC__out COLORREF* pClr) { return E_NOTIMPL; }
		STDMETHODIMP GetCurrentImage(__RPC__inout BITMAPINFOHEADER* pBih, __RPC__deref_out_ecount_full_opt(*pcbDib) BYTE** pDib, __RPC__out DWORD* pcbDib, __RPC__inout_opt LONGLONG* pTimestamp) { return E_NOTIMPL; }
		STDMETHODIMP GetFullscreen(__RPC__out BOOL* pfFullscreen);
		STDMETHODIMP GetIdealVideoSize(__RPC__inout_opt SIZE* pszMin, __RPC__inout_opt SIZE* pszMax) { return E_NOTIMPL; }
		STDMETHODIMP GetNativeVideoSize(__RPC__inout_opt SIZE* pszVideo, __RPC__inout_opt SIZE* pszARVideo) { return E_NOTIMPL; }
		STDMETHODIMP GetRenderingPrefs(__RPC__out DWORD* pdwRenderFlags) { return E_NOTIMPL; }
		STDMETHODIMP GetVideoPosition(__RPC__out MFVideoNormalizedRect* pnrcSource, __RPC__out LPRECT prcDest) { return E_NOTIMPL; }
		STDMETHODIMP GetVideoWindow(__RPC__deref_out_opt HWND* phwndVideo) { return E_NOTIMPL; }
		STDMETHODIMP RepaintVideo(void) { return E_NOTIMPL; }
		STDMETHODIMP SetAspectRatioMode(DWORD dwAspectRatioMode) { return E_NOTIMPL; }
		STDMETHODIMP SetBorderColor(COLORREF Clr) { return E_NOTIMPL; }
		STDMETHODIMP SetFullscreen(BOOL fFullscreen);
		STDMETHODIMP SetRenderingPrefs(DWORD dwRenderingPrefs) { return E_NOTIMPL; }
		STDMETHODIMP SetVideoPosition(__RPC__in_opt const MFVideoNormalizedRect* pnrcSource, __RPC__in_opt const LPRECT prcDest) { return E_NOTIMPL; }
		STDMETHODIMP SetVideoWindow(__RPC__in HWND hwndVideo);

		// IMFGetService
		STDMETHODIMP GetService(__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppvObject);

		BOOL    can_process_next_sample(void);
		HRESULT flush(void);
		HRESULT get_monitor_refresh_rate(DWORD * pdw_monitor_refresh_rate);
		HRESULT is_media_type_supported(IMFMediaType * pmedia_type, DXGI_FORMAT dxgi_format);
		HRESULT present_frame(void);
		HRESULT process_frame(IMFMediaType * pcurrent_type, IMFSample * psample, UINT32 * pinterlace_mode, BOOL * pdevice_changed, BOOL * pprocess_again, IMFSample ** ppoutput_sample = NULL);
		HRESULT set_current_media_type(IMFMediaType * pmedia_type);
		HRESULT shutdown(void);

	private:

		void AspectRatioCorrectSize(LPSIZE lpSizeImage,     // size to be aspect ratio corrected
									const SIZE& sizeAr,     // aspect ratio of image
									const SIZE& sizeOrig,   // original image size
									BOOL ScaleXorY);          // axis to correct in
		void    CheckDecodeSwitchRegKey(void);
		HRESULT CheckDeviceState(BOOL* pbDeviceChanged);
		BOOL    CheckEmptyRect(RECT* pDst);
		HRESULT CheckShutdown(void) const;
		HRESULT CreateDCompDeviceAndVisual(void);
		HRESULT CreateDXGIManagerAndDevice(D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE);
		HRESULT CreateXVP(void);
		HRESULT FindBOBProcessorIndex(DWORD* pIndex);
		HRESULT GetVideoDisplayArea(IMFMediaType* pType, MFVideoArea* pArea);
		void    LetterBoxDstRect(
			LPRECT lprcLBDst,   // output letterboxed rectangle
			const RECT& rcSrc,  // input source rectangle
			const RECT& rcDst   // input destination rectangle
			);
		void    PixelAspectToPictureAspect(int Width, int Height, int PixelAspectX, int PixelAspectY, int* pPictureAspectX, int* pPictureAspectY);
		HRESULT ProcessFrameUsingD3D11(ID3D11Texture2D* pLeftTexture2D, ID3D11Texture2D* pRightTexture2D, UINT dwLeftViewIndex, UINT dwRightViewIndex, RECT rcDest, UINT32 unInterlaceMode, IMFSample** ppVideoOutFrame);
		HRESULT ProcessFrameUsingXVP(IMFMediaType* pCurrentType, IMFSample* pVideoFrame, ID3D11Texture2D* pTexture2D, RECT rcDest, IMFSample** ppVideoOutFrame, BOOL* pbInputFrameUsed);
		void    ReduceToLowestTerms(int NumeratorIn, int DenominatorIn, int* pNumeratorOut, int* pDenominatorOut);
		HRESULT SetMonitor(UINT adapterID);
		void    SetVideoContextParameters(ID3D11VideoContext* pVideoContext, const RECT* pSRect, const RECT* pTRect, UINT32 unInterlaceMode);
		HRESULT SetVideoMonitor(HWND hwndVideo);
		HRESULT SetXVPOutputMediaType(IMFMediaType* pType, DXGI_FORMAT vpOutputFormat);
		_Post_satisfies_(this->m_pSwapChain1 != NULL)
		HRESULT UpdateDXGISwapChain(void);
		void    UpdateRectangles(RECT* pDst, RECT* pSrc);

		long								_ref_count;                // reference count
		critical_section					_cs;                  // critical section for thread safety
		BOOL								_is_shutdown;               // Flag to indicate if Shutdown() method was called.
		IDXGIFactory2 *						_dxgi_factory2;
		ID3D11Device *						_d3d11_device;
		ID3D11DeviceContext *				_d3d_immediate_context;
		IMFDXGIDeviceManager *				_dxgi_manager;
		IDXGIOutput1 *						_dxgi_output1;
		IMFVideoSampleAllocatorEx *			_sample_allocator_ex;
		IDCompositionDevice *				_dcomp_device;
		IDCompositionTarget *				_hwnd_target;
		IDCompositionVisual *				_root_visual;
		BOOL								_software_dxva_device_in_use;
		HWND								_hwnd_video;
		monitor_array *						_monitors;
		cam_ddraw_monitor_info_t *			_current_monitor;
		UINT								_device_reset_token;
		UINT								_connection_guid;
		UINT								_dx_sw_switch;
		UINT								_use_xvp;
		UINT								_use_dcomp_visual;
		UINT								_use_debug_layer;
		ID3D11VideoDevice *					_dx11_video_device;
		ID3D11VideoProcessorEnumerator *	_video_processor_enum;
		ID3D11VideoProcessor *				_video_processor;
		IDXGISwapChain1 *					_swap_chain1;
		BOOL								_device_changed;
		BOOL								_resize;
		BOOL								_3d_video;
		BOOL								_stereo_enabled;
		MFVideo3DFormat						_vp_3d_output;
		BOOL								_fullscreen_state;
		BOOL								_can_process_next_sample;
		RECT								_display_rect;
		UINT32								_image_width_in_pixels;
		UINT32								_image_height_in_pixels;
		UINT32								_real_display_width;
		UINT32								_real_display_height;
		RECT								_rc_src_app;
		RECT								_rc_dst_app;
		IMFTransform *						_xvp;
		IMFVideoProcessorControl *			_xvp_control;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Wrapper class for D3D11 Device and D3D11 Video device used for DXVA to Software decode switch
	class PrivateID3D11VideoDevice : public ID3D11VideoDevice
	{
	private:
		ID3D11VideoDevice * _real;
		ULONG _ref_count;
	public:

		PrivateID3D11VideoDevice(ID3D11VideoDevice * real)
			: _real(real)
			, _ref_count(0)
		{}

		virtual ~PrivateID3D11VideoDevice(void) {}

		STDMETHODIMP QueryInterface(/* [in] */ REFIID riid, /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR* __RPC_FAR * ppvobj)
		{
			if (__uuidof(ID3D11VideoDevice) == riid)
			{
				this->AddRef();
				*ppvobj = this;
				return S_OK;
			}
			else
			{
				return _real->QueryInterface(riid, ppvobj);
			}
		}

		STDMETHODIMP_(ULONG) AddRef(void)
		{
			InterlockedIncrement(&_ref_count);
			return _real->AddRef();
		}

		STDMETHODIMP_(ULONG) Release(void)
		{
			ULONG ulVal = _real->Release();
			if (0 == InterlockedDecrement(&_ref_count))
			{
			}
			return ulVal;
		}

		STDMETHODIMP CreateVideoDecoder(_In_  const D3D11_VIDEO_DECODER_DESC* pVideoDesc, _In_  const D3D11_VIDEO_DECODER_CONFIG* pConfig, _Out_  ID3D11VideoDecoder** ppDecoder)
		{
			return E_FAIL;
		}

		STDMETHODIMP CreateVideoProcessor(_In_  ID3D11VideoProcessorEnumerator* pEnum, _In_  UINT RateConversionIndex, _Out_  ID3D11VideoProcessor** ppVideoProcessor)
		{
			return _real->CreateVideoProcessor(pEnum, RateConversionIndex, ppVideoProcessor);
		}

		STDMETHODIMP CreateAuthenticatedChannel(_In_  D3D11_AUTHENTICATED_CHANNEL_TYPE ChannelType, _Out_  ID3D11AuthenticatedChannel** ppAuthenticatedChannel)
		{
			return _real->CreateAuthenticatedChannel(ChannelType, ppAuthenticatedChannel);
		}

		STDMETHODIMP CreateCryptoSession(_In_  const GUID* pCryptoType, _In_opt_  const GUID* pDecoderProfile, _In_  const GUID* pKeyExchangeType, _Outptr_  ID3D11CryptoSession** ppCryptoSession)
		{
			return _real->CreateCryptoSession(pCryptoType, pDecoderProfile, pKeyExchangeType, ppCryptoSession);
		}

		STDMETHODIMP CreateVideoDecoderOutputView(_In_  ID3D11Resource* pResource, _In_  const D3D11_VIDEO_DECODER_OUTPUT_VIEW_DESC* pDesc, _Out_opt_  ID3D11VideoDecoderOutputView** ppVDOVView)
		{
			return _real->CreateVideoDecoderOutputView(pResource, pDesc, ppVDOVView);
		}

		STDMETHODIMP CreateVideoProcessorInputView(_In_  ID3D11Resource* pResource, _In_  ID3D11VideoProcessorEnumerator* pEnum, _In_  const D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC* pDesc, _Out_opt_  ID3D11VideoProcessorInputView** ppVPIView)
		{
			return _real->CreateVideoProcessorInputView(pResource, pEnum, pDesc, ppVPIView);
		}

		STDMETHODIMP CreateVideoProcessorOutputView(_In_  ID3D11Resource* pResource, _In_  ID3D11VideoProcessorEnumerator* pEnum, _In_  const D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC* pDesc, _Out_opt_  ID3D11VideoProcessorOutputView** ppVPOView)
		{
			return _real->CreateVideoProcessorOutputView(pResource, pEnum, pDesc, ppVPOView);
		}

		STDMETHODIMP CreateVideoProcessorEnumerator(_In_  const D3D11_VIDEO_PROCESSOR_CONTENT_DESC* pDesc, _Out_  ID3D11VideoProcessorEnumerator** ppEnum)
		{
			return _real->CreateVideoProcessorEnumerator(pDesc, ppEnum);
		}

		STDMETHODIMP_(UINT) GetVideoDecoderProfileCount(void)
		{
			return _real->GetVideoDecoderProfileCount();
		}

		STDMETHODIMP GetVideoDecoderProfile(_In_  UINT Index, _Out_  GUID* pDecoderProfile)
		{
			return _real->GetVideoDecoderProfile(Index, pDecoderProfile);
		}

		STDMETHODIMP CheckVideoDecoderFormat(_In_  const GUID* pDecoderProfile, _In_  DXGI_FORMAT Format, _Out_  BOOL* pSupported)
		{
			return _real->CheckVideoDecoderFormat(pDecoderProfile, Format, pSupported);
		}

		STDMETHODIMP GetVideoDecoderConfigCount(_In_  const D3D11_VIDEO_DECODER_DESC* pDesc, _Out_  UINT* pCount)
		{
			return _real->GetVideoDecoderConfigCount(pDesc, pCount);
		}

		STDMETHODIMP GetVideoDecoderConfig(_In_  const D3D11_VIDEO_DECODER_DESC* pDesc, _In_  UINT Index, _Out_  D3D11_VIDEO_DECODER_CONFIG* pConfig)
		{
			return _real->GetVideoDecoderConfig(pDesc, Index, pConfig);
		}

		STDMETHODIMP GetContentProtectionCaps(_In_opt_  const GUID* pCryptoType, _In_opt_  const GUID* pDecoderProfile, _Out_  D3D11_VIDEO_CONTENT_PROTECTION_CAPS* pCaps)
		{
			return _real->GetContentProtectionCaps(pCryptoType, pDecoderProfile, pCaps);
		}

		STDMETHODIMP CheckCryptoKeyExchange(_In_  const GUID* pCryptoType, _In_opt_  const GUID* pDecoderProfile, _In_  UINT Index, _Out_  GUID* pKeyExchangeType)
		{
			return _real->CheckCryptoKeyExchange(pCryptoType, pDecoderProfile, Index, pKeyExchangeType);
		}

		STDMETHODIMP SetPrivateData(_In_  REFGUID guid, _In_  UINT DataSize, _In_reads_bytes_opt_(DataSize)  const void * pData)
		{
			return _real->SetPrivateData(guid, DataSize, pData);
		}

		STDMETHODIMP SetPrivateDataInterface(_In_  REFGUID guid, _In_opt_  const IUnknown* pData)
		{
			return _real->SetPrivateDataInterface(guid, pData);
		}
	};

	class PrivateID3D11Device : public ID3D11Device
	{
	private:
		ID3D11Device * _real;
		ULONG _ref_count;
		PrivateID3D11VideoDevice * _video_device;

	public:
		PrivateID3D11Device(ID3D11Device * real)
			: _real(real)
			, _ref_count(1)
			, _video_device(NULL)
		{
			ID3D11VideoDevice * device;
			_real->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&device);
			_video_device = new PrivateID3D11VideoDevice(device);
			if (device != NULL)
			{
				device->Release();
			}
		}

		virtual ~PrivateID3D11Device(void)
		{
			safe_delete(_video_device);
		}

		STDMETHODIMP QueryInterface(/* [in] */ REFIID riid, /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR* __RPC_FAR * ppvobj)
		{
			if (__uuidof(ID3D11VideoDevice) == riid)
			{
				_video_device->AddRef();
				*ppvobj = _video_device;
				return S_OK;
			}
			else if (__uuidof(ID3D11Device) == riid)
			{
				this->AddRef();
				*ppvobj = this;
				return S_OK;
			}
			else
			{
				return _real->QueryInterface(riid, ppvobj);
			}
		}

		STDMETHODIMP_(ULONG) AddRef(void)
		{
			InterlockedIncrement(&_ref_count);
			return _real->AddRef();
		}

		STDMETHODIMP_(ULONG) Release(void)
		{
			ULONG ulVal = _real->Release();
			if (0 == InterlockedDecrement(&_ref_count))
			{
				delete this;
			}
			return ulVal;
		}

		STDMETHODIMP CreateBuffer(_In_  const D3D11_BUFFER_DESC* pDesc, _In_opt_  const D3D11_SUBRESOURCE_DATA* pInitialData, _Out_opt_  ID3D11Buffer ** ppBuffer)
		{
			return _real->CreateBuffer(pDesc, pInitialData, ppBuffer);
		}

		STDMETHODIMP CreateTexture1D(_In_  const D3D11_TEXTURE1D_DESC* pDesc, _In_reads_opt_(pDesc->MipLevels * pDesc->ArraySize)  const D3D11_SUBRESOURCE_DATA* pInitialData, _Out_opt_  ID3D11Texture1D** ppTexture1D)
		{
			return _real->CreateTexture1D(pDesc, pInitialData, ppTexture1D);
		}

		STDMETHODIMP CreateTexture2D(_In_  const D3D11_TEXTURE2D_DESC* pDesc, _In_reads_opt_(pDesc->MipLevels * pDesc->ArraySize)  const D3D11_SUBRESOURCE_DATA* pInitialData, _Out_opt_  ID3D11Texture2D** ppTexture2D)
		{
			return _real->CreateTexture2D(pDesc, pInitialData, ppTexture2D);
		}

		STDMETHODIMP CreateTexture3D(_In_  const D3D11_TEXTURE3D_DESC* pDesc, _In_reads_opt_(pDesc->MipLevels)  const D3D11_SUBRESOURCE_DATA* pInitialData, _Out_opt_  ID3D11Texture3D** ppTexture3D)
		{
			return _real->CreateTexture3D(pDesc, pInitialData, ppTexture3D);
		}

		STDMETHODIMP CreateShaderResourceView(_In_  ID3D11Resource* pResource, _In_opt_  const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, _Out_opt_  ID3D11ShaderResourceView** ppSRView)
		{
			return _real->CreateShaderResourceView(pResource, pDesc, ppSRView);
		}

		STDMETHODIMP CreateUnorderedAccessView(_In_  ID3D11Resource* pResource, _In_opt_  const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, _Out_opt_  ID3D11UnorderedAccessView** ppUAView)
		{
			return _real->CreateUnorderedAccessView(pResource, pDesc, ppUAView);
		}

		STDMETHODIMP CreateRenderTargetView(_In_  ID3D11Resource* pResource, _In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, _Out_opt_  ID3D11RenderTargetView** ppRTView)
		{
			return _real->CreateRenderTargetView(pResource, pDesc, ppRTView);
		}

		STDMETHODIMP CreateDepthStencilView(_In_  ID3D11Resource* pResource, _In_opt_  const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, _Out_opt_  ID3D11DepthStencilView** ppDepthStencilView)
		{
			return _real->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
		}

		STDMETHODIMP CreateInputLayout(_In_reads_(NumElements)  const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, _In_range_(0, D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT)  UINT NumElements, _In_  const void* pShaderBytecodeWithInputSignature, _In_  SIZE_T BytecodeLength, _Out_opt_  ID3D11InputLayout** ppInputLayout)
		{
			return _real->CreateInputLayout(pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength, ppInputLayout);
		}

		STDMETHODIMP CreateVertexShader(_In_  const void* pShaderBytecode, _In_  SIZE_T BytecodeLength, _In_opt_  ID3D11ClassLinkage* pClassLinkage, _Out_opt_  ID3D11VertexShader** ppVertexShader)
		{
			return _real->CreateVertexShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);
		}

		STDMETHODIMP CreateGeometryShader(_In_  const void* pShaderBytecode, _In_  SIZE_T BytecodeLength, _In_opt_  ID3D11ClassLinkage* pClassLinkage, _Out_opt_  ID3D11GeometryShader** ppGeometryShader)
		{
			return _real->CreateGeometryShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppGeometryShader);
		}

		STDMETHODIMP CreateGeometryShaderWithStreamOutput(_In_  const void* pShaderBytecode, _In_  SIZE_T BytecodeLength, _In_reads_opt_(NumEntries)  const D3D11_SO_DECLARATION_ENTRY* pSODeclaration, _In_range_(0, D3D11_SO_STREAM_COUNT * D3D11_SO_OUTPUT_COMPONENT_COUNT)  UINT NumEntries, _In_reads_opt_(NumStrides)  const UINT* pBufferStrides, _In_range_(0, D3D11_SO_BUFFER_SLOT_COUNT)  UINT NumStrides, _In_  UINT RasterizedStream, _In_opt_  ID3D11ClassLinkage* pClassLinkage, _Out_opt_  ID3D11GeometryShader** ppGeometryShader)
		{
			return _real->CreateGeometryShaderWithStreamOutput(pShaderBytecode, BytecodeLength, pSODeclaration, NumEntries, pBufferStrides, NumStrides, RasterizedStream, pClassLinkage, ppGeometryShader);
		}

		STDMETHODIMP CreatePixelShader(_In_  const void* pShaderBytecode, _In_  SIZE_T BytecodeLength, _In_opt_  ID3D11ClassLinkage* pClassLinkage, _Out_opt_  ID3D11PixelShader** ppPixelShader)
		{
			return _real->CreatePixelShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader);
		}

		STDMETHODIMP CreateHullShader(_In_  const void* pShaderBytecode, _In_  SIZE_T BytecodeLength, _In_opt_  ID3D11ClassLinkage* pClassLinkage, _Out_opt_  ID3D11HullShader** ppHullShader)
		{
			return _real->CreateHullShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppHullShader);
		}

		STDMETHODIMP CreateDomainShader(_In_  const void* pShaderBytecode, _In_  SIZE_T BytecodeLength, _In_opt_  ID3D11ClassLinkage* pClassLinkage, _Out_opt_  ID3D11DomainShader** ppDomainShader)
		{
			return _real->CreateDomainShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppDomainShader);
		}

		STDMETHODIMP CreateComputeShader(_In_  const void* pShaderBytecode, _In_  SIZE_T BytecodeLength, _In_opt_  ID3D11ClassLinkage* pClassLinkage, _Out_opt_  ID3D11ComputeShader** ppComputeShader)
		{
			return _real->CreateComputeShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppComputeShader);
		}

		STDMETHODIMP CreateClassLinkage(_Out_  ID3D11ClassLinkage** ppLinkage)
		{
			return _real->CreateClassLinkage(ppLinkage);
		}

		STDMETHODIMP CreateBlendState(_In_  const D3D11_BLEND_DESC* pBlendStateDesc, _Out_opt_  ID3D11BlendState** ppBlendState)
		{
			return _real->CreateBlendState(pBlendStateDesc, ppBlendState);
		}

		STDMETHODIMP CreateDepthStencilState(_In_  const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, _Out_opt_  ID3D11DepthStencilState** ppDepthStencilState)
		{
			return _real->CreateDepthStencilState(pDepthStencilDesc, ppDepthStencilState);
		}

		STDMETHODIMP CreateRasterizerState(_In_  const D3D11_RASTERIZER_DESC* pRasterizerDesc, _Out_opt_  ID3D11RasterizerState** ppRasterizerState)
		{
			return _real->CreateRasterizerState(pRasterizerDesc, ppRasterizerState);
		}

		STDMETHODIMP CreateSamplerState(_In_  const D3D11_SAMPLER_DESC* pSamplerDesc, _Out_opt_  ID3D11SamplerState** ppSamplerState)
		{
			return _real->CreateSamplerState(pSamplerDesc, ppSamplerState);
		}

		STDMETHODIMP CreateQuery(_In_  const D3D11_QUERY_DESC* pQueryDesc, _Out_opt_  ID3D11Query** ppQuery)
		{
			return _real->CreateQuery(pQueryDesc, ppQuery);
		}

		STDMETHODIMP CreatePredicate(_In_  const D3D11_QUERY_DESC* pPredicateDesc, _Out_opt_  ID3D11Predicate** ppPredicate)
		{
			return _real->CreatePredicate(pPredicateDesc, ppPredicate);
		}

		STDMETHODIMP CreateCounter(_In_  const D3D11_COUNTER_DESC* pCounterDesc, _Out_opt_  ID3D11Counter** ppCounter)
		{
			return _real->CreateCounter(pCounterDesc, ppCounter);
		}

		STDMETHODIMP CreateDeferredContext(UINT ContextFlags, _Out_opt_  ID3D11DeviceContext** ppDeferredContext)
		{
			return _real->CreateDeferredContext(ContextFlags, ppDeferredContext);
		}

		STDMETHODIMP OpenSharedResource(_In_  HANDLE hResource, _In_  REFIID ReturnedInterface, _Out_opt_  void** ppResource)
		{
			return _real->OpenSharedResource(hResource, ReturnedInterface, ppResource);
		}

		STDMETHODIMP CheckFormatSupport(_In_  DXGI_FORMAT Format, _Out_  UINT* pFormatSupport)
		{
			return _real->CheckFormatSupport(Format, pFormatSupport);
		}

		STDMETHODIMP CheckMultisampleQualityLevels(_In_  DXGI_FORMAT Format, _In_  UINT SampleCount, _Out_  UINT* pNumQualityLevels)
		{
			return _real->CheckMultisampleQualityLevels(Format, SampleCount, pNumQualityLevels);
		}

		STDMETHODIMP_(void) CheckCounterInfo(_Out_  D3D11_COUNTER_INFO* pCounterInfo)
		{
			return _real->CheckCounterInfo(pCounterInfo);
		}

		STDMETHODIMP CheckCounter(_In_  const D3D11_COUNTER_DESC* pDesc, _Out_  D3D11_COUNTER_TYPE* pType,  _Out_  UINT* pActiveCounters, _Out_writes_opt_(*pNameLength)  LPSTR szName, _Inout_opt_  UINT* pNameLength, _Out_writes_opt_(*pUnitsLength)  LPSTR szUnits, _Inout_opt_  UINT* pUnitsLength, _Out_writes_opt_(*pDescriptionLength)  LPSTR szDescription, _Inout_opt_  UINT* pDescriptionLength)
		{
			return _real->CheckCounter(pDesc, pType, pActiveCounters, szName, pNameLength, szUnits, pUnitsLength, szDescription, pDescriptionLength);
		}

		STDMETHODIMP CheckFeatureSupport(D3D11_FEATURE Feature, _Out_writes_bytes_(FeatureSupportDataSize)  void* pFeatureSupportData, UINT FeatureSupportDataSize)
		{
			return _real->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
		}

		STDMETHODIMP GetPrivateData(_In_  REFGUID guid, _Inout_  UINT* pDataSize, _Out_writes_bytes_opt_(*pDataSize)  void* pData)
		{
			return _real->GetPrivateData(guid, pDataSize, pData);
		}

		STDMETHODIMP SetPrivateData(_In_  REFGUID guid, _In_  UINT DataSize, _In_reads_bytes_opt_(DataSize)  const void* pData)
		{
			return _real->SetPrivateData(guid, DataSize, pData);
		}

		STDMETHODIMP SetPrivateDataInterface(_In_  REFGUID guid, _In_opt_  const IUnknown* pData)
		{
			return _real->SetPrivateDataInterface(guid, pData);
		}

		STDMETHODIMP_(D3D_FEATURE_LEVEL) GetFeatureLevel(void)
		{
			return _real->GetFeatureLevel();
		}

		STDMETHODIMP_(UINT) GetCreationFlags(void)
		{
			return _real->GetCreationFlags();
		}

		STDMETHODIMP GetDeviceRemovedReason(void)
		{
			return _real->GetDeviceRemovedReason();
		}

		STDMETHODIMP_(void) GetImmediateContext(_Out_  ID3D11DeviceContext** ppImmediateContext)
		{
			return _real->GetImmediateContext(ppImmediateContext);
		}

		STDMETHODIMP SetExceptionMode(UINT RaiseFlags)
		{
			return _real->SetExceptionMode(RaiseFlags);
		}

		STDMETHODIMP_(UINT) GetExceptionMode(void)
		{
			return _real->GetExceptionMode();
		}
	};
};
