/**********************************************************************
Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

#include "SimpleDX9.hpp"

/****************************************************************
***************global variable for D3D9 *************************
****************************************************************/
#define SAFE_RELEASE(p)             if (p) {(p)->Release(); p = NULL;}

typedef HRESULT (WINAPI *PFNDirect3DCreate9Ex)(UINT SDKVersion, IDirect3D9Ex**);

HWND hWnd;
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
WNDCLASSEX wc =
{
    sizeof(WNDCLASSEX),
    CS_CLASSDC,
    MsgProc,
    0L,
    0L,
    GetModuleHandle(NULL),
    NULL,
    NULL,
    NULL,
    NULL,
    reinterpret_cast<LPCSTR>(WINDOW_CLASS_NAME),
    NULL
};

PFNDirect3DCreate9Ex m_pfDirect3DCreate9Ex;

HMODULE m_hD3D9Dll;
IDirect3DDevice9Ex *m_pD3DDevice9 = NULL;//important
IDirect3DSurface9 *pOffScreenSur = NULL;
IDirect3DSurface9 *pBackBuffer = NULL;
IDirect3DTexture9 * d3dtex = NULL;
IDirect3DVertexBuffer9 * d3dvtc = NULL;

DWORD m_eCardType;
cl_uint gBackBufferWidth,gBackBufferHeight;
D3DFORMAT BackBufferFormat;

/* */
typedef struct
{
    FLOAT       x,y,z;      // vertex untransformed position
    FLOAT       rhw;        // eye distance
    D3DCOLOR    diffuse;    // diffuse color
    FLOAT       tu, tv;     // texture relative coordinates
} CUSTOMVERTEX;


//clock used to compute fpt
clock_t t1,t2;
int frameCount = 0;
int frameRefCount = 90;
double totalElapsedTime = 0.0;


/***********************************************************************
***********************************************************************/

