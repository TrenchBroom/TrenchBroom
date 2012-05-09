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
#include "PreferencesDialog.h"
#include "afxdialogex.h"


// PreferencesDialog dialog

IMPLEMENT_DYNAMIC(PreferencesDialog, CDialog)

PreferencesDialog::PreferencesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(PreferencesDialog::IDD, pParent)
{

}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PreferencesDialog, CDialog)
END_MESSAGE_MAP()


// PreferencesDialog message handlers
