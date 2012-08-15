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
#include "TrenchBroom.h"

#include "MainFrm.h"
#include "MapDocument.h"
#include "MapView.h"
#include "PreferencesDialog.h"
#include "WinUtilities.h"

#include "Controller/Editor.h"
#include "Controller/InputController.h"
#include "Controller/Options.h"
#include "Model/Assets/Alias.h"
#include "Model/Assets/Bsp.h"
#include "Model/Map/Map.h"
#include "Model/Map/EntityDefinition.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"
#include "IO/Pak.h"
#include "Utilities/Console.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_COMMAND(ID_TOOLS_OPTIONS, &CMainFrame::OnToolsOptions)
	ON_COMMAND(ID_EDIT_UNDO, &CMainFrame::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CMainFrame::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CMainFrame::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CMainFrame::OnUpdateEditRedo)
	ON_COMMAND(ID_TOOLS_TOGGLE_VERTEX_TOOL, &CMainFrame::OnToolsToggleVertexTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_VERTEX_TOOL, &CMainFrame::OnUpdateToolsToggleVertexTool)
	ON_COMMAND(ID_TOOLS_TOGGLE_EDGE_TOOL, &CMainFrame::OnToolsToggleEdgeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_EDGE_TOOL, &CMainFrame::OnUpdateToolsToggleEdgeTool)
	ON_COMMAND(ID_TOOLS_TOGGLE_FACE_TOOL, &CMainFrame::OnToolsToggleFaceTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_FACE_TOOL, &CMainFrame::OnUpdateToolsToggleFaceTool)
	ON_COMMAND(ID_TOOLS_TOGGLE_TEXTURE_LOCK, &CMainFrame::OnToolsToggleTextureLock)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_TEXTURE_LOCK, &CMainFrame::OnUpdateToolsToggleTextureLock)
	ON_COMMAND(ID_EDIT_DELETE, &CMainFrame::OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_EDIT_SELECT_ALL, &CMainFrame::OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_EDIT_SELECT_SIBLINGS, &CMainFrame::OnEditSelectSiblings)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_SIBLINGS, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_EDIT_SELECT_TOUCHING, &CMainFrame::OnEditSelectTouching)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_TOUCHING, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_EDIT_SELECT_NONE, &CMainFrame::OnEditSelectNone)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_NONE, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_VIEW_ISOLATE_SELECTION, &CMainFrame::OnViewIsolateSelection)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ISOLATE_SELECTION, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_SHOW_GRID, &CMainFrame::OnGridShowGrid)
	ON_UPDATE_COMMAND_UI(ID_GRID_SHOW_GRID, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_SNAP_TO_GRID, &CMainFrame::OnGridSnapToGrid)
	ON_UPDATE_COMMAND_UI(ID_GRID_SNAP_TO_GRID, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_1, &CMainFrame::OnGridGridSize1)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_1, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_2, &CMainFrame::OnGridGridSize2)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_2, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_4, &CMainFrame::OnGridGridSize4)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_4, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_8, &CMainFrame::OnGridGridSize8)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_8, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_16, &CMainFrame::OnGridGridSize16)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_16, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_32, &CMainFrame::OnGridGridSize32)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_32, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_64, &CMainFrame::OnGridGridSize64)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_64, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_128, &CMainFrame::OnGridGridSize128)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_128, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_256, &CMainFrame::OnGridGridSize256)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_256, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_CAMERA_MOVE_FORWARD, &CMainFrame::OnCameraMoveForward)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_FORWARD, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_CAMERA_MOVE_BACKWARD, &CMainFrame::OnCameraMoveBackward)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_BACKWARD, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_CAMERA_MOVE_LEFT, &CMainFrame::OnCameraMoveLeft)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_LEFT, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_CAMERA_MOVE_RIGHT, &CMainFrame::OnCameraMoveRight)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_RIGHT, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_CAMERA_MOVE_UP, &CMainFrame::OnCameraMoveUp)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_UP, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_CAMERA_MOVE_DOWN, &CMainFrame::OnCameraMoveDown)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_DOWN, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_MOVE_FORWARD, &CMainFrame::OnEditCursorUp)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_FORWARD, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_MOVE_BACKWARD, &CMainFrame::OnEditCursorDown)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_BACKWARD, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_MOVE_UP, &CMainFrame::OnEditPageUp)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_UP, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_MOVE_DOWN, &CMainFrame::OnEditPageDown)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_DOWN, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_MOVE_LEFT, &CMainFrame::OnEditCursorLeft)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_LEFT, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_MOVE_RIGHT, &CMainFrame::OnEditCursorRight)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_RIGHT, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_ROLL_90_CW, &CMainFrame::OnObjectRoll90Cw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_ROLL_90_CW, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_ROLL_90_CCW, &CMainFrame::OnObjectRoll90Ccw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_ROLL_90_CCW, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_PITCH_90_CW, &CMainFrame::OnObjectPitch90Cw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_PITCH_90_CW, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_PITCH_90_CCW, &CMainFrame::OnObjectPitch90Ccw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_PITCH_90_CCW, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_YAW_90_CW, &CMainFrame::OnObjectYaw90Cw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_YAW_90_CW, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_YAW_90_CCW, &CMainFrame::OnObjectYaw90Ccw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_YAW_90_CCW, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_FLIP_HORIZONTALLY, &CMainFrame::OnObjectFlipHorizontally)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_FLIP_HORIZONTALLY, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_FLIP_VERTICALLY, &CMainFrame::OnObjectFlipVertically)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_FLIP_VERTICALLY, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_DUPLICATE, &CMainFrame::OnObjectDuplicate)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_DUPLICATE, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_OBJECT_ENLARGE_BRUSHES, &CMainFrame::OnObjectEnlargeBrushes)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_ENLARGE_BRUSHES, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_TEXTURE_MOVE_UP, &CMainFrame::OnEditCursorUp)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_MOVE_UP, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_TEXTURE_MOVE_DOWN, &CMainFrame::OnEditCursorDown)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_MOVE_DOWN, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_TEXTURE_MOVE_LEFT, &CMainFrame::OnEditCursorLeft)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_MOVE_LEFT, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_TEXTURE_MOVE_RIGHT, &CMainFrame::OnEditCursorRight)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_MOVE_RIGHT, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_TEXTURE_ROTATE_CW_BY_15, &CMainFrame::OnEditPageUp)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_ROTATE_CW_BY_15, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_TEXTURE_ROTATE_CCW_BY_15, &CMainFrame::OnEditPageDown)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_ROTATE_CCW_BY_15, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_EDIT_CURSOR_UP, &CMainFrame::OnEditCursorUp)
	ON_COMMAND(ID_EDIT_CURSOR_DOWN, &CMainFrame::OnEditCursorDown)
	ON_COMMAND(ID_EDIT_CURSOR_LEFT, &CMainFrame::OnEditCursorLeft)
	ON_COMMAND(ID_EDIT_CURSOR_RIGHT, &CMainFrame::OnEditCursorRight)
	ON_COMMAND(ID_EDIT_PAGE_UP, &CMainFrame::OnEditPageUp)
	ON_COMMAND(ID_EDIT_PAGE_DOWN, &CMainFrame::OnEditPageDown)
	ON_COMMAND_RANGE(2000, 3999, &CMainFrame::OnCreatePointEntity)
	ON_UPDATE_COMMAND_UI_RANGE(2000, 3999, &CMainFrame::OnUpdatePointEntityMenuItem)
	ON_COMMAND_RANGE(4000, 5999, &CMainFrame::OnCreateBrushEntity)
	ON_UPDATE_COMMAND_UI_RANGE(4000, 5999, &CMainFrame::OnUpdateBrushEntityMenuItem)
	ON_COMMAND(ID_EDIT_MOVE_BRUSHES_TO_WORLD, &CMainFrame::OnEditMoveBrushesToWorld)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE_BRUSHES_TO_WORLD, &CMainFrame::OnUpdateMenuItem)
	ON_WM_INITMENU()
	END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_originalAccelTable = NULL;
	m_entityMenuCreated = false;
}

