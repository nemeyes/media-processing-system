
// ParallelRecordServerDlg.h : 헤더 파일
//

#pragma once

// CParallelRecordServerDlg 대화 상자
#define TRAY_NOTIFY        (WM_APP + 100)
class CParallelRecordServerDlg : public CDialogEx
{
// 생성입니다.
public:
	CParallelRecordServerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_PARALLELRECORDSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

private:
	void Position2Center(void);
	void StartRecording(void);
	void StopRecording(void);

	LRESULT OnTrayIconClick(WPARAM wParam, LPARAM lParam);
	void EnableTray(BOOL enable);


	BOOL _is_recording;
// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStartRecord();
	afx_msg void OnBnClickedButtonStopRecord();
	afx_msg void OnBnClickedButtonToTray();
	afx_msg void OnTrayStartRecording();
	afx_msg void OnTrayStopRecording();
	afx_msg void OnTrayExit();
	virtual BOOL DestroyWindow();
	afx_msg void OnUpdateTrayStartRecording(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTrayStopRecording(CCmdUI *pCmdUI);
	afx_msg void OnTrayShutdown();
};
