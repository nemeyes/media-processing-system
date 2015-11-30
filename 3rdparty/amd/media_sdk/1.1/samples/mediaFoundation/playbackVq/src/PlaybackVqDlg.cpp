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
 * @file <PlaybackVqDlg.cpp>
 *
 * @brief This file contains functions for building playback topology
 *
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include "PlaybackVqDlg.h"

#define WM_UPDATE_UI_EVENT           (WM_USER + 1)
#define WM_CACHE_BULD_STARTED_EVENT  (WM_USER + 2)
#define WM_CACHE_BULD_FAILED_EVENT   (WM_USER + 3)
#define WM_CACHE_BULD_PROGRESS_EVENT (WM_USER + 4)
/**
 *******************************************************************************
 *  @fn     PlaybackVQDlg
 *  @brief  playback  dialogue constructor
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
PlaybackVQDlg::PlaybackVQDlg(CWnd* pParent) :
    CDialogEx(PlaybackVQDlg::IDD, pParent), mEnableDx11Acceleration(false),
                    mEnableSteadyVideo(FALSE), mSteadyVideoStrength(3),
                    mSteadyVideoZoom(100), mSteadyVideoDelay(1),
                    mEdgeEnhancement(50), mEnableEdgeEnhancement(FALSE),
                    mEnableDenoise(FALSE), mEnableMosquitoDenoise(FALSE),
                    mEnableDeblocking(FALSE), mEnableDynamicContrast(FALSE),
                    mEnableColorVibrance(FALSE), mEnableFleshToneFix(FALSE),
                    mEnableBrighterWhites(FALSE), mEnableVideoGamma(FALSE),
                    mEnableDynamicRange(FALSE), mDenoiseStrength(50),
                    mMosquitoDenoiseStrength(68), mDeblockingStrength(50),
                    mColorVibranceStrength(50), mFleshToneFixStrength(50),
                    mVideoGammaStrength(1000), mDeinterlaceMode(0),
                    mEnablePulldownDetection(FALSE), mDynamicRange(0),
                    mBrightnessValue(0), mContrastValue(1000),
                    mSaturationValue(1000), mTintValue(0), mDirectXMode(0),
                    mEnableSuperresDT(FALSE), mSuperresDTValue(50),
                    mEnableSuperresMCTNR(FALSE), mSuperresMCTNRStrength(5),
                    mEnableFalseContourReduce(FALSE),
                    mFalseContourReduceStrength(50), mEnableDemoMode(FALSE),
                    mScaleMode(0), isFileOpened(0)
{
    mHIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    mLogFile = fopen("ErrorLogPlaybackVq.txt", "w");
    if (mLogFile == NULL)
    {
        MessageBox(_T("Failed to open log file."), _T("Error"), MB_OK
                        | MB_ICONERROR);
        return;
    }
}
/**
 *******************************************************************************
 *  @fn     PlaybackVQDlg
 *  @brief  playback  dialogue destructor
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
PlaybackVQDlg::~PlaybackVQDlg()
{
    if (mPlaybackSession != NULL)
    {
        HRESULT hr = mPlaybackSession->shutdown();
        LOGHRESULT(hr, "failed to shut down media Sessoin");
    }
    return;
}
/**
 *******************************************************************************
 *  @fn     DoDataExchange
 *  @brief  Called to exchange and validate dialog data
 *
 *  @param[in] pParent   : Dialog data exchange (DDX) and dialog data validation
 *                         (DDV) routines used by the Microsoft Foundation classes
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_STEADY_VIDEO, mEnableSteadyVideo);
    DDX_Slider(pDX, IDC_SLIDER_STEADY_VIDEO_STRENGTH, mSteadyVideoStrength);
    DDV_MinMaxInt(pDX, mSteadyVideoStrength, 0, 3);
    DDX_Slider(pDX, IDC_SLIDER_STEADY_VIDEO_ZOOM, mSteadyVideoZoom);
    DDV_MinMaxInt(pDX, mSteadyVideoZoom, 90, 100);
    DDX_Slider(pDX, IDC_SLIDER_STEADY_VIDEO_DELAY, mSteadyVideoDelay);
    DDV_MinMaxInt(pDX, mSteadyVideoDelay, 0, 6);
    DDX_Control(pDX, IDC_SLIDER_STEADY_VIDEO_STRENGTH, mSteadyVideoStrengthCtrl);
    DDX_Control(pDX, IDC_SLIDER_STEADY_VIDEO_DELAY, mSteadyVideoDelayCtrl);
    DDX_Control(pDX, IDC_SLIDER_STEADY_VIDEO_ZOOM, mSteadyVideoZoomCtrl);
    DDX_Control(pDX, IDC_SLIDER_EDGE_ENHANCEMENT, mEdgeEnhancementCtrl);
    DDX_Slider(pDX, IDC_SLIDER_EDGE_ENHANCEMENT, mEdgeEnhancement);
    DDX_Check(pDX, IDC_CHECK_ENABLE_EDGE_ENHANCEMENT, mEnableEdgeEnhancement);
    DDX_Check(pDX, IDC_CHECK_ENABLE_DENOISE, mEnableDenoise);
    DDX_Check(pDX, IDC_CHECK_ENABLE_MOSQUITO_DENOISE, mEnableMosquitoDenoise);
    DDX_Check(pDX, IDC_CHECK_ENABLE_DEBLOCKING, mEnableDeblocking);
    DDX_Check(pDX, IDC_CHECK_ENABLE_DYNAMIC_CONTRAST, mEnableDynamicContrast);
    DDX_Check(pDX, IDC_CHECK_ENABLE_COLOR_VIBRANCE, mEnableColorVibrance);
    DDX_Check(pDX, IDC_CHECK_ENABLE_FLESH_TONE_FIX, mEnableFleshToneFix);
    DDX_Check(pDX, IDC_CHECK_ENABLE_BRIGHTER_WHITES, mEnableBrighterWhites);
    DDX_Check(pDX, IDC_CHECK_ENABLE_VIDEO_GAMMA, mEnableVideoGamma);
    DDX_Check(pDX, IDC_CHECK_ENABLE_DYNAMIC_RANGE, mEnableDynamicRange);
    DDX_Check(pDX, IDC_CHECK_ENABLE_DEMOMODE, mEnableDemoMode);
    DDX_Control(pDX, IDC_SLIDER_DENOISE, mDenoiseStrengthCtrl);
    DDX_Slider(pDX, IDC_SLIDER_DENOISE, mDenoiseStrength);
    DDV_MinMaxInt(pDX, mDenoiseStrength, 1, 100);
    DDX_Control(pDX, IDC_SLIDER_MOSQUITO_DENOISE, mMosquitoDenoiseStrengthCtrl);
    DDX_Slider(pDX, IDC_SLIDER_MOSQUITO_DENOISE, mMosquitoDenoiseStrength);
    DDV_MinMaxInt(pDX, mMosquitoDenoiseStrength, 0, 100);
    DDX_Control(pDX, IDC_SLIDER_DEBLOCKING, mDeblockingStrengthCtrl);
    DDX_Slider(pDX, IDC_SLIDER_DEBLOCKING, mDeblockingStrength);
    DDV_MinMaxInt(pDX, mDeblockingStrength, 1, 100);
    DDX_Control(pDX, IDC_SLIDER_COLOR_VIBRANCE, mColorVibranceStrengthCtrl);
    DDX_Slider(pDX, IDC_SLIDER_COLOR_VIBRANCE, mColorVibranceStrength);
    DDV_MinMaxInt(pDX, mColorVibranceStrength, 1, 100);
    DDX_Control(pDX, IDC_SLIDER_FLESH_TONE_FIX, mFleshToneFixStrengthCtrl);
    DDX_Slider(pDX, IDC_SLIDER_FLESH_TONE_FIX, mFleshToneFixStrength);
    DDV_MinMaxInt(pDX, mFleshToneFixStrength, 1, 100);
    DDX_Control(pDX, IDC_SLIDER_VIDEO_GAMMA, mVideoGammaStrengthCtrl);
    DDX_Slider(pDX, IDC_SLIDER_VIDEO_GAMMA, mVideoGammaStrength);
    DDV_MinMaxInt(pDX, mVideoGammaStrength, 500, 2500);
    DDX_Radio(pDX, IDC_RADIO_DEINTERLACE_AUTO, mDeinterlaceMode);
    DDV_MinMaxInt(pDX, mDeinterlaceMode, 0, 5);
    DDX_Check(pDX, IDC_CHECK_ENABLE_PULL_DOWN_DETECTION,
                    mEnablePulldownDetection);
    DDX_Radio(pDX, IDC_RADIO_FULL_RANGE, mDynamicRange);
    DDV_MinMaxInt(pDX, mDynamicRange, 0, 1);
    DDX_Slider(pDX, IDC_SLIDER_BRIGHTNESS, mBrightnessValue);
    DDV_MinMaxInt(pDX, mBrightnessValue, -100, 100);
    DDX_Slider(pDX, IDC_SLIDER_CONTRAST, mContrastValue);
    DDV_MinMaxInt(pDX, mContrastValue, 0, 2000);
    DDX_Slider(pDX, IDC_SLIDER_SATURATION, mSaturationValue);
    DDV_MinMaxInt(pDX, mSaturationValue, 0, 2000);
    DDX_Slider(pDX, IDC_SLIDER_TINT, mTintValue);
    DDV_MinMaxInt(pDX, mTintValue, -30000, 30000);
    DDX_Control(pDX, IDC_SLIDER_BRIGHTNESS, mBrightnessCtrl);
    DDX_Control(pDX, IDC_SLIDER_CONTRAST, mContrastCtrl);
    DDX_Control(pDX, IDC_SLIDER_SATURATION, mSaturationCtrl);
    DDX_Control(pDX, IDC_SLIDER_TINT, mTintCtrl);
    DDX_Radio(pDX, IDC_RADIO_DX9_ACCELERATION, mDirectXMode);
    DDV_MinMaxInt(pDX, mDirectXMode, 0, 1);
    DDX_Control(pDX, IDC_VIDEOPANELDX11, mVideoWindowDx11);
    DDX_Control(pDX, IDC_VIDEOPANELDX9, mVideoWindowDx9);
    DDX_Check(pDX, IDC_CHECK_ENABLE_FALSE_CONTOUR, mEnableFalseContourReduce);
    DDX_Slider(pDX, IDC_SLIDER_FALSE_CONTOUR_STRENGTH,
                    mFalseContourReduceStrength);
    DDV_MinMaxInt(pDX, mFalseContourReduceStrength, 0, 100);
    DDX_Control(pDX, IDC_SLIDER_FALSE_CONTOUR_STRENGTH, mFalseContourReduceCtrl);
    DDX_Radio(pDX, IDC_RADIO_BILINEAR_SCALE, mScaleMode);
    DDV_MinMaxInt(pDX, mScaleMode, 0, 1);
}
/*****************************************************************************
 * Begin Message map for playbackVQ dialog                                    *
 *****************************************************************************/