CMainFrame::~CMainFrame()
{
	if (m_originalAccelTable != NULL) {
		DestroyAcceleratorTable(m_originalAccelTable);
		m_originalAccelTable = NULL;
	}
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

TrenchBroom::Controller::Editor* CMainFrame::currentEditor()
{
	CMapDocument* mapDocument = DYNAMIC_DOWNCAST(CMapDocument, GetActiveDocument());
	if (mapDocument == NULL)
		return NULL;

	return &mapDocument->editor();
}

bool CMainFrame::mapViewFocused()
{
	CMapView* mapView = DYNAMIC_DOWNCAST(CMapView, GetActiveView());
	if (mapView == NULL)
		return false;

	return mapView->mapViewFocused();
}

bool CMainFrame::validateCommand(UINT id)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	TrenchBroom::Model::Selection& selection = editor->map().selection();

	switch (id) {
	case ID_FILE_NEW:
	case ID_FILE_OPEN:
	case ID_FILE_SAVE:
		return true;
	case ID_EDIT_UNDO:
		return !undoManager.undoStackEmpty();
	case ID_EDIT_REDO:
		return !undoManager.redoStackEmpty();
	case ID_TOOLS_TOGGLE_VERTEX_TOOL:
		return mapViewFocused() && (inputController.moveVertexToolActive() || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES);
	case ID_TOOLS_TOGGLE_EDGE_TOOL:
		return mapViewFocused() && (inputController.moveEdgeToolActive() || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES);
	case ID_TOOLS_TOGGLE_FACE_TOOL:
		return mapViewFocused() && (inputController.moveFaceToolActive() || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES);
	case ID_TOOLS_TOGGLE_TEXTURE_LOCK:
		return true;
	case ID_EDIT_DELETE:
		return mapViewFocused() && (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES);
	case ID_EDIT_SELECT_ALL:
		return mapViewFocused();
	case ID_EDIT_SELECT_SIBLINGS:
		return mapViewFocused() && !selection.selectedBrushes().empty();
	case ID_EDIT_SELECT_TOUCHING:
		return mapViewFocused() && selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES && selection.selectedBrushes().size() == 1;
	case ID_EDIT_SELECT_NONE:
		return mapViewFocused() && !selection.empty();
	case ID_VIEW_ISOLATE_SELECTION:
		return !selection.empty();
	case ID_GRID_SHOW_GRID:
	case ID_GRID_SNAP_TO_GRID:
	case ID_GRID_GRID_SIZE_1:
	case ID_GRID_GRID_SIZE_2:
	case ID_GRID_GRID_SIZE_4:
	case ID_GRID_GRID_SIZE_8:
	case ID_GRID_GRID_SIZE_16:
	case ID_GRID_GRID_SIZE_32:
	case ID_GRID_GRID_SIZE_64:
	case ID_GRID_GRID_SIZE_128:
	case ID_GRID_GRID_SIZE_256:
		return mapViewFocused();
	case ID_CAMERA_MOVE_FORWARD:
	case ID_CAMERA_MOVE_BACKWARD:
	case ID_CAMERA_MOVE_LEFT:
	case ID_CAMERA_MOVE_RIGHT:
	case ID_CAMERA_MOVE_UP:
	case ID_CAMERA_MOVE_DOWN:
		return mapViewFocused();
	case ID_OBJECT_MOVE_FORWARD:
	case ID_OBJECT_MOVE_BACKWARD:
	case ID_OBJECT_MOVE_LEFT:
	case ID_OBJECT_MOVE_RIGHT:
	case ID_OBJECT_MOVE_UP:
	case ID_OBJECT_MOVE_DOWN:
	case ID_OBJECT_ROLL_90_CW:
	case ID_OBJECT_ROLL_90_CCW:
	case ID_OBJECT_PITCH_90_CW:
	case ID_OBJECT_PITCH_90_CCW:
	case ID_OBJECT_YAW_90_CW:
	case ID_OBJECT_YAW_90_CCW:
	case ID_OBJECT_FLIP_HORIZONTALLY:
	case ID_OBJECT_FLIP_VERTICALLY:
	case ID_OBJECT_DUPLICATE:
		return mapViewFocused() && (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES);
	case ID_OBJECT_ENLARGE_BRUSHES:
		return mapViewFocused() && selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES;
	case ID_TEXTURE_MOVE_UP:
	case ID_TEXTURE_MOVE_DOWN:
	case ID_TEXTURE_MOVE_LEFT:
	case ID_TEXTURE_MOVE_RIGHT:
	case ID_TEXTURE_ROTATE_CW_BY_15:
	case ID_TEXTURE_ROTATE_CCW_BY_15:
		return mapViewFocused() && selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES;
	case ID_EDIT_CURSOR_UP:
		if (!mapViewFocused())
			return false;
		if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_MOVE_FORWARD);
		else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_MOVE_UP);
		return false;
	case ID_EDIT_CURSOR_DOWN:
		if (!mapViewFocused())
			return false;
		if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_MOVE_BACKWARD);
		else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_MOVE_DOWN);
		return false;
	case ID_EDIT_CURSOR_LEFT:
		if (!mapViewFocused())
			return false;
		if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_MOVE_LEFT);
		else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_MOVE_LEFT);
		return false;
	case ID_EDIT_CURSOR_RIGHT:
		if (!mapViewFocused())
			return false;
		if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_MOVE_RIGHT);
		else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_MOVE_RIGHT);
		return false;
	case ID_EDIT_PAGE_UP:
		if (!mapViewFocused())
			return false;
		if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_PITCH_90_CW);
		else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_ROTATE_CW_BY_15);
		return false;
	case ID_EDIT_PAGE_DOWN:
		if (!mapViewFocused())
			return false;
		if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_PITCH_90_CCW);
		else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_ROTATE_CCW_BY_15);
		return false;
	case ID_EDIT_MOVE_BRUSHES_TO_WORLD:
		const TrenchBroom::Model::BrushList& brushes = selection.selectedBrushes();
		for (unsigned int i = 0; i < brushes.size(); i++)
			if (!brushes[i]->entity()->worldspawn())
				return true;
		return false;
	}

	return TRUE;
}


