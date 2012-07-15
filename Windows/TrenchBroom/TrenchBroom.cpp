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
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "TrenchBroom.h"
#include "MainFrm.h"

#include "MapDocument.h"
#include "MapView.h"

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

#include "WinFileManager.h"
#include "WinPreferences.h"
#include "PreferencesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTrenchBroomApp

BEGIN_MESSAGE_MAP(CTrenchBroomApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CTrenchBroomApp::OnAppAbout)
	ON_COMMAND(ID_FILE_NEW_FRAME, &CTrenchBroomApp::OnFileNewFrame)
	ON_COMMAND(ID_FILE_NEW, &CTrenchBroomApp::OnFileNew)
	// Standard file based document commands
	ON_COMMAND(ID_TOOLS_OPTIONS, &CTrenchBroomApp::OnToolsOptions)
	ON_COMMAND(ID_EDIT_UNDO, &CTrenchBroomApp::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CTrenchBroomApp::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CTrenchBroomApp::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CTrenchBroomApp::OnUpdateEditRedo)
	ON_COMMAND(ID_TOOLS_TOGGLE_VERTEX_TOOL, &CTrenchBroomApp::OnToolsToggleVertexTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_VERTEX_TOOL, &CTrenchBroomApp::OnUpdateToolsToggleVertexTool)
	ON_COMMAND(ID_TOOLS_TOGGLE_EDGE_TOOL, &CTrenchBroomApp::OnToolsToggleEdgeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_EDGE_TOOL, &CTrenchBroomApp::OnUpdateToolsToggleEdgeTool)
	ON_COMMAND(ID_TOOLS_TOGGLE_FACE_TOOL, &CTrenchBroomApp::OnToolsToggleFaceTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TOGGLE_FACE_TOOL, &CTrenchBroomApp::OnUpdateToolsToggleFaceTool)
	ON_COMMAND(ID_EDIT_DELETE, &CTrenchBroomApp::OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, &CTrenchBroomApp::OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_SELECT_ALL, &CTrenchBroomApp::OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, &CTrenchBroomApp::OnUpdateEditSelectAll)
	ON_COMMAND(ID_EDIT_SELECT_ENTITY, &CTrenchBroomApp::OnEditSelectEntity)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ENTITY, &CTrenchBroomApp::OnUpdateEditSelectEntity)
	ON_COMMAND(ID_EDIT_SELECT_TOUCHING, &CTrenchBroomApp::OnEditSelectTouching)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_TOUCHING, &CTrenchBroomApp::OnUpdateEditSelectTouching)
	ON_COMMAND(ID_EDIT_SELECT_NONE, &CTrenchBroomApp::OnEditSelectNone)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_NONE, &CTrenchBroomApp::OnUpdateEditSelectNone)
	ON_COMMAND(ID_VIEW_ISOLATE_SELECTION, &CTrenchBroomApp::OnViewIsolateSelection)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ISOLATE_SELECTION, &CTrenchBroomApp::OnUpdateViewIsolateSelection)
	ON_COMMAND(ID_GRID_SHOW_GRID, &CTrenchBroomApp::OnGridShowGrid)
	ON_UPDATE_COMMAND_UI(ID_GRID_SHOW_GRID, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_SNAP_TO_GRID, &CTrenchBroomApp::OnGridSnapToGrid)
	ON_UPDATE_COMMAND_UI(ID_GRID_SNAP_TO_GRID, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_1, &CTrenchBroomApp::OnGridGridSize1)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_1, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_2, &CTrenchBroomApp::OnGridGridSize2)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_2, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_4, &CTrenchBroomApp::OnGridGridSize4)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_4, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_8, &CTrenchBroomApp::OnGridGridSize8)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_8, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_16, &CTrenchBroomApp::OnGridGridSize16)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_16, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_32, &CTrenchBroomApp::OnGridGridSize32)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_32, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_64, &CTrenchBroomApp::OnGridGridSize64)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_64, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_128, &CTrenchBroomApp::OnGridGridSize128)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_128, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_GRID_GRID_SIZE_256, &CTrenchBroomApp::OnGridGridSize256)
	ON_UPDATE_COMMAND_UI(ID_GRID_GRID_SIZE_256, &CTrenchBroomApp::OnUpdateGridItem)
	ON_COMMAND(ID_CAMERA_MOVE_FORWARD, &CTrenchBroomApp::OnCameraMoveForward)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_FORWARD, &CTrenchBroomApp::OnUpdateCameraItem)
	ON_COMMAND(ID_CAMERA_MOVE_BACKWARD, &CTrenchBroomApp::OnCameraMoveBackward)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_BACKWARD, &CTrenchBroomApp::OnUpdateCameraItem)
	ON_COMMAND(ID_CAMERA_MOVE_LEFT, &CTrenchBroomApp::OnCameraMoveLeft)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_LEFT, &CTrenchBroomApp::OnUpdateCameraItem)
	ON_COMMAND(ID_CAMERA_MOVE_RIGHT, &CTrenchBroomApp::OnCameraMoveRight)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_RIGHT, &CTrenchBroomApp::OnUpdateCameraItem)
	ON_COMMAND(ID_CAMERA_MOVE_UP, &CTrenchBroomApp::OnCameraMoveUp)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_UP, &CTrenchBroomApp::OnUpdateCameraItem)
	ON_COMMAND(ID_CAMERA_MOVE_DOWN, &CTrenchBroomApp::OnCameraMoveDown)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_MOVE_DOWN, &CTrenchBroomApp::OnUpdateCameraItem)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_FORWARD, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_BACKWARD, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_LEFT, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_RIGHT, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_UP, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVE_DOWN, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_ROLL_90_CW, &CTrenchBroomApp::OnObjectRoll90Cw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_ROLL_90_CW, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_ROLL_90_CCW, &CTrenchBroomApp::OnObjectRoll90Ccw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_ROLL_90_CCW, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_PITCH_90_CW, &CTrenchBroomApp::OnObjectPitch90Cw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_PITCH_90_CW, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_PITCH_90_CCW, &CTrenchBroomApp::OnObjectPitch90Ccw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_PITCH_90_CCW, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_YAW_90_CW, &CTrenchBroomApp::OnObjectYaw90Cw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_YAW_90_CW, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_YAW_90_CCW, &CTrenchBroomApp::OnObjectYaw90Ccw)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_YAW_90_CCW, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_FLIP_HORIZONTALLY, &CTrenchBroomApp::OnObjectFlipHorizontally)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_FLIP_HORIZONTALLY, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_FLIP_VERTICALLY, &CTrenchBroomApp::OnObjectFlipVertically)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_FLIP_VERTICALLY, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_DUPLICATE, &CTrenchBroomApp::OnObjectDuplicate)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_DUPLICATE, &CTrenchBroomApp::OnUpdateObjectItem)
	ON_COMMAND(ID_OBJECT_ENLARGE_BRUSHES, &CTrenchBroomApp::OnObjectEnlargeBrushes)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_ENLARGE_BRUSHES, &CTrenchBroomApp::OnUpdateObjectEnlargeBrushes)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_MOVE_UP, &CTrenchBroomApp::OnUpdateTextureItem)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_MOVE_DOWN, &CTrenchBroomApp::OnUpdateTextureItem)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_MOVE_LEFT, &CTrenchBroomApp::OnUpdateTextureItem)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_MOVE_RIGHT, &CTrenchBroomApp::OnUpdateTextureItem)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_ROTATE_CW_BY_15, &CTrenchBroomApp::OnUpdateTextureItem)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_ROTATE_CCW_BY_15, &CTrenchBroomApp::OnUpdateTextureItem)
	ON_COMMAND(ID_EDIT_CURSOR_UP, &CTrenchBroomApp::OnEditCursorUp)
	ON_COMMAND(ID_EDIT_CURSOR_DOWN, &CTrenchBroomApp::OnEditCursorDown)
	ON_COMMAND(ID_EDIT_CURSOR_LEFT, &CTrenchBroomApp::OnEditCursorLeft)
	ON_COMMAND(ID_EDIT_CURSOR_RIGHT, &CTrenchBroomApp::OnEditCursorRight)
	ON_COMMAND(ID_EDIT_PAGE_UP, &CTrenchBroomApp::OnEditPageUp)
	ON_COMMAND(ID_EDIT_PAGE_DOWN, &CTrenchBroomApp::OnEditPageDown)
