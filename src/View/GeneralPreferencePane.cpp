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

#include "GeneralPreferencePane.h"

#include "TrenchBroomApp.h"
#include "StringUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        namespace GeneralPreferencePaneLayout {
            static const int MinimumLabelWidth = 100;
        }

        GeneralPreferencePane::GeneralPreferencePane(wxWindow* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
            updateControls();
        }


        void GeneralPreferencePane::OnBrightnessChanged(wxScrollEvent& event) {
            const int value = m_brightnessSlider->GetValue();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::Brightness, value / 40.0f);
        }
        
        void GeneralPreferencePane::OnGridAlphaChanged(wxScrollEvent& event) {
            const int value = m_gridAlphaSlider->GetValue();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            const int max = m_gridAlphaSlider->GetMax();
            const float floatValue = static_cast<float>(value) / static_cast<float>(max);
            prefs.set(Preferences::GridAlpha, floatValue);
        }

        void GeneralPreferencePane::OnTextureBrowserIconSizeChanged(wxCommandEvent& event) {
            PreferenceManager& prefs = PreferenceManager::instance();

            const int selection = m_textureBrowserIconSizeChoice->GetSelection();
            switch (selection) {
                case 0:
                    prefs.set(Preferences::TextureBrowserIconSize, 0.25f);
                    break;
                case 1:
                    prefs.set(Preferences::TextureBrowserIconSize, 0.5f);
                    break;
                case 2:
                    prefs.set(Preferences::TextureBrowserIconSize, 1.0f);
                    break;
                case 3:
                    prefs.set(Preferences::TextureBrowserIconSize, 1.5f);
                    break;
                case 4:
                    prefs.set(Preferences::TextureBrowserIconSize, 2.0f);
                    break;
                case 5:
                    prefs.set(Preferences::TextureBrowserIconSize, 2.5f);
                    break;
                case 6:
                    prefs.set(Preferences::TextureBrowserIconSize, 3.0f);
                    break;
            }
        }

        void GeneralPreferencePane::OnLookSpeedChanged(wxScrollEvent& event) {
            const float value = m_lookSpeedSlider->GetValue() / 100.0f;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraLookSpeed, value);
        }
        
        void GeneralPreferencePane::OnInvertLookHAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraLookInvertH, value);
        }
        
        void GeneralPreferencePane::OnInvertLookVAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraLookInvertV, value);
        }

        void GeneralPreferencePane::OnPanSpeedChanged(wxScrollEvent& event) {
            const float value = m_panSpeedSlider->GetValue() / 100.0f;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraPanSpeed, value);
        }
        
        void GeneralPreferencePane::OnInvertPanHAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraPanInvertH, value);
        }
        
        void GeneralPreferencePane::OnInvertPanVAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraPanInvertV, value);
        }

        void GeneralPreferencePane::OnMoveSpeedChanged(wxScrollEvent& event) {
            const float value = m_moveSpeedSlider->GetValue() / 100.0f;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraMoveSpeed, value);
        }

        void GeneralPreferencePane::OnEnableAltMoveChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraEnableAltMove, value);
        }

        void GeneralPreferencePane::OnInvertAltMoveAxisChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraAltMoveInvert, value);
        }

        void GeneralPreferencePane::OnMoveCameraInCursorDirChanged(wxCommandEvent& event) {
            const bool value = event.GetInt() != 0;

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraMoveInCursorDir, value);
        }

        void GeneralPreferencePane::createGui() {
            wxWindow* viewPreferences = createViewPreferences();
            wxWindow* mousePreferences = createMousePreferences();
            
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(viewPreferences, 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(mousePreferences, 0, wxEXPAND);
            
            SetSizerAndFit(innerSizer);
        }
        
        wxWindow* GeneralPreferencePane::createViewPreferences() {
            wxStaticBox* viewBox = new wxStaticBox(this, wxID_ANY, _("View"));
            
            wxStaticText* brightnessLabel = new wxStaticText(viewBox, wxID_ANY, _("Brightness"));
            m_brightnessSlider = new wxSlider(viewBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* gridLabel = new wxStaticText(viewBox, wxID_ANY, _("Grid"));
            m_gridAlphaSlider = new wxSlider(viewBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* textureBrowserFakeLabel = new wxStaticText(viewBox, wxID_ANY, _(""));
            wxStaticText* textureBrowserIconSizeLabel = new wxStaticText(viewBox, wxID_ANY, _("Texture Browser Icon Size"));
            wxString iconSizes[7] = {"25%", "50%", "100%", "150%", "200%", "250%", "300%"};
            m_textureBrowserIconSizeChoice = new wxChoice(viewBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, 7, iconSizes);
            
            wxSizer* textureBrowserIconSizeSizer = new wxBoxSizer(wxHORIZONTAL);
            textureBrowserIconSizeSizer->Add(textureBrowserIconSizeLabel, 0, wxALIGN_CENTER_VERTICAL);
            textureBrowserIconSizeSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            textureBrowserIconSizeSizer->Add(m_textureBrowserIconSizeChoice, 0, wxALIGN_CENTER_VERTICAL);
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(brightnessLabel);
            innerSizer->Add(m_brightnessSlider, 0, wxEXPAND);
            innerSizer->Add(gridLabel);
            innerSizer->Add(m_gridAlphaSlider, 0, wxEXPAND);
            innerSizer->Add(textureBrowserFakeLabel);
            innerSizer->Add(textureBrowserIconSizeSizer);
            innerSizer->SetItemMinSize(brightnessLabel, GeneralPreferencePaneLayout::MinimumLabelWidth, brightnessLabel->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            
            viewBox->SetSizerAndFit(outerSizer);
            return viewBox;
        }
        
        wxWindow* GeneralPreferencePane::createMousePreferences() {
            wxStaticBox* mouseBox = new wxStaticBox(this, wxID_ANY, _("Mouse"));
            
            wxStaticText* lookSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, _("Mouse Look"));
            m_lookSpeedSlider = new wxSlider(mouseBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* invertLookFakeLabel = new wxStaticText(mouseBox, wxID_ANY, _(""));
            m_invertLookHAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, _("Invert X Axis"));
            m_invertLookVAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, _("Invert Y Axis"));
            wxSizer* invertLookSizer = new wxBoxSizer(wxHORIZONTAL);
            invertLookSizer->Add(m_invertLookHAxisCheckBox);
            invertLookSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            invertLookSizer->Add(m_invertLookVAxisCheckBox);
            
            wxStaticText* panSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, _("Mouse Pan"));
            m_panSpeedSlider = new wxSlider(mouseBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* invertPanFakeLabel = new wxStaticText(mouseBox, wxID_ANY, "");
            m_invertPanHAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, _("Invert X Axis"));
            m_invertPanVAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, _("Invert Y Axis"));
            wxSizer* invertPanSizer = new wxBoxSizer(wxHORIZONTAL);
            invertPanSizer->Add(m_invertPanHAxisCheckBox);
            invertPanSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            invertPanSizer->Add(m_invertPanVAxisCheckBox);
            
            wxStaticText* moveSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, "Mouse Move");
            m_moveSpeedSlider = new wxSlider(mouseBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            wxStaticText* altMoveOptionsFakeLabel = new wxStaticText(mouseBox, wxID_ANY, "");
            wxStaticText* moveInCursorDirFakeLabel = new wxStaticText(mouseBox, wxID_ANY, "");
            m_enableAltMoveCheckBox = new wxCheckBox(mouseBox, wxID_ANY, _("Alt+MMB drag to move camera"));
            m_invertAltMoveAxisCheckBox = new wxCheckBox(mouseBox, wxID_ANY, _("Invert Z axis in Alt+MMB drag"));
            m_moveInCursorDirCheckBox = new wxCheckBox(mouseBox, wxID_ANY, _("Move camera towards cursor"));
            wxSizer* altMoveOptionsSizer = new wxBoxSizer(wxHORIZONTAL);
            altMoveOptionsSizer->Add(m_enableAltMoveCheckBox);
            altMoveOptionsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            altMoveOptionsSizer->Add(m_invertAltMoveAxisCheckBox);
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(lookSpeedLabel);
            innerSizer->Add(m_lookSpeedSlider, 0, wxEXPAND);
            innerSizer->Add(invertLookFakeLabel);
            innerSizer->Add(invertLookSizer);
            innerSizer->Add(panSpeedLabel);
            innerSizer->Add(m_panSpeedSlider, 0, wxEXPAND);
            innerSizer->Add(invertPanFakeLabel);
            innerSizer->Add(invertPanSizer);
            innerSizer->Add(moveSpeedLabel);
            innerSizer->Add(m_moveSpeedSlider, 0, wxEXPAND);
            innerSizer->Add(altMoveOptionsFakeLabel);
            innerSizer->Add(altMoveOptionsSizer);
            innerSizer->Add(moveInCursorDirFakeLabel);
            innerSizer->Add(m_moveInCursorDirCheckBox);
            innerSizer->SetItemMinSize(lookSpeedLabel, GeneralPreferencePaneLayout::MinimumLabelWidth, lookSpeedLabel->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            
            mouseBox->SetSizerAndFit(outerSizer);
            return mouseBox;
        }
        
        void GeneralPreferencePane::bindEvents() {
            m_textureBrowserIconSizeChoice->Bind(wxEVT_CHOICE, &GeneralPreferencePane::OnTextureBrowserIconSizeChanged, this);
            m_invertLookHAxisCheckBox->Bind(wxEVT_CHECKBOX, &GeneralPreferencePane::OnInvertLookHAxisChanged, this);
            m_invertLookVAxisCheckBox->Bind(wxEVT_CHECKBOX, &GeneralPreferencePane::OnInvertLookVAxisChanged, this);
            m_invertPanHAxisCheckBox->Bind(wxEVT_CHECKBOX, &GeneralPreferencePane::OnInvertPanHAxisChanged, this);
            m_invertPanVAxisCheckBox->Bind(wxEVT_CHECKBOX, &GeneralPreferencePane::OnInvertPanVAxisChanged, this);
            m_enableAltMoveCheckBox->Bind(wxEVT_CHECKBOX, &GeneralPreferencePane::OnEnableAltMoveChanged, this);
            m_moveInCursorDirCheckBox->Bind(wxEVT_CHECKBOX, &GeneralPreferencePane::OnMoveCameraInCursorDirChanged, this);
            
            bindSliderEvents(m_brightnessSlider, &GeneralPreferencePane::OnBrightnessChanged, this);
            bindSliderEvents(m_gridAlphaSlider, &GeneralPreferencePane::OnGridAlphaChanged, this);
            bindSliderEvents(m_lookSpeedSlider, &GeneralPreferencePane::OnLookSpeedChanged, this);
            bindSliderEvents(m_panSpeedSlider, &GeneralPreferencePane::OnPanSpeedChanged, this);
            bindSliderEvents(m_moveSpeedSlider, &GeneralPreferencePane::OnMoveSpeedChanged, this);
        }

        void GeneralPreferencePane::updateControls() {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_brightnessSlider->SetValue(static_cast<int>(prefs.get(Preferences::Brightness) * 40.0f));
            m_gridAlphaSlider->SetValue(static_cast<int>(prefs.get(Preferences::GridAlpha) * m_gridAlphaSlider->GetMax()));
            
            const float textureBrowserIconSize = prefs.get(Preferences::TextureBrowserIconSize);
            if (textureBrowserIconSize == 0.25f)
                m_textureBrowserIconSizeChoice->SetSelection(0);
            else if (textureBrowserIconSize == 0.5f)
                m_textureBrowserIconSizeChoice->SetSelection(1);
            else if (textureBrowserIconSize == 1.5f)
                m_textureBrowserIconSizeChoice->SetSelection(3);
            else if (textureBrowserIconSize == 2.0f)
                m_textureBrowserIconSizeChoice->SetSelection(4);
            else if (textureBrowserIconSize == 2.5f)
                m_textureBrowserIconSizeChoice->SetSelection(5);
            else if (textureBrowserIconSize == 3.0f)
                m_textureBrowserIconSizeChoice->SetSelection(6);
            else
                m_textureBrowserIconSizeChoice->SetSelection(2);
            
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

        bool GeneralPreferencePane::doValidate() {
            return true;
        }
	}
}
