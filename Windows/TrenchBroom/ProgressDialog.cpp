// ProgressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TrenchBroom.h"
#include "ProgressDialog.h"
#include "afxdialogex.h"


// ProgressDialog dialog

IMPLEMENT_DYNAMIC(ProgressDialog, CDialogEx)

ProgressDialog::ProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(ProgressDialog::IDD, pParent), ProgressIndicator(100)
{
}

ProgressDialog::~ProgressDialog()
{
}

BOOL ProgressDialog::OnInitDialog()
{
	if (!CDialogEx::OnInitDialog())
		return FALSE;

	m_label.SetWindowTextA("Please wait...");
	m_progressbar.SetRange32(0, 100);
	return TRUE;
}

void ProgressDialog::setText(const string& text)
{
	TCHAR* chText = new TCHAR[text.length()];
	m_label.SetWindowTextA(chText);
	delete [] chText;
}

void ProgressDialog::doReset() 
{
	m_progressbar.SetPos(0);
}

void ProgressDialog::doUpdate()
{
	m_progressbar.SetPos(static_cast<int>(percent()));

	CWinApp* app = (CWinApp*)AfxGetApp();
	app->PumpMessage();
}

void ProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LABEL, m_label);
	DDX_Control(pDX, IDC_PROGRESSBAR, m_progressbar);
}


BEGIN_MESSAGE_MAP(ProgressDialog, CDialogEx)
END_MESSAGE_MAP()


// ProgressDialog message handlers
