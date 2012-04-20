
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
	ON_WM_CREATE()
	ON_WM_DESTROY()
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
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	return CView::PreCreateWindow(cs);
}

// CMapView drawing

void CMapView::OnDraw(CDC* /*pDC*/)
{
	wglMakeCurrent(m_deviceContext, m_openGLContext);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glFlush();
	SwapBuffers(m_deviceContext);
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


int CMapView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	static PIXELFORMATDESCRIPTOR descriptor =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,								// version
		PFD_DRAW_TO_WINDOW |			//draw to window
		PFD_SUPPORT_OPENGL |			// use OpenGL
		PFD_DOUBLEBUFFER,				// use double buffering
		PFD_TYPE_RGBA,					// use RGBA pixels
		32,								// 32 bit color depth
		0, 0, 0,						// RGB bits and shifts
		0, 0, 0,						// ...
		0, 0,							// alpha buffer info
		0, 0, 0, 0, 0,					// accumulation buffer
		32,								// depth buffer
		8,								// stencil buffer
		0,								// aux buffers
		PFD_MAIN_PLANE,					// layer type
		0,								// reserved (must be 0)
		0,								// no layer mask
		0,								// no visible mask
		0								// no damage mask
	};

	m_deviceContext = GetDC()->m_hDC;
	int pixelFormat = ChoosePixelFormat(m_deviceContext, &descriptor);
	SetPixelFormat(m_deviceContext, pixelFormat, &descriptor);
	m_openGLContext = wglCreateContext(m_deviceContext);
	wglMakeCurrent(m_deviceContext, m_openGLContext);

	// m_editorGui = new TrenchBroom::Gui::EditorGui();

	return 0;
}


void CMapView::OnDestroy()
{
	CView::OnDestroy();

	delete m_editorGui;

	wglMakeCurrent(m_deviceContext, m_openGLContext);
	wglDeleteContext(m_openGLContext);
}
