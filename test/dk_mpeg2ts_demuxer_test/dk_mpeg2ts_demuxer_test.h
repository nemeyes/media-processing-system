
// dk_mpeg2ts_demuxer_test.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cdk_mpeg2ts_demuxer_testApp:
// See dk_mpeg2ts_demuxer_test.cpp for the implementation of this class
//

class Cdk_mpeg2ts_demuxer_testApp : public CWinApp
{
public:
	Cdk_mpeg2ts_demuxer_testApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cdk_mpeg2ts_demuxer_testApp theApp;