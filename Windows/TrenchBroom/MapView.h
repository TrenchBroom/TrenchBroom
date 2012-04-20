
// MapView.h : interface of the CMapView class
//

#pragma once
#include "GL/GLee.h"
#include "GUI/EditorGui.h"

class CMapView : public CView
{
protected: // create from serialization only
	CMapView();
	DECLARE_DYNCREATE(CMapView)

// Attributes
public:
	CMapDocument* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CMapView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
	HDC m_deviceContext;
	HGLRC m_openGLContext;
	TrenchBroom::Gui::EditorGui* m_editorGui;
public:
	afx_msg void OnDestroy();
};

#ifndef _DEBUG  // debug version in MapView.cpp
inline CMapDocument* CMapView::GetDocument() const
   { return reinterpret_cast<CMapDocument*>(m_pDocument); }
#endif

