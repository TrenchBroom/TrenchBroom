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

#pragma once


class PreferencesDialog : public CDialog
{
	DECLARE_DYNAMIC(PreferencesDialog)

public:
	CButton m_okButton;
	CButton m_cancelButton;
	CButton m_selectQuakePathButton;
	CButton m_invertMouseYCheckbox;
	CSliderCtrl m_brightnessSlider;
	CSliderCtrl m_fovSlider;
	CStatic m_brightnessLabel;
	CStatic m_fovLabel;
	CStatic m_quakePathLabel;

	PreferencesDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~PreferencesDialog();
	
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_PREFERENCESDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void updateControls();
	void updateSliderLabels();
	float brightness();
	float fov();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedButtonOk();
	afx_msg void OnClickedButtonCancel();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClickedButtonSelectquakepath();
};