// CMainFrame message handlers

void CMainFrame::OnUpdateMenuItem(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(validateCommand(pCmdUI->m_nID) ? TRUE : FALSE);
}

void CMainFrame::OnEditUndo()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
	undoManager.undo();
}

void CMainFrame::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(validateCommand(pCmdUI->m_nID));

	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
	if (undoManager.undoStackEmpty()) {
		pCmdUI->SetText("Undo\tCtrl+Z");
	} else {
		std::string name = undoManager.topUndoName();
		CString text = "Undo ";
		text += name.c_str();
		text += "\tCtrl+Z";
		pCmdUI->SetText(text);
	}
}

void CMainFrame::OnEditRedo()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
	undoManager.redo();
}

void CMainFrame::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(validateCommand(pCmdUI->m_nID));

	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
	if (undoManager.redoStackEmpty()) {
		pCmdUI->SetText("Redo\tCtrl+Y");
	} else {
		std::string name = undoManager.topRedoName();
		CString text = "Redo ";
		text += name.c_str();
		text += "\tCtrl+Y";
		pCmdUI->SetText(text);
	}
}

void CMainFrame::OnToolsOptions()
{
	PreferencesDialog preferencesDialog(NULL);
	preferencesDialog.DoModal();
}


void CMainFrame::OnToolsToggleVertexTool()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	inputController.toggleMoveVertexTool();
}


