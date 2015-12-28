#include "stdafx.h"
#include "dkIDirect3DDevice9Ex.h"
#include <sstream>


dkIDirect3DDevice9Ex::dkIDirect3DDevice9Ex(IDirect3DDevice9Ex* pDevice, IDirect3D9Ex* pCreatedBy)
{
	m_pD3DDevice9Ex = pDevice;
	m_pMyD3D9Ex = pCreatedBy;

	m_nRefCount = 1;
}


dkIDirect3DDevice9Ex::~dkIDirect3DDevice9Ex(void)
{
	if (m_pD3DDevice9Ex)
		m_pD3DDevice9Ex->Release();
}



/**
* Base QueryInterface functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::QueryInterface(REFIID riid, LPVOID* ppv)
{
	return m_pD3DDevice9Ex->QueryInterface(riid, ppv);

}

/**
* Base AddRef functionality.
***/
ULONG WINAPI dkIDirect3DDevice9Ex::AddRef()
{
	++m_nRefCount;
	return m_pD3DDevice9Ex->AddRef();
}

/**
* Base Release functionality.
***/
ULONG WINAPI dkIDirect3DDevice9Ex::Release()
{
	if (--m_nRefCount == 0)
	{
		delete this;
		return 0;
	}

	return m_pD3DDevice9Ex->Release();
}

/**
* Base TestCooperativeLevel functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::TestCooperativeLevel()
{
	return m_pD3DDevice9Ex->TestCooperativeLevel();
}

/**
* Base GetAvailableTextureMem functionality.
***/
UINT WINAPI dkIDirect3DDevice9Ex::GetAvailableTextureMem()
{
	return m_pD3DDevice9Ex->GetAvailableTextureMem();
}

/**
* Base EvictManagedResources functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::EvictManagedResources()
{
	return m_pD3DDevice9Ex->EvictManagedResources();
}

/**
* Base GetDirect3D functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetDirect3D(IDirect3D9** ppD3D9)
{
	if (!m_pMyD3D9Ex)
		return D3DERR_INVALIDCALL;
	else {
		*ppD3D9 = m_pMyD3D9Ex;
		m_pMyD3D9Ex->AddRef();
		return D3D_OK;
	}
}

/**
* Base GetDeviceCaps functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetDeviceCaps(D3DCAPS9* pCaps)
{
	return m_pD3DDevice9Ex->GetDeviceCaps(pCaps);
}

/**
* Base GetDisplayMode functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	return m_pD3DDevice9Ex->GetDisplayMode(iSwapChain, pMode);
}

/**
* Base GetCreationParameters functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return m_pD3DDevice9Ex->GetCreationParameters(pParameters);
}

/**
* Base SetCursorProperties functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	return m_pD3DDevice9Ex->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

/**
* Base SetCursorPosition functionality.
***/
void WINAPI dkIDirect3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags)
{
	return m_pD3DDevice9Ex->SetCursorPosition(X, Y, Flags);
}

/**
* Base ShowCursor functionality.
***/
BOOL WINAPI dkIDirect3DDevice9Ex::ShowCursor(BOOL bShow)
{
	return m_pD3DDevice9Ex->ShowCursor(bShow);
}

/**
* Base CreateAdditionalSwapChain functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{
	return m_pD3DDevice9Ex->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

/**
* Base GetSwapChain functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	return m_pD3DDevice9Ex->GetSwapChain(iSwapChain, pSwapChain);
}

/**
* Base GetNumberOfSwapChains functionality.
***/
UINT WINAPI dkIDirect3DDevice9Ex::GetNumberOfSwapChains()
{
	return m_pD3DDevice9Ex->GetNumberOfSwapChains();
}