int SimpleDX9::SetupD3D9()
{
    //load d3d9.dll
    HRESULT hr = S_OK;
    cl_int status;
    m_hD3D9Dll = LoadLibrary(TEXT("d3d9.dll"));
    if (m_hD3D9Dll)
    {
        m_pfDirect3DCreate9Ex = (PFNDirect3DCreate9Ex)GetProcAddress(m_hD3D9Dll,
                                "Direct3DCreate9Ex");
    }
    else
    {
        hr = S_FALSE;
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return hr;

    }

    // Get all adapters
    std::vector<IDXGIAdapter*> vAdapters;
    unsigned int numAdapters;
    IDXGIFactory* factory;
    CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    IDXGIAdapter * pAdapter = 0;
    UINT i = 0;
    while(factory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        vAdapters.push_back(pAdapter);
        ++i;
    }

    //Push NULL to vAdapters and assign zero to deviceId if adapter is not found
    if(i == 0 && sampleArgs->deviceId >= 0)
    {
        sampleArgs->deviceId = 0;
        vAdapters.push_back(NULL);
    }

    numAdapters = i;

    DISPLAY_DEVICE dispDevice;
    DWORD deviceNum;
    dispDevice.cb = sizeof(DISPLAY_DEVICE);

    DWORD displayDevices = 0;
    DWORD connectedDisplays = 0;

    int xCoordinate = 0;
    int yCoordinate = 0;
    int xCoordinate1 = 0;

	int num_adaptors_count = 0;
	int num_adaptors_test = numAdapters;

    //Used to store the number of OpenCL devices corresponding to the Direct3D 9 object
    cl_uint num = 0;

	HWND *hWndArray;

	hWndArray = new HWND [numAdapters] ;
	
	IDirect3DDevice9Ex **num_adaptors_ptr ;
	
	num_adaptors_ptr = new IDirect3DDevice9Ex * [ numAdapters] ;

	cl_uint * type;

	type = new cl_uint [numAdapters];

	for(unsigned int i = 0;i < numAdapters;i++)
		type[i] = CL_ADAPTER_D3D9EX_KHR;

    for (deviceNum = 0; EnumDisplayDevices(NULL, deviceNum, &dispDevice, 0);
            deviceNum++)
    {
        if (dispDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
        {
            continue;
        }

        if(!(dispDevice.StateFlags & DISPLAY_DEVICE_ACTIVE))
        {
            std::cout <<"Display device " << deviceNum << " is not connected!!" <<
                      std::endl;
            continue;
        }

        DEVMODE deviceMode;

        // initialize the DEVMODE structure
        ZeroMemory(&deviceMode, sizeof(deviceMode));
        deviceMode.dmSize = sizeof(deviceMode);
        deviceMode.dmDriverExtra = 0;

        EnumDisplaySettingsEx(dispDevice.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode,
                              EDS_ROTATEDMODE);

        xCoordinate = deviceMode.dmPosition.x;
        yCoordinate = deviceMode.dmPosition.y;

        // Register the window class
        RegisterClassEx(&wc);
        // Create the application's window
        hWnd = CreateWindow(reinterpret_cast<LPCSTR>(WINDOW_CLASS_NAME),
                            reinterpret_cast<LPCSTR>(CAPTION_NAME),
                            WS_CAPTION | WS_POPUPWINDOW,
                          	((xCoordinate == 0) &&
                            (sampleArgs->deviceId < numAdapters))? xCoordinate1 : xCoordinate,
                            yCoordinate,
                            g_dwImgWidth,
                            g_dwImgHeight,
                            NULL,
                            NULL,
                            wc.hInstance,
                            NULL);


        BOOL bWindowed = TRUE;

        IDirect3DDevice9Ex *pD3D9Dev = NULL;
        IDirect3D9Ex *pD3D9 = NULL;
        if (m_pfDirect3DCreate9Ex)
        {
            hr = m_pfDirect3DCreate9Ex(D3D_SDK_VERSION, &pD3D9);//create device D3d9
        }

        if (D3D_OK != hr)
        {
            printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
			delete num_adaptors_ptr;
			delete type ;
			delete hWndArray;
            return hr;
        }

        D3DDISPLAYMODE d3ddm;
        pD3D9->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

        D3DPRESENT_PARAMETERS d3dpp;
        ZeroMemory( &d3dpp, sizeof(d3dpp) );
        if (bWindowed)
        {
            d3dpp.BackBufferWidth  = g_dwImgWidth;
            d3dpp.BackBufferHeight = g_dwImgHeight;
        }
        else
        {
            d3dpp.BackBufferWidth  = GetSystemMetrics(SM_CXSCREEN);
            d3dpp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
        }
        gBackBufferWidth = d3dpp.BackBufferWidth;
        gBackBufferHeight = d3dpp.BackBufferHeight;
        BackBufferFormat = d3ddm.Format;

        d3dpp.BackBufferFormat = d3ddm.Format;//VIDEO_FORMAT_RENDER_TARGET;
        d3dpp.BackBufferCount = 1;
        d3dpp.Windowed = bWindowed;
        d3dpp.EnableAutoDepthStencil = FALSE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        d3dpp.Flags = D3DPRESENTFLAG_VIDEO;
        d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
        d3dpp.hDeviceWindow = hWnd;

        //
        // First try to create a hardware D3D9 device.
        //
        {
            hr = pD3D9->CreateDeviceEx(D3DADAPTER_DEFAULT,
                                       D3DDEVTYPE_HAL,
                                       hWnd,
                                       D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                       &d3dpp,
                                       NULL,
                                       &pD3D9Dev);
            if (S_OK != hr)
            {
                hr = pD3D9->CreateDeviceEx(D3DADAPTER_DEFAULT,
                                           D3DDEVTYPE_SW,
                                           hWnd,
                                           D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                           &d3dpp,
                                           NULL,
                                           &pD3D9Dev);
            }

        }

        if (S_OK == hr)
        {
            SAFE_RELEASE(pD3D9);
            m_pD3DDevice9 = pD3D9Dev;
        }
        else
        {
            SAFE_RELEASE(pD3D9);
            SAFE_RELEASE(pD3D9Dev);
            printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);

			delete num_adaptors_ptr;
			delete type ;
			delete hWndArray;

            return hr;
        }

       
		num_adaptors_ptr[num_adaptors_count] = m_pD3DDevice9;
		hWndArray[num_adaptors_count++] = hWnd;

		}

		 /**
         * get the number of devices can be used for the DX9 and OpenCL interOp
         */

		if(numAdapters > 1)
		{
			m_pD3DDevice9 = num_adaptors_ptr[sampleArgs->deviceId];
			hWnd = hWndArray[sampleArgs->deviceId];
			num_adaptors_ptr[0] = num_adaptors_ptr[sampleArgs->deviceId];
			numAdapters = 1;
		}
		
        status = pfn_clGetDeviceIDsFromDX9MediaAdapterKHR(platform, numAdapters,type,
                 (void *)num_adaptors_ptr,
                 CL_PREFERRED_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR, 1,&device, &num);



		if ( (status != CL_SUCCESS)  || (num==0))
        {

			for(int num_adaptors_count = 0 ;num_adaptors_count < num_adaptors_test;num_adaptors_count++)
			{
				DestroyWindow(hWndArray[num_adaptors_count]);
				
			}
			delete num_adaptors_ptr;
			delete type ;
			delete hWndArray;
			CHECK_OPENCL_ERROR(status, "clGetDeviceIDsFromDX9MediaAdapterKHR failed.");
		}


        std::cout<<"Number of OpenCL devices corresponding to the Direct3D 0 object  "<<num<<std::endl;
       

		


    devices = new cl_device_id[num];
    CHECK_ALLOCATION(devices,"Allocation failed(openCLInteropDevices)");

    /**
    * To get all the OpenCL devices corresponding to the Direct3D object.
    */

    status = pfn_clGetDeviceIDsFromDX9MediaAdapterKHR(platform,numAdapters,type,
             (void *)num_adaptors_ptr,
             CL_PREFERRED_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR, num,devices, NULL);

    CHECK_OPENCL_ERROR(status, "clGetDeviceIDsFromDX9MediaAdapterKHR failed.");


    device = devices[0];

    std::cout<<"Interoperable deviceID "<<device<<std::endl;


    //Create D3D9 offScreenSurface,the cl_mem will be created from this surface

    hr = m_pD3DDevice9->CreateOffscreenPlainSurface( g_dwImgWidth,g_dwImgHeight,
            BackBufferFormat,D3DPOOL_DEFAULT,(IDirect3DSurface9**)&pOffScreenSur, NULL);
    if(S_OK != hr)
        if(hr != S_OK)
        {
            printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
			delete num_adaptors_ptr;
			delete type ;
			delete hWndArray;
            return SDK_FAILURE;
        }

    return hr;

}

