
// ParallelRecordClientDlg.h : header file
//

#pragma once

#include <ParallelRecordMediaClient.h>
#include "afxwin.h"

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
	//int index_2;
	//int index_3;
	//int index_4;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL DestroyWindow();
private:
	CEdit _parallel_recorder_address;
	CEdit _parallel_recorder_port_number;
	CEdit _parallel_recorder_username;
	CEdit _parallel_recorder_password;
public:
	afx_msg void OnBnClickedButtonParallelRecorderConnect();
	afx_msg void OnBnClickedButtonParallelRecorderDisconnect();
};
