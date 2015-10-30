#include <new>
#include <windows.h>
#include <dshow.h>
#include "dk_dshow_player.h"

#pragma comment(lib, "strmiids")

dk_dshow_player * g_player = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK OnGraphEvent(HWND hwnd, long eventCode, LONG_PTR param1, LONG_PTR param2);
void OnChar(HWND hwnd, wchar_t c);
void OnFileOpen(HWND hwnd);
void OnPaint(HWND hwnd);
void OnSize(HWND hwnd);
void NotifyError(HWND hwnd, PCWSTR pszMessage);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Entrix App Platform Client Class";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	RECT fullrect = { 0 };
	SetRect(&fullrect, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Entrix App Platform Client", WS_OVERLAPPEDWINDOW, fullrect.left, fullrect.top, fullrect.right, fullrect.bottom, NULL, NULL, hInstance, NULL);
	if (hwnd == NULL)
	{
		NotifyError(NULL, L"CreateWindowEx failed.");
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	// Run the message loop.

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CHAR:
		OnChar(hwnd, (wchar_t)wParam);
		return 0;

	case WM_CREATE:
		g_player = new (std::nothrow) dk_dshow_player(hwnd);
		if (g_player == NULL)
		{
			return -1;
		}
		return 0;

	case WM_DESTROY:
		delete g_player;
		PostQuitMessage(0);
		return 0;

	case WM_DISPLAYCHANGE:
		g_player->on_change_displaymode();
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		OnPaint(hwnd);
		return 0;

	case WM_SIZE:
		OnSize(hwnd);
		return 0;

	case WM_GRAPH_EVENT:
		g_player->handle_graphevent(OnGraphEvent);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint(hwnd, &ps);

	if (g_player->state() != dk_dshow_player::STATE_NO_GRAPH && g_player->has_video())
	{
		// The player has video, so ask the player to repaint. 
		g_player->repaint(hdc);
	}
	else
	{
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
	}

	EndPaint(hwnd, &ps);
}

void OnSize(HWND hwnd)
{
	if (g_player)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		g_player->update_video_windows(&rc);
	}
}

void CALLBACK OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2)
{
	switch (evCode)
	{
	case EC_COMPLETE:
	case EC_USERABORT:
		g_player->stop();
		break;

	case EC_ERRORABORT:
		NotifyError(hwnd, L"Playback error.");
		g_player->stop();
		break;
	}
}

void OnChar(HWND hwnd, wchar_t c)
{
	switch (c)
	{
	case L'o':
	case L'O':
		OnFileOpen(hwnd);
		break;

	case L'p':
	case L'P':
		if (g_player->state() == dk_dshow_player::STATE_RUNNING)
		{
			g_player->pause();
		}
		else
		{
			g_player->play();
		}
		break;
	}
}

void OnFileOpen(HWND hwnd)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	WCHAR szFileName[MAX_PATH];
	szFileName[0] = L'\0';

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = L"All (*.*)/0*.*/0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;

	HRESULT hr;

	if (GetOpenFileName(&ofn))
	{
		hr = g_player->open_file(szFileName);

		InvalidateRect(hwnd, NULL, FALSE);

		if (SUCCEEDED(hr))
		{
			// If this file has a video stream, notify the video renderer 
			// about the size of the destination rectangle.
			OnSize(hwnd);
		}
		else
		{
			NotifyError(hwnd, TEXT("Cannot open this file."));
		}
	}
}

void NotifyError(HWND hwnd, PCWSTR pszMessage)
{
	MessageBox(hwnd, pszMessage, TEXT("Error"), MB_OK | MB_ICONERROR);
}