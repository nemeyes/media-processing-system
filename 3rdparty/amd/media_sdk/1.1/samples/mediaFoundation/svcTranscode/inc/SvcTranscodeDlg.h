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
 * @file <SvcTranscodeDlg.h>                          
 *                                       
 * @brief Contains typedefs for data types
 *         
 ********************************************************************************
 */
#ifndef _SVCTRANSCODEDLG_H_
#define _SVCTRANSCODEDLG_H_
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
/*******************************************************************************
 * Including SDKDDKVer.h defines the highest available Windows platform.        *
 * If you wish to build your application for a previous Windows platform,       *
 * include WinSDKVer.h and set the _WIN32_WINNT macro to the platform you wish  *
 * to support before including SDKDDKVer.h.                                     *
 *******************************************************************************/
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <SDKDDKVer.h>

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS
/*******************************************************************************
 * MFC core and standard components                                             *
 *******************************************************************************/
#include <afxwin.h>         
/*******************************************************************************
 * MFC extensions                                                               *
 *******************************************************************************/
#include <afxext.h>         
#ifndef _AFX_NO_OLE_SUPPORT
/*******************************************************************************
 * MFC support for Internet Explorer 4 Common Controls                          *
 *******************************************************************************/
#include <afxdtctl.h>
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
/*******************************************************************************
 *  MFC support for Windows Common Controls                                     *
 *******************************************************************************/
#include <afxcmn.h>
#endif // _AFX_NO_AFXCMN_SUPPORT
/*******************************************************************************
 * MFC support for ribbons and control bars                                     *
 *******************************************************************************/
#include <afxcontrolbars.h>
#include <mfapi.h>
#include "resource.h"
#include "KS.H"
#include "TranscodeSession.h"
#include <afxdialogex.h>
#include "SvcTranscodeApp.h"
#include "resource.h"
/*******************************************************************************
 * Contains interface details of AMD video encoder and decoder MFT              *
 *******************************************************************************/
#include "MftUtils.h"
/*******************************************************************************
 * Helper functions for printing log file                                       *
 *******************************************************************************/
#include "PrintLog.h"
/*******************************************************************************
 * Contains interface details of AMD video quality MFT                          *
 *******************************************************************************/
#include "VqMft.h"

#define WM_UPDATE_UI_EVENT (WM_USER + 1)

#include "TranscodeSession.h"
/**
 * @brief Class SvcTranscodeDlg.
 */
class SvcTranscodeDlg: public CDialogEx, public PlaybackStateSubscriber
{
public:

    /**
     * @brief Constructor.
     */
    SvcTranscodeDlg(CWnd* pParent = NULL);
    ~SvcTranscodeDlg(void);
    enum
    {
        IDD = IDD_SvcTranscode_DIALOG
    };
    static BOOL useDx9;

protected:

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();afx_msg
    void onPaint();afx_msg
    HCURSOR onQueryDragIcon();DECLARE_MESSAGE_MAP()

public:

    /**
     * @brief PlaybackStateSubscriber::OnStateChange().
     */
    virtual void onStateChange(SvcTranscodeSession::State newState);

    afx_msg
    void onBnClickedButtonOpen();afx_msg
    void onBnClickedButtonPlay();afx_msg
    void onBnClickedButtonStop();afx_msg
    void onRadioBtn1Select();afx_msg
    void onRadioBtn2Select();afx_msg
    void onRadioBtn3Select();afx_msg
    void onBnClickedButtonApplyVqSettings();afx_msg
    void onHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);afx_msg
    void OnTimer(UINT_PTR nIDEvent);

private:

    CComPtr<SvcTranscodeSession> mPlaybackSession;
    HICON _hIcon;
    UINT_PTR _timerId;
    LRESULT onUpdateUI(WPARAM wParam, LPARAM lParam);
    FILE *mLogFile;
    void updatePlayerControls(SvcTranscodeSession::State playerState);
};

#endif
