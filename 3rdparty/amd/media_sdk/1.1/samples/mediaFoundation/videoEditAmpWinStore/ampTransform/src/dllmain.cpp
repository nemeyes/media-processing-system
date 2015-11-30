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
* @file <dllmain.cpp> 
*   
* @brief This file contains implementation of dllmain functions
*
***************************************************************
*/
#include <initguid.h>
#include "AmpTransform.h"

using namespace Microsoft::WRL;

ActivatableClass(CAmpTransform);

/** 
 *******************************************************************************
 *  @fn     DllMain
 *  @brief  DLL entry point
 *           
 *  @param[in] hInstance      : Instance
 *  @param[in] dwReason       :
 *  @param[in] lpReserved          : 
 *          
 *  @return BOOL : TRUE if successful; otherwise returns FALSE.
 *******************************************************************************
 */
BOOL WINAPI DllMain( _In_ HINSTANCE hInstance, _In_ DWORD dwReason, _In_opt_ LPVOID lpReserved )
{
    if( DLL_PROCESS_ATTACH == dwReason )
    {
        DisableThreadLibraryCalls( hInstance );

        Module<InProc>::GetModule().Create();
    }
    else if( DLL_PROCESS_DETACH == dwReason )
    {
        Module<InProc>::GetModule().Terminate();
    }

    return TRUE;
}

/** 
 *******************************************************************************
 *  @fn     DllGetActivationFactory
 *  @brief  Retrieves activation factory interface
 *           
 *  @param[in] activatibleClassId      : Activable class ID
 *  @param[out] factory                : Pointer to  activation factory interface
 *          
 *  @return BOOL : TRUE if successful; otherwise returns FALSE.
 *******************************************************************************
 */
HRESULT WINAPI DllGetActivationFactory( _In_ HSTRING activatibleClassId, _Outptr_ IActivationFactory** factory )
{
    auto &module = Microsoft::WRL::Module< Microsoft::WRL::InProc >::GetModule();
    return module.GetActivationFactory( activatibleClassId, factory );
}

/** 
 *******************************************************************************
 *  @fn     DllCanUnloadNow
 *  @brief  
 *           
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes
 *******************************************************************************
 */
HRESULT WINAPI DllCanUnloadNow()
{
    auto &module = Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule();    
    return (module.Terminate()) ? S_OK : S_FALSE;
}

/** 
 *******************************************************************************
 *  @fn     DllGetClassObject
 *  @brief  Get DLL class object
 *           
 *  @param[in] clsid      : Class ID
 *  @param[in] riid       :
 *  @param[out] ppv         : 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes
 *******************************************************************************
 */
STDAPI DllGetClassObject( _In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv )
{
    auto &module = Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule();
    return module.GetClassObject( rclsid, riid, ppv );
}
