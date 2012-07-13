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

void ProgressDialog::setText(const std::string& text)
{
	m_label.SetWindowTextA(text.c_str());
}

void ProgressDialog::doReset() 
{
	m_progressbar.SetPos(0);
}

void ProgressDialog::doUpdate()
{
	m_progressbar.SetPos(static_cast<int>(percent()));
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
