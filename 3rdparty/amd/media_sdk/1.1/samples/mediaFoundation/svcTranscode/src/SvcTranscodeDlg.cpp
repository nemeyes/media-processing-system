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
 * @file <SvcTranscodeDlg.cpp>                          
 *                                       
 * @brief This file contains functions for building transcode topology
 *         
 ********************************************************************************
 */

#include "SvcTranscodeDlg.h"

#define WM_UPDATE_UI_EVENT (WM_USER + 1)

bool gUseDX9 = false;

/** 
 *******************************************************************************
 *  @fn     SvcTranscodeDlg
 *  @brief  transcode  dialogue constructor
 *           
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
SvcTranscodeDlg::SvcTranscodeDlg(CWnd* pParent /*=NULL*/) :
    CDialogEx(SvcTranscodeDlg::IDD, pParent)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    mLogFile = fopen("ErrorLogSvcTranscode.txt", "w");
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
 *  @brief  transcode  dialogue destructor
 *           
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
SvcTranscodeDlg::~SvcTranscodeDlg(void)
{
    mPlaybackSession->shutdown();
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
void SvcTranscodeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}
/*****************************************************************************
 * Begin Message map for playbackVQ dialog                                    *
 *****************************************************************************/
BEGIN_MESSAGE_MAP(SvcTranscodeDlg, CDialogEx)
ON_WM_PAINT()
ON_WM_SIZE()
ON_WM_TIMER()
ON_WM_QUERYDRAGICON()
ON_MESSAGE(WM_UPDATE_UI_EVENT, &SvcTranscodeDlg::onUpdateUI)
ON_BN_CLICKED(IDC_BUTTON_OPEN, &SvcTranscodeDlg::onBnClickedButtonOpen)
ON_BN_CLICKED(IDC_BUTTON_PLAY, &SvcTranscodeDlg::onBnClickedButtonPlay)
ON_BN_CLICKED(IDC_BUTTON_STOP, &SvcTranscodeDlg::onBnClickedButtonStop)
ON_BN_CLICKED(IDC_RADIO1, &SvcTranscodeDlg::onRadioBtn1Select)
ON_BN_CLICKED(IDC_RADIO2, &SvcTranscodeDlg::onRadioBtn2Select)
ON_BN_CLICKED(IDC_RADIO3, &SvcTranscodeDlg::onRadioBtn3Select)
ON_WM_HSCROLL()
END_MESSAGE_MAP()

/** 
 *******************************************************************************
 *  @fn     WndProc
 *  @brief  
 *           
 *
 *  @return LRESULT 
 *******************************************************************************
 */
