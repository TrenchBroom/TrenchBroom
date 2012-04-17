
// TrenchBroom.h: Hauptheaderdatei für die TrenchBroom-Anwendung
//
#pragma once

#ifndef __AFXWIN_H__
	#error "'stdafx.h' vor dieser Datei für PCH einschließen"
#endif

#include "resource.h"       // Hauptsymbole


// CTrenchBroomApp:
// Siehe TrenchBroom.cpp für die Implementierung dieser Klasse
//

class CTrenchBroomApp : public CWinApp
{
public:
	CTrenchBroomApp();

	CArray<HWND, HWND> m_aryFrames;
public:

// Überschreibungen
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementierung
protected:
	HMENU  m_hMDIMenu;
	HACCEL m_hMDIAccel;

public:
	afx_msg void OnAppAbout();
	afx_msg void OnFileNewFrame();
	DECLARE_MESSAGE_MAP()
};

extern CTrenchBroomApp theApp;
