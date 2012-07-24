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

// visual leak detector
//#ifdef _DEBUG
//#include <vld.h>
//#endif

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
#include "Utilities/Console.h"

#include "WinConsole.h"
#include "WinFileManager.h"
#include "WinPreferences.h"

#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTrenchBroomApp

BEGIN_MESSAGE_MAP(CTrenchBroomApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CTrenchBroomApp::OnAppAbout)
    ON_COMMAND(ID_FILE_NEW, &CTrenchBroomApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, &CTrenchBroomApp::OnFileOpen)
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

// The one and only CTrenchBroomApp object

CTrenchBroomApp theApp;


// CTrenchBroomApp initialization

BOOL CTrenchBroomApp::InitInstance()
{
	// disable MFC's automatic leak dumping
#ifdef _DEBUG
	AfxEnableMemoryLeakDump(FALSE);
#endif

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
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMapDocument),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMapView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Initialize TrenchBroom globals
	TrenchBroom::IO::FileManager::sharedFileManager = new TrenchBroom::IO::WinFileManager();
	TrenchBroom::IO::FileManager& fileManager = *TrenchBroom::IO::FileManager::sharedFileManager;

	char appPath [MAX_PATH] = "";
	::GetModuleFileName(0, appPath, sizeof(appPath) - 1);
	std::string appDirectory = fileManager.deleteLastPathComponent(appPath);
	std::string logFilePath = fileManager.appendPath(appDirectory, "TrenchBroom.log");
	TrenchBroom::winLogStream = new std::ofstream(logFilePath, std::ios::app);
	if (TrenchBroom::winLogStream->good()) {
		TrenchBroom::log(TrenchBroom::TB_LL_INFO, "Opened log file at %s\n", logFilePath.c_str());
	} else {
		TrenchBroom::log(TrenchBroom::TB_LL_ERR, "Can't open log file at %s\n", logFilePath.c_str());
	}

	TrenchBroom::log(TrenchBroom::TB_LL_INFO, "==================================================\n");
	TrenchBroom::log(TrenchBroom::TB_LL_INFO, "Starting TrenchBroom\n");

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
	// m_pMainWnd->DragAcceptFiles();
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
	delete TrenchBroom::winLogStream;
	TrenchBroom::winLogStream = NULL;

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