END_MESSAGE_MAP()

/*
// force memory leak dumping when the program terminates in debug mode
struct _DEBUG_STATE
{
	_DEBUG_STATE() {}
	~_DEBUG_STATE() {
		_CrtDumpMemoryLeaks();
	}
};

_DEBUG_STATE g_ds;
*/

// CTrenchBroomApp construction

CTrenchBroomApp::CTrenchBroomApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("TrenchBroom.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

TrenchBroom::Controller::Editor* CTrenchBroomApp::currentEditor()
{
	CFrameWnd* frame = DYNAMIC_DOWNCAST(CFrameWnd, CWnd::GetActiveWindow());
	if (frame == NULL)
		return NULL;

	CMapDocument* mapDocument = DYNAMIC_DOWNCAST(CMapDocument, frame->GetActiveDocument());
	if (mapDocument == NULL)
		return NULL;

	return &mapDocument->editor();
}

bool CTrenchBroomApp::mapViewFocused()
{
	CFrameWnd* frame = DYNAMIC_DOWNCAST(CFrameWnd, CWnd::GetActiveWindow());
	if (frame == NULL)
		return false;

	CMapView* mapView = DYNAMIC_DOWNCAST(CMapView, frame->GetActiveView());
	if (mapView == NULL)
		return false;

	return mapView->mapViewFocused();
}

// The one and only CTrenchBroomApp object

CTrenchBroomApp theApp;


// CTrenchBroomApp initialization

BOOL CTrenchBroomApp::InitInstance()
{
	// disable MFC's automatic leak dumping
//	AfxEnableMemoryLeakDump(FALSE);

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("TrenchBroom"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMapDocument),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMapView));
	if (!pDocTemplate)
		return FALSE;
	m_pDocTemplate = pDocTemplate;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Initialize TrenchBroom globals
	TrenchBroom::IO::FileManager::sharedFileManager = new TrenchBroom::IO::WinFileManager();
	TrenchBroom::Model::Preferences::sharedPreferences = new TrenchBroom::Model::WinPreferences();
	TrenchBroom::Model::Preferences::sharedPreferences->init();
	TrenchBroom::Model::EntityDefinitionManager::sharedManagers = new TrenchBroom::Model::EntityDefinitionManagerMap();
	TrenchBroom::IO::PakManager::sharedManager = new TrenchBroom::IO::PakManager();
	TrenchBroom::Model::Assets::AliasManager::sharedManager = new TrenchBroom::Model::Assets::AliasManager();
	TrenchBroom::Model::Assets::BspManager::sharedManager = new TrenchBroom::Model::Assets::BspManager();

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}

