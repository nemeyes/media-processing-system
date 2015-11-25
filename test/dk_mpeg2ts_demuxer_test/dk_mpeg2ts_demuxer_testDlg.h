
// dk_mpeg2ts_demuxer_testDlg.h : header file
//

#pragma once

#include <dk_mpeg2ts_demuxer.h>

// Cdk_mpeg2ts_demuxer_testDlg dialog
class Cdk_mpeg2ts_demuxer_testDlg : public CDialogEx
{
// Construction
public:
	Cdk_mpeg2ts_demuxer_testDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DK_MPEG2TS_DEMUXER_TEST_DIALOG };


private:
	CString _filename;
	HANDLE _ts_demuxing_thread;
	BOOL _ts_demuxing_run;

private:
	static unsigned __stdcall DemuxingProc(void * param);
	void demuxing(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonFile();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
};
