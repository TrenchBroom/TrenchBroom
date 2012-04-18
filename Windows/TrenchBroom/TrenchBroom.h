
// TrenchBroom.h : main header file for the TrenchBroom application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CTrenchBroomApp:
// See TrenchBroom.cpp for the implementation of this class
//

class CTrenchBroomApp : public CWinApp
{
public:
	CTrenchBroomApp();

protected:
	CMultiDocTemplate* m_pDocTemplate;
public:

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	afx_msg void OnFileNewFrame();
	afx_msg void OnFileNew();
	DECLARE_MESSAGE_MAP()
};

extern CTrenchBroomApp theApp;
