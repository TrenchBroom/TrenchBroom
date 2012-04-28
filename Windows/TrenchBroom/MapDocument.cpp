/*
 Copyright (C) 2010-2012 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "TrenchBroom.h"
#endif

#include "MapDocument.h"
#include "ProgressDialog.h"
#include "Utilities/Utils.h"
#include <cassert>

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMapDocument

IMPLEMENT_DYNCREATE(CMapDocument, CDocument)

BEGIN_MESSAGE_MAP(CMapDocument, CDocument)
END_MESSAGE_MAP()


// CMapDocument construction/destruction

CMapDocument::CMapDocument() : m_editor(NULL)
{
	char appPath [MAX_PATH] = "";

	::GetModuleFileName(0, appPath, sizeof(appPath) - 1);
	string appDirectory = TrenchBroom::deleteLastPathComponent(appPath);

	string definitionPath	= TrenchBroom::appendPath(appDirectory, "../../Resources/Defs/quake.def");
	string palettePath		= TrenchBroom::appendPath(appDirectory, "../../Resources/Graphics/QuakePalette.lmp");

//	string definitionPath	= TrenchBroom::appendPath(appDirectory, "quake.def");
//	string palettePath		= TrenchBroom::appendPath(appDirectory, "QuakePalette.lmp");

	assert(TrenchBroom::fileExists(definitionPath));
	assert(TrenchBroom::fileExists(palettePath));

	m_editor = new TrenchBroom::Controller::Editor(definitionPath, palettePath);
}

CMapDocument::~CMapDocument()
{
	if (m_editor != NULL)
		delete m_editor;
}

BOOL CMapDocument::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

BOOL CMapDocument::OnOpenDocument(LPCTSTR lpszPathName)
{
	ProgressDialog progressDialog(NULL);
	progressDialog.Create(ProgressDialog::IDD);
	progressDialog.ShowWindow(SW_SHOW);

	std::string path(lpszPathName);
	m_editor->loadMap(path, &progressDialog);

	progressDialog.DestroyWindow();
	return TRUE;
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CMapDocument::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CMapDocument::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CMapDocument::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMapDocument diagnostics

#ifdef _DEBUG
void CMapDocument::AssertValid() const
{
	CDocument::AssertValid();
}

void CMapDocument::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMapDocument commands


TrenchBroom::Controller::Editor& CMapDocument::editor()
{
	return *m_editor;
}
