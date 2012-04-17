
// TrenchBroom.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "TrenchBroom.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTrenchBroomApp

BEGIN_MESSAGE_MAP(CTrenchBroomApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CTrenchBroomApp::OnAppAbout)
	ON_COMMAND(ID_FILE_NEW_FRAME, &CTrenchBroomApp::OnFileNewFrame)
END_MESSAGE_MAP()


// CTrenchBroomApp-Erstellung

CTrenchBroomApp::CTrenchBroomApp()
{
	// Neustart-Manager unterstützen
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
#ifdef _MANAGED
	// Wenn die Anwendung mit Common Language Runtime-Unterstützung (/clr) erstellt wurde:
	//     1) Diese zusätzliche Eigenschaft ist erforderlich, damit der Neustart-Manager ordnungsgemäß unterstützt wird.
	//     2) Für die Projekterstellung müssen Sie im Projekt einen Verweis auf System.Windows.Forms hinzufügen.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: Ersetzen Sie die Anwendungs-ID-Zeichenfolge unten durch eine eindeutige ID-Zeichenfolge; empfohlen
	// Das Format für die Zeichenfolge ist: CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("TrenchBroom.AppID.NoVersion"));

	// TODO: Hier Code zur Konstruktion einfügen
	// Alle wichtigen Initialisierungen in InitInstance positionieren
}

// Das einzige CTrenchBroomApp-Objekt

CTrenchBroomApp theApp;


// CTrenchBroomApp-Initialisierung

BOOL CTrenchBroomApp::InitInstance()
{
	// InitCommonControlsEx() ist für Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder höher zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Legen Sie dies fest, um alle allgemeinen Steuerelementklassen einzubeziehen,
	// die Sie in Ihrer Anwendung verwenden möchten.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() ist für die Verwendung des RichEdit-Steuerelements erforderlich.	
	// AfxInitRichEdit2();

	// Standardinitialisierung
	// Wenn Sie diese Features nicht verwenden und die Größe
	// der ausführbaren Datei verringern möchten, entfernen Sie
	// die nicht erforderlichen Initialisierungsroutinen.
	// Ändern Sie den Registrierungsschlüssel, unter dem Ihre Einstellungen gespeichert sind.
	// TODO: Ändern Sie diese Zeichenfolge entsprechend,
	// z.B. zum Namen Ihrer Firma oder Organisation.
	SetRegistryKey(_T("Vom lokalen Anwendungs-Assistenten generierte Anwendungen"));


	// Dieser Code erstellt ein neues Rahmenfensterobjekt und legt dieses
	// als Hauptfensterobjekt der Anwendung fest, um das Hauptfenster zu erstellen.
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// Haupt-MDI-Rahmenfenster erstellen
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	// Laden Sie freigegebene MDI-Menüs und Zugriffstastentabellen.
	//TODO: Fügen Sie zusätzliche Membervariablen hinzu und
	//	 laden Sie Aufrufe für zusätzliche Menütypen, die Ihre Anwendung möglicherweise erfordert
	HINSTANCE hInst = AfxGetResourceHandle();
	m_hMDIMenu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_TrenchBroomTYPE));
	m_hMDIAccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_TrenchBroomTYPE));






	// Das einzige Fenster ist initialisiert und kann jetzt angezeigt und aktualisiert werden.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// Rufen Sie DragAcceptFiles nur auf, wenn eine Suffix vorhanden ist.
	//  In einer SDI-Anwendung ist dies nach ProcessShellCommand erforderlich
	return TRUE;
}

int CTrenchBroomApp::ExitInstance()
{
	//TODO: Zusätzliche Ressourcen behandeln, die Sie möglicherweise hinzugefügt haben
	return CWinApp::ExitInstance();
}

// CTrenchBroomApp-Meldungshandler


// CAboutDlg-Dialogfeld für Anwendungsbefehl "Info"

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialogfelddaten
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

// Implementierung
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

// Anwendungsbefehl zum Ausführen des Dialogfelds
void CTrenchBroomApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CTrenchBroomApp-Meldungshandler

void CTrenchBroomApp::OnFileNewFrame() 
{
	CMainFrame* pFrame = new CMainFrame;
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 
					  NULL, NULL);
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	m_aryFrames.Add(pFrame->GetSafeHwnd());
}