BOOL CTrenchBroomApp::ExitInstance() {
	delete TrenchBroom::Model::Assets::BspManager::sharedManager;
	TrenchBroom::Model::Assets::BspManager::sharedManager = NULL;
	delete TrenchBroom::Model::Assets::AliasManager::sharedManager;
	TrenchBroom::Model::Assets::AliasManager::sharedManager = NULL;
	delete TrenchBroom::IO::PakManager::sharedManager;
	TrenchBroom::IO::PakManager::sharedManager = NULL;
	delete TrenchBroom::Model::EntityDefinitionManager::sharedManagers;
	TrenchBroom::Model::EntityDefinitionManager::sharedManagers = NULL;
	delete TrenchBroom::Model::Preferences::sharedPreferences;
	TrenchBroom::Model::Preferences::sharedPreferences = NULL;
	delete TrenchBroom::IO::FileManager::sharedFileManager;
	TrenchBroom::IO::FileManager::sharedFileManager = NULL;

	return  CWinApp::ExitInstance();
}

// CTrenchBroomApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CTrenchBroomApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CTrenchBroomApp message handlers

void CTrenchBroomApp::OnFileNewFrame() 
{
	ASSERT(m_pDocTemplate != NULL);

	CDocument* pDoc = NULL;
	CFrameWnd* pFrame = NULL;

	// Create a new instance of the document referenced
	// by the m_pDocTemplate member.
	if (m_pDocTemplate != NULL)
		pDoc = m_pDocTemplate->CreateNewDocument();

	if (pDoc != NULL)
	{
		// If creation worked, use create a new frame for
		// that document.
		pFrame = m_pDocTemplate->CreateNewFrame(pDoc, NULL);
		if (pFrame != NULL)
		{
			// Set the title, and initialize the document.
			// If document initialization fails, clean-up
			// the frame window and document.

			m_pDocTemplate->SetDefaultTitle(pDoc);
			if (!pDoc->OnNewDocument())
			{
				pFrame->DestroyWindow();
				pFrame = NULL;
			}
			else
			{
				// Otherwise, update the frame
				m_pDocTemplate->InitialUpdateFrame(pFrame, pDoc, TRUE);
			}
		}
	}

	// If we failed, clean up the document and show a
	// message to the user.

	if (pFrame == NULL || pDoc == NULL)
	{
		delete pDoc;
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
	}
}