/**
* Name: MsgProc()
* Desc: The window's message handler
*/
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {

    case WM_DESTROY:
        D3D9Cleanup();
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        switch(wParam)
        {
        case 'q':
        case 'Q':
        case VK_ESCAPE:
            D3D9Cleanup();
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}



int CreateScence()
{
    HRESULT hr;

    /*
    * Create a texture for use when rendering a scene
    * for performance reason, texture format is identical to backbuffer
    * which would usually be a RGB format
    */
    hr = IDirect3DDevice9_CreateTexture(m_pD3DDevice9,
                                        gBackBufferWidth,
                                        gBackBufferHeight,
                                        1,
                                        D3DUSAGE_RENDERTARGET,
                                        BackBufferFormat,
                                        D3DPOOL_DEFAULT,
                                        &d3dtex,
                                        NULL);

    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    /*
    ** Create a vertex buffer for use when rendering scene
    */

    hr = IDirect3DDevice9_CreateVertexBuffer(m_pD3DDevice9,
            sizeof(CUSTOMVERTEX)*4,
            D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,
            D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1,
            D3DPOOL_DEFAULT,
            &d3dvtc,
            NULL);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    // Texture coordinates outside the range [0.0, 1.0] are set
    // to the texture color at 0.0 or 1.0, respectively.
    IDirect3DDevice9_SetSamplerState(m_pD3DDevice9, 0, D3DSAMP_ADDRESSU,
                                     D3DTADDRESS_CLAMP);
    IDirect3DDevice9_SetSamplerState(m_pD3DDevice9, 0, D3DSAMP_ADDRESSV,
                                     D3DTADDRESS_CLAMP);

    // Set linear filtering quality
    IDirect3DDevice9_SetSamplerState(m_pD3DDevice9, 0, D3DSAMP_MINFILTER,
                                     D3DTEXF_LINEAR);
    IDirect3DDevice9_SetSamplerState(m_pD3DDevice9, 0, D3DSAMP_MAGFILTER,
                                     D3DTEXF_LINEAR);

    // set maximum ambient light
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_AMBIENT, D3DCOLOR_XRGB(255,
                                    255,255));

    // Turn off culling
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_CULLMODE, D3DCULL_NONE);

    // Turn off the zbuffer
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_ZENABLE, D3DZB_FALSE);

    // Turn off lights
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_LIGHTING, FALSE);

    // Enable dithering
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_DITHERENABLE, TRUE);

    // disable stencil
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_STENCILENABLE, FALSE);

    // manage blending
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_ALPHABLENDENABLE, TRUE);
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_SRCBLEND,
                                    D3DBLEND_SRCALPHA);
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_DESTBLEND,
                                    D3DBLEND_INVSRCALPHA);
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_ALPHATESTENABLE,TRUE);
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_ALPHAREF, 0x10);
    IDirect3DDevice9_SetRenderState(m_pD3DDevice9, D3DRS_ALPHAFUNC,D3DCMP_GREATER);

    // Set texture states
    IDirect3DDevice9_SetTextureStageState(m_pD3DDevice9, 0, D3DTSS_COLOROP,
                                          D3DTOP_MODULATE);
    IDirect3DDevice9_SetTextureStageState(m_pD3DDevice9, 0, D3DTSS_COLORARG1,
                                          D3DTA_TEXTURE);
    IDirect3DDevice9_SetTextureStageState(m_pD3DDevice9, 0, D3DTSS_COLORARG2,
                                          D3DTA_DIFFUSE);

    // turn off alpha operation
    IDirect3DDevice9_SetTextureStageState(m_pD3DDevice9, 0, D3DTSS_ALPHAOP,
                                          D3DTOP_DISABLE);

    return SDK_SUCCESS;

}

