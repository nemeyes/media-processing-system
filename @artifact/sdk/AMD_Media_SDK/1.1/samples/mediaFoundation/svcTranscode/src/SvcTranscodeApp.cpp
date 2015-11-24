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
 * @file <SvcTranscodeApp.cpp>                          
 *                                       
 * @brief This file contains functions for building transcode topology
 *         
 ********************************************************************************
 */

/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include "SvcTranscodeApp.h"
#include "SvcTranscodeDlg.h"

BEGIN_MESSAGE_MAP(SvcTranscodeApp, CWinApp)
ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

/** 
 *******************************************************************************
 *  @fn     SvcTranscodeApp
 *  @brief  SvcTranscodeApp constructor
 *
 *******************************************************************************
 */
SvcTranscodeApp::SvcTranscodeApp()
{
}
/******************************************************************************
 * The one and only SvcTranscodeApp object                                       *
 ******************************************************************************/
SvcTranscodeApp theApp;

/** 
 *******************************************************************************
 *  @fn     InitInstance
 *  @brief  This function creates transcode dialog instance 
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
BOOL SvcTranscodeApp::InitInstance()
{

    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    /***************************************************************************
     * Set this to include all the common control classes you want to use in    *
     * your appliaction                                                         *
     ***************************************************************************/
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    /***************************************************************************
     * Create transcode dialog instance                                          *
     ***************************************************************************/
    SvcTranscodeDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == -1)
    {
        TRACE(
                        traceAppMsg,
                        0,
                        "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
        TRACE(
                        traceAppMsg,
                        0,
                        "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
    }
    return true;
}