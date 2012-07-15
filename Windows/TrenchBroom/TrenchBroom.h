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
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
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
	afx_msg void OnViewIsolateSelection();
	afx_msg void OnUpdateViewIsolateSelection(CCmdUI *pCmdUI);
	afx_msg void OnGridShowGrid();
	afx_msg void OnGridSnapToGrid();
	afx_msg void OnGridGridSize1();
	afx_msg void OnGridGridSize2();
	afx_msg void OnGridGridSize4();
	afx_msg void OnGridGridSize8();
	afx_msg void OnGridGridSize16();
	afx_msg void OnGridGridSize32();
	afx_msg void OnGridGridSize64();
	afx_msg void OnGridGridSize128();
	afx_msg void OnGridGridSize256();
	afx_msg void OnUpdateGridItem(CCmdUI *pCmdUI);
	afx_msg void OnCameraMoveForward();
	afx_msg void OnCameraMoveBackward();
	afx_msg void OnCameraMoveLeft();
	afx_msg void OnCameraMoveRight();
	afx_msg void OnCameraMoveUp();
	afx_msg void OnCameraMoveDown();
	afx_msg void OnUpdateCameraItem(CCmdUI *pCmdUI);
	afx_msg void OnObjectRoll90Cw();
	afx_msg void OnObjectRoll90Ccw();
	afx_msg void OnObjectPitch90Cw();
	afx_msg void OnObjectPitch90Ccw();
	afx_msg void OnObjectYaw90Cw();
	afx_msg void OnObjectYaw90Ccw();
	afx_msg void OnObjectFlipHorizontally();
	afx_msg void OnObjectFlipVertically();
	afx_msg void OnObjectDuplicate();
	afx_msg void OnUpdateObjectItem(CCmdUI *pCmdUI);
	afx_msg void OnObjectEnlargeBrushes();
	afx_msg void OnUpdateObjectEnlargeBrushes(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTextureItem(CCmdUI *pCmdUI);
	afx_msg void OnEditCursorUp();
	afx_msg void OnEditCursorDown();
	afx_msg void OnEditCursorLeft();
	afx_msg void OnEditCursorRight();
	afx_msg void OnEditPageUp();
	afx_msg void OnEditPageDown();
};

extern CTrenchBroomApp theApp;
