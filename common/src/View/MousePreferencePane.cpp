/*
 Copyright (C) 2010-2014 Kristian Duske

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

#include "MousePreferencePane.h"

#include "StringUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/BorderLine.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dirdlg.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        namespace MousePreferencePaneLayout {
            static const int MinimumLabelWidth = 100;
        }

        MousePreferencePane::MousePreferencePane(wxWindow* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
        }


        void MousePreferencePane::OnLookSpeedChanged(wxScrollEvent& event) {
            const float value = m_lookSpeedSlider->GetValue() / 100.0f;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraLookSpeed, value);
        }
        
        void MousePreferencePane::OnInvertLookHAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraLookInvertH, value);
        }
        
        void MousePreferencePane::OnInvertLookVAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraLookInvertV, value);
        }

        void MousePreferencePane::OnPanSpeedChanged(wxScrollEvent& event) {
            const float value = m_panSpeedSlider->GetValue() / 100.0f;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraPanSpeed, value);
        }
        
        void MousePreferencePane::OnInvertPanHAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraPanInvertH, value);
        }
        
        void MousePreferencePane::OnInvertPanVAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraPanInvertV, value);
        }

        void MousePreferencePane::OnMoveSpeedChanged(wxScrollEvent& event) {
            const float value = m_moveSpeedSlider->GetValue() / 100.0f;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraMoveSpeed, value);
        }

        void MousePreferencePane::OnEnableAltMoveChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraEnableAltMove, value);
        }

        void MousePreferencePane::OnInvertAltMoveAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraAltMoveInvert, value);
        }

        void MousePreferencePane::OnMoveCameraInCursorDirChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraMoveInCursorDir, value);
        }

        void MousePreferencePane::createGui() {
            wxWindow* mousePreferences = createMousePreferences();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            sizer->Add(mousePreferences, 1, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            
            SetSizer(sizer);
            SetMinSize(wxSize(600, 300));
            SetBackgroundColour(*wxWHITE);
        }
        
        wxWindow* MousePreferencePane::createMousePreferences() {
            wxPanel* mouseBox = new wxPanel(this);
            mouseBox->SetBackgroundColour(*wxWHITE);
            
            wxStaticText* lookSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, "Mouse Look");
            lookSpeedLabel->SetFont(lookSpeedLabel->GetFont().Bold());
            m_lookSpeedSlider = new wxSlider(mouseBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            m_invertLookHAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, "Invert X Axis");
            m_invertLookVAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, "Invert Y Axis");
            
            wxStaticText* panSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, "Mouse Pan");
            panSpeedLabel->SetFont(panSpeedLabel->GetFont().Bold());
            m_panSpeedSlider = new wxSlider(mouseBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            m_invertPanHAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, "Invert X Axis");
            m_invertPanVAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, "Invert Y Axis");
            
            wxStaticText* moveSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, "Mouse Move");
            moveSpeedLabel->SetFont(moveSpeedLabel->GetFont().Bold());
            m_moveSpeedSlider = new wxSlider(mouseBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);

            m_enableAltMoveCheckBox = new wxCheckBox(mouseBox, wxID_ANY, "Alt+MMB drag to move camera");
            m_invertAltMoveAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, "Invert Z axis in Alt+MMB drag");
            m_moveInCursorDirCheckBox = new wxCheckBox(mouseBox, wxID_ANY, "Move camera towards cursor");

            const int HMargin       = LayoutConstants::WideHMargin;
            const int LMargin       = LayoutConstants::WideVMargin;
            const int LabelFlags    = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT;
            const int SliderFlags   = wxEXPAND | wxRIGHT;
            const int CheckBoxFlags = wxRIGHT;
            const int LineFlags     = wxEXPAND | wxTOP | wxBOTTOM;

            wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::WideHMargin);
            sizer->Add(lookSpeedLabel,              wxGBPosition( 0, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_lookSpeedSlider,           wxGBPosition( 0, 1), wxDefaultSpan, SliderFlags, HMargin);
            sizer->Add(m_invertLookHAxisCheckBox,   wxGBPosition( 1, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            sizer->Add(m_invertLookVAxisCheckBox,   wxGBPosition( 2, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            sizer->Add(new BorderLine(mouseBox),    wxGBPosition( 3, 0), wxGBSpan(1,2), LineFlags, LMargin);
            
            sizer->Add(panSpeedLabel,               wxGBPosition( 4, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_panSpeedSlider,            wxGBPosition( 4, 1), wxDefaultSpan, SliderFlags, HMargin);
            sizer->Add(m_invertPanHAxisCheckBox,    wxGBPosition( 5, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            sizer->Add(m_invertPanVAxisCheckBox,    wxGBPosition( 6, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            sizer->Add(new BorderLine(mouseBox),    wxGBPosition( 7, 0), wxGBSpan(1,2), LineFlags, LMargin);
            
            sizer->Add(moveSpeedLabel,              wxGBPosition( 8, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_moveSpeedSlider,           wxGBPosition( 8, 1), wxDefaultSpan, SliderFlags, HMargin);
            sizer->Add(m_enableAltMoveCheckBox,     wxGBPosition( 9, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            sizer->Add(m_invertAltMoveAxisCheckBox, wxGBPosition(10, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            sizer->Add(m_moveInCursorDirCheckBox,   wxGBPosition(11, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            
            sizer->AddGrowableCol(1);
            mouseBox->SetSizer(sizer);
            return mouseBox;
        }
        
        void MousePreferencePane::bindEvents() {
            m_invertLookHAxisCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnInvertLookHAxisChanged, this);
            m_invertLookVAxisCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnInvertLookVAxisChanged, this);
            m_invertPanHAxisCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnInvertPanHAxisChanged, this);
            m_invertPanVAxisCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnInvertPanVAxisChanged, this);
            m_enableAltMoveCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnEnableAltMoveChanged, this);
            m_moveInCursorDirCheckBox->Bind(wxEVT_CHECKBOX, &MousePreferencePane::OnMoveCameraInCursorDirChanged, this);
            
            bindSliderEvents(m_lookSpeedSlider, &MousePreferencePane::OnLookSpeedChanged, this);
            bindSliderEvents(m_panSpeedSlider, &MousePreferencePane::OnPanSpeedChanged, this);
            bindSliderEvents(m_moveSpeedSlider, &MousePreferencePane::OnMoveSpeedChanged, this);
        }

        void MousePreferencePane::doUpdateControls() {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_lookSpeedSlider->SetValue(static_cast<int>(prefs.get(Preferences::CameraLookSpeed) * m_lookSpeedSlider->GetMax()));
            m_invertLookHAxisCheckBox->SetValue(prefs.get(Preferences::CameraLookInvertH));
            m_invertLookVAxisCheckBox->SetValue(prefs.get(Preferences::CameraLookInvertV));
            
            m_panSpeedSlider->SetValue(static_cast<int>(prefs.get(Preferences::CameraPanSpeed) * m_panSpeedSlider->GetMax()));
            m_invertPanHAxisCheckBox->SetValue(prefs.get(Preferences::CameraPanInvertH));
            m_invertPanVAxisCheckBox->SetValue(prefs.get(Preferences::CameraPanInvertV));
            
            m_moveSpeedSlider->SetValue(static_cast<int>(prefs.get(Preferences::CameraMoveSpeed) * m_moveSpeedSlider->GetMax()));
            m_enableAltMoveCheckBox->SetValue(prefs.get(Preferences::CameraEnableAltMove));
            m_invertAltMoveAxisCheckBox->SetValue(prefs.get(Preferences::CameraAltMoveInvert));
            m_moveInCursorDirCheckBox->SetValue(prefs.get(Preferences::CameraMoveInCursorDir));
        }

        bool MousePreferencePane::doValidate() {
            return true;
        }
	}
}
