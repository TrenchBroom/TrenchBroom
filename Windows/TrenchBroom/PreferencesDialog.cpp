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
#include <sstream>
#include <iostream>
#include <iomanip>
#include "Model/Preferences.h"

// PreferencesDialog dialog

IMPLEMENT_DYNAMIC(PreferencesDialog, CDialogEx)

PreferencesDialog::PreferencesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(PreferencesDialog::IDD, pParent)
{
}

PreferencesDialog::~PreferencesDialog()
{
}

BOOL PreferencesDialog::OnInitDialog()
{
	if (!CDialog::OnInitDialog())
		return FALSE;

	m_brightnessSlider.SetRange(0, 20, TRUE);
	m_brightnessSlider.SetTicFreq(1);
	m_fovSlider.SetRange(0, 20, TRUE);
	m_fovSlider.SetTicFreq(1);

	updateControls();
	return TRUE;
}

void PreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_okButton);
	DDX_Control(pDX, IDC_BUTTON_OK, m_cancelButton);
	DDX_Control(pDX, IDC_BUTTON_SELECTQUAKEPATH, m_selectQuakePathButton);
	DDX_Control(pDX, IDC_CHECK_INVERTMOUSEY, m_invertMouseYCheckbox);
	DDX_Control(pDX, IDC_SLIDER_BRIGHTNESS, m_brightnessSlider);
	DDX_Control(pDX, IDC_SLIDER_FOV, m_fovSlider);
	DDX_Control(pDX, IDC_STATIC_BRIGHTNESS, m_brightnessLabel);
	DDX_Control(pDX, IDC_STATIC_FOV, m_fovLabel);
	DDX_Control(pDX, IDC_STATIC_QUAKEPATH, m_quakePathLabel);
}

void PreferencesDialog::updateControls()
{
	TrenchBroom::Model::Preferences& prefs = *TrenchBroom::Model::Preferences::sharedPreferences;

	m_quakePathLabel.SetWindowTextA(_T(prefs.quakePath().c_str()));
	m_brightnessSlider.SetPos(static_cast<int>((prefs.brightness() - 0.3f) * 10.0f));
	m_fovSlider.SetPos(static_cast<int>((prefs.cameraFov() - 45.0f) / 5.0f));
	m_invertMouseYCheckbox.SetCheck(prefs.cameraInvertY() ? TRUE : FALSE);

	updateSliderLabels();
}

void PreferencesDialog::updateSliderLabels()
{
	std::stringstream brightnessStr;
	brightnessStr << std::setiosflags(std::ios::fixed) << std::setprecision(2) << brightness();
	m_brightnessLabel.SetWindowTextA(_T(brightnessStr.str().c_str()));

	std::stringstream fovStr;
	fovStr << std::setiosflags(std::ios::fixed) << std::setprecision(0) << fov();
	m_fovLabel.SetWindowTextA(_T(fovStr.str().c_str()));
}

float PreferencesDialog::brightness()
{
	return m_brightnessSlider.GetPos() / 10.0f + 0.3f;
}

float PreferencesDialog::fov()
{
	return m_fovSlider.GetPos() * 5.0f + 45.0f;
}

BEGIN_MESSAGE_MAP(PreferencesDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK, &PreferencesDialog::OnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &PreferencesDialog::OnClickedButtonCancel)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_SELECTQUAKEPATH, &PreferencesDialog::OnClickedButtonSelectquakepath)
END_MESSAGE_MAP()


// PreferencesDialog message handlers


void PreferencesDialog::OnClickedButtonOk()
{
	TrenchBroom::Model::Preferences& prefs = *TrenchBroom::Model::Preferences::sharedPreferences;
	prefs.setBrightness(brightness());
	prefs.setCameraFov(fov());
	prefs.setCameraInvertY(m_invertMouseYCheckbox.GetCheck() == TRUE);

	TCHAR quakePath[MAX_PATH];
	m_quakePathLabel.GetWindowTextA(quakePath, MAX_PATH);
	prefs.setQuakePath(quakePath);

	prefs.save();

	EndDialog(IDOK);
}


void PreferencesDialog::OnClickedButtonCancel()
{
	EndDialog(IDCANCEL);
}


void PreferencesDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	updateSliderLabels();
}

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    // If the BFFM_INITIALIZED message is received
    // set the path to the start path.
    switch (uMsg)
    {
        case BFFM_INITIALIZED:
        {
            if (lpData != NULL)
            {
                SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
            }
        }
    }
 
    return 0; // The function should always return 0.
}

void PreferencesDialog::OnClickedButtonSelectquakepath()
{
	BROWSEINFO bi = { 0 };
    LPITEMIDLIST pidl;
    TCHAR szDisplay[MAX_PATH];
	TCHAR initialPath[MAX_PATH];
	TCHAR resultPath[MAX_PATH];
	BOOL retval;

	CoInitialize(NULL);

	m_quakePathLabel.GetWindowTextA(initialPath, MAX_PATH);

	bi.hwndOwner      = GetSafeHwnd();
    bi.pszDisplayName = szDisplay;
    bi.lpszTitle      = TEXT("Please choose a folder.");
    bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn           = BrowseCallbackProc;
	bi.lParam         =	(LPARAM)initialPath;
 
    pidl = SHBrowseForFolder(&bi);
 
    if (pidl != NULL) {
        retval = SHGetPathFromIDList(pidl, resultPath);
        CoTaskMemFree(pidl);
    } else {
        retval = FALSE;
    }
 
    if (retval) {
		m_quakePathLabel.SetWindowTextA(resultPath);
    }

	CoUninitialize();
}
