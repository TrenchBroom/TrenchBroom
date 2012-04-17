
// ChildView.h: Schnittstelle der Klasse CChildView
//


#pragma once


// CChildView-Fenster

class CChildView : public CWnd
{
// Konstruktion
public:
	CChildView();

// Attribute
public:

// Vorgänge
public:

// Überschreibungen
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementierung
public:
	virtual ~CChildView();

	// Generierte Funktionen für die Meldungstabellen
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};

