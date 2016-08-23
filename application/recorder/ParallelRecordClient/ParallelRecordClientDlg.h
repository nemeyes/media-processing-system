
// ParallelRecordClientDlg.h : header file
//

#pragma once

#if !defined(WITH_DISABLE_PRMC)
#include <ParallelRecordMediaClient.h>
#pragma comment(lib, "ParallelRecordMediaClient.lib")
#endif

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
	int PRMC_index;
	int PRMC_RTSP_index;

	CEdit _parallel_recorder_address;
	CEdit _parallel_recorder_port_number;
	CEdit _parallel_recorder_username;
	CEdit _parallel_recorder_password;

	CEdit _uuid;
	CEdit _rtsp_address;
	CEdit _rtsp_username;
	CEdit _rtsp_password;

	CComboBox _recording_years;
	CComboBox _recording_months;
	CComboBox _recording_days;
	CComboBox _recording_hours;
	CComboBox _recording_minutes;
	CComboBox _recording_seconds;

	CEdit _manual_recording_year;
	CEdit _manual_recording_month;
	CEdit _manual_recording_day;
	CEdit _manual_recording_hour;
	CEdit _manual_recording_minute;
	CEdit _manual_recording_second;
	
	CEdit _recording_play_scale;
	CEdit _recording_play_duration;

	CComboBox _osd_enable;
	CEdit _osd_x;
	CEdit _osd_y;
	CEdit _exp_uuid;
	CEdit _exp_begin_year;
	CEdit _exp_begin_month;
	CEdit _exp_begin_day;
	CEdit _exp_begin_hour;
	CEdit _exp_begin_minute;
	CEdit _exp_begin_second;
	CEdit _exp_end_year;
	CEdit _exp_end_month;
	CEdit _exp_end_day;
	CEdit _exp_end_hour;
	CEdit _exp_end_minute;
	CEdit _exp_end_second;

	BOOL _paused;
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

public:
	afx_msg void OnBnClickedButtonParallelRecorderConnect();
	afx_msg void OnBnClickedButtonParallelRecorderDisconnect();
	afx_msg void OnBnClickedButtonGetRecordingYears();
	afx_msg void OnBnClickedButtonStartPlayback();
	afx_msg void OnBnClickedButtonStopPlayback();
	afx_msg void OnBnClickedButtonPausePlayback();
	afx_msg void OnCbnSelchangeComboRecordingYears();
	afx_msg void OnCbnSelchangeComboRecordingMonths();
	afx_msg void OnCbnSelchangeComboRecordingDays();
	afx_msg void OnCbnSelchangeComboRecordingHours();
	afx_msg void OnCbnSelchangeComboMinutes();
	afx_msg void OnBnClickedButtonManualStartPlayback();
	afx_msg void OnBnClickedButtonManualStopPlayback();
	afx_msg void OnBnClickedButtonManualPausePlayback();
	afx_msg void OnBnClickedButtonRtspPlay();
	afx_msg void OnBnClickedButtonRtspStop();
	afx_msg void OnBnClickedButtonStartExport();
	afx_msg void OnBnClickedButtonStopExport();
};