void CMainFrame::OnUpdateToolsToggleVertexTool(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(validateCommand(pCmdUI->m_nID));
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	pCmdUI->SetCheck(inputController.moveVertexToolActive());
}


void CMainFrame::OnToolsToggleEdgeTool()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	inputController.toggleMoveEdgeTool();
}


void CMainFrame::OnUpdateToolsToggleEdgeTool(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(validateCommand(pCmdUI->m_nID));
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	pCmdUI->SetCheck(inputController.moveEdgeToolActive());
}


void CMainFrame::OnToolsToggleFaceTool()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	inputController.toggleMoveFaceTool();
}


void CMainFrame::OnUpdateToolsToggleFaceTool(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(validateCommand(pCmdUI->m_nID));
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	pCmdUI->SetCheck(inputController.moveFaceToolActive());
}


void CMainFrame::OnToolsToggleTextureLock()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->toggleTextureLock();
}

void CMainFrame::OnUpdateToolsToggleTextureLock(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(validateCommand(pCmdUI->m_nID));
	TrenchBroom::Controller::Editor* editor = currentEditor();
	pCmdUI->SetCheck(editor->options().lockTextures());
}

void CMainFrame::OnEditDelete()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->map().deleteObjects();
}


void CMainFrame::OnEditSelectAll()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->selectAll();
}


