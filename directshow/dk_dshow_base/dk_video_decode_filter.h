#pragma once
#include <d3d9.h>
#include <dxva2api.h>
#include <mfidl.h>	
#include <evr.h>
#include "dk_dxva2_allocator.h"

class dk_video_decode_filter : public CTransformFilter
{
	friend class dk_dxva2_allocator;
public:
	dk_video_decode_filter(TCHAR * name, LPUNKNOWN unk, REFCLSID clsid, HRESULT * hr);
	virtual ~dk_video_decode_filter(void);



	//HRESULT Receive(IMediaSample* pIn);
	virtual HRESULT Transform(IMediaSample* pIn) = 0;

	STDMETHODIMP init_allocator(IMemAllocator ** allocator);

	HRESULT initialize_d3d(void);
	void set_dxva2_decoder_service(IDirectXVideoDecoderService * service) { _dxva2_decoder_service = service; }
	void set_width(DWORD width) { _width = width; }
	void set_height(DWORD height) { _height = height; }
	void set_format(DWORD format) { _format = _format; }

protected:
	IDirectXVideoDecoderService * _dxva2_decoder_service;
	DWORD _stride;
	DWORD _width;
	DWORD _height;
	DWORD _format;
	dk_dxva2_allocator * _dxva2_allocator;


	IDirect3D9 * _d3d;
	IDirect3DDevice9 * _d3d_device;
	IDirect3DDeviceManager9 * _d3d_device_manager;
	UINT _d3d_reset_token;

};