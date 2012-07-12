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

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

// CTrenchBroomApp:
// See TrenchBroom.cpp for the implementation of this class
//

namespace TrenchBroom {
	namespace Controller {
		class Editor;
	}
}

class CTrenchBroomApp : public CWinApp
{
public:
	CTrenchBroomApp();
protected:
	CMultiDocTemplate* m_pDocTemplate;
	TrenchBroom::Controller::Editor* currentEditor();
	bool mapViewFocused();
public:
	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();

	DECLARE_MESSAGE_MAP()

// Implementation
	afx_msg void OnAppAbout();
	afx_msg void OnFileNewFrame();
	afx_msg void OnFileNew();
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnToolsOptions();
	afx_msg void OnToolsToggleVertexTool();
	afx_msg void OnUpdateToolsToggleVertexTool(CCmdUI *pCmdUI);
	afx_msg void OnToolsToggleEdgeTool();
	afx_msg void OnUpdateToolsToggleEdgeTool(CCmdUI *pCmdUI);
	afx_msg void OnToolsToggleFaceTool();
	afx_msg void OnUpdateToolsToggleFaceTool(CCmdUI *pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditSelectAll(CCmdUI *pCmdUI);
	afx_msg void OnEditSelectEntity();
	afx_msg void OnUpdateEditSelectEntity(CCmdUI *pCmdUI);
	afx_msg void OnEditSelectTouching();
	afx_msg void OnUpdateEditSelectTouching(CCmdUI *pCmdUI);
	afx_msg void OnEditSelectNone();
	afx_msg void OnUpdateEditSelectNone(CCmdUI *pCmdUI);
};

extern CTrenchBroomApp theApp;
