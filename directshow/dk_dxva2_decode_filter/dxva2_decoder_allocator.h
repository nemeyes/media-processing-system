
class dxva2_decoder_allocator : public CBaseAllocator
{
public:
	dxva2_decoder_allocator(IDirectXVideoDecoderService * dxva2_decoder_service, DWORD width, DWORD height, DWORD format, HRESULT * hr);
	virtual ~dxva2_decoder_allocator(void);

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	HRESULT Alloc(void);
	void Free(void);

private:
	template <class T> void safe_release(T ** ppT);

private:
	IDirectXVideoDecoderService * _dxva2_decoder_service;
	DWORD _width;
	DWORD _height;
	DWORD _format;
	IDirect3DSurface9 ** _rt_surface_array;
	UINT _count_surface_array;
};