void CMainFrame::OnEditSelectSiblings()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->selectSiblings();
}


void CMainFrame::OnEditSelectTouching()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->selectTouching();
}


void CMainFrame::OnEditSelectNone()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->selectNone();
}


void CMainFrame::OnViewIsolateSelection()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->toggleIsolateSelection();
}


void CMainFrame::OnGridShowGrid()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->toggleGrid();
}


void CMainFrame::OnGridSnapToGrid()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->toggleSnapToGrid();
}


void CMainFrame::OnGridGridSize1()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->setGridSize(0);
}


void CMainFrame::OnGridGridSize2()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->setGridSize(1);
}


void CMainFrame::OnGridGridSize4()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->setGridSize(2);
}


void CMainFrame::OnGridGridSize8()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->setGridSize(3);
}


void CMainFrame::OnGridGridSize16()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->setGridSize(4);
}


void CMainFrame::OnGridGridSize32()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->setGridSize(5);
}


void CMainFrame::OnGridGridSize64()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->setGridSize(6);
}


void CMainFrame::OnGridGridSize128()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->setGridSize(7);
}


void CMainFrame::OnGridGridSize256()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->setGridSize(8);
}


void CMainFrame::OnCameraMoveForward()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->moveCamera(TrenchBroom::Controller::Editor::FORWARD, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
}


void CMainFrame::OnCameraMoveBackward()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->moveCamera(TrenchBroom::Controller::Editor::BACKWARD, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
}


void CMainFrame::OnCameraMoveLeft()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->moveCamera(TrenchBroom::Controller::Editor::LEFT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
}


void CMainFrame::OnCameraMoveRight()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->moveCamera(TrenchBroom::Controller::Editor::RIGHT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
}


void CMainFrame::OnCameraMoveUp()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->moveCamera(TrenchBroom::Controller::Editor::UP, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
}


void CMainFrame::OnCameraMoveDown()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->moveCamera(TrenchBroom::Controller::Editor::DOWN, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
}


void CMainFrame::OnObjectRoll90Cw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->rotateObjects(TrenchBroom::Controller::Editor::ROLL, true);
}


void CMainFrame::OnObjectRoll90Ccw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->rotateObjects(TrenchBroom::Controller::Editor::ROLL, false);
}


void CMainFrame::OnObjectPitch90Cw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->rotateObjects(TrenchBroom::Controller::Editor::PITCH, true);
}


void CMainFrame::OnObjectPitch90Ccw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->rotateObjects(TrenchBroom::Controller::Editor::PITCH, false);
}