int SimpleDX9::Direct3DRenderScene()
{

    HRESULT hr;

    // check if device is still available
    hr = IDirect3DDevice9_TestCooperativeLevel(m_pD3DDevice9);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }
    /* Clear the backbuffer and the zbuffer */
    hr = IDirect3DDevice9_Clear(m_pD3DDevice9, 0, NULL, D3DCLEAR_TARGET,
                                D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    IDirect3DSurface9 *d3dsrc = pOffScreenSur;
    if(!d3dsrc)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }
    /* retrieve texture top-level surface */
    IDirect3DSurface9 * d3ddest;
    hr = IDirect3DTexture9_GetSurfaceLevel(d3dtex, 0, &d3ddest);
    /* Copy picture surface into texture surface
    * color space conversion and scaling happen here */
    RECT src = {0,0,g_dwImgWidth,g_dwImgHeight};
    RECT dst = {0,0,g_dwImgWidth,g_dwImgHeight};

    hr = IDirect3DDevice9_StretchRect(m_pD3DDevice9, d3dsrc, &src, d3ddest, &dst,
                                      D3DTEXF_LINEAR);
    IDirect3DSurface9_Release(d3ddest);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }
    /* Update the vertex buffer */
    CUSTOMVERTEX *vertices;

    hr = d3dvtc->Lock(0, 0, (void **)&vertices, D3DLOCK_DISCARD);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }
    /* Setup vertices */
    const float f_width  =(float)gBackBufferWidth;
    const float f_height = (float)gBackBufferHeight;


    /* -0.5f is a "feature" of DirectX and it seems to apply to Direct3d also */


    vertices[0].x       = -0.5f;       // left
    vertices[0].y       = f_height-0.5f;       // top
    vertices[0].z       = 0.0f;
    vertices[0].diffuse = D3DCOLOR_ARGB(255, 255, 255, 255);
    vertices[0].rhw     = 1.0f;
    vertices[0].tu      = 0.0f;
    vertices[0].tv      = 0.0f;

    vertices[1].x       = f_width - 0.5f;    // right
    vertices[1].y       = f_height-0.5f;       // top
    vertices[1].z       = 0.0f;
    vertices[1].diffuse = D3DCOLOR_ARGB(255, 255, 255, 255);
    vertices[1].rhw     = 1.0f;
    vertices[1].tu      = 1.0f;
    vertices[1].tv      = 0.0f;

    vertices[2].x       = f_width - 0.5f;    // right
    vertices[2].y       = - 0.5f;   // bottom
    vertices[2].z       = 0.0f;
    vertices[2].diffuse = D3DCOLOR_ARGB(255, 255, 255, 255);
    vertices[2].rhw     = 1.0f;
    vertices[2].tu      = 1.0f;
    vertices[2].tv      = 1.0f;

    vertices[3].x       = -0.5f;       // left
    vertices[3].y       =  - 0.5f;   // bottom
    vertices[3].z       = 0.0f;
    vertices[3].diffuse = D3DCOLOR_ARGB(255, 255, 255, 255);
    vertices[3].rhw     = 1.0f;
    vertices[3].tu      = 0.0f;
    vertices[3].tv      = 1.0f;

    hr= IDirect3DVertexBuffer9_Unlock(d3dvtc);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    // Begin the scene
    hr = IDirect3DDevice9_BeginScene(m_pD3DDevice9);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    // Setup our texture. Using textures introduces the texture stage states,
    // which govern how textures get blended together (in the case of multiple
    // textures) and lighting information. In this case, we are modulating
    // (blending) our texture with the diffuse color of the vertices.
    hr = IDirect3DDevice9_SetTexture(m_pD3DDevice9, 0,
                                     (LPDIRECT3DBASETEXTURE9)d3dtex);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    // Render the vertex buffer contents
    hr = IDirect3DDevice9_SetStreamSource(m_pD3DDevice9, 0, d3dvtc, 0,
                                          sizeof(CUSTOMVERTEX));
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    hr = IDirect3DDevice9_SetStreamSource(m_pD3DDevice9, 0, d3dvtc, 0,
                                          sizeof(CUSTOMVERTEX));
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    // we use FVF instead of vertex shader
    hr = IDirect3DDevice9_SetFVF(m_pD3DDevice9,
                                 D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    // draw rectangle
    hr = IDirect3DDevice9_DrawPrimitive(m_pD3DDevice9, D3DPT_TRIANGLEFAN, 0, 2);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }

    // End the scene
    hr = IDirect3DDevice9_EndScene(m_pD3DDevice9);
    if(hr != S_OK)
    {
        return SDK_FAILURE;
    }

    //prensent
    hr = IDirect3DDevice9_Present(m_pD3DDevice9, NULL ,NULL, NULL, NULL);
    if(hr != S_OK)
    {
        printf("error is in function:%s ,line:%d\n",__FUNCTIONW__,__LINE__);
        return SDK_FAILURE;
    }


    return SDK_SUCCESS;

}

