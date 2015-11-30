/*******************************************************************************
Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1              Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer.
2              Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the 
                documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
/** 
***************************************************************
* @file <Transform.cpp> 
*   
* @brief This file contains implementation of CTransform class
*
***************************************************************
*/
#include <amp.h>
#include <amp_graphics.h>
#include <amp_math.h>

#include "CopyRGshader.h"
#include "CopyRshader.h"
#include "Common.h"

#include "Transform.h"

using namespace concurrency;
using namespace concurrency::fast_math;
using namespace concurrency::direct3d;
using namespace concurrency::graphics;
using namespace concurrency::graphics::direct3d;

/** 
 *******************************************************************************
 *  @fn    CTransform
 *  @brief Constructor
 *******************************************************************************
 */
CTransform::CTransform():
    mIsInited(false)
{
}

/** 
 *******************************************************************************
 *  @fn     initComputeShaders
 *  @brief  Initializes shaders 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTransform::initComputeShaders()
{
    mCopyRGshader = nullptr;
    HRESULT hr = mDevice->CreateComputeShader(CopyRGshader, ARRAYSIZE(CopyRGshader), nullptr, &mCopyRGshader);
    if (FAILED(hr)) return hr;

    mCopyRshader = nullptr;
    hr = mDevice->CreateComputeShader(CopyRshader, ARRAYSIZE(CopyRshader), nullptr, &mCopyRshader);
    if (FAILED(hr)) return hr;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     init
 *  @brief  Initialized resizer class object 
 *  @param[in] deviceManager          : Pointer to DXGI 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTransform::init(CComPtr<IMFDXGIDeviceManager> deviceManager)
{
    HRESULT hr = S_OK;

    mDeviceManager = deviceManager;
    if (FAILED(hr)) return hr;
    
    HANDLE deviceHandle;
    hr = mDeviceManager->OpenDeviceHandle(&deviceHandle);
    if (FAILED(hr)) return hr;

    mDevice = nullptr;
    hr = mDeviceManager->GetVideoService(deviceHandle, IID_PPV_ARGS( &mDevice ));
    if (FAILED(hr)) return hr;

    hr = initComputeShaders();
    if (FAILED(hr)) return hr;

    mIsInited = true;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     cleanup
 *  @brief  Releases DXGI objects 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTransform::cleanup()
{
    mIsInited = false;
    return S_OK;
}

 /** 
 *******************************************************************************
 *  @fn     applyShader
 *  @brief  Applies shader
 *  @param[in] pInText        : Pointer to input Texture 
 *  @param[in] inFormat       : Format of input texture 
 *  @param[out] pOutText      : Pointer to output Texture
 *  @param[in] outFormat      : Format of output Texture
 *  @param[in] pShader        : Pointer to shader
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT applyShader( ID3D11Texture2D* pInText, DXGI_FORMAT inFormat, ID3D11Texture2D* pOutText, DXGI_FORMAT outFormat, ID3D11ComputeShader* pShader)
{
    CComPtr<ID3D11Device> ptrDevice;
    pOutText->GetDevice(&ptrDevice);

    CComPtr<ID3D11DeviceContext> spDeviceContext;
    ptrDevice->GetImmediateContext(&spDeviceContext);

	/***************************************************************************
    * In SRV                                                                   *
    ***************************************************************************/
    CComPtr<ID3D11ShaderResourceView> srv;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    ZeroMemory( &srvDesc, sizeof(srvDesc) );
    srvDesc.Format = inFormat;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    HRESULT hr = ptrDevice->CreateShaderResourceView( pInText, &srvDesc, &srv ); 
    if (FAILED(hr)) return hr;
	/***************************************************************************
    * out UAV                                                                  *
    ***************************************************************************/
    CComPtr<ID3D11UnorderedAccessView> uav;
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;

    ZeroMemory( &uavDesc, sizeof(uavDesc) );
    uavDesc.Format = outFormat;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    hr = ptrDevice->CreateUnorderedAccessView( pOutText, &uavDesc, &uav ); 
    if (FAILED(hr)) return hr;
    
    spDeviceContext->CSSetShader( pShader, nullptr, 0 );

    ID3D11ShaderResourceView* aSRViews[] = {srv};
    spDeviceContext->CSSetShaderResources( 0, ARRAYSIZE(aSRViews), aSRViews );

    ID3D11UnorderedAccessView* aUAViews[] = {uav};
    spDeviceContext->CSSetUnorderedAccessViews( 0, ARRAYSIZE(aUAViews), aUAViews, nullptr );

    D3D11_TEXTURE2D_DESC outTextDesc;
    pOutText->GetDesc( &outTextDesc );

    spDeviceContext->Dispatch( outTextDesc.Width/2, outTextDesc.Height/2, 1 );

	/***************************************************************************
    * inbinding resources                                                      *
    ***************************************************************************/
    spDeviceContext->CSSetShader( nullptr, nullptr, 0 );

    ID3D11ShaderResourceView* aSRNullViews[] = {nullptr};
    spDeviceContext->CSSetShaderResources( 0, 1, aSRNullViews );
    
    ID3D11UnorderedAccessView* aUANullViews[] = {nullptr};
    spDeviceContext->CSSetUnorderedAccessViews( 0, 1, aUANullViews, nullptr );

    return S_OK;
}

 /** 
 *******************************************************************************
 *  @fn     createNv12Planes
 *  @brief  Gets NV12 planes from texture
 *  @param[in] nv12Texture             : Pointer to NV12 Texture 
 *  @param[out] luminanceAsTexture     : Pointer to luma Texture 
 *  @param[out] chrominanceAsTexture   : Pointer to chroma Texture
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTransform::createNv12Planes(ID3D11Texture2D* nv12Texture, ID3D11Texture2D** luminanceAsTexture, ID3D11Texture2D** chrominanceAsTexture )
{
    D3D11_TEXTURE2D_DESC nv12TextureDesc;
    nv12Texture->GetDesc(&nv12TextureDesc);

    CComPtr<ID3D11Texture2D> luminanceTarget;
    HRESULT hr = createTexture(nv12TextureDesc.Width, nv12TextureDesc.Height,
        D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, DXGI_FORMAT_R8_UNORM, mDevice, &luminanceTarget); 
    if (FAILED(hr)) return hr;

    hr = applyShader(nv12Texture, DXGI_FORMAT_R8_UNORM, luminanceTarget, DXGI_FORMAT_R8_UNORM, mCopyRshader);
    if (FAILED(hr)) return hr;

    CComPtr<ID3D11Texture2D> chrominanceTarget;
    hr = createTexture(nv12TextureDesc.Width / 2, nv12TextureDesc.Height / 2,
        D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, DXGI_FORMAT_R8G8_UNORM, mDevice, &chrominanceTarget); 
    if (FAILED(hr)) return hr;

    hr = applyShader(nv12Texture, DXGI_FORMAT_R8G8_UNORM, chrominanceTarget, DXGI_FORMAT_R8G8_UNORM, mCopyRGshader);
    if (FAILED(hr)) return hr;

    *luminanceAsTexture = luminanceTarget.Detach();
    *chrominanceAsTexture = chrominanceTarget.Detach();

    return S_OK;
}

 /** 
 *******************************************************************************
 *  @fn     mergeYandUvplanesToNv12
 *  @brief  Merges luma and chroma planes to get NV12 texture
 *  @param[in] pInY             : Pointer to luma Texture 
 *  @param[in] pInUV            : Pointer to UV Texture 
 *  @param[out] pOutText        : Pointer to output Texture
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT mergeYandUvplanesToNv12(ID3D11Texture2D* pInY, ID3D11Texture2D* pInUV, 
                                ID3D11Texture2D* pOutText, 
                                ID3D11ComputeShader* shaderY,
                                ID3D11ComputeShader* shaderUV)
{
    HRESULT hr = applyShader( pInY, DXGI_FORMAT_R8_UNORM, pOutText, DXGI_FORMAT_R8_UNORM, shaderY );
    if (FAILED(hr)) return hr;

    hr = applyShader( pInUV, DXGI_FORMAT_R8G8_UNORM, pOutText, DXGI_FORMAT_R8G8_UNORM, shaderUV );
    if (FAILED(hr)) return hr;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     process
 *  @brief  Scales input texture to output 
 *  @param[in] d3d11Texture2dIn           : Input texture 
 *  @param[in] srcViewIndex               : View Index
 *  @param[out] d3d11Texture2dOut         : Output texture
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTransform::process(ID3D11Texture2D* d3d11Texture2dIn, 
                            UINT srcViewIndex,
                            ID3D11Texture2D* d3d11Texture2dOut)
{
    if (!mIsInited)
    {
        return E_FAIL;
    }

    if (!mDevice) 
    {
        return E_FAIL;
    }
    
    /***************************************************************************
    * It is necessary to lock D3D11 device before using                        *
    ***************************************************************************/
    LockDevice lockDevice = LockDevice(mDeviceManager);
    lockDevice.Lock();
   
    D3D11_TEXTURE2D_DESC srcDesc;
    d3d11Texture2dIn->GetDesc(&srcDesc);

    CComPtr<ID3D11Texture2D> nv12text;
    HRESULT hr = createTexture(srcDesc.Width, srcDesc.Height, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, DXGI_FORMAT_NV12, mDevice, &nv12text);
    if (FAILED(hr)) return hr;

    hr = convertTextureFormat(nv12text, d3d11Texture2dIn, srcViewIndex);
    if (FAILED(hr)) return hr;
    
    D3D11_TEXTURE2D_DESC dstDesc;
    d3d11Texture2dOut->GetDesc(&dstDesc);
    
    /***************************************************************************
    * AMP does not have interop between NV12 format texture and internal AMP   *
    * texture. Interop between AMP texture and NV12 DX11 texture based on      *
    * conversion NV12 texture to R8 and R8G8 DX11 texture. We use these two    *
    * textures for creating internal AMP textures.R8 and R8G8 texture represent* 
    * Y and UV plane accordingly and processing by the AMP separately.         *
    ***************************************************************************/
    CComPtr<ID3D11Texture2D> lumaTex;
    CComPtr<ID3D11Texture2D> chromTex;
    hr = createNv12Planes(nv12text, &lumaTex, &chromTex );
    
    CComPtr<ID3D11Texture2D> spOutLuma;
    hr = createTexture( dstDesc.Width, dstDesc.Height, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,  DXGI_FORMAT_R8_UNORM, mDevice, &spOutLuma );
    if (FAILED(hr)) return hr;

    CComPtr<ID3D11Texture2D> spOutChroma;
    hr = createTexture( dstDesc.Width/2, dstDesc.Height/2, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,  DXGI_FORMAT_R8G8_UNORM, mDevice, &spOutChroma );
    if (FAILED(hr)) return hr;

    hr = doTransform(spOutLuma, spOutChroma, lumaTex, chromTex);
    if (FAILED(hr)) return hr;

    hr = mergeYandUvplanesToNv12(spOutLuma, spOutChroma, d3d11Texture2dOut, mCopyRshader, mCopyRGshader);
    if (FAILED(hr)) return hr;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     doTransform
 *  @brief  Scales input texture to output 
 *  @param[out] pOutLuminance       : Pointer to luma Texture 
 *  @param[out] pOutChrominance     : Pointer to chroma Texture 
 *  @param[in] pInLuminance         : Pointer to luma Texture
 *  @param[in] pInChrominance       : Pointer to chroma Texture
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */

 HRESULT CTransform::doTransform(ID3D11Texture2D* pOutLuminance, 
     ID3D11Texture2D* pOutChrominance, ID3D11Texture2D* pInLuminance,
     ID3D11Texture2D* pInChrominance)
{
    CComPtr<ID3D11DeviceContext> pDeviceContext;
    mDevice->GetImmediateContext(&pDeviceContext);

    accelerator_view accView = accelerator(accelerator::direct3d_warp).default_view;
    accView = create_accelerator_view(reinterpret_cast<IUnknown *>(mDevice.p));

    /**************************************************************************
    * allocate temporary texture for output                                   *
    **************************************************************************/
    texture<unorm, 2> outputLuma = make_texture<unorm, 2>(accView, pOutLuminance);
    writeonly_texture_view<unorm, 2> outputLumView(outputLuma);

    texture<unorm2, 2> outputChroma = make_texture<unorm2, 2>(accView, pOutChrominance);
    writeonly_texture_view<unorm2, 2> outputChromView(outputChroma);
    /**************************************************************************
    * map input texture                                                       *
    **************************************************************************/
    const texture<unorm, 2> luminanceTex = make_texture<unorm, 2>(accView, pInLuminance);
    const texture<unorm2, 2> chrominanceTex = make_texture<unorm2, 2>(accView, pInChrominance);

    D3D11_TEXTURE2D_DESC lumaDesc;
    pInLuminance->GetDesc(&lumaDesc);

    D3D11_TEXTURE2D_DESC dstLumaDesc;
    pOutLuminance->GetDesc(&dstLumaDesc);

    parallel_for_each(accView, outputChromView.extent, [=, &luminanceTex, &chrominanceTex] (index<2> idx) restrict(amp)
    {
       
        const int map[][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};
        const int mapSize = sizeof(map)/sizeof(map[0]);
        
        index<2> doubledIdx = idx*2;

        for (int i = 0; i < mapSize; ++i)
        {
             outputLumView.set( index<2>(doubledIdx[0] + map[i][0],
                                         doubledIdx[1] + map[i][1]), 
                                luminanceTex[index<2>( doubledIdx[0] + map[i][0],
                                doubledIdx[1] + map[i][1])] );
        }
        /**********************************************************************
        * Set UV to 128, to make the output gray scale                        *
        **********************************************************************/
        outputChromView.set(idx, unorm2(0.5f, 0.5f));
    });

    return S_OK;    
}
