
// ParallelRecordServer.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CParallelRecordServerApp:
// �� Ŭ������ ������ ���ؼ��� ParallelRecordServer.cpp�� �����Ͻʽÿ�.
//

class CParallelRecordServerApp : public CWinApp
{
public:
	CParallelRecordServerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CParallelRecordServerApp theApp;