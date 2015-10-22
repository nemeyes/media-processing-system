class dk_dxva2_sample : public CMediaSample, public IMFGetService
{
	friend class dk_dxva2_allocator;
public:

	dk_dxva2_sample(dk_dxva2_allocator * alloc, HRESULT * hr);

	// Note: CMediaSample does not derive from CUnknown, so we cannot use the
	//       DECLARE_IUNKNOWN macro that is used by most of the filter classes.
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFGetService::GetService
	STDMETHODIMP GetService(REFGUID guidService, REFIID riid, LPVOID *ppv);

	// Override GetPointer because this class does not manage a system memory buffer.
	// The EVR uses the MR_BUFFER_SERVICE service to get the Direct3D surface.
	STDMETHODIMP GetPointer(BYTE ** ppBuffer);

private:
	// Sets the pointer to the Direct3D surface. 
	void set_surface(DWORD surface_id, IDirect3DSurface9 * d3d9_surface);

	template <class T> void safe_release(T ** ppT);

	IDirect3DSurface9 * _d3d9_surface;
	DWORD _surface_id;
};