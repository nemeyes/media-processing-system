
// ParallelRecordClient.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CParallelRecordClientApp:
// See ParallelRecordClient.cpp for the implementation of this class
//

class CParallelRecordClientApp : public CWinApp
{
public:
	CParallelRecordClientApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CParallelRecordClientApp theApp;