void CTrenchBroomApp::OnFileNew() 
{
	CDocument* pDoc = NULL;
	CFrameWnd* pFrame;
	pFrame = DYNAMIC_DOWNCAST(CFrameWnd, CWnd::GetActiveWindow());
	
	if (pFrame != NULL)
		pDoc = pFrame->GetActiveDocument();

	if (pFrame == NULL || pDoc == NULL)
	{
		// if it's the first document, create as normal
		CWinApp::OnFileNew();
	}
	else
	{
		// Otherwise, see if we have to save modified, then
		// ask the document to reinitialize itself.
		if (!pDoc->SaveModified())
			return;

		CDocTemplate* pTemplate = pDoc->GetDocTemplate();
		ASSERT(pTemplate != NULL);

		if (pTemplate != NULL)
			pTemplate->SetDefaultTitle(pDoc);
		pDoc->OnNewDocument();
	}
}

CDocument* CTrenchBroomApp::OpenDocumentFile(LPCTSTR lpszFileName) {
	CDocument* pDoc = NULL;
	CFrameWnd* pFrame;
	pFrame = DYNAMIC_DOWNCAST(CFrameWnd, CWnd::GetActiveWindow());
	
	if (pFrame != NULL)
		pDoc = pFrame->GetActiveDocument();

	// if it's the first document, open as normal
	if (pFrame == NULL || pDoc == NULL)
		return CWinApp::OpenDocumentFile(lpszFileName);

	// Otherwise, see if we have to save modified, then
	// ask the document to load itself.
	if (!pDoc->SaveModified())
		return NULL;

	CDocTemplate* pTemplate = pDoc->GetDocTemplate();
	ASSERT(pTemplate != NULL);

	if (pTemplate != NULL)
		pTemplate->SetDefaultTitle(pDoc);
	pDoc->DeleteContents();
	pDoc->OnOpenDocument(lpszFileName);
	return pDoc;
}

void CTrenchBroomApp::OnEditUndo()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
	undoManager.undo();
}

void CTrenchBroomApp::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL) {
		pCmdUI->Enable(FALSE);
		pCmdUI->SetText("Undo\tCtrl+Z");
	} else {
		TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
		if (undoManager.undoStackEmpty()) {
			pCmdUI->Enable(FALSE);
			pCmdUI->SetText("Undo\tCtrl+Z");
		} else {
			pCmdUI->Enable(TRUE);
			std::string name = undoManager.topUndoName();
			CString text = "Undo ";
			text += name.c_str();
			text += "\tCtrl+Z";
			pCmdUI->SetText(text);
		}
	}
}

void CTrenchBroomApp::OnEditRedo()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
	undoManager.redo();
}

