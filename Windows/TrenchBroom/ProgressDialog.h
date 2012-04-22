#pragma once

#include "Controller/ProgressIndicator.h"
#include "Resource.h"

// ProgressDialog dialog


class ProgressDialog : public CDialogEx, public TrenchBroom::Controller::ProgressIndicator
{
	DECLARE_DYNAMIC(ProgressDialog)

public:
	CStatic m_label;
	CProgressCtrl m_progressbar;

	ProgressDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~ProgressDialog();

	virtual BOOL OnInitDialog();
	void setText(const string& text);

// Dialog Data
	enum { IDD = IDD_PROGRESSDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void doReset();
	void doUpdate();
	DECLARE_MESSAGE_MAP()
};