void CMainFrame::OnObjectYaw90Cw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->rotateObjects(TrenchBroom::Controller::Editor::YAW, true);
}


void CMainFrame::OnObjectYaw90Ccw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->rotateObjects(TrenchBroom::Controller::Editor::YAW, false);
}


void CMainFrame::OnObjectFlipHorizontally()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->flipObjects(true);
}


void CMainFrame::OnObjectFlipVertically()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->flipObjects(false);
}


void CMainFrame::OnObjectDuplicate()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->duplicateObjects();
}


void CMainFrame::OnObjectEnlargeBrushes()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->enlargeBrushes();
}


void CMainFrame::OnEditCursorUp()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::FORWARD, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::UP, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditCursorDown()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::BACKWARD, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::DOWN, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditCursorLeft()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::LEFT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::LEFT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditCursorRight()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::RIGHT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::RIGHT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditPageUp()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::UP, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->rotateTextures(true, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditPageDown()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.selectionMode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.selectionMode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::DOWN, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.selectionMode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->rotateTextures(false, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnCreatePointEntity(UINT nId)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Model::Map& map = editor->map();
	TrenchBroom::Model::EntityDefinitionManager& definitionManager = map.entityDefinitionManager();
	TrenchBroom::Model::EntityDefinitionList pointDefinitions = definitionManager.definitions(TrenchBroom::Model::TB_EDT_POINT);
	
	if (nId >= 3000) {
		// the event came from the main menu
		unsigned int index = nId - 3000;
		if (index < 0 || index >= pointDefinitions.size())
			return;
		TrenchBroom::Model::EntityDefinitionPtr definition = pointDefinitions[index];
		editor->createEntityAtDefaultPos(definition->name);
	} else {
		// the event came from the popup menu
		unsigned int index = nId - 2000;
		if (index < 0 || index >= pointDefinitions.size())
			return;
		TrenchBroom::Model::EntityDefinitionPtr definition = pointDefinitions[index];
		editor->createEntityAtClickPos(definition->name);
	}
}


void CMainFrame::OnUpdatePointEntityMenuItem(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	pCmdUI->Enable(editor != NULL);
}


void CMainFrame::OnCreateBrushEntity(UINT nId)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Model::Map& map = editor->map();
	TrenchBroom::Model::EntityDefinitionManager& definitionManager = map.entityDefinitionManager();
	TrenchBroom::Model::EntityDefinitionList pointDefinitions = definitionManager.definitions(TrenchBroom::Model::TB_EDT_POINT);
	TrenchBroom::Model::EntityDefinitionList brushDefinitions = definitionManager.definitions(TrenchBroom::Model::TB_EDT_BRUSH);

	if (nId >= 5000) {
		// the event came from the main menu
		unsigned int index = nId - 5000;
		if (index < 0 || index >= brushDefinitions.size())
			return;
		TrenchBroom::Model::EntityDefinitionPtr definition = brushDefinitions[index];
		editor->createEntityAtDefaultPos(definition->name);
	} else {
		// the event came from the popup menu
		unsigned int index = nId - 4000;
		if (index < 0 || index >= brushDefinitions.size())
			return;
		TrenchBroom::Model::EntityDefinitionPtr definition = brushDefinitions[index];
		editor->createEntityAtClickPos(definition->name);
	}
}


void CMainFrame::OnUpdateBrushEntityMenuItem(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL) {
		pCmdUI->Enable(false);
		return;
	}

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	pCmdUI->Enable(!selection.selectedBrushes().empty());
}