void CTrenchBroomApp::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL) {
		pCmdUI->Enable(FALSE);
		pCmdUI->SetText("Redo\tCtrl+Y");
	} else {
		TrenchBroom::Model::UndoManager& undoManager = editor->map().undoManager();
		if (undoManager.redoStackEmpty()) {
			pCmdUI->Enable(FALSE);
			pCmdUI->SetText("Redo\tCtrl+Y");
		} else {
			pCmdUI->Enable(TRUE);
			std::string name = undoManager.topRedoName();
			CString text = "Redo ";
			text += name.c_str();
			text += "\tCtrl+Y";
			pCmdUI->SetText(text);
		}
	}
}

void CTrenchBroomApp::OnToolsOptions()
{
	PreferencesDialog preferencesDialog(NULL);
	preferencesDialog.DoModal();
}


void CTrenchBroomApp::OnToolsToggleVertexTool()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	inputController.toggleMoveVertexTool();
}


void CTrenchBroomApp::OnUpdateToolsToggleVertexTool(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Controller::InputController& inputController = editor->inputController();
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(inputController.moveVertexToolActive() || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES);
	}
}


void CTrenchBroomApp::OnToolsToggleEdgeTool()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	inputController.toggleMoveEdgeTool();
}


void CTrenchBroomApp::OnUpdateToolsToggleEdgeTool(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Controller::InputController& inputController = editor->inputController();
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(inputController.moveEdgeToolActive() || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES);
	}
}


void CTrenchBroomApp::OnToolsToggleFaceTool()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	TrenchBroom::Controller::InputController& inputController = editor->inputController();
	inputController.toggleMoveFaceTool();
}


void CTrenchBroomApp::OnUpdateToolsToggleFaceTool(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Controller::InputController& inputController = editor->inputController();
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(inputController.moveFaceToolActive() || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES);
	}
}


void CTrenchBroomApp::OnEditDelete()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->map().deleteObjects();
}


void CTrenchBroomApp::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES || selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES);
	}
}


void CTrenchBroomApp::OnEditSelectAll()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->selectAll();
}


void CTrenchBroomApp::OnUpdateEditSelectAll(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	pCmdUI->Enable(editor != NULL && mapViewFocused());
}


void CTrenchBroomApp::OnEditSelectEntity()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->selectEntities();
}


void CTrenchBroomApp::OnUpdateEditSelectEntity(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES);
	}
}


void CTrenchBroomApp::OnEditSelectTouching()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->selectTouching();
}


void CTrenchBroomApp::OnUpdateEditSelectTouching(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES && selection.brushes().size() == 1);
	}
}


void CTrenchBroomApp::OnEditSelectNone()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->selectNone();
}


void CTrenchBroomApp::OnUpdateEditSelectNone(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(!selection.empty());
	}
}


void CTrenchBroomApp::OnViewIsolateSelection()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->toggleIsolateSelection();
}


void CTrenchBroomApp::OnUpdateViewIsolateSelection(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	pCmdUI->Enable(editor != NULL && mapViewFocused());
}


void CTrenchBroomApp::OnGridShowGrid()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->toggleGrid();
}


void CTrenchBroomApp::OnGridSnapToGrid()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->toggleSnapToGrid();
}


void CTrenchBroomApp::OnGridGridSize1()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->setGridSize(0);
}


void CTrenchBroomApp::OnGridGridSize2()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->setGridSize(1);
}


void CTrenchBroomApp::OnGridGridSize4()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->setGridSize(2);
}


void CTrenchBroomApp::OnGridGridSize8()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->setGridSize(3);
}


void CTrenchBroomApp::OnGridGridSize16()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->setGridSize(4);
}


void CTrenchBroomApp::OnGridGridSize32()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->setGridSize(5);
}


void CTrenchBroomApp::OnGridGridSize64()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->setGridSize(6);
}


void CTrenchBroomApp::OnGridGridSize128()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->setGridSize(7);
}


void CTrenchBroomApp::OnGridGridSize256()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL)
		return;

	editor->setGridSize(8);
}


void CTrenchBroomApp::OnUpdateGridItem(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	pCmdUI->Enable(editor != NULL && mapViewFocused());
}


void CTrenchBroomApp::OnCameraMoveForward()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->moveCamera(TrenchBroom::Controller::Editor::FORWARD, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
}


