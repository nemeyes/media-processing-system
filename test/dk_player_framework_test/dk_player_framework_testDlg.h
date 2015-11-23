
// dk_player_framework_testDlg.h : header file
//

#pragma once
#include <dk_player_framework.h>
#include "afxwin.h"
#include "afxcmn.h"


// Cdk_player_framework_testDlg dialog
class Cdk_player_framework_testDlg : public CDialogEx
{
// Construction
public:
	Cdk_player_framework_testDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DK_PLAYER_FRAMEWORK_TEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
	dk_player_framework _player;
	CRect _original_rect;
	BOOL _fullscreen;
	CString _filename;
	long long _play_elapsed;
	CComboBox _dxva2_decoder_guids;

private:
	static void CALLBACK OnGraphEvent(HWND hwnd, long eventCode, LONG_PTR param1, LONG_PTR param2);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonOpenFile();
	afx_msg void OnBnClickedButtonOpenRtsp();
	afx_msg void OnBnClickedButtonHsl();
	afx_msg void OnBnClickedButtonOpenRtmp();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedCheckAspectRatio();
	DECLARE_MESSAGE_MAP()

private:
	CProgressCtrl _progress_play;
public:
	afx_msg void OnAppCommand(CWnd* pWnd, UINT nCmd, UINT nDevice, UINT nKey);
};