/**
* Base Reset functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	return m_pD3DDevice9Ex->Reset(pPresentationParameters);
}

/**
* Base Present functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
#if 1
	return m_pD3DDevice9Ex->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
#else
	this->ShowWeAreHere();

	// call original routine
	HRESULT hres = m_pD3DDevice9Ex->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	return (hres);
#endif
}

/**
* Base GetBackBuffer functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	return m_pD3DDevice9Ex->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

/**
* Base GetRasterStatus functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	return m_pD3DDevice9Ex->GetRasterStatus(iSwapChain, pRasterStatus);
}

/**
* Base SetDiaBoxMode functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetDialogBoxMode(BOOL bEnableDialogs)
{
	return m_pD3DDevice9Ex->SetDialogBoxMode(bEnableDialogs);
}

/**
* Base SetGammaRamp functionality.
***/
void WINAPI dkIDirect3DDevice9Ex::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	return m_pD3DDevice9Ex->SetGammaRamp(iSwapChain, Flags, pRamp);
}

/**
* Base GetGammaRamp functionality.
***/
void WINAPI dkIDirect3DDevice9Ex::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	return m_pD3DDevice9Ex->GetGammaRamp(iSwapChain, pRamp);
}

/**
* Base CreateTexture functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	return m_pD3DDevice9Ex->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

/**
* Base CreateVolumeTexture functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	return m_pD3DDevice9Ex->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

/**
* Base CreateCubeTexture functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	return m_pD3DDevice9Ex->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

/**
* Base CreateVertexBuffer functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	return m_pD3DDevice9Ex->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

/**
* Base CreateIndexBuffer functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	return m_pD3DDevice9Ex->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

/**
* Base CreateRenderTarget functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return m_pD3DDevice9Ex->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

/**
* Base CreateDepthStencilSurface functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return m_pD3DDevice9Ex->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

/**
* Base UpdateSurface functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	return m_pD3DDevice9Ex->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

/**
* Base UpdateTexture functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
{
	return m_pD3DDevice9Ex->UpdateTexture(pSourceTexture, pDestinationTexture);
}

/**
* Base GetRenderTargetData functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	return m_pD3DDevice9Ex->GetRenderTargetData(pRenderTarget, pDestSurface);
}

/**
* Base GetFrontBufferData functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	return m_pD3DDevice9Ex->GetFrontBufferData(iSwapChain, pDestSurface);
}

/**
* Base StretchRect functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	return m_pD3DDevice9Ex->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

/**
* Base ColorFill functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	return m_pD3DDevice9Ex->ColorFill(pSurface, pRect, color);
}

/**
* Base CreateOffscreenPlainSurface functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return m_pD3DDevice9Ex->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

/**
* Base SetRenderTarget functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	return m_pD3DDevice9Ex->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

/**
* Base GetRenderTarget functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	return m_pD3DDevice9Ex->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

/**
* Base SetDepthStencilSurface functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	return m_pD3DDevice9Ex->SetDepthStencilSurface(pNewZStencil);
}

/**
* Base GetDepthStencilSurface functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	return m_pD3DDevice9Ex->GetDepthStencilSurface(ppZStencilSurface);
}

/**
* Base BeginScene functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::BeginScene()
{
	return m_pD3DDevice9Ex->BeginScene();
}

/**
* Base EndScene functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::EndScene()
{
	return m_pD3DDevice9Ex->EndScene();
}

/**
* Base Clear functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return m_pD3DDevice9Ex->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

/**
* Base SetTransform functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	return m_pD3DDevice9Ex->SetTransform(State, pMatrix);
}

/**
* Base GetTransform functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
	return m_pD3DDevice9Ex->GetTransform(State, pMatrix);
}

/**
* Base MultiplyTransform functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	return m_pD3DDevice9Ex->MultiplyTransform(State, pMatrix);
}

/**
* Base SetViewport functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	return m_pD3DDevice9Ex->SetViewport(pViewport);
}

/**
* Base GetViewport functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetViewport(D3DVIEWPORT9* pViewport)
{
	return m_pD3DDevice9Ex->GetViewport(pViewport);
}

/**
* Base SetMaterial functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	return m_pD3DDevice9Ex->SetMaterial(pMaterial);
}

/**
* Base GetMaterial functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetMaterial(D3DMATERIAL9* pMaterial)
{
	return m_pD3DDevice9Ex->GetMaterial(pMaterial);
}

/**
* Base SetLight functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{
	return m_pD3DDevice9Ex->SetLight(Index, pLight);
}

/**
* Base GetLight functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9* pLight)
{
	return m_pD3DDevice9Ex->GetLight(Index, pLight);
}

/**
* Base LightEnable functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::LightEnable(DWORD Index, BOOL Enable)
{
	return m_pD3DDevice9Ex->LightEnable(Index, Enable);
}

/**
* Base GetLightEnable functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetLightEnable(DWORD Index, BOOL* pEnable)
{
	return m_pD3DDevice9Ex->GetLightEnable(Index, pEnable);
}

/**
* Base SetClipPlane functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	return m_pD3DDevice9Ex->SetClipPlane(Index, pPlane);
}

/**
* Base GetClipPlane functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetClipPlane(DWORD Index, float* pPlane)
{
	return m_pD3DDevice9Ex->GetClipPlane(Index, pPlane);
}

/**
* Base SetRenderState functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	return m_pD3DDevice9Ex->SetRenderState(State, Value);
}

/**
* Base GetRenderState functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
	return m_pD3DDevice9Ex->GetRenderState(State, pValue);
}

/**
* Base CreateStateBlock functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	return m_pD3DDevice9Ex->CreateStateBlock(Type, ppSB);
}

/**
* Base BeginStateBlock functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::BeginStateBlock()
{
	return m_pD3DDevice9Ex->BeginStateBlock();
}

/**
* Base EndStateBlock functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	return m_pD3DDevice9Ex->EndStateBlock(ppSB);
}

/**
* Base SetClipStatus functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
	return m_pD3DDevice9Ex->SetClipStatus(pClipStatus);
}

/**
* Base GetClipStatus functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	return m_pD3DDevice9Ex->GetClipStatus(pClipStatus);
}

/**
* Base GetTexture functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
	return m_pD3DDevice9Ex->GetTexture(Stage, ppTexture);
}

/**
* Base SetTexture functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	return m_pD3DDevice9Ex->SetTexture(Stage, pTexture);
}

/**
* Base GetTextureStageState functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	return m_pD3DDevice9Ex->GetTextureStageState(Stage, Type, pValue);
}

/**
* Base SetTextureStageState functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return m_pD3DDevice9Ex->SetTextureStageState(Stage, Type, Value);
}

/**
* Base GetSamplerState functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	return m_pD3DDevice9Ex->GetSamplerState(Sampler, Type, pValue);
}

/**
* Base SetSamplerState functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return m_pD3DDevice9Ex->SetSamplerState(Sampler, Type, Value);
}

/**
* Base ValidateDevice functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::ValidateDevice(DWORD* pNumPasses)
{
	return m_pD3DDevice9Ex->ValidateDevice(pNumPasses);
}

/**
* Base SetPaletteEntries functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{
	return m_pD3DDevice9Ex->SetPaletteEntries(PaletteNumber, pEntries);
}

/**
* Base GetPaletteEntries functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	return m_pD3DDevice9Ex->GetPaletteEntries(PaletteNumber, pEntries);
}

/**
* Base SetCurrentTexturePalette functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return m_pD3DDevice9Ex->SetCurrentTexturePalette(PaletteNumber);
}

/**
* Base GetCurrentTexturePalette functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	return m_pD3DDevice9Ex->GetCurrentTexturePalette(PaletteNumber);
}

/**
* Base SetScissorRect functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetScissorRect(CONST RECT* pRect)
{
	return m_pD3DDevice9Ex->SetScissorRect(pRect);
}

/**
* Base GetScissorRect functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetScissorRect(RECT* pRect)
{
	return m_pD3DDevice9Ex->GetScissorRect(pRect);
}

/**
* Base SetSoftwareVertexProcessing functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	return m_pD3DDevice9Ex->SetSoftwareVertexProcessing(bSoftware);
}

/**
* Base GetSoftwareVertexProcessing functionality.
***/
BOOL WINAPI dkIDirect3DDevice9Ex::GetSoftwareVertexProcessing()
{
	return m_pD3DDevice9Ex->GetSoftwareVertexProcessing();
}

