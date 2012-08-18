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

#include <string>

namespace TrenchBroom {
	namespace Controller {
		class Editor;
	}
}

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HACCEL m_originalAccelTable;
	BOOL m_entityMenuCreated;
	TrenchBroom::Controller::Editor* currentEditor();
	bool mapViewFocused();
	bool validateCommand(UINT id);
	void copyToClipboard(const std::string& data);
	std::string clipboardContents();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnUpdateMenuItem(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnToolsOptions();
	afx_msg void OnToolsToggleVertexTool();
	afx_msg void OnUpdateToolsToggleVertexTool(CCmdUI* pCmdUI);
	afx_msg void OnToolsToggleEdgeTool();
	afx_msg void OnUpdateToolsToggleEdgeTool(CCmdUI* pCmdUI);
	afx_msg void OnToolsToggleFaceTool();
	afx_msg void OnUpdateToolsToggleFaceTool(CCmdUI* pCmdUI);
	afx_msg void OnToolsToggleTextureLock();
	afx_msg void OnUpdateToolsToggleTextureLock(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditSelectSiblings();
	afx_msg void OnEditSelectTouching();
	afx_msg void OnEditSelectNone();
	afx_msg void OnViewIsolateSelection();
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
	afx_msg void OnCameraMoveForward();
	afx_msg void OnCameraMoveBackward();
	afx_msg void OnCameraMoveLeft();
	afx_msg void OnCameraMoveRight();
	afx_msg void OnCameraMoveUp();
	afx_msg void OnCameraMoveDown();
	afx_msg void OnObjectRoll90Cw();
	afx_msg void OnObjectRoll90Ccw();
	afx_msg void OnObjectPitch90Cw();
	afx_msg void OnObjectPitch90Ccw();
	afx_msg void OnObjectYaw90Cw();
	afx_msg void OnObjectYaw90Ccw();
	afx_msg void OnObjectFlipHorizontally();
	afx_msg void OnObjectFlipVertically();
	afx_msg void OnObjectDuplicate();
	afx_msg void OnObjectEnlargeBrushes();
	afx_msg void OnUpdateObjectEnlargeBrushes(CCmdUI *pCmdUI);
	afx_msg void OnEditCursorUp();
	afx_msg void OnEditCursorDown();
	afx_msg void OnEditCursorLeft();
	afx_msg void OnEditCursorRight();
	afx_msg void OnEditPageUp();
	afx_msg void OnEditPageDown();
	afx_msg void OnCreatePointEntity(UINT nId);
	afx_msg void OnUpdatePointEntityMenuItem(CCmdUI *pCmdUI);
	afx_msg void OnCreateBrushEntity(UINT nId);
	afx_msg void OnUpdateBrushEntityMenuItem(CCmdUI *pCmdUI);
	afx_msg void OnPopupMoveBrushesToEntity();
	afx_msg void OnUpdatePopupMoveBrushesToEntityMenuItem(CCmdUI *pCmdUI);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnInitMenu(CMenu* pMenu);
};