void CMainFrame::OnEditMoveBrushesToWorld()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->moveBrushesToWorld();
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// The following code prevents CFrameWnd::PreTranslateMessage from swallowing key presses that
	// correspond to accelerator keys. The idea is to remove all keyboard accelerators from the
	// accelerator table that are disabled in the current state of the editor. If we don't do this,
	// it would not be possible to enter characters into Gwen text fields when those characters are
	// accelerator keys.

	if (pMsg->message == WM_KEYDOWN) {
		if (m_originalAccelTable == NULL) { // make a copy of the original accelerator table
			int entryCount = CopyAcceleratorTable(this->m_hAccelTable, NULL, 0);
			LPACCEL entries = (LPACCEL) LocalAlloc(LPTR, entryCount * sizeof(ACCEL));
			CopyAcceleratorTable(this->m_hAccelTable, entries, entryCount);
			m_originalAccelTable = CreateAcceleratorTable(entries, entryCount);
		}

		// create a new accelerator table containing only those accelerators that are not disabled
		int totalEntryCount = CopyAcceleratorTable(m_originalAccelTable, NULL, 0);
		LPACCEL allEntries = (LPACCEL) LocalAlloc(LPTR, totalEntryCount * sizeof(ACCEL));
		CopyAcceleratorTable(m_originalAccelTable, allEntries, totalEntryCount);

		LPACCEL actualEntries = (LPACCEL) LocalAlloc(LPTR, totalEntryCount * sizeof(ACCEL));
		int actualEntryCount = 0;

		for (int i = 0; i < totalEntryCount; i++) {
			UINT accelId = allEntries[i].cmd;
			if (validateCommand(accelId))
				actualEntries[actualEntryCount++] = allEntries[i];
		}

		DestroyAcceleratorTable(m_hAccelTable);
		m_hAccelTable = CreateAcceleratorTable(actualEntries, actualEntryCount);
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}


void CMainFrame::OnInitMenu(CMenu* pMenu)
{
	CFrameWnd::OnInitMenu(pMenu);

	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	CMenu* editMenu = pMenu->GetSubMenu(1);
	if (editMenu != NULL && m_entityMenuCreated)
		return;

	TrenchBroom::Model::Map& map = editor->map();
	TrenchBroom::Model::EntityDefinitionManager& definitionManager = map.entityDefinitionManager();
	TrenchBroom::Model::EntityDefinitionList pointDefinitions = definitionManager.definitions(TrenchBroom::Model::TB_EDT_POINT);
	TrenchBroom::Model::EntityDefinitionList brushDefinitions = definitionManager.definitions(TrenchBroom::Model::TB_EDT_BRUSH);

	BOOL success;
	if (editMenu == NULL) {
		// it's the popup menu
		CMenu* pointEntityMenu = TrenchBroom::Gui::createEntityMenu(pointDefinitions, 2000);
		CMenu* brushEntityMenu = TrenchBroom::Gui::createEntityMenu(brushDefinitions, 4000);


		success = pMenu->AppendMenu(MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)pointEntityMenu->GetSafeHmenu(), "Create Point Entity\0");
		assert(success);
		success = pMenu->AppendMenu(MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)brushEntityMenu->GetSafeHmenu(), "Create Brush Entity\0");
		assert(success);

		// delete dummy item
		pMenu->DeleteMenu(0, MF_BYPOSITION);
	} else {
		CMenu* pointEntityMenu = TrenchBroom::Gui::createEntityMenu(pointDefinitions, 3000);
		CMenu* brushEntityMenu = TrenchBroom::Gui::createEntityMenu(brushDefinitions, 5000);


		success = editMenu->InsertMenu(ID_EDIT_CREATE_PREFAB, MF_ENABLED | MF_STRING | MF_POPUP | MF_BYCOMMAND, (UINT_PTR)pointEntityMenu->GetSafeHmenu(), "Create Point Entity\0");
		assert(success);
		success = editMenu->InsertMenu(ID_EDIT_CREATE_PREFAB, MF_ENABLED | MF_STRING | MF_POPUP | MF_BYCOMMAND, (UINT_PTR)brushEntityMenu->GetSafeHmenu(), "Create Brush Entity\0");
		assert(success);
		m_entityMenuCreated = true;
	}
}

