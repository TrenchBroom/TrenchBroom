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

#include "Controller/Editor.h"
#include "Controller/InputController.h"
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
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_VERTEX_TOOL, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_TOOLS_TOGGLE_EDGE_TOOL, &CMainFrame::OnToolsToggleEdgeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_EDGE_TOOL, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_TOOLS_TOGGLE_FACE_TOOL, &CMainFrame::OnToolsToggleFaceTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_FACE_TOOL, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_EDIT_DELETE, &CMainFrame::OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_EDIT_SELECT_ALL, &CMainFrame::OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, &CMainFrame::OnUpdateMenuItem)
	ON_COMMAND(ID_EDIT_SELECT_ENTITY, &CMainFrame::OnEditSelectEntity)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ENTITY, &CMainFrame::OnUpdateMenuItem)
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
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_originalAccelTable = NULL;
	// TODO: add member initialization code here
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
	if (!mapViewFocused())
		return false;

	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	TrenchBroom::Model::Selection& selection = editor->map().selection();



	switch (id) {
	case ID_EDIT_UNDO:
		return !undoManager.undoStackEmpty();
	case ID_EDIT_REDO:
		return !undoManager.redoStackEmpty();
	case ID_TOOLS_TOGGLE_VERTEX_TOOL:
		return (inputController.moveVertexToolActive() || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES);
	case ID_TOOLS_TOGGLE_EDGE_TOOL:
		return (inputController.moveEdgeToolActive() || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES);
	case ID_TOOLS_TOGGLE_FACE_TOOL:
		return (inputController.moveFaceToolActive() || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES);
	case ID_EDIT_DELETE:
		return (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES);
	case ID_EDIT_SELECT_ALL:
		return true;
	case ID_EDIT_SELECT_ENTITY:
		return selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES;
	case ID_EDIT_SELECT_TOUCHING:
		return selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES && selection.brushes().size() == 1;
	case ID_EDIT_SELECT_NONE:
		return !selection.empty();
	case ID_VIEW_ISOLATE_SELECTION:
		return true;
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
		return true;
	case ID_CAMERA_MOVE_FORWARD:
	case ID_CAMERA_MOVE_BACKWARD:
	case ID_CAMERA_MOVE_LEFT:
	case ID_CAMERA_MOVE_RIGHT:
	case ID_CAMERA_MOVE_UP:
	case ID_CAMERA_MOVE_DOWN:
		return true;
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
		return selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES;
	case ID_OBJECT_ENLARGE_BRUSHES:
		return selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES;
	case ID_TEXTURE_MOVE_UP:
	case ID_TEXTURE_MOVE_DOWN:
	case ID_TEXTURE_MOVE_LEFT:
	case ID_TEXTURE_MOVE_RIGHT:
	case ID_TEXTURE_ROTATE_CW_BY_15:
	case ID_TEXTURE_ROTATE_CCW_BY_15:
		return selection.mode() == TrenchBroom::Model::TB_SM_FACES;
		break;
	case ID_EDIT_CURSOR_UP:
		if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_MOVE_FORWARD);
		else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_MOVE_UP);
		return false;
	case ID_EDIT_CURSOR_DOWN:
		if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_MOVE_BACKWARD);
		else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_MOVE_DOWN);
		return false;
	case ID_EDIT_CURSOR_LEFT:
		if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_MOVE_LEFT);
		else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_MOVE_LEFT);
		return false;
	case ID_EDIT_CURSOR_RIGHT:
		if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_MOVE_RIGHT);
		else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_MOVE_RIGHT);
		return false;
	case ID_EDIT_PAGE_UP:
		if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_PITCH_90_CW);
		else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_ROTATE_CW_BY_15);
		return false;
	case ID_EDIT_PAGE_DOWN:
		if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES)
			return validateCommand(ID_OBJECT_PITCH_90_CCW);
		else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES)
			return validateCommand(ID_TEXTURE_ROTATE_CCW_BY_15);
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


void CMainFrame::OnToolsToggleEdgeTool()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	inputController.toggleMoveEdgeTool();
}


void CMainFrame::OnToolsToggleFaceTool()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	inputController.toggleMoveFaceTool();
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


void CMainFrame::OnEditSelectEntity()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	editor->selectEntities();
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
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::FORWARD, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::UP, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditCursorDown()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::BACKWARD, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::DOWN, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditCursorLeft()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::LEFT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::LEFT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditCursorRight()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::RIGHT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::RIGHT, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditPageUp()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::UP, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->rotateTextures(true, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CMainFrame::OnEditPageDown()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::DOWN, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->rotateTextures(false, GetAsyncKeyState(VK_LMENU) != 0 || GetAsyncKeyState(VK_RMENU) != 0);
	}
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		if (m_originalAccelTable == NULL) {
			int entryCount = CopyAcceleratorTable(this->m_hAccelTable, NULL, 0);
			LPACCEL entries = (LPACCEL) LocalAlloc(LPTR, entryCount * sizeof(ACCEL));
			CopyAcceleratorTable(this->m_hAccelTable, entries, entryCount);
			m_originalAccelTable = CreateAcceleratorTable(entries, entryCount);
		}

		CMenu* menu = GetMenu();

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