BEGIN_MESSAGE_MAP(PlaybackVQDlg, CDialogEx)
ON_WM_SIZE()
ON_WM_PAINT()
ON_WM_TIMER()
ON_WM_CLOSE()
ON_WM_HSCROLL()
ON_WM_GETMINMAXINFO()
ON_WM_QUERYDRAGICON()
ON_MESSAGE(WM_UPDATE_UI_EVENT, &PlaybackVQDlg::onUpdateUi)

ON_BN_CLICKED(IDC_BUTTON_OPEN, &PlaybackVQDlg::onClickButtonOpen)
ON_BN_CLICKED(IDC_BUTTON_PLAY, &PlaybackVQDlg::onBnClickedButtonPlay)
ON_BN_CLICKED(IDC_BUTTON_STOP, &PlaybackVQDlg::onBnClickedButtonStop)
ON_BN_CLICKED(IDC_BUTTON_RECOMMENDED_VQ_SETTINGS, &PlaybackVQDlg::onBnClickedRecommendedVqSettings)
ON_BN_CLICKED(IDC_CHECK_STEADY_VIDEO, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_DYNAMIC_CONTRAST, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_PULL_DOWN_DETECTION, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_EDGE_ENHANCEMENT, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_BRIGHTER_WHITES, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_COLOR_VIBRANCE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_DENOISE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_MOSQUITO_DENOISE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_DEBLOCKING, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_FLESH_TONE_FIX, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_VIDEO_GAMMA, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_FALSE_CONTOUR, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_RADIO_DEINTERLACE_AUTO, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_RADIO_DEINTERLACE_WEAVE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_RADIO_DEINTERLACE_BOB, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_RADIO_DEINTERLACE_ADAPTIVE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_RADIO_DEINTERLACE_MOTION_ADAPTIVE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_RADIO_DEINTERLACE_VECTOR_ADAPTIVE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_DYNAMIC_RANGE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_RADIO_FULL_RANGE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_SWITCH_TO_FULLSCREEN, &PlaybackVQDlg::onBnClickedSwitchToFullscreen)
ON_BN_CLICKED(IDC_RADIO_BILINEAR_SCALE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_RADIO_BICUBIC_SCALE, &PlaybackVQDlg::onUpdateVqSettings)
ON_BN_CLICKED(IDC_CHECK_ENABLE_DEMOMODE, &PlaybackVQDlg::onUpdateVqSettings)
END_MESSAGE_MAP()