/**
* Base SetNPatchMode functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetNPatchMode(float nSegments)
{
	return m_pD3DDevice9Ex->SetNPatchMode(nSegments);
}

/**
* Base GetNPatchMode functionality.
***/
float WINAPI dkIDirect3DDevice9Ex::GetNPatchMode()
{
	return m_pD3DDevice9Ex->GetNPatchMode();
}

/**
* Base DrawPrimitive functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return m_pD3DDevice9Ex->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

/**
* Base DrawIndexedPrimitive functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	return m_pD3DDevice9Ex->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

/**
* Base DrawPrimitiveUP functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return m_pD3DDevice9Ex->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

/**
* Base DrawIndexedPrimitiveUP functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return m_pD3DDevice9Ex->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

/**
* Base ProcessVertices functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	return m_pD3DDevice9Ex->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

/**
* Base CreateVertexDeclaration functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	return m_pD3DDevice9Ex->CreateVertexDeclaration(pVertexElements, ppDecl);
}

/**
* Base SetVertexDeclaration functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	return m_pD3DDevice9Ex->SetVertexDeclaration(pDecl);
}

/**
* Base GetVertexDeclaration functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	return m_pD3DDevice9Ex->GetVertexDeclaration(ppDecl);
}

/**
* Base SetFVF functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetFVF(DWORD FVF)
{
	return m_pD3DDevice9Ex->SetFVF(FVF);
}

/**
* Base GetFVF functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetFVF(DWORD* pFVF)
{
	return m_pD3DDevice9Ex->GetFVF(pFVF);
}

/**
* Base CreateVertexShader functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	return m_pD3DDevice9Ex->CreateVertexShader(pFunction, ppShader);
}

/**
* Base SetVertexShader functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	return m_pD3DDevice9Ex->SetVertexShader(pShader);
}

/**
* Base GetVertexShader functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	return m_pD3DDevice9Ex->GetVertexShader(ppShader);
}

/**
* Base SetVertexShaderConstantF functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return m_pD3DDevice9Ex->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

/**
* Base GetVertexShaderConstantF functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return m_pD3DDevice9Ex->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

/**
* Base SetVertexShaderConstantI functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return m_pD3DDevice9Ex->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

/**
* Base GetVertexShaderConstantI functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return m_pD3DDevice9Ex->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

/**
* Base SetVertexShaderConstantB functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return m_pD3DDevice9Ex->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

/**
* Base GetVertexShaderConstantB functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return m_pD3DDevice9Ex->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

/**
* Base SetStreamSource functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	return m_pD3DDevice9Ex->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

/**
* Base GetStreamSource functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride)
{
	return m_pD3DDevice9Ex->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

/**
* Base SetStreamSourceFreq functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetStreamSourceFreq(UINT StreamNumber, UINT Setting)
{
	return m_pD3DDevice9Ex->SetStreamSourceFreq(StreamNumber, Setting);
}

/**
* Base GetStreamSourceFreq functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting)
{
	return m_pD3DDevice9Ex->GetStreamSourceFreq(StreamNumber, pSetting);
}

/**
* Base SetIndices functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	return m_pD3DDevice9Ex->SetIndices(pIndexData);
}

/**
* Base GetIndices functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	return m_pD3DDevice9Ex->GetIndices(ppIndexData);
}

/**
* Base CreatePixelShader functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	return m_pD3DDevice9Ex->CreatePixelShader(pFunction, ppShader);
}

/**
* Base SetPixelShader functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	return m_pD3DDevice9Ex->SetPixelShader(pShader);
}

/**
* Base GetPixelShader functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	return m_pD3DDevice9Ex->GetPixelShader(ppShader);
}

/**
* Base SetPixelShaderConstantF functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return m_pD3DDevice9Ex->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

/**
* Base GetPixelShaderConstantF functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return m_pD3DDevice9Ex->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

/**
* Base SetPixelShaderConstantI functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return m_pD3DDevice9Ex->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

/**
* Base GetPixelShaderConstantI functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return m_pD3DDevice9Ex->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

/**
* Base SetPixelShaderConstantB functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return m_pD3DDevice9Ex->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

/**
* Base GetPixelShaderConstantB functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return m_pD3DDevice9Ex->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

/**
* Base DrawRectPatch functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	return m_pD3DDevice9Ex->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

/**
* Base DrawTriPatch functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	return m_pD3DDevice9Ex->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

/**
* Base DeletePatch functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::DeletePatch(UINT Handle)
{
	return m_pD3DDevice9Ex->DeletePatch(Handle);
}

/**
* Base CreateQuery functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	return m_pD3DDevice9Ex->CreateQuery(Type, ppQuery);
}

/**
* Base SetConvolutionMonoKernel functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetConvolutionMonoKernel(UINT width, UINT height, float* rows, float* columns)
{
	return m_pD3DDevice9Ex->SetConvolutionMonoKernel(width, height, rows, columns);
}

/**
* Base ComposeRects functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::ComposeRects(IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset)
{
	return m_pD3DDevice9Ex->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
}

/**
* Base PresentEx functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::PresentEx(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
	return m_pD3DDevice9Ex->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

/**
* Base GetGPUThreadPriority functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetGPUThreadPriority(INT* pPriority)
{
	return m_pD3DDevice9Ex->GetGPUThreadPriority(pPriority);
}

/**
* Base SetGPUThreadPriority functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetGPUThreadPriority(INT Priority)
{
	return m_pD3DDevice9Ex->SetGPUThreadPriority(Priority);
}

/**
* Base WaitForVBlank functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::WaitForVBlank(UINT iSwapChain)
{
	return m_pD3DDevice9Ex->WaitForVBlank(iSwapChain);
}

/**
* Base CheckResourceResidency functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CheckResourceResidency(IDirect3DResource9** pResourceArray, UINT32 NumResources)
{
	return m_pD3DDevice9Ex->CheckResourceResidency(pResourceArray, NumResources);
}

/**
* Base SetMaximumFrameLatency functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::SetMaximumFrameLatency(UINT MaxLatency)
{
	return m_pD3DDevice9Ex->SetMaximumFrameLatency(MaxLatency);
}

/**
* Base GetMaximumFrameLatency functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetMaximumFrameLatency(UINT* pMaxLatency)
{
	return m_pD3DDevice9Ex->GetMaximumFrameLatency(pMaxLatency);
}

/**
* Base CheckDeviceState functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CheckDeviceState(HWND hDestinationWindow)
{
	return m_pD3DDevice9Ex->CheckDeviceState(hDestinationWindow);
}

/**
* Base CreateRenderTargetEx functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	return m_pD3DDevice9Ex->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
}

/**
* Base CreateOffscreenPlainSurfaceEx functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	return m_pD3DDevice9Ex->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
}

/**
* Base CreateDepthStencilSurfaceEx functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	return m_pD3DDevice9Ex->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);
}

/**
* Base ResetEx functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::ResetEx(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
	return m_pD3DDevice9Ex->ResetEx(pPresentationParameters, pFullscreenDisplayMode);
}

/**
* Base GetDisplayModeEx functionality.
***/
HRESULT WINAPI dkIDirect3DDevice9Ex::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
{
	return m_pD3DDevice9Ex->GetDisplayModeEx(iSwapChain, pMode, pRotation);
}

/**
* Returns the actual embedded device pointer.
***/
IDirect3DDevice9Ex* dkIDirect3DDevice9Ex::getActual()
{
	return m_pD3DDevice9Ex;
}

// This is our test function
void dkIDirect3DDevice9Ex::ShowWeAreHere(void)
{
	D3DRECT rec = { 1, 1, 50, 50 };
	m_pD3DDevice9Ex->Clear(1, &rec, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 255, 0), 0, 0);
}

