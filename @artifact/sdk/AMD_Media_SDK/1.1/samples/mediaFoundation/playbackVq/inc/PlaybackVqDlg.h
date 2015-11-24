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
 * @file <PlaybackVqDlg.h>
 *
 * @brief This file contains common functionality required for creating the
 *        dialog for playback
 *
 ********************************************************************************
 */
#ifndef _PLAYBACKVQDLG_H_
#define _PLAYBACKVQDLG_H_
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
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit
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
#include <mfapi.h>
#include "resource.h"
#include "KS.H"
#include "PlaybackSession.h"
#include <afxdialogex.h>
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
/*******************************************************************************
 * Contains interface details of AMD video quality MFT                          *
 *******************************************************************************/
#define VIDEOWINDOWLEFTPADDING 500
#define VIDEOWINDOWBOTTOMPADDING 90
/**
 * @brief Class PlaybackVQDlg.
 */
class PlaybackVQDlg: public CDialogEx, public PlaybackStateSubscriber
{
public:

    /**
     * @brief Constructor.
     */
    PlaybackVQDlg(CWnd* pParent = NULL);

    /**
     * @brief Destructor.
     */
    ~PlaybackVQDlg();

    enum
    {
        IDD = IDD_PLAYBACKVQ_DIALOG
    };

protected:

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();afx_msg
    void onPaint();afx_msg
    HCURSOR onQueryDragIcon();DECLARE_MESSAGE_MAP()

public:

    /**
     * @brief PlaybackStateSubscriber::onStateChange().
     */
    virtual void onStateChange(PlaybackSession::State newState);

    /**
     * @brief UI handlers.
     */
    afx_msg
    void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);afx_msg
    void OnGetMinMaxInfo(MINMAXINFO* lpMMI);afx_msg
    void OnClose();afx_msg
    void OnSize(UINT nType, int cx, int cy);afx_msg
    void onUpdateVqSettings();afx_msg
    void onClickButtonOpen();afx_msg
    void onBnClickedButtonPlay();afx_msg
    void onBnClickedButtonStop();afx_msg
    void onBnClickedSwitchToFullscreen();afx_msg
    void OnTimer(UINT_PTR nIDEvent);

private:

    FILE *mLogFile;
    bool mEnableDx11Acceleration;

    void moveControl(int32 controlId, LONG dx, LONG dy);
    CComPtr<PlaybackSession> mPlaybackSession;

    HICON mHIcon;
    UINT_PTR mTimerId;
    CString mFilePath;

    HRESULT openFile(CString filePath);

    LRESULT onUpdateUi(WPARAM wParam, LPARAM lParam);

    void updatePlayerControls(PlaybackSession::State playerState);
    void updateVQControls();
    HRESULT updateVQSettings();

    BOOL mEnableSteadyVideo;
    CSliderCtrl mSteadyVideoStrengthCtrl;
    int32 mSteadyVideoStrength;

    CSliderCtrl mSteadyVideoDelayCtrl;
    int32 mSteadyVideoDelay;

    CSliderCtrl mSteadyVideoZoomCtrl;
    int32 mSteadyVideoZoom;

    BOOL mEnableEdgeEnhancement;
    int32 mEdgeEnhancement;
    CSliderCtrl mEdgeEnhancementCtrl;

    BOOL mEnableDynamicContrast;
    BOOL mEnableBrighterWhites;

    BOOL mEnableDenoise;
    CSliderCtrl mDenoiseStrengthCtrl;
    int32 mDenoiseStrength;

    BOOL mEnableMosquitoDenoise;
    CSliderCtrl mMosquitoDenoiseStrengthCtrl;
    int32 mMosquitoDenoiseStrength;

    BOOL mEnableDeblocking;
    CSliderCtrl mDeblockingStrengthCtrl;
    int32 mDeblockingStrength;

    BOOL mEnableColorVibrance;
    CSliderCtrl mColorVibranceStrengthCtrl;
    int32 mColorVibranceStrength;

    BOOL mEnableFleshToneFix;
    CSliderCtrl mFleshToneFixStrengthCtrl;
    int32 mFleshToneFixStrength;

    BOOL mEnableVideoGamma;
    CSliderCtrl mVideoGammaStrengthCtrl;
    int32 mVideoGammaStrength;

    int32 mDeinterlaceMode;

    BOOL mEnablePulldownDetection;

    BOOL mEnableDynamicRange;
    int32 mDynamicRange;

    int32 mBrightnessValue;
    CSliderCtrl mBrightnessCtrl;

    int32 mContrastValue;
    CSliderCtrl mContrastCtrl;

    int32 mSaturationValue;
    CSliderCtrl mSaturationCtrl;

    int32 mTintValue;
    CSliderCtrl mTintCtrl;

    int32 mDirectXMode;

    BOOL mEnableSuperresDT;
    CSliderCtrl mSuperresDTCtrl;
    int32 mSuperresDTValue;

    BOOL mEnableSuperresMCTNR;
    CSliderCtrl mSuperresMCTNRCtrl;
    int32 mSuperresMCTNRStrength;

    BOOL mEnableFalseContourReduce;
    CSliderCtrl mFalseContourReduceCtrl;
    int32 mFalseContourReduceStrength;

    BOOL mEnableDemoMode;
    int32 mScaleMode;

    CButton mVideoWindowDx11;
    CButton mVideoWindowDx9;

    BOOL isFileOpened;

public:
    void onBnClickedRecommendedVqSettings();afx_msg
    void OnStnClickedStaticCpuUsage();
};
#endif //_PLAYBACKVQDLG_H_