/**
 *******************************************************************************
 *  @fn     OnInitDialog
 *  @brief  Overridding OnInitDialog() function. This will be called after
 *          DoModal call
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
BOOL PlaybackVQDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(mHIcon, TRUE);
    SetIcon(mHIcon, FALSE);
    /**************************************************************************
     * Create PlaybackSession instance to build and run playback topology      *
     **************************************************************************/
    HRESULT hr = PlaybackSession::create(this, &mPlaybackSession);
    if (FAILED(hr))
    {
        MessageBox(_T("Failed to create playback sessoin."), _T("Error"), MB_OK | MB_ICONERROR);
        return false;
    }
    mEnableDx11Acceleration = mPlaybackSession->isDx11RendererSupported();

    /**************************************************************************
     * Set the log file of playbacksession instance only in DEBUG mode         *
     **************************************************************************/
    mPlaybackSession->setLogFile(mLogFile);

    /**************************************************************************
     * Set default parameters                                                  *
     **************************************************************************/
    mTimerId = this->SetTimer(1, 1000, NULL);

    mSteadyVideoStrengthCtrl.SetRange(0, 3, TRUE);

    mSteadyVideoDelayCtrl.SetRange(0, 6, TRUE);

    mSteadyVideoZoomCtrl.SetRange(90, 100, TRUE);

    mEdgeEnhancementCtrl.SetRange(1, 100, TRUE);

    mDenoiseStrengthCtrl.SetRange(1, 100, TRUE);

    mMosquitoDenoiseStrengthCtrl.SetRange(0, 100, TRUE);

    mDeblockingStrengthCtrl.SetRange(1, 100, TRUE);

    mColorVibranceStrengthCtrl.SetRange(1, 100, TRUE);

    mFleshToneFixStrengthCtrl.SetRange(1, 100, TRUE);

    mVideoGammaStrengthCtrl.SetRange(500, 2500, TRUE);

    mBrightnessCtrl.SetRange(-100, 100, TRUE);

    mContrastCtrl.SetRange(0, 2000, TRUE);

    mSaturationCtrl.SetRange(0, 2000, TRUE);

    mTintCtrl.SetRange(-30000, 30000, TRUE);

    mFalseContourReduceCtrl.SetRange(0, 100, TRUE);;

    GetDlgItem(IDC_RADIO_DX11_ACCELERATION)->EnableWindow(mEnableDx11Acceleration);

    UpdateData(FALSE);
    updatePlayerControls(PlaybackSession::State::Building);
    /**************************************************************************
     *  return TRUE  unless you set the focus to a control                     *
     **************************************************************************/
    return TRUE;
}
/**
 *******************************************************************************
 *  @fn     onPaint
 *  @brief  Called when an application makes a request to repaint a
 *          portion of an application's window
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::onPaint()
{
    if (IsIconic())
    {
        /**********************************************************************
         * device context for painting                                         *
         **********************************************************************/
        CPaintDC dc(this);

        SendMessage(WM_ICONERASEBKGND,
                        reinterpret_cast<WPARAM> (dc.GetSafeHdc()), 0);
        /**********************************************************************
         * Center icon in client rectangle                                     *
         **********************************************************************/
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;
        /**********************************************************************
         * Draw the icon                                                       *
         **********************************************************************/
        dc.DrawIcon(x, y, mHIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}
