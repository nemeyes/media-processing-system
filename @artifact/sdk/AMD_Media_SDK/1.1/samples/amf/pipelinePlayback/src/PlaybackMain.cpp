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
 * @file <PlaybackMain.cpp>
 *
 * @brief Source file for the playback
 *
 ********************************************************************************
 */

#include "stdafx.h"
#include "PlaybackMain.h"
#include <objbase.h>
#include <Commdlg.h>
#include <Windowsx.h>
#include <CommCtrl.h>

#include "Debug.h"

#include "PlaybackPipeline.h"
#include "CmdLogger.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst; // current instance
TCHAR szTitle[MAX_LOADSTRING]; // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

static PlaybackPipeline s_Pipeline;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, HWND*, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void FileOpen(HWND hwnd);
void UpdateMenuItems(HMENU hMenu);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                HINSTANCE hPrevInstance,
                LPTSTR lpCmdLine,
                int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    ::CoInitializeEx(NULL,COINIT_MULTITHREADED);

    amf::AMFAssertsEnable(false);
    //    amf::AMFEnablePerformanceMonitor(true);

    MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_PIPELINEPLAYBACK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    HWND hWnd = NULL;

    // Perform application initialization:
    if (!InitInstance (hInstance, &hWnd, nCmdShow))
    {
        CoUninitialize();
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PIPELINEPLAYBACK));

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CoUninitialize();

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIPELINEPLAYBACK));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_PIPELINEPLAYBACK);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, HWND* phWnd, int nCmdShow)
{
    HWND hWnd = NULL;

    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance,
                    NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    UpdateMenuItems(::GetMenu(hWnd));
    *phWnd = hWnd;

    return TRUE;
}

void UpdateMenuItems(HMENU hMenu)
{
    amf::AMF_MEMORY_TYPE presenterType = amf::AMF_MEMORY_DX9;

    {
        amf_int64 engineInt = amf::AMF_MEMORY_UNKNOWN;
        if (s_Pipeline.GetParam(PlaybackPipeline::PARAM_NAME_PRESENTER,
                        engineInt) == AMF_OK)
        {
            if (amf::AMF_MEMORY_UNKNOWN != engineInt)
            {
                presenterType = (amf::AMF_MEMORY_TYPE) engineInt;
            }
        }
        else
        {
            s_Pipeline.SetParam(PlaybackPipeline::PARAM_NAME_PRESENTER,
                            presenterType);
        }
    }

    CheckMenuItem(hMenu, ID_OPTIONS_PRESENTER_DX11, MF_BYCOMMAND
                    | (presenterType == amf::AMF_MEMORY_DX11 ? MF_CHECKED
                                    : MF_UNCHECKED));
    CheckMenuItem(hMenu, ID_OPTIONS_PRESENTER_DX9, MF_BYCOMMAND
                    | (presenterType == amf::AMF_MEMORY_DX9 ? MF_CHECKED
                                    : MF_UNCHECKED));
    CheckMenuItem(hMenu,ID_OPTIONS_PRESENTER_OPENGL,    MF_BYCOMMAND| ( presenterType == amf::AMF_MEMORY_OPENGL ? MF_CHECKED: MF_UNCHECKED));
									
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND    - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY    - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ID_FILE_OPEN:
            FileOpen(hWnd);
            break;
        case ID_PLAYBACK_PLAY:
            if (s_Pipeline.GetState() == PipelineStateEof
                            || s_Pipeline.GetState() == PipelineStateNotReady)
            {
                if (s_Pipeline.Init(hWnd) == AMF_OK)
                {
                    s_Pipeline.Play();
                    UpdateMenuItems(::GetMenu(hWnd));
                }
            }
            else if (s_Pipeline.GetState() == PipelineStateRunning)
            {
                s_Pipeline.Play();
            }

            break;
        case ID_PLAYBACK_PAUSE:
            s_Pipeline.Pause();
            break;
        case ID_PLAYBACK_STEP:
            s_Pipeline.Step();
            break;
        case ID_PLAYBACK_STOP:
            s_Pipeline.Stop();
            break;
        case ID_OPTIONS_PRESENTER_DX11:
            s_Pipeline.SetParam(PlaybackPipeline::PARAM_NAME_PRESENTER,
                            amf::AMF_MEMORY_DX11);
            UpdateMenuItems(::GetMenu(hWnd));
            break;
        case ID_OPTIONS_PRESENTER_DX9:
            s_Pipeline.SetParam(PlaybackPipeline::PARAM_NAME_PRESENTER,
                            amf::AMF_MEMORY_DX9);
            UpdateMenuItems(::GetMenu(hWnd));
            break;
        case ID_OPTIONS_PRESENTER_OPENGL:
            s_Pipeline.SetParam(PlaybackPipeline::PARAM_NAME_PRESENTER, amf::AMF_MEMORY_OPENGL);
            UpdateMenuItems(::GetMenu(hWnd));
            break;
		

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        s_Pipeline.Terminate();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void FileOpen(HWND hwnd)
{
    OPENFILENAME ofn; // common dialog box structure
    WCHAR szFile[260]; // buffer for file name

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = _countof(szFile);
    ofn.lpstrFilter = L"H.264 Video\0*.h264;*.264;\0All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        s_Pipeline.SetParam(PlaybackPipeline::PARAM_NAME_INPUT, ofn.lpstrFile);
        s_Pipeline.Stop();
        if (s_Pipeline.Init(hwnd) == AMF_OK)
        {
            s_Pipeline.Play();
            UpdateMenuItems(::GetMenu(hwnd));
        }
    }
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR) TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR) TRUE;
        }
        break;
    }
    return (INT_PTR) FALSE;
}
