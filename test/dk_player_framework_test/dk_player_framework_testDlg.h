
// dk_player_framework_testDlg.h : header file
//

#pragma once
#include <dk_player_framework.h>


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

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedCheckAspectRatio();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};