void PlaybackVQDlg::OnClose()
{
    ShowWindow( SW_HIDE);

    mPlaybackSession->shutdown();

    EndDialog(0);
}
/**
 *******************************************************************************
 *  @fn     onQueryDragIcon
 *  @brief  calls this function to obtain the cursor to display while the user
 *          drags the minimized window.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HCURSOR PlaybackVQDlg::onQueryDragIcon()
{
    return static_cast<HCURSOR> (mHIcon);
}
/**
 *******************************************************************************
 *  @fn     onStateChange
 *  @brief  Called when state is changed. virtual function from PlaybackStateSubscriber
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::onStateChange(PlaybackSession::State newState)
{
    if (GetSafeHwnd() != 0)
    {
        PostMessage(WM_UPDATE_UI_EVENT, static_cast<WPARAM> (newState));
    }
}
/**
 *******************************************************************************
 *  @fn     onUpdateUi
 *  @brief  Called on update GUI
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
LRESULT PlaybackVQDlg::onUpdateUi(WPARAM wParam, LPARAM lParam)
{
    PlaybackSession::State state = static_cast<PlaybackSession::State> (wParam);
    if (state == PlaybackSession::Failed)
    {
        mPlaybackSession.Release();

        HRESULT hr = PlaybackSession::create(this, &mPlaybackSession);
        if (SUCCEEDED(hr))
        {
            state = mPlaybackSession->getState();
        }
        else
        {
            TRACE_MSG("Failed recreate playback session.", hr);
            EndDialog(1);
        }
    }

    updatePlayerControls(state);
    updateVQControls();
    updateVQSettings();
    (void) lParam;

    return 0;
}

/**
 *******************************************************************************
 *  @fn     onClickButtonOpen
 *  @brief  Called on clicking the button OPEN
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::onClickButtonOpen()
{
    UpdateData();

    wchar_t filePathBuff[2048] = { 0 };
    CFileDialog
                    openFileDlg(
                                    TRUE,
                                    NULL,
                                    NULL,
                                    OFN_FILEMUSTEXIST,
                                    _T(
                                                    "*.mp4|*.mp4|*.asf|*.asf|*.avi|*.avi|All Files (*.*)|*.*||"),
                                    this);
    OPENFILENAME& ofn = openFileDlg.GetOFN();
    ofn.Flags |= OFN_FILEMUSTEXIST;
    ofn.lpstrFile = filePathBuff;
    ofn.nMaxFile = sizeof(filePathBuff) / sizeof(filePathBuff[0]);

    INT_PTR openResult = openFileDlg.DoModal();
    if (openResult == IDOK)
    {
        mFilePath = filePathBuff;
        isFileOpened = true;

        HRESULT hr = openFile(mFilePath);
        if (FAILED(hr))
        {
            LOG(mLogFile, "Error code : 0x%x Failed to open file @ %s %d \n",
                            hr, __FILE__, __LINE__);
            MessageBox(
                            _T(
                                            "Failed to open video file, \nUn-supported platform or stream."),
                            _T("Error"), MB_OK | MB_ICONERROR);
        }
    }
}
/**
 *******************************************************************************
 *  @fn     onBnClickedButtonPlay
 *  @brief  Called on clicking the button PLAY
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::onBnClickedButtonPlay()
{
    UpdateData();
    PlaybackSession::State state = mPlaybackSession->getState();
    HRESULT hr;
    if (PlaybackSession::Playing == state)
    {
        hr = mPlaybackSession->pause();
        if (FAILED(hr))
        {
            LOG(
                            mLogFile,
                            "Error code : 0x%x Failed to pause playback session @ %s %d \n",
                            hr, __FILE__, __LINE__);
        }
    }
    else if (PlaybackSession::Paused == state)
    {
        hr = mPlaybackSession->resume();
        if (FAILED(hr))
        {
            LOG(
                            mLogFile,
                            "Error code : 0x%x Failed to Resume playback session @ %s %d \n",
                            hr, __FILE__, __LINE__);
        }
    }
    else if (PlaybackSession::Stopped == state)
    {
        PlaybackSession::RendererType selectedRendererType =
                        mDirectXMode ? PlaybackSession::RendererDx11
                                        : PlaybackSession::RendererDx9;

        if (selectedRendererType != mPlaybackSession->getRendererType())
        {
            hr = openFile(mFilePath);
        }
        else
        {
            hr = mPlaybackSession->play();
        }
        if (FAILED(hr))
        {
            LOG(mLogFile, "Error code : 0x%x  @ %s %d \n", hr, __FILE__,
                            __LINE__);
        }
    }
    else
    {
        return;
    }
    if (FAILED(hr))
    {
        MessageBox(_T("Failed to play/resume/pause video file."), _T("Error"),
                        MB_OK | MB_ICONERROR);
    }
}
/**
 *******************************************************************************
 *  @fn     onBnClickedButtonStop
 *  @brief  Called on clicking the button STOP
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::onBnClickedButtonStop()
{
    HRESULT hr = mPlaybackSession->stop();
    if (FAILED(hr))
    {
        MessageBox(_T("Failed to stop video file."), _T("Error"), MB_OK
                        | MB_ICONERROR);
    }
}
/**
 *******************************************************************************
 *  @fn     updatePlayerControls
 *  @brief  Updates the player controls
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::updatePlayerControls(PlaybackSession::State playerState)
{
    CWnd* openButton = GetDlgItem(IDC_BUTTON_OPEN);
    CWnd* playButton = GetDlgItem(IDC_BUTTON_PLAY);
    CWnd* stopButton = GetDlgItem(IDC_BUTTON_STOP);
    CWnd* dx9Button = GetDlgItem(IDC_RADIO_DX9_ACCELERATION);
    CWnd* dx11Button = GetDlgItem(IDC_RADIO_DX11_ACCELERATION);
    CWnd* recommendedButton = GetDlgItem(IDC_BUTTON_RECOMMENDED_VQ_SETTINGS);
    CWnd* fullscreenButton = GetDlgItem(IDC_SWITCH_TO_FULLSCREEN);

    TRACE_MSG("PlaybackSession playerState", playerState);
    /**************************************************************************
     * Update the player buttons depending on the state of playback session    *
     **************************************************************************/
    switch (playerState)
    {
    case PlaybackSession::Playing:
        openButton->EnableWindow(true);
        playButton->EnableWindow(true);
        playButton->SetWindowText(_T("Pause"));
        stopButton->EnableWindow(true);
        dx9Button->EnableWindow(false);
        dx11Button->EnableWindow(false);
        recommendedButton->EnableWindow(true);
        fullscreenButton->EnableWindow(true);
        break;

    case PlaybackSession::Paused:
        openButton->EnableWindow(true);
        playButton->EnableWindow(true);
        playButton->SetWindowText(_T("Continue"));
        stopButton->EnableWindow(true);
        dx9Button->EnableWindow(false);
        dx11Button->EnableWindow(false);
        recommendedButton->EnableWindow(true);
        fullscreenButton->EnableWindow(true);
        break;

    case PlaybackSession::Stopped:
        openButton->EnableWindow(true);
        playButton->EnableWindow(true);
        playButton->SetWindowText(_T("Play"));
        stopButton->EnableWindow(false);
        dx9Button->EnableWindow(true);
        dx11Button->EnableWindow(mEnableDx11Acceleration);
        recommendedButton->EnableWindow(true);
        fullscreenButton->EnableWindow(true);
        break;

    case PlaybackSession::Building:
        openButton->EnableWindow(true);
        playButton->EnableWindow(false);
        stopButton->EnableWindow(false);
        dx9Button->EnableWindow(true);
        dx11Button->EnableWindow(mEnableDx11Acceleration);
        recommendedButton->EnableWindow(false);
        fullscreenButton->EnableWindow(false);
        break;

    case PlaybackSession::Ready:
    case PlaybackSession::PlayPending:
    case PlaybackSession::PausePending:
    case PlaybackSession::StopPending:
    case PlaybackSession::Failed:
    default:
        openButton->EnableWindow(false);
        playButton->EnableWindow(false);
        stopButton->EnableWindow(false);
        dx9Button->EnableWindow(false);
        dx11Button->EnableWindow(false);
        recommendedButton->EnableWindow(false);
        fullscreenButton->EnableWindow(false);
        break;
    }
}
/**
 *******************************************************************************
 *  @fn     updateVQControls
 *  @brief  Update VQ buttons
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::updateVQControls()
{
    mSteadyVideoDelayCtrl.EnableWindow(mEnableSteadyVideo);
    mSteadyVideoStrengthCtrl.EnableWindow(mEnableSteadyVideo);
    mSteadyVideoZoomCtrl.EnableWindow(mEnableSteadyVideo);
    mColorVibranceStrengthCtrl.EnableWindow(mEnableColorVibrance);
    mEdgeEnhancementCtrl.EnableWindow(mEnableEdgeEnhancement);
    mMosquitoDenoiseStrengthCtrl.EnableWindow(mEnableMosquitoDenoise);
    mDeblockingStrengthCtrl.EnableWindow(mEnableDeblocking);
    mFleshToneFixStrengthCtrl.EnableWindow(mEnableFleshToneFix);
    mVideoGammaStrengthCtrl.EnableWindow(mEnableVideoGamma);
    mDenoiseStrengthCtrl.EnableWindow(mEnableDenoise);
    mFalseContourReduceCtrl.EnableWindow(mEnableFalseContourReduce);

    GetDlgItem(IDC_RADIO_FULL_RANGE)->EnableWindow(mEnableDynamicRange);
    GetDlgItem(IDC_RADIO_LIMITED_RANGE)->EnableWindow(mEnableDynamicRange);
}
/**
 *******************************************************************************
 *  @fn     subtractTimes
 *  @brief  Subtract Time
 *
 *  @param[in] ftA   : Time A
 *  @param[in] ftB   : Time B
 *
 *  @return  ULONGLONG : Subtracted Time
 *******************************************************************************
 */