LRESULT CALLBACK WndProc(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return 0;
}
/** 
 *******************************************************************************
 *  @fn     startOptionsDlgProc
 *  @brief  starts the dialog window to choose between Dx9 or Dx11 device 
 *           
 *  @param[in] hWndDlg   : Window
 *  @param[in] Msg       : Message
 *  @param[in] wParam    : param
 *  @param[in] lParam    : param
 *
 *  @return LRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
LRESULT CALLBACK startOptionsDlgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (Msg)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (wParam)
        {
        case ID_DIRECTX_9:
        {
            gUseDX9 = true;
            EndDialog(hwnd, 0);
            return TRUE;
        }
        case ID_DIRECTX_11:
        {
            gUseDX9 = false;
            EndDialog(hwnd, 0);
            return TRUE;
        }
        case IDOK:
            EndDialog(hwnd, 0);
            return TRUE;

        case IDCANCEL:
            EndDialog(hwnd, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/** 
 *******************************************************************************
 *  @fn     OnInitDialog
 *  @brief  Overridding OnInitDialog() function. This will be called after 
 *          DoModal call
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
BOOL SvcTranscodeDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CheckRadioButton(IDC_RADIO1, IDC_RADIO3, IDC_RADIO3);

    SetIcon(_hIcon, TRUE);
    SetIcon(_hIcon, FALSE);

    CWnd* videoPanel = GetDlgItem(IDC_VIDEOPANEL);
    HWND videoWindow = videoPanel->GetSafeHwnd();

    HWND directxDialog = NULL;

    /**************************************************************************
     * Create TranscodeSession instance to build and run transcode topology      *
     **************************************************************************/
    HRESULT hr = SvcTranscodeSession::create(videoWindow, this,
                    &mPlaybackSession);
    if (FAILED(hr))
    {
        MessageBox(_T("Failed to get video window handle."), _T("Error"), MB_OK
                        | MB_ICONERROR);
    }

    _timerId = this->SetTimer(1, 1000, NULL);
    /**************************************************************************
     * Set the log file of TranscodeSession instance only in DEBUG mode         *
     **************************************************************************/
    mPlaybackSession->setLogFile(mLogFile);
    /**************************************************************************
     * Check if Dx11 device is supported. If supported open window for user to *
     * choose between Dx11 or Dx9 device                                       *
     **************************************************************************/
    if (mPlaybackSession->isDx11RendererSupported())
    {
        DialogBox(0, MAKEINTRESOURCE(IDD_DIALOG1), directxDialog,
                        reinterpret_cast<DLGPROC> (startOptionsDlgProc));
    }
    else
    {
        gUseDX9 = true;
    }
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
void SvcTranscodeDlg::onPaint()
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
        dc.DrawIcon(x, y, _hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
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
HCURSOR SvcTranscodeDlg::onQueryDragIcon()
{
    return static_cast<HCURSOR> (_hIcon);
}
/**  
 *******************************************************************************
 *  @fn     onStateChange
 *  @brief  Called when state is changed. virtual function from PlaybackStateSubscriber
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void SvcTranscodeDlg::onStateChange(SvcTranscodeSession::State newState)
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
LRESULT SvcTranscodeDlg::onUpdateUI(WPARAM wParam, LPARAM /*lParam*/)
{
    SvcTranscodeSession::State state =
                    static_cast<SvcTranscodeSession::State> (wParam);

    updatePlayerControls(state);
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
void SvcTranscodeDlg::onBnClickedButtonOpen()
{
    wchar_t p[2048] = { 0 };
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
    ofn.lpstrFile = p;
    ofn.nMaxFile = 2048;

    INT_PTR openResult = openFileDlg.DoModal();
    if (openResult == IDOK)
    {
        HRESULT hr = mPlaybackSession->openFile(p, gUseDX9);
        if (FAILED(hr))
        {
            MessageBox(_T("Failed to open video file."), _T("Error"), MB_OK
                            | MB_ICONERROR);
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
void SvcTranscodeDlg::onBnClickedButtonPlay()
{
    SvcTranscodeSession::State state = mPlaybackSession->getState();

    HRESULT hr;

    if (SvcTranscodeSession::Playing == state)
    {
        hr = mPlaybackSession->pause();
    }
    else if (SvcTranscodeSession::Paused == state)
    {
        hr = mPlaybackSession->resume();
    }
    else if (SvcTranscodeSession::Stopped == state)
    {
        hr = mPlaybackSession->play();
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
void SvcTranscodeDlg::onBnClickedButtonStop()
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
 *  @fn     onRadioBtn1Select
 *  @brief  Called on clicking the SVC layer selection button
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void SvcTranscodeDlg::onRadioBtn1Select()
{
    HRESULT hr = mPlaybackSession->setSvcLayersNumber(1);
    if (FAILED(hr))
    {
        MessageBox(_T("Failed to select number of SVC layers"), _T("Error"),
                        MB_OK | MB_ICONERROR);
    }
}
/** 
 *******************************************************************************
 *  @fn     onRadioBtn2Select
 *  @brief  Called on clicking the SVC layer selection button
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void SvcTranscodeDlg::onRadioBtn2Select()
{
    HRESULT hr = mPlaybackSession->setSvcLayersNumber(2);
    if (FAILED(hr))
    {
        MessageBox(_T("Failed to select number of SVC layers"), _T("Error"),
                        MB_OK | MB_ICONERROR);
    }
}
/** 
 *******************************************************************************
 *  @fn     onRadioBtn3Select
 *  @brief  Called on clicking the SVC layer selection button
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void SvcTranscodeDlg::onRadioBtn3Select()
{
    HRESULT hr = mPlaybackSession->setSvcLayersNumber(3);
    if (FAILED(hr))
    {
        MessageBox(_T("Failed to select number of SVC layers"), _T("Error"),
                        MB_OK | MB_ICONERROR);
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
void SvcTranscodeDlg::updatePlayerControls(
                SvcTranscodeSession::State playerState)
{
    CWnd* openButton = GetDlgItem(IDC_BUTTON_OPEN);
    CWnd* playButton = GetDlgItem(IDC_BUTTON_PLAY);
    CWnd* stopButton = GetDlgItem(IDC_BUTTON_STOP);
    /**************************************************************************
     * Update the player buttons depending on the state of transcode session    *
     **************************************************************************/
    switch (playerState)
    {
    case SvcTranscodeSession::Playing:
        openButton->EnableWindow(true);
        playButton->EnableWindow(true);
        playButton->SetWindowText(_T("Pause"));
        stopButton->EnableWindow(true);
        break;

    case SvcTranscodeSession::Paused:
        openButton->EnableWindow(true);
        playButton->EnableWindow(true);
        playButton->SetWindowText(_T("Continue"));
        stopButton->EnableWindow(true);
        break;

    case SvcTranscodeSession::Stopped:
        openButton->EnableWindow(true);
        playButton->EnableWindow(true);
        playButton->SetWindowText(_T("Play"));
        stopButton->EnableWindow(false);
        break;

    case SvcTranscodeSession::PlayPending:
        LOGHRESULT(0, "PlayPending");
        openButton->EnableWindow(false);
        playButton->EnableWindow(false);
        stopButton->EnableWindow(false);
        break;

    case SvcTranscodeSession::PausePending:
        LOGHRESULT(0, "PausePending");
        openButton->EnableWindow(false);
        playButton->EnableWindow(false);
        stopButton->EnableWindow(false);
        break;

    case SvcTranscodeSession::Building:
        LOGHRESULT(0, "Building");
        openButton->EnableWindow(false);
        playButton->EnableWindow(false);
        stopButton->EnableWindow(false);
        break;

    case SvcTranscodeSession::Ready:
        LOGHRESULT(0, "Ready");
        openButton->EnableWindow(false);
        playButton->EnableWindow(false);
        stopButton->EnableWindow(false);
        break;

    case SvcTranscodeSession::StopPending:
        LOGHRESULT(0, "StopPending");
        openButton->EnableWindow(false);
        playButton->EnableWindow(false);
        stopButton->EnableWindow(false);
        break;

    case SvcTranscodeSession::Failed:
        LOGHRESULT(0, "Failed");

        openButton->EnableWindow(false);
        playButton->EnableWindow(false);
        stopButton->EnableWindow(false);
        break;
    default:
        LOGHRESULT(0, "default");

        openButton->EnableWindow(false);
        playButton->EnableWindow(false);
        stopButton->EnableWindow(false);
        break;

    }
}
/** 
 *******************************************************************************
 *  @fn     SubtractTimes
 *  @brief  subtract function
 *
 *  @returns ULONGLONG : time
 *******************************************************************************
 */
ULONGLONG SubtractTimes(const FILETIME& ftA, const FILETIME& ftB)
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
 *  @fn     GetCpuUsage
 *  @brief  Returns the cpu usage the timer
 *
 *  @returns cpu usage 
 *******************************************************************************
 */
std::pair<float, unsigned long long> GetCpuUsage()
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

    ULONGLONG systemKernelDiff = SubtractTimes(systemKernel,
                    previousSystemKernelTime);
    ULONGLONG systemUserDiff =
                    SubtractTimes(systemUser, previousSystemUserTime);
    ULONGLONG systemTotal = systemKernelDiff + systemUserDiff;

    ULONGLONG processKernelDiff = SubtractTimes(processKernel,
                    previousProcessKernelTime);
    ULONGLONG processUserDiff = SubtractTimes(processUser,
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
 *  @brief  Updates the timer
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void SvcTranscodeDlg::OnTimer(UINT_PTR nIDEvent)
{
    std::pair<float, unsigned long long> cpuUsage = GetCpuUsage();

    CString cpuUsageVal;
    cpuUsageVal.Format(_T("CPU usage: %.1f"), cpuUsage.first);

    CWnd* cpuUsageLabel = GetDlgItem(IDC_STATIC_CPU_USAGE);
    cpuUsageLabel->SetWindowText(cpuUsageVal);

    if (nullptr == mPlaybackSession)
    {
        return;
    }

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
    /**************************************************************************
     * duration is measured in 100 nanosecond intervals (10E-7).               *
     **************************************************************************/
    const unsigned rate = 10000000U;

    CTimeSpan durationTS(static_cast<time_t> (duration / rate));
    CTimeSpan timeTS(static_cast<time_t> (time / rate));

    CString positionVal = timeTS.Format(_T("%H:%M:%S")) + _T(" / ")
                    + durationTS.Format(_T("%H:%M:%S"));

    CWnd* curerntTimeLabel = GetDlgItem(IDC_STATIC_CURRENT_TIME);
    curerntTimeLabel->SetWindowText(positionVal);
    (void) nIDEvent; // unused
}
/** 
 *******************************************************************************
 *  @fn     onHScroll
 *  @brief  On changing the scroll buttons
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
void SvcTranscodeDlg::onHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* /*pScrollBar*/)
{
    UpdateData();
}
