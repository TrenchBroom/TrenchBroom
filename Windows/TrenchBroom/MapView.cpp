
// MapView.cpp : implementation of the CMapView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "TrenchBroom.h"
#endif

#include "MapDocument.h"
#include "MapView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMapView

IMPLEMENT_DYNCREATE(CMapView, CView)

BEGIN_MESSAGE_MAP(CMapView, CView)
END_MESSAGE_MAP()

// CMapView construction/destruction

CMapView::CMapView()
{
	// TODO: add construction code here

}

CMapView::~CMapView()
{
}

BOOL CMapView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMapView drawing

void CMapView::OnDraw(CDC* /*pDC*/)
{
	CMapDocument* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CMapView diagnostics

#ifdef _DEBUG
void CMapView::AssertValid() const
{
	CView::AssertValid();
}

void CMapView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMapDocument* CMapView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMapDocument)));
	return (CMapDocument*)m_pDocument;
}
#endif //_DEBUG


// CMapView message handlers