ULONGLONG subtractTimes(const FILETIME& ftA, const FILETIME& ftB)
{
    LARGE_INTEGER a, b;
    a.LowPart = ftA.dwLowDateTime;
    a.HighPart = ftA.dwHighDateTime;

    b.LowPart = ftB.dwLowDateTime;
    b.HighPart = ftB.dwHighDateTime;

    return a.QuadPart - b.QuadPart;
}
/**
 *******************************************************************************
 *  @fn     getCpuUsage
 *  @brief  Returns CPU usage
 *
 *
 *  @return  ULONGLONG : Subtracted Time
 *******************************************************************************
 */
std::pair<float, unsigned long long> getCpuUsage()
{
    FILETIME systemIdle = { 0 };
    FILETIME systemKernel = { 0 };
    FILETIME systemUser = { 0 };
    FILETIME processCreation = { 0 };
    FILETIME processExit = { 0 };
    FILETIME processKernel = { 0 };
    FILETIME processUser = { 0 };

    if (!GetSystemTimes(&systemIdle, &systemKernel, &systemUser)
                    || !GetProcessTimes(GetCurrentProcess(), &processCreation,
                                    &processExit, &processKernel, &processUser))
    {
        return std::pair<float, unsigned long long>(0.f, 0ULL);
    }

    static FILETIME previousSystemKernelTime = { 0 };
    static FILETIME previousSystemUserTime = { 0 };
    static FILETIME previousProcessKernelTime = { 0 };
    static FILETIME previousProcessUserTime = { 0 };

    static bool isFirstCall = true;
    if (isFirstCall)
    {
        previousSystemKernelTime = systemKernel;
        previousSystemUserTime = systemUser;
        previousProcessKernelTime = processKernel;
        previousProcessUserTime = processUser;

        isFirstCall = false;
    }

    static ULONGLONG previousTickCount = GetTickCount64();

    ULONGLONG currentTickCount = GetTickCount64();

    ULONGLONG duration = currentTickCount - previousTickCount;

    ULONGLONG systemKernelDiff = subtractTimes(systemKernel,
                    previousSystemKernelTime);
    ULONGLONG systemUserDiff =
                    subtractTimes(systemUser, previousSystemUserTime);
    ULONGLONG systemTotal = systemKernelDiff + systemUserDiff;

    ULONGLONG processKernelDiff = subtractTimes(processKernel,
                    previousProcessKernelTime);
    ULONGLONG processUserDiff = subtractTimes(processUser,
                    previousProcessUserTime);
    ULONGLONG processTotal = processKernelDiff + processUserDiff;

    previousSystemKernelTime = systemKernel;
    previousSystemUserTime = systemUser;
    previousProcessKernelTime = processKernel;
    previousProcessUserTime = processUser;

    previousTickCount = currentTickCount;

    if (systemTotal > 0)
    {
        float cpuUsage = 100.f * processTotal / systemTotal;
        return std::pair<float, unsigned long long>(cpuUsage, duration);
    }

    return std::pair<float, unsigned long long>(100.f, duration);
}
/**
 *******************************************************************************
 *  @fn     OnTimer
 *  @brief  Virtual Function
 *
 *  @param[in] nIDEvent   : Specifies the identifier of the timer.
 *
 *  @return  void
 *******************************************************************************
 */
void PlaybackVQDlg::OnTimer(UINT_PTR nIDEvent)
{
    std::pair<float, unsigned long long> cpuUsage = getCpuUsage();

    CString cpuUsageVal;
    cpuUsageVal.Format(_T("CPU usage: %.1f"), cpuUsage.first);

    CWnd* cpuUsageLabel = GetDlgItem(IDC_STATIC_CPU_USAGE);
    cpuUsageLabel->SetWindowText(cpuUsageVal);
    HRESULT hr;

    MFTIME time;
    hr = mPlaybackSession->getTime(&time);
    if (FAILED(hr))
    {
        return;
    }
    UINT64 duration;
    hr = mPlaybackSession->getDuration(&duration);
    if (FAILED(hr))
    {
        return;
    }
    /***************************************************************************
     * duration is measured in 100 nanosecond intervals (10E-7).                *
     ***************************************************************************/
    const unsigned rate = 10000000U;

    CTimeSpan durationTS(static_cast<time_t> (duration / rate));
    CTimeSpan timeTS(static_cast<time_t> (time / rate));

    CString positionVal = timeTS.Format(_T("%H:%M:%S")) + _T(" / ")
                    + durationTS.Format(_T("%H:%M:%S"));

    CWnd* curerntTimeLabel = GetDlgItem(IDC_STATIC_CURRENT_TIME);
    curerntTimeLabel->SetWindowText(positionVal);

    UINT32 processingTime = 0;
    CComPtr < IMFAttributes > vqAttributes;
    hr = mPlaybackSession->getVideoQualityAttributes(&vqAttributes);
    if (SUCCEEDED(hr) && vqAttributes)
    {
        vqAttributes->GetUINT32(AMF_PROCESSING_TRANSFORM_AVR_TIME,
                        &processingTime);
    }

    CString strProcessingTime;
    strProcessingTime.Format(_T("VQ Transform avg time: %d ms"), processingTime);
    CWnd* processingTimeLabel = GetDlgItem(IDC_STATIC_VQ_TRANSFORM_TIME);
    if (processingTimeLabel)
    {
        processingTimeLabel->SetWindowText(strProcessingTime);
    }
    (void) nIDEvent; /* Supress Warnings */

    bool isVqSupported;
    CString vqVal;
    CWnd* vqStatusLabel = GetDlgItem(IDC_STATIC_VQ_STATUS);
    hr = mPlaybackSession->vqStatus(&isVqSupported);
    if (isVqSupported == false)
    {
        vqVal.Format(_T("VQ status: VQ is not supported on this platform"));
    }
    else
    {
        vqVal.Format(_T("VQ status: VQ is supported"));
    }
    vqStatusLabel->SetWindowText(vqVal);
}
/**
 *******************************************************************************
 *  @fn     updateVQSettings
 *  @brief  updates the Video quality controls based on what is set on UI
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackVQDlg::updateVQSettings()
{
    HRESULT hr;

    CComPtr < IMFAttributes > vqAttributes;
    hr = MFCreateAttributes(&vqAttributes, 15);
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    vqAttributes->SetUINT32(AMF_EFFECT_STEADY_VIDEO,
                    static_cast<UINT32> (mEnableSteadyVideo));
    vqAttributes->SetUINT32(AMF_EFFECT_STEADY_VIDEO_STRENGTH,
                    static_cast<UINT32> (mSteadyVideoStrength));
    vqAttributes->SetUINT32(AMF_EFFECT_STEADY_VIDEO_DELAY,
                    static_cast<UINT32> (mSteadyVideoDelay));
    vqAttributes->SetUINT32(AMF_EFFECT_STEADY_VIDEO_ZOOM,
                    static_cast<UINT32> (mSteadyVideoZoom));

    vqAttributes->SetUINT32(AMF_EFFECT_DEINTERLACING,
                    static_cast<UINT32> (mDeinterlaceMode));

    vqAttributes->SetUINT32(AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION,
                    static_cast<UINT32> (mEnablePulldownDetection));

    vqAttributes->SetUINT32(AMF_EFFECT_EDGE_ENHANCEMENT,
                    static_cast<UINT32> (mEnableEdgeEnhancement));
    vqAttributes->SetUINT32(AMF_EFFECT_EDGE_ENHANCEMENT_STRENGTH,
                    static_cast<UINT32> (mEdgeEnhancement));

    vqAttributes->SetUINT32(AMF_EFFECT_DENOISE,
                    static_cast<UINT32> (mEnableDenoise));
    vqAttributes->SetUINT32(AMF_EFFECT_DENOISE_STRENGTH,
                    static_cast<UINT32> (mDenoiseStrength));

    vqAttributes->SetUINT32(AMF_EFFECT_MOSQUITO_NOISE,
                    static_cast<UINT32> (mEnableMosquitoDenoise));
    vqAttributes->SetUINT32(AMF_EFFECT_MOSQUITO_NOISE_STRENGTH,
                    static_cast<UINT32> (mMosquitoDenoiseStrength));

    vqAttributes->SetUINT32(AMF_EFFECT_DEBLOCKING,
                    static_cast<UINT32> (mEnableDeblocking));
    vqAttributes->SetUINT32(AMF_EFFECT_DEBLOCKING_STRENGTH,
                    static_cast<UINT32> (mDeblockingStrength));

    vqAttributes->SetUINT32(AMF_EFFECT_DYNAMIC_CONTRAST,
                    static_cast<UINT32> (mEnableDynamicContrast));

    vqAttributes->SetUINT32(AMF_EFFECT_COLOR_VIBRANCE,
                    static_cast<UINT32> (mEnableColorVibrance));
    vqAttributes->SetUINT32(AMF_EFFECT_COLOR_VIBRANCE_STRENGTH,
                    static_cast<UINT32> (mColorVibranceStrength));

    vqAttributes->SetUINT32(AMF_EFFECT_SKINTONE_CORRECTION,
                    static_cast<UINT32> (mEnableFleshToneFix));
    vqAttributes->SetUINT32(AMF_EFFECT_SKINTONE_CORRECTION_STRENGTH,
                    static_cast<UINT32> (mFleshToneFixStrength));

    vqAttributes->SetUINT32(AMF_EFFECT_BRIGHTER_WHITES,
                    static_cast<UINT32> (mEnableBrighterWhites));

    vqAttributes->SetUINT32(AMF_EFFECT_GAMMA_CORRECTION,
                    static_cast<UINT32> (mEnableVideoGamma));
    vqAttributes->SetDouble(AMF_EFFECT_GAMMA_CORRECTION_STRENGTH,
                    mVideoGammaStrength / 1000.);

    vqAttributes->SetUINT32(AMF_EFFECT_FALSE_CONTOUR_REDUCTION,
                    static_cast<UINT32> (mEnableFalseContourReduce));
    vqAttributes->SetDouble(AMF_EFFECT_FALSE_CONTOUR_REDUCTION_STRENGTH,
                    mFalseContourReduceStrength);

    vqAttributes->SetUINT32(AMF_EFFECT_DYNAMIC_RANGE,
                    static_cast<UINT32> (mDynamicRange));

    vqAttributes->SetDouble(AMF_EFFECT_BRIGHTNESS, mBrightnessValue);
    vqAttributes->SetDouble(AMF_EFFECT_CONTRAST, ((double) mContrastValue
                    / 1000));
    vqAttributes->SetDouble(AMF_EFFECT_SATURATION, ((double) mSaturationValue
                    / 1000));
    vqAttributes->SetDouble(AMF_EFFECT_TINT, ((double) mTintValue / 1000));

    vqAttributes->SetUINT32(AMF_EFFECT_DEMOMODE,
                    static_cast<UINT32> (mEnableDemoMode));

    CRect videoRect;
    mVideoWindowDx9.GetClientRect(&videoRect);
    vqAttributes->SetUINT32(AMF_EFFECT_SCALE, mScaleMode);
    vqAttributes->SetUINT32(AMF_EFFECT_SCALE_WIDTH, videoRect.Width());
    vqAttributes->SetUINT32(AMF_EFFECT_SCALE_HEIGHT, videoRect.Height());
    if (mPlaybackSession != nullptr)
    {
        hr = mPlaybackSession->setVideoQualityAttributes(vqAttributes);
        LOGIFFAILED(mLogFile, hr,
                        "Failed to set video quality attributes @ %s %d \n",
                        __FILE__, __LINE__);
    }

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     onUpdateVqSettings
 *  @brief  Called on update VQ settings
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::onUpdateVqSettings()
{
    UpdateData();
    updateVQControls();
    updateVQSettings();
}
/**
 *******************************************************************************
 *  @fn     onHScroll
 *  @brief  On changing the scroll buttons
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void PlaybackVQDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    UpdateData();
    updateVQSettings();

    (void) nSBCode;
    (void) nPos;
    (void) pScrollBar;
}
/**
 *******************************************************************************
 *  @fn     openFile
 *  @brief  Implements opening the file. This also calls openFile function of
 *          playbacksession which will build the topology
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackVQDlg::openFile(CString filePath)
{
    HWND rendererWindowHwnd;
    /**************************************************************************
     * Select the renderer type. Win7 - only DX9 renderer(EVR) win8 - Dx9 or   *
     * Dx11                                                                    *
     **************************************************************************/
    PlaybackSession::RendererType rendererType =
                    mDirectXMode ? PlaybackSession::RendererDx11
                                    : PlaybackSession::RendererDx9;
    if (PlaybackSession::RendererDx11 == rendererType)
    {
        mVideoWindowDx11.ShowWindow(SW_SHOW);
        mVideoWindowDx9.ShowWindow(SW_HIDE);

        rendererWindowHwnd = mVideoWindowDx11.GetSafeHwnd();
    }
    else
    {
        mVideoWindowDx11.ShowWindow(SW_HIDE);
        mVideoWindowDx9.ShowWindow(SW_SHOW);

        rendererWindowHwnd = mVideoWindowDx9.GetSafeHwnd();
    }
    /**************************************************************************
     * playback sessoin will build the topology required for playback operation*
     **************************************************************************/
    return mPlaybackSession->openFile(mFilePath, rendererWindowHwnd,
                    rendererType);
}

