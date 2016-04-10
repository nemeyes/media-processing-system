
// ParallelRecordClientDlg.h : header file
//

#pragma once

#include <dk_media_player_framework.h>

// CParallelRecordClientDlg dialog
class CParallelRecordClientDlg : public CDialogEx
{
// Construction
public:
	CParallelRecordClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PARALLELRECORDCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	int index_1;
	int index_2;
	int index_3;
	int index_4;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL DestroyWindow();
};
