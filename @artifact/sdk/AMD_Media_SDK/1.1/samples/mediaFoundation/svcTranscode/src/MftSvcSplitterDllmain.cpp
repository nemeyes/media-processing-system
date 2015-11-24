/*******************************************************************************
 Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 1   Redistributions of source code must retain the above copyright notice, 
 this list of conditions and the following disclaimer.
 2   Redistributions in binary form must reproduce the above copyright notice, 
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
 ********************************************************************************
 * @file <MftSvcSplitterDllmain.cpp>                          
 *                                       
 * @brief contains dll entry point functions
 *         
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include <assert.h>
#include <new>
#include <Shlwapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <strmif.h>
#include <initguid.h>

#include "MftSvcSplitter.h"
#include "MftSvcSplitterTransform.h"
#include "MftSvcSplitterGuids.h"
#include "MftUtils.h"

#include <assert.h>
#include <new>
#include <Shlwapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <strmif.h>
#include <initguid.h>
#include "MftSvcSplitterTransformApi.h"

/**
 *   @fn RegisterObject.
 *   @brief Registers DLL as COM object.
 */
HRESULT RegisterObject(HMODULE hModule, const GUID& guid,
                const TCHAR *pszDescription, const TCHAR *pszThreadingModel);

/**
 *   @fn UnregisterObject.
 *   @brief Unregisters DLL.
 */
HRESULT UnregisterObject(const GUID& guid);

/**
 *   @fn DllAddRef.
 */
void DllAddRef();

/**
 *   @fn DllRelease.
 */
void DllRelease();

/**
 *   @fn DllMain.
 */
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *);

/**
 *   @fn DllCanUnloadNow.
 */
STDAPI DllCanUnloadNow();

/**
 *   @fn DllGetClassObject.
 */
STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv);

/**
 *   @fn DllRegisterServer.
 */
STDAPI DllRegisterServer();

/**
 *   @fn DllUnregisterServer.
 */
STDAPI DllUnregisterServer();

/**
 *   @fn CreateObjectKeyName.
 */
HRESULT CreateObjectKeyName(const GUID& guid, TCHAR *sName, DWORD cchMax);

/**
 *   @fn CreateRegKeyAndValue.
 */
HRESULT CreateRegKeyAndValue(HKEY hKey, PCWSTR pszSubKeyName,
                PCWSTR pszValueName, PCWSTR pszData, PHKEY phkResult);

/**
 *   @fn RegisterObject.
 */
HRESULT RegisterObject(HMODULE hModule, const GUID& guid,
                const TCHAR *pszDescription, const TCHAR *pszThreadingModel);

/**
 *   @fn UnregisterObject.
 */
HRESULT UnregisterObject(const GUID& guid);

/**
 *   @brief Global ref counter for module usage.
 */
long g_cRefModule = 0;

/**
 *   @brief Global variable which holds module handle.
 */
HMODULE g_hModule = NULL;
/**
 *   @class MFTransform MFT.
 */

/** 
 *******************************************************************************
 *  @fn     DllAddRef
 *  @brief  Increments DLL ref count
 *          
 *          
 *  @return void
 *******************************************************************************
 */
void DllAddRef()
{
    InterlockedIncrement(&g_cRefModule);
}

/** 
 *******************************************************************************
 *  @fn     DllRelease
 *  @brief  Decreases DLL ref count
 *           
 *          
 *  @return void
 *******************************************************************************
 */
void DllRelease()
{
    InterlockedDecrement(&g_cRefModule);
}

/** 
 *******************************************************************************
 *  @fn     DllMain
 *  @brief  DLL entry point
 *           
 *  @param[in] hInstance : Instance
 *  @param[in] dwReason       :
 *  @param[in] void*          : 
 *          
 *  @return BOOL : TRUE if successful; otherwise returns FALSE.
 *******************************************************************************
 */
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hModule = (HMODULE)hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    return TRUE;
}

/** 
 *******************************************************************************
 *  @fn     DllCanUnloadNow
 *  @brief  
 *           
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns Microsoft error codes
 *******************************************************************************
 */
STDAPI DllCanUnloadNow()
{
    return (g_cRefModule == 0) ? S_OK : E_FAIL;
}
EXTERN_GLOBAL(HRESULT) mftSplitterCreateInstance(REFIID riid, void **ppv)
{
    return Transform::createInstance(riid, ppv);
}