void CTrenchBroomApp::OnCameraMoveBackward()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->moveCamera(TrenchBroom::Controller::Editor::BACKWARD, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
}


void CTrenchBroomApp::OnCameraMoveLeft()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->moveCamera(TrenchBroom::Controller::Editor::LEFT, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
}


void CTrenchBroomApp::OnCameraMoveRight()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->moveCamera(TrenchBroom::Controller::Editor::RIGHT, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
}


void CTrenchBroomApp::OnCameraMoveUp()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->moveCamera(TrenchBroom::Controller::Editor::UP, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
}


void CTrenchBroomApp::OnCameraMoveDown()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->moveCamera(TrenchBroom::Controller::Editor::DOWN, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
}


void CTrenchBroomApp::OnUpdateCameraItem(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	pCmdUI->Enable(editor != NULL && mapViewFocused());
}


void CTrenchBroomApp::OnObjectRoll90Cw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->rotateObjects(TrenchBroom::Controller::Editor::ROLL, true);
}


void CTrenchBroomApp::OnObjectRoll90Ccw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->rotateObjects(TrenchBroom::Controller::Editor::ROLL, false);
}


void CTrenchBroomApp::OnObjectPitch90Cw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->rotateObjects(TrenchBroom::Controller::Editor::PITCH, true);
}


void CTrenchBroomApp::OnObjectPitch90Ccw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->rotateObjects(TrenchBroom::Controller::Editor::PITCH, false);
}


void CTrenchBroomApp::OnObjectYaw90Cw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->rotateObjects(TrenchBroom::Controller::Editor::YAW, true);
}


void CTrenchBroomApp::OnObjectYaw90Ccw()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->rotateObjects(TrenchBroom::Controller::Editor::YAW, false);
}


void CTrenchBroomApp::OnObjectFlipHorizontally()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->flipObjects(true);
}


void CTrenchBroomApp::OnObjectFlipVertically()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->flipObjects(false);
}


void CTrenchBroomApp::OnObjectDuplicate()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->duplicateObjects();
}


void CTrenchBroomApp::OnUpdateObjectItem(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(
			selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
			selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
			selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES);
	}
}


void CTrenchBroomApp::OnObjectEnlargeBrushes()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor != NULL)
		editor->enlargeBrushes();
}


void CTrenchBroomApp::OnUpdateObjectEnlargeBrushes(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(
			selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES);
	}
}


void CTrenchBroomApp::OnUpdateTextureItem(CCmdUI *pCmdUI)
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused()) {
		pCmdUI->Enable(FALSE);
	} else {
		TrenchBroom::Model::Selection& selection = editor->map().selection();
		pCmdUI->Enable(
			selection.mode() == TrenchBroom::Model::TB_SM_FACES);
	}
}


void CTrenchBroomApp::OnEditCursorUp()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::FORWARD, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::UP, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CTrenchBroomApp::OnEditCursorDown()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::BACKWARD, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::DOWN, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CTrenchBroomApp::OnEditCursorLeft()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::LEFT, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::LEFT, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CTrenchBroomApp::OnEditCursorRight()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::RIGHT, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->moveTextures(TrenchBroom::Controller::Editor::RIGHT, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CTrenchBroomApp::OnEditPageUp()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::UP, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->rotateTextures(true, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	}
}


void CTrenchBroomApp::OnEditPageDown()
{
	TrenchBroom::Controller::Editor* editor = currentEditor();
	if (editor == NULL || !mapViewFocused())
		return;

	TrenchBroom::Model::Selection& selection = editor->map().selection();
	if (selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES || 
		selection.mode() == TrenchBroom::Model::TB_SM_ENTITIES ||
		selection.mode() == TrenchBroom::Model::TB_SM_BRUSHES_ENTITIES) {
		editor->moveObjects(TrenchBroom::Controller::Editor::DOWN, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	} else if (selection.mode() == TrenchBroom::Model::TB_SM_FACES) {
		editor->rotateTextures(false, GetAsyncKeyState(VK_LMENU) != 0 | GetAsyncKeyState(VK_RMENU) != 0);
	}
}