void D3D9Cleanup()
{

    if(m_pD3DDevice9 != NULL)
    {
        SAFE_RELEASE(m_pD3DDevice9);
    }
    if(pOffScreenSur != NULL)
    {
        SAFE_RELEASE(pOffScreenSur);
    }
    if(d3dtex != NULL)
    {
        SAFE_RELEASE(d3dtex);
    }
    if(d3dvtc!= NULL)
    {
        SAFE_RELEASE(d3dvtc);
    }
    if(pBackBuffer != NULL)
    {
        SAFE_RELEASE(pBackBuffer);
    }


}

int SimpleDX9::readInputImage(std::string inputImageName)
{

    // load input bitmap image
    std::string filePath = getPath() + std::string("SimpleDX9_input.bmp");
    inputBitmap.load(filePath.c_str());
    if(!inputBitmap.isLoaded())
    {
        std::cout << "Failed to load input image!";
        return SDK_FAILURE;
    }


    // get width and height of input image
    g_dwImgHeight = inputBitmap.getHeight();
    g_dwImgWidth = inputBitmap.getWidth();

    framesize = outFrameSize = g_dwImgWidth * g_dwImgHeight *4;

    // allocate memory for input & output image data
    inputImageData  = (cl_uchar4*)malloc(g_dwImgWidth * g_dwImgHeight * sizeof(
            cl_uchar4));
    CHECK_ALLOCATION(inputImageData, "Failed to allocate memory! (inputImageData)");

    // allocate memory for output image data
    outputImageData = (cl_uchar4*)malloc(g_dwImgWidth * g_dwImgHeight * sizeof(
            cl_uchar4));
    CHECK_ALLOCATION(outputImageData,
                     "Failed to allocate memory! (outputImageData)");

    memset(outputImageData, 0, g_dwImgWidth * g_dwImgHeight * sizeof(cl_uchar4));

    if(sampleArgs->verify)
    {

        verifyOutput = (unsigned char *)malloc(outFrameSize * sizeof(unsigned char));
        CHECK_ALLOCATION(verifyOutput,
                         "Failed to allocate host memory. (verifyOutput)");
        memset(verifyOutput,0,outFrameSize * sizeof(unsigned char));

    }
    // get the pointer to pixel data
    pixelData = inputBitmap.getPixels();
    if(pixelData == NULL)
    {
        std::cout << "Failed to read pixel Data!";
        return SDK_FAILURE;
    }

    // Copy pixel data into inputImageData
    memcpy(inputImageData, pixelData,
           g_dwImgHeight * g_dwImgWidth * sizeof(cl_uchar4));


    return SDK_SUCCESS;


}


int SimpleDX9::genBinaryImage()
{
    bifData binaryData;
    binaryData.kernelName = std::string("SimpleDX9_Kernels.cl");
    binaryData.flagsStr = std::string("");
    if(sampleArgs->isComplierFlagsSpecified())
    {
        binaryData.flagsFileName = std::string(sampleArgs->flags.c_str());
    }

    binaryData.binaryName = std::string(sampleArgs->dumpBinary.c_str());
    int status = generateBinaryImage(binaryData);
    return status;

}



