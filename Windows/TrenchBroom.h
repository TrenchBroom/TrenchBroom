
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

	CArray<HWND, HWND> m_aryFrames;
public:

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
protected:
	HMENU  m_hMDIMenu;
	HACCEL m_hMDIAccel;

public:
	afx_msg void OnAppAbout();
	afx_msg void OnFileNewFrame();
	DECLARE_MESSAGE_MAP()
};

extern CTrenchBroomApp theApp;