/**
 *******************************************************************************
 *  @fn     OnSize
 *  @brief  The framework calls this member function after the window's size
 *          has changed
 *
 *  @param[in] nType   : Specifies the type of resizing requested.
 *                       Eg maximize or minimize
 *  @param[in] cx      : Specifies the new width of the client area.
 *  @param[in] cy      : Specifies the new height of the client area.
 *
 *  @return void
 *******************************************************************************
 */
void PlaybackVQDlg::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);

    if (NULL == mVideoWindowDx9.GetSafeHwnd() || NULL
                    == mVideoWindowDx11.GetSafeHwnd())
    {
        return;
    }

    RECT clientRect;
    GetClientRect(&clientRect);

    RECT currentRect;
    mVideoWindowDx9.GetWindowRect(&currentRect);
    ScreenToClient(&currentRect);

    const LONG videoWindowWidth = clientRect.right - currentRect.left;
    const LONG videoWindowHeight = clientRect.bottom - VIDEOWINDOWBOTTOMPADDING;

    const LONG heightDelta = videoWindowHeight - (currentRect.bottom
                    - currentRect.top);

    moveControl(IDC_BUTTON_OPEN, 0, heightDelta);
    moveControl(IDC_BUTTON_PLAY, 0, heightDelta);
    moveControl(IDC_BUTTON_STOP, 0, heightDelta);
    moveControl(IDC_SWITCH_TO_FULLSCREEN, 0, heightDelta);
    moveControl(IDC_RADIO_DX9_ACCELERATION, 0, heightDelta);
    moveControl(IDC_RADIO_DX11_ACCELERATION, 0, heightDelta);
    moveControl(IDC_STATIC_CURRENT_TIME, 0, heightDelta);
    moveControl(IDC_STATIC_CPU_USAGE, 0, heightDelta);
    moveControl(IDC_STATIC_VQ_TRANSFORM_TIME, 0, heightDelta);
    moveControl(IDC_STATIC_VQ_STATUS, 0, heightDelta);

    mVideoWindowDx9.SetWindowPos(&CWnd::wndTop, currentRect.left,
                    currentRect.top, videoWindowWidth, videoWindowHeight,
                    SWP_SHOWWINDOW | SWP_NOMOVE);

    mVideoWindowDx11.SetWindowPos(&CWnd::wndTop, currentRect.left,
                    currentRect.top, videoWindowWidth, videoWindowHeight,
                    SWP_SHOWWINDOW | SWP_NOMOVE);

    if (mPlaybackSession != nullptr)
    {
        mPlaybackSession->resizeVideo(videoWindowWidth, videoWindowHeight);
    }
}
/**
 *******************************************************************************
 *  @fn     onBnClickedSwitchToFullscreen
 *  @brief  The framework calls this member function when expand video button
 *          is clicked
 *
 *  @return void
 *******************************************************************************
 */
void PlaybackVQDlg::onBnClickedSwitchToFullscreen()
{
    if (NULL == mVideoWindowDx9.GetSafeHwnd() || NULL
                    == mVideoWindowDx11.GetSafeHwnd())
    {
        return;
    }
    if (!isFileOpened)
    {
        return;
    }
    static bool expanded = false;

    expanded = !expanded;

    RECT clientRect;
    GetClientRect(&clientRect);

    const LONG videoWindowWidth = expanded ? clientRect.right
                    : clientRect.right - VIDEOWINDOWLEFTPADDING;
    const LONG videoWindowHeight = clientRect.bottom - VIDEOWINDOWBOTTOMPADDING;
    const LONG videoWindowLeft = expanded ? 0 : VIDEOWINDOWLEFTPADDING;

    mVideoWindowDx9.SetWindowPos(&CWnd::wndTop, videoWindowLeft, 0,
                    videoWindowWidth, videoWindowHeight, SWP_SHOWWINDOW);

    mVideoWindowDx11.SetWindowPos(&CWnd::wndTop, videoWindowLeft, 0,
                    videoWindowWidth, videoWindowHeight, SWP_SHOWWINDOW);

    if (mPlaybackSession != nullptr)
    {
        mPlaybackSession->resizeVideo(videoWindowWidth, videoWindowHeight);
    }
}
/**
 *******************************************************************************
 *  @fn     moveControl
 *  @brief  Moves the button location after resizing the window
 *
 *  @param[in] controlId   : butoon Id
 *  @param[in] dx          : Delta x
 *  @param[in] dy          : Delta y
 *
 *  @return void
 *******************************************************************************
 */