int SimpleDX9::setupCL()
{

    cl_int status = 0;
    cl_device_type dType;

    if(sampleArgs->deviceType.compare("cpu") == 0)
    {
        dType = CL_DEVICE_TYPE_CPU;
    }
    else //deviceType = "gpu"
    {
        dType = CL_DEVICE_TYPE_GPU;
        if(sampleArgs->isThereGPU() == false)
        {
            std::cout << "GPU not found. Falling back to CPU device" << std::endl;
            dType = CL_DEVICE_TYPE_CPU;
        }
    }

    /*
    * Have a look at the available platforms and pick either
    * the AMD one if available or a reasonable default.
    */
    int retValue = getPlatform(platform, sampleArgs->platformId,
                               sampleArgs->isPlatformEnabled());
    CHECK_ERROR(retValue, SDK_SUCCESS, "getPlatform() failed");

    // Init extension function pointers
#define INIT_CL_EXT_FCN_PTR(platform, name) \
    if(!pfn_##name) { \
        pfn_##name = (name##_fn) \
                     clGetExtensionFunctionAddressForPlatform(platform, #name); \
        if(!pfn_##name) { \
            std::cout << "Cannot get pointer to ext. fcn. " #name << std::endl; \
            return SDK_FAILURE; \
        } \
    }

    INIT_CL_EXT_FCN_PTR(platform, clGetDeviceIDsFromDX9MediaAdapterKHR);
    INIT_CL_EXT_FCN_PTR(platform, clCreateFromDX9MediaSurfaceKHR);
    INIT_CL_EXT_FCN_PTR(platform, clEnqueueAcquireDX9MediaSurfacesKHR);
    INIT_CL_EXT_FCN_PTR(platform, clEnqueueReleaseDX9MediaSurfacesKHR);

    // Display available devices.
    retValue = displayDevices(platform, dType);
    CHECK_ERROR(retValue, SDK_SUCCESS, "displayDevices() failed");

    cl_uint deviceCount = 0;
    status = clGetDeviceIDs(platform, dType, 0, NULL, &deviceCount);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed");

    cl_device_id* deviceIds = (cl_device_id*)malloc(sizeof(cl_device_id) *
                              deviceCount);
    CHECK_ALLOCATION(deviceIds, "Failed to allocate memory(deviceIds)");

    // Get device ids
    status = clGetDeviceIDs(platform, dType, deviceCount, deviceIds, NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed");

    bD3D9Share = CL_FALSE;

    //Search whether any of the devices supports DX9 extensions.
    for(cl_uint i = 0; i < deviceCount; ++i)
    {

        retValue = deviceInfo.setDeviceInfo(deviceIds[i]);
        CHECK_ERROR(retValue, 0, "SDKDeviceInfo::setDeviceInfo() failed");

        if(strstr(deviceInfo.extensions, "cl_khr_dx9_media_sharing") != NULL)
        {
            bD3D9Share = CL_TRUE;
            break;
        }
    }

	free(deviceIds); 
    if(!bD3D9Share)
    {
        printf("None of the existing devices support interOp between DX9 and OpenCL!");
        return SDK_EXPECTED_FAILURE;
    }

    //setup D3D9
    status = SetupD3D9();
    if(status != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    //cl_context_properties used to DX9 and OpenCL interOp
    cl_context_properties cps[5] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0, 0, 0};
    cps[2] = CL_CONTEXT_ADAPTER_D3D9EX_KHR;
    cps[3] = (cl_context_properties)m_pD3DDevice9;

    //Create cl context
    context = clCreateContext(cps, 1, &device, NULL, NULL, &status);
    CHECK_OPENCL_ERROR(status, "clCreateContext failed. (clShareBufIn)");

    //Create CommandQueue
    cl_command_queue_properties prop = 0;
    commandQueue = clCreateCommandQueue(context, device, prop, &status);
    CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed. (clShareBufIn)");

    //the _cl_dx9_surface_info_khr struct used to create cl_mem,
    //the first member is the D3D9 OffscreenSurface
    typedef struct _cl_dx9_surface_info_khr
    {
        IDirect3DSurface9 *resource;
        HANDLE shared_handle;
    } cl_dx9_surface_info_khr;

    cl_dx9_surface_info_khr suface_info;
    suface_info.resource = pOffScreenSur;
    suface_info.shared_handle = NULL;

    //Create a cl_mem from D3D9 surface, the type of result is clImage
    clShareBufIn = pfn_clCreateFromDX9MediaSurfaceKHR(context,CL_MEM_WRITE_ONLY,
                   CL_ADAPTER_D3D9EX_KHR,&suface_info,0,&status);
    CHECK_OPENCL_ERROR(status,
                       "g_pfnCreateFromDX9MediaSurfaceKHR failed. (clShareBufIn)");

    //Create the input buffer
    clRGBbuf = clCreateBuffer(context, CL_MEM_READ_ONLY,
                              g_dwImgWidth * g_dwImgHeight *sizeof(cl_uchar4), NULL, &status);
    CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (clShareBufIn)");

    //Write buffer
    cl_uchar4 * temp =   (cl_uchar4*) clEnqueueMapBuffer(commandQueue,clRGBbuf,
                                                         CL_TRUE,CL_MAP_WRITE ,0,
                                                         g_dwImgWidth * g_dwImgHeight *sizeof(cl_uchar4),
                                                         0,
                                                         NULL,
                                                         NULL,
                                                         NULL);

    memcpy(temp,inputImageData,g_dwImgWidth * g_dwImgHeight *sizeof(cl_uchar4));
    
    clEnqueueUnmapMemObject(commandQueue,clRGBbuf,temp,0,NULL,NULL);

    // create a CL program using the kernel source
    buildProgramData buildData;
    buildData.kernelName = std::string("SimpleDX9_Kernels.cl");
    buildData.devices = devices;
  	buildData.deviceId = 0;    
	buildData.flagsStr = std::string("");
    if(sampleArgs->isLoadBinaryEnabled())
    {
        buildData.binaryName = std::string(sampleArgs->loadBinary.c_str());
    }

    if(sampleArgs->isComplierFlagsSpecified())
    {
        buildData.flagsFileName = std::string(sampleArgs->flags.c_str());
    }

    retValue = buildOpenCLProgram(program, context, buildData);
    CHECK_ERROR(retValue, 0, "buildOpenCLProgram() failed");

    // get a kernel object handle for a kernel with the given name
    mirrokernel = clCreateKernel(program, "mirrorimage_rgba", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel failed.");

    status = kernelInfo.setKernelWorkGroupInfo(mirrokernel,
             device);
	  
    CHECK_ERROR(status, SDK_SUCCESS, " setKernelWorkGroupInfo() failed");

    //set local and global threads
    localThreads[0] = 16;
    localThreads[1] = 8;
    globalThreads[0] = RoundUp((int)localThreads[0], g_dwImgWidth/2);
    globalThreads[1] = RoundUp((int)localThreads[1], g_dwImgHeight);

    //set kernel args
    int arg = 0;
    status = clSetKernelArg(mirrokernel, arg++, sizeof(cl_mem), (void *)&clRGBbuf);
    CHECK_ERROR(status, SDK_SUCCESS, " clSetKernelArg() failed");

    status |= clSetKernelArg(mirrokernel, arg++, sizeof(cl_int),
                             (void *)&g_dwImgWidth);
    CHECK_ERROR(status, SDK_SUCCESS, " clSetKernelArg() failed");

    status |= clSetKernelArg(mirrokernel, arg++, sizeof(cl_int),
                             (void *)&g_dwImgHeight);
    CHECK_ERROR(status, SDK_SUCCESS, " clSetKernelArg() failed");

    //Create Scence
    if(CreateScence()!= SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

     

    return SDK_SUCCESS;


}



size_t SimpleDX9::RoundUp(int group_size, int global_size)
{
    int r = global_size % group_size;
    if(r == 0)
    {
        return global_size;
    }
    else
    {
        return global_size + group_size - r;
    }
}


int SimpleDX9 ::GPUtrans()
{
    int status;
    size_t region[3] = {g_dwImgWidth, g_dwImgHeight, 1};
    size_t origin[3]= {0, 0, 0};

    //acquire OpenCL memory objects that have been created from D3D9 objects
    hr = pfn_clEnqueueAcquireDX9MediaSurfacesKHR(commandQueue, 1, &clShareBufIn, 0,
            NULL, 0);

    //the Image created from the D3D9 offscreensSurface is the last arg of the kernel
    status = clSetKernelArg(mirrokernel, 3, sizeof(cl_mem), (void *)&clShareBufIn);
    CHECK_ERROR(status, SDK_SUCCESS, " clSetKernelArg() failed");

    //NDRange kernel to do the mirror operation
    status = clEnqueueNDRangeKernel(commandQueue, mirrokernel, 2, NULL,
                                    globalThreads, localThreads, 0, NULL, NULL);
    CHECK_ERROR(status, SDK_SUCCESS, " clEnqueueNDRangeKernel failed");

    //Read the output Image to do verification
    status = clEnqueueReadImage(commandQueue,clShareBufIn,CL_TRUE,origin,region,0,0,
                                outputImageData,0,NULL,NULL);
    clFinish(commandQueue);

    //release OpenCL memory objects that have been created from D3D9 objects
    status = pfn_clEnqueueReleaseDX9MediaSurfacesKHR(commandQueue, 1, &clShareBufIn,
             0, NULL, 0);
    CHECK_ERROR(status, SDK_SUCCESS,
                " g_pfnEnqueueReleaseDX9MediaSurfacesKHR failed");

    return SDK_SUCCESS;

}


int SimpleDX9::run()
{


    if(sampleArgs->verify || sampleArgs->timing || sampleArgs->quiet)
    {
        int ktimer = sampleTimer->createTimer();
        sampleTimer->resetTimer(ktimer);
        sampleTimer->startTimer(ktimer);

        if(GPUtrans() != SDK_SUCCESS)
        {
            return SDK_FAILURE;
        }
        sampleTimer->stopTimer(ktimer);
        kernelTime = (double)(sampleTimer->readTimer(ktimer));
        if(sampleArgs->verify)
        {
            unsigned char *psrc = (unsigned char *)inputImageData;
            for(cl_uint i =0; i<g_dwImgHeight; i++)
            {
                for(cl_uint j=0; j<g_dwImgWidth/2; j++)
                {
                    for(int k = 0; k < 4; k++)
                    {
                        verifyOutput[(i*g_dwImgWidth+g_dwImgWidth-1-j)*4 + k]=
                            verifyOutput[(i*g_dwImgWidth+j)*4+k]
                            = psrc[(i*g_dwImgWidth+j)*4+((4+(2-k)))%4];
                    }

                }
            }

            if(!memcmp(outputImageData,verifyOutput,outFrameSize*sizeof(unsigned char)))
            {
                printf("passed!\n");

            }
            else
            {
                printf("failed!\n");

            }
        }

    }
    else
    {
        // Show the window
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);

        MSG msg;
        ZeroMemory(&msg, sizeof(msg));

        while( msg.message != WM_QUIT )
        {
            if( PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                t1 = clock() * CLOCKS_PER_SEC;
                frameCount ++;
                GPUtrans();
                Direct3DRenderScene();

                t2 = clock() * CLOCKS_PER_SEC;
                totalElapsedTime += (double)(t2 - t1);
                if(frameCount && frameCount > numFrames)
                {
                    double fMs = (double)((totalElapsedTime / (double)CLOCKS_PER_SEC) /
                                          (double) frameCount);
                    int framesPerSec = (int)(1.0 / (fMs / CLOCKS_PER_SEC));
                    char str[128];
                    sprintf_s(str, 128, "OpenCLDX9 InterOp | %d FPS", framesPerSec);
                    SetWindowText(hWnd, (str));
                    frameCount = 0;
                    totalElapsedTime = 0.0;


                }
            }
        }

    }

    return SDK_SUCCESS;

}


int SimpleDX9::setup()
{
    if(readInputImage("SimpleDX9_input.bmp") == SDK_FAILURE)
    {
        return SDK_FAILURE;
    }

    int timer = sampleTimer->createTimer();
    sampleTimer->resetTimer(timer);
    sampleTimer->startTimer(timer);


    int errorStatus = setupCL();
    if(errorStatus != SDK_SUCCESS)
    {
        return errorStatus;
    }

    sampleTimer->stopTimer(timer);
    // Compute setup time
    setupTime = (double)(sampleTimer->readTimer(timer));

    return SDK_SUCCESS;
}


int SimpleDX9::initialize()
{
    if(sampleArgs->initialize())
    {
        return SDK_FAILURE;
    }

    Option* frame_option = new Option;
    CHECK_ALLOCATION(frame_option,
                     "Error. Failed to allocate memory (length_option)\n");

    frame_option->_sVersion = "n";
    frame_option->_lVersion = "numFrames";
    frame_option->_description = "number of the frames";
    frame_option->_type = CA_ARG_INT;
    frame_option->_value = &numFrames;

    sampleArgs->AddOption(frame_option);
    delete frame_option;

    return SDK_SUCCESS;

}

void SimpleDX9::printStats()
{
    if(sampleArgs->timing)
    {
        std::string strArray[4] = {"frameWidth", "frameHeight" ,"Setup Time(sec)", "Kernel Time(sec)"};

        std::string stats[4];
        stats[0] = toString(g_dwImgWidth, std::dec);
        stats[1] = toString(g_dwImgHeight, std::dec);
        stats[2] = toString(setupTime, std::dec);
        stats[3] = toString(kernelTime, std::dec);

        printStatistics(strArray, stats, 4);
    }
}
int SimpleDX9::verifyResults()
{
    return SDK_SUCCESS;
}


int SimpleDX9::cleanup()
{
    cl_int status;

    status = clReleaseMemObject(clRGBbuf);
    CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(clRGBbuf)");

    status = clReleaseMemObject(clShareBufIn);
    CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(dOutDataBuf)");

    status = clReleaseKernel(mirrokernel);
    CHECK_OPENCL_ERROR(status, "clReleaseKernel failed.(kernel)");

    status = clReleaseProgram(program);
    CHECK_OPENCL_ERROR(status, "clReleaseProgram failed.(program)");

    status = clReleaseCommandQueue(commandQueue);
    CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.(commandQueue)");

    status = clReleaseContext(context);
    CHECK_OPENCL_ERROR(status, "clReleaseContext failed.(context)");

    // Release program resources (input memory etc.)

    FREE(devices);
    FREE(inputImageData);
    FREE(outputImageData);
    if(sampleArgs->verify)
    {
        FREE(verifyOutput);
    }
    D3D9Cleanup();

    return SDK_SUCCESS;
}

int
main(int argc, char * argv[])
{
    // Create SimpleDX9 object
    SimpleDX9 sample;

    // Initialization
    if(sample.initialize() != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    // Parse command line options
    if(sample.sampleArgs->parseCommandLine(argc, argv) != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    if(sample.sampleArgs->isDumpBinaryEnabled())
    {
        return sample.genBinaryImage();
    }

    int errorStatus = sample.setup();
    if(errorStatus != SDK_SUCCESS)
    {
        return errorStatus;
    }

    // Run
    if(sample.run()!=SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    // Verify
    if(sample.verifyResults()!=SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    // Cleanup resources created
    if(sample.cleanup()!=SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    // Print performance statistics
    sample.printStats();
    UnregisterClass(reinterpret_cast<LPCSTR>(WINDOW_CLASS_NAME), wc.hInstance);

    return SDK_SUCCESS;
}