void PlaybackVQDlg::moveControl(INT32 controlId, LONG dx, LONG dy)
{
    CWnd* control = GetDlgItem(controlId);
    if (nullptr == control)
    {
        return;
    }

    RECT currentRect;
    control->GetWindowRect(&currentRect);

    ScreenToClient(&currentRect);

    control->SetWindowPos(NULL, currentRect.left + dx, currentRect.top + dy, 0,
                    0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
}
/**
 *******************************************************************************
 *  @fn     moveControl
 *  @brief  The framework calls this member function whenever Windows needs
 *          to know the maximized position or dimensions, or the minimum or
 *          maximum tracking size.
 *
 *  @param[in] lpMMI   : Points to a MINMAXINFO structure that contains
 *                       information about a window's maximized size
 *
 *  @return void
 *******************************************************************************
 */
void PlaybackVQDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    RECT minimalSize = { 0, 0, 600, 250 };
    ::MapDialogRect(GetSafeHwnd(), &minimalSize);

    lpMMI->ptMinTrackSize.x = minimalSize.right;
    lpMMI->ptMinTrackSize.y = minimalSize.bottom;

    __super::OnGetMinMaxInfo(lpMMI);
}

void PlaybackVQDlg::onBnClickedRecommendedVqSettings()
{
    if (nullptr == mPlaybackSession)
    {
        return;
    }

    if (FAILED(mPlaybackSession->setRealtimePlaybackSettings()))
    {
        MessageBox(_T("Failed to apply VQ settings."), _T("Error"), MB_OK
                        | MB_ICONERROR);
        return;
    }

    HRESULT hr;

    CComPtr < IMFAttributes > vqAttributes;
    hr = mPlaybackSession->getVideoQualityAttributes(&vqAttributes);
    if (FAILED(hr))
    {
        LOG(mLogFile, "Failed to getVideoQualityAttributes @ %s %d \n",
                        __FILE__, __LINE__);
    }

    if (SUCCEEDED(hr))
    {
        auto readAndSetUint32 = [vqAttributes](REFIID attributeId, int& val) -> HRESULT
        {
            UINT32 readValue;
            HRESULT hr = vqAttributes->GetUINT32(attributeId, &readValue);
            if (SUCCEEDED(hr))
            {
                val = static_cast<int>(readValue);
            }

            return hr;
        };

        readAndSetUint32(AMF_EFFECT_STEADY_VIDEO, mEnableSteadyVideo);
        readAndSetUint32(AMF_EFFECT_STEADY_VIDEO_STRENGTH, mSteadyVideoStrength);
        readAndSetUint32(AMF_EFFECT_STEADY_VIDEO_DELAY, mSteadyVideoDelay);
        readAndSetUint32(AMF_EFFECT_STEADY_VIDEO_ZOOM, mSteadyVideoZoom);

        readAndSetUint32(AMF_EFFECT_DEINTERLACING, mDeinterlaceMode);

        readAndSetUint32(AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION,
                        mEnablePulldownDetection);

        readAndSetUint32(AMF_EFFECT_EDGE_ENHANCEMENT, mEnableEdgeEnhancement);
        readAndSetUint32(AMF_EFFECT_EDGE_ENHANCEMENT_STRENGTH, mEdgeEnhancement);

        readAndSetUint32(AMF_EFFECT_DENOISE, mEnableDenoise);
        readAndSetUint32(AMF_EFFECT_DENOISE_STRENGTH, mDenoiseStrength);

        readAndSetUint32(AMF_EFFECT_MOSQUITO_NOISE, mEnableMosquitoDenoise);
        readAndSetUint32(AMF_EFFECT_MOSQUITO_NOISE_STRENGTH,
                        mMosquitoDenoiseStrength);

        readAndSetUint32(AMF_EFFECT_DEBLOCKING, mEnableDeblocking);
        readAndSetUint32(AMF_EFFECT_DEBLOCKING_STRENGTH, mDeblockingStrength);

        readAndSetUint32(AMF_EFFECT_DYNAMIC_CONTRAST, mEnableDynamicContrast);

        readAndSetUint32(AMF_EFFECT_COLOR_VIBRANCE, mEnableColorVibrance);
        readAndSetUint32(AMF_EFFECT_COLOR_VIBRANCE_STRENGTH,
                        mColorVibranceStrength);

        readAndSetUint32(AMF_EFFECT_SKINTONE_CORRECTION, mEnableFleshToneFix);
        readAndSetUint32(AMF_EFFECT_SKINTONE_CORRECTION_STRENGTH,
                        mFleshToneFixStrength);

        readAndSetUint32(AMF_EFFECT_BRIGHTER_WHITES, mEnableBrighterWhites);

        readAndSetUint32(AMF_EFFECT_DYNAMIC_RANGE, mEnableDynamicRange);

        readAndSetUint32(AMF_EFFECT_GAMMA_CORRECTION, mEnableVideoGamma);
        vqAttributes->SetDouble(AMF_EFFECT_GAMMA_CORRECTION_STRENGTH,
                        mVideoGammaStrength / 1000.);

        readAndSetUint32(AMF_EFFECT_FALSE_CONTOUR_REDUCTION,
                        mEnableFalseContourReduce);
        vqAttributes->SetDouble(AMF_EFFECT_FALSE_CONTOUR_REDUCTION_STRENGTH,
                        mFalseContourReduceStrength);

        readAndSetUint32(AMF_EFFECT_DYNAMIC_RANGE, mDynamicRange);

        readAndSetUint32(AMF_EFFECT_DEMOMODE, mEnableDemoMode);

        auto readAndSetDouble = [vqAttributes](REFIID attributeId, int rate, int& val) -> HRESULT
        {
            double readValue;
            HRESULT hr = vqAttributes->GetDouble(attributeId, &readValue);
            if (SUCCEEDED(hr))
            {
                val = static_cast<int>(readValue * rate);
            }

            return hr;
        };

        readAndSetDouble(AMF_EFFECT_BRIGHTNESS, 1, mBrightnessValue);
        readAndSetDouble(AMF_EFFECT_CONTRAST, 1000, mContrastValue);
        readAndSetDouble(AMF_EFFECT_SATURATION, 1000, mSaturationValue);
        readAndSetDouble(AMF_EFFECT_TINT, 1000, mTintValue);

        updateVQControls();
        UpdateData( FALSE);
    }
}
