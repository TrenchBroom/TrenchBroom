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

#include "GeneralPreferencePane.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "TrenchBroomApp.h"
#include "Controller/Command.h"
#include "Utility/Preferences.h"
#include "Utility/String.h"
#include "View/CommandIds.h"
#include "View/LayoutConstants.h"

namespace TrenchBroom {
    namespace View {
        namespace GeneralPreferencePaneLayout {
            static const int MinimumLabelWidth = 100;
        }
        
        BEGIN_EVENT_TABLE(GeneralPreferencePane, wxPanel)
        EVT_BUTTON(CommandIds::GeneralPreferencePane::ChooseQuakePathButtonId, GeneralPreferencePane::OnChooseQuakePathClicked)
        
        EVT_COMMAND_SCROLL(CommandIds::GeneralPreferencePane::BrightnessSliderId, GeneralPreferencePane::OnViewSliderChanged)
        EVT_COMMAND_SCROLL(CommandIds::GeneralPreferencePane::GridAlphaSliderId, GeneralPreferencePane::OnViewSliderChanged)
        EVT_CHOICE(CommandIds::GeneralPreferencePane::GridModeChoiceId, GeneralPreferencePane::OnGridModeChoice)
        EVT_CHOICE(CommandIds::GeneralPreferencePane::InstancingModeModeChoiceId, GeneralPreferencePane::OnInstancingModeChoice)
        
        EVT_COMMAND_SCROLL(CommandIds::GeneralPreferencePane::LookSpeedSliderId, GeneralPreferencePane::OnMouseSliderChanged)
        EVT_CHECKBOX(CommandIds::GeneralPreferencePane::InvertLookXAxisCheckBoxId, GeneralPreferencePane::OnInvertAxisChanged)
        EVT_CHECKBOX(CommandIds::GeneralPreferencePane::InvertLookYAxisCheckBoxId, GeneralPreferencePane::OnInvertAxisChanged)
        
        EVT_COMMAND_SCROLL(CommandIds::GeneralPreferencePane::PanSpeedSliderId, GeneralPreferencePane::OnMouseSliderChanged)
        EVT_CHECKBOX(CommandIds::GeneralPreferencePane::InvertPanXAxisCheckBoxId, GeneralPreferencePane::OnInvertAxisChanged)
        EVT_CHECKBOX(CommandIds::GeneralPreferencePane::InvertPanYAxisCheckBoxId, GeneralPreferencePane::OnInvertAxisChanged)
        
        EVT_COMMAND_SCROLL(CommandIds::GeneralPreferencePane::MoveSpeedSliderId, GeneralPreferencePane::OnMouseSliderChanged)
        EVT_CHECKBOX(CommandIds::GeneralPreferencePane::EnableAltMoveCheckBoxId, GeneralPreferencePane::OnEnableAltMoveChanged)
        EVT_CHECKBOX(CommandIds::GeneralPreferencePane::MoveCameraInCursorDirCheckBoxId, GeneralPreferencePane::OnMoveCameraInCursorDirChanged)
        
        
		END_EVENT_TABLE()
        
        void GeneralPreferencePane::updateControls() {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            m_quakePathValueLabel->SetLabel(prefs.getString(Preferences::QuakePath));
            
            m_brightnessSlider->SetValue(static_cast<int>(prefs.getFloat(Preferences::RendererBrightness) * 40.0f));
            m_gridAlphaSlider->SetValue(static_cast<int>(prefs.getFloat(Preferences::GridAlpha) * m_gridAlphaSlider->GetMax()));
            m_gridModeChoice->SetSelection(prefs.getBool(Preferences::GridCheckerboard) ? 1 : 0);
            
            int instancingMode = prefs.getInt(Preferences::RendererInstancingMode);
            if (instancingMode == Preferences::RendererInstancingModeAutodetect)
                m_instancingModeChoice->SetSelection(instancingMode);
            else if (instancingMode == Preferences::RendererInstancingModeForceOn)
                m_instancingModeChoice->SetSelection(instancingMode);
            else
                m_instancingModeChoice->SetSelection(Preferences::RendererInstancingModeForceOff);
            
            m_lookSpeedSlider->SetValue(static_cast<int>(prefs.getFloat(Preferences::CameraLookSpeed) * m_lookSpeedSlider->GetMax()));
            m_invertLookXAxisCheckBox->SetValue(prefs.getBool(Preferences::CameraLookInvertX));
            m_invertLookYAxisCheckBox->SetValue(prefs.getBool(Preferences::CameraLookInvertY));
            
            m_panSpeedSlider->SetValue(static_cast<int>(prefs.getFloat(Preferences::CameraPanSpeed) * m_panSpeedSlider->GetMax()));
            m_invertPanXAxisCheckBox->SetValue(prefs.getBool(Preferences::CameraPanInvertX));
            m_invertPanYAxisCheckBox->SetValue(prefs.getBool(Preferences::CameraPanInvertY));
            
            m_moveSpeedSlider->SetValue(static_cast<int>(prefs.getFloat(Preferences::CameraMoveSpeed) * m_moveSpeedSlider->GetMax()));
            m_enableAltMoveCheckBox->SetValue(prefs.getBool(Preferences::CameraEnableAltMove));
            m_moveInCursorDirCheckBox->SetValue(prefs.getBool(Preferences::CameraMoveInCursorDir));
        }
        
        wxWindow* GeneralPreferencePane::createQuakePreferences() {
            wxStaticBox* quakeBox = new wxStaticBox(this, wxID_ANY, wxT("Quake"));
            
            wxStaticText* quakePathLabel = new wxStaticText(quakeBox, wxID_ANY, wxT("Quake Path"));
            m_quakePathValueLabel = new wxStaticText(quakeBox, wxID_ANY, wxT("Not Set"));
            wxButton* chooseQuakePathButton = new wxButton(quakeBox, CommandIds::GeneralPreferencePane::ChooseQuakePathButtonId, wxT("Choose..."));
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(3, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(quakePathLabel, 0, wxALIGN_CENTER_VERTICAL);
            innerSizer->Add(m_quakePathValueLabel, 0, wxALIGN_CENTER_VERTICAL);
            innerSizer->Add(chooseQuakePathButton, 0, wxALIGN_CENTER_VERTICAL);
            innerSizer->SetItemMinSize(quakePathLabel, GeneralPreferencePaneLayout::MinimumLabelWidth, wxDefaultSize.y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            
            quakeBox->SetSizerAndFit(outerSizer);
            return quakeBox;
        }
        
        wxWindow* GeneralPreferencePane::createViewPreferences() {
            wxStaticBox* viewBox = new wxStaticBox(this, wxID_ANY, wxT("View"));
            
            wxStaticText* brightnessLabel = new wxStaticText(viewBox, wxID_ANY, wxT("Brightness"));
            m_brightnessSlider = new wxSlider(viewBox, CommandIds::GeneralPreferencePane::BrightnessSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* gridLabel = new wxStaticText(viewBox, wxID_ANY, wxT("Grid"));
            m_gridAlphaSlider = new wxSlider(viewBox, CommandIds::GeneralPreferencePane::GridAlphaSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* gridModeFakeLabel = new wxStaticText(viewBox, wxID_ANY, wxT(""));
            wxStaticText* gridModeLabel = new wxStaticText(viewBox, wxID_ANY, wxT("Render grid as"));
            wxString gridModes[2] = {"Lines", "Checkerboard"};
            m_gridModeChoice = new wxChoice(viewBox, CommandIds::GeneralPreferencePane::GridModeChoiceId, wxDefaultPosition, wxDefaultSize, 2, gridModes);
            
            wxStaticText* instancingModeFakeLabel = new wxStaticText(viewBox, wxID_ANY, wxT(""));
            wxStaticText* instancingModeLabel = new wxStaticText(viewBox, wxID_ANY, wxT("Use OpenGL instancing"));
            wxString instancingModes[3] = {"Autodetect", "Force on", "Force off"};
            m_instancingModeChoice = new wxChoice(viewBox, CommandIds::GeneralPreferencePane::InstancingModeModeChoiceId, wxDefaultPosition, wxDefaultSize, 3, instancingModes);;
            
            wxSizer* gridModeSizer = new wxBoxSizer(wxHORIZONTAL);
            gridModeSizer->Add(gridModeLabel);
            gridModeSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            gridModeSizer->Add(m_gridModeChoice);
            
            wxSizer* instancingModeSizer = new wxBoxSizer(wxHORIZONTAL);
            instancingModeSizer->Add(instancingModeLabel);
            instancingModeSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            instancingModeSizer->Add(m_instancingModeChoice);
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(brightnessLabel);
            innerSizer->Add(m_brightnessSlider, 0, wxEXPAND);
            innerSizer->Add(gridLabel);
            innerSizer->Add(m_gridAlphaSlider, 0, wxEXPAND);
            innerSizer->Add(gridModeFakeLabel);
            innerSizer->Add(gridModeSizer);
            innerSizer->Add(instancingModeFakeLabel);
            innerSizer->Add(instancingModeSizer);
            innerSizer->SetItemMinSize(brightnessLabel, GeneralPreferencePaneLayout::MinimumLabelWidth, brightnessLabel->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            
            viewBox->SetSizerAndFit(outerSizer);
            return viewBox;
        }
        
        wxWindow* GeneralPreferencePane::createMousePreferences() {
            wxStaticBox* mouseBox = new wxStaticBox(this, wxID_ANY, wxT("Mouse"));
            
            wxStaticText* lookSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, wxT("Mouse Look"));
            m_lookSpeedSlider = new wxSlider(mouseBox, CommandIds::GeneralPreferencePane::LookSpeedSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* invertLookFakeLabel = new wxStaticText(mouseBox, wxID_ANY, wxT(""));
            m_invertLookXAxisCheckBox = new wxCheckBox(mouseBox, CommandIds::GeneralPreferencePane::InvertLookXAxisCheckBoxId, wxT("Invert X Axis"));
            m_invertLookYAxisCheckBox = new wxCheckBox(mouseBox, CommandIds::GeneralPreferencePane::InvertLookYAxisCheckBoxId, wxT("Invert Y Axis"));
            wxSizer* invertLookSizer = new wxBoxSizer(wxHORIZONTAL);
            invertLookSizer->Add(m_invertLookXAxisCheckBox);
            invertLookSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            invertLookSizer->Add(m_invertLookYAxisCheckBox);
            
            wxStaticText* panSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, wxT("Mouse Pan"));
            m_panSpeedSlider = new wxSlider(mouseBox, CommandIds::GeneralPreferencePane::PanSpeedSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* invertPanFakeLabel = new wxStaticText(mouseBox, wxID_ANY, "");
            m_invertPanXAxisCheckBox = new wxCheckBox(mouseBox, CommandIds::GeneralPreferencePane::InvertPanXAxisCheckBoxId, wxT("Invert X Axis"));
            m_invertPanYAxisCheckBox = new wxCheckBox(mouseBox, CommandIds::GeneralPreferencePane::InvertPanYAxisCheckBoxId, wxT("Invert Y Axis"));
            wxSizer* invertPanSizer = new wxBoxSizer(wxHORIZONTAL);
            invertPanSizer->Add(m_invertPanXAxisCheckBox);
            invertPanSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            invertPanSizer->Add(m_invertPanYAxisCheckBox);
            
            wxStaticText* moveSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, "Mouse Move");
            m_moveSpeedSlider = new wxSlider(mouseBox, CommandIds::GeneralPreferencePane::MoveSpeedSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            wxStaticText* enableAltMoveFakeLabel = new wxStaticText(mouseBox, wxID_ANY, "");
            m_enableAltMoveCheckBox = new wxCheckBox(mouseBox, CommandIds::GeneralPreferencePane::EnableAltMoveCheckBoxId, wxT("Alt+MMB drag to move camera"));
            m_moveInCursorDirCheckBox = new wxCheckBox(mouseBox, CommandIds::GeneralPreferencePane::MoveCameraInCursorDirCheckBoxId, wxT("Move camera towards cursor"));
            wxSizer* moveOptionsSizer = new wxBoxSizer(wxHORIZONTAL);
            moveOptionsSizer->Add(m_enableAltMoveCheckBox);
            moveOptionsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            moveOptionsSizer->Add(m_moveInCursorDirCheckBox);
            
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
            innerSizer->Add(enableAltMoveFakeLabel);
            innerSizer->Add(moveOptionsSizer);
            innerSizer->SetItemMinSize(lookSpeedLabel, GeneralPreferencePaneLayout::MinimumLabelWidth, lookSpeedLabel->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            
            mouseBox->SetSizerAndFit(outerSizer);
            return mouseBox;
        }
        
        GeneralPreferencePane::GeneralPreferencePane(wxWindow* parent) :
        PreferencePane(parent) {
            wxWindow* quakePreferences = createQuakePreferences();
            wxWindow* viewPreferences = createViewPreferences();
            wxWindow* mousePreferences = createMousePreferences();
            
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(quakePreferences, 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(viewPreferences, 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(mousePreferences, 0, wxEXPAND);
            
            SetSizerAndFit(innerSizer);
            
            updateControls();
        }
        
        bool GeneralPreferencePane::validate() {
            return true;
        }

        void GeneralPreferencePane::OnChooseQuakePathClicked(wxCommandEvent& event) {
            wxDirDialog chooseQuakePathDialog(NULL, wxT("Choose quake directory"), wxT(""), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (chooseQuakePathDialog.ShowModal() == wxID_OK) {
                String quakePath = chooseQuakePathDialog.GetPath().ToStdString();
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                prefs.setString(Preferences::QuakePath, quakePath);
                
                updateControls();
                
#ifdef __APPLE__
                Controller::Command command(Controller::Command::InvalidateEntityModelRendererCache);
                static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &command);
#endif
            }
        }
        
        void GeneralPreferencePane::OnViewSliderChanged(wxScrollEvent& event) {
            wxSlider* sender = static_cast<wxSlider*>(event.GetEventObject());
            int value = sender->GetValue();
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            switch (event.GetId()) {
                case CommandIds::GeneralPreferencePane::BrightnessSliderId:
                    prefs.setFloat(Preferences::RendererBrightness, value / 40.0f);
                    break;
                case CommandIds::GeneralPreferencePane::GridAlphaSliderId: {
                    int max = sender->GetMax();
                    float floatValue = static_cast<float>(value) / static_cast<float>(max);
                    prefs.setFloat(Preferences::GridAlpha, floatValue);
                    break;
                }
                default:
                    break;
            }
            
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews();
        }
        
        void GeneralPreferencePane::OnGridModeChoice(wxCommandEvent& event) {
            bool checkerboard = m_gridModeChoice->GetSelection() == 1;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            prefs.setBool(Preferences::GridCheckerboard, checkerboard);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews();
        }
        
        void GeneralPreferencePane::OnInstancingModeChoice(wxCommandEvent& event) {
            int mode = m_instancingModeChoice->GetSelection();
            assert(mode >= 0 && mode <= 2);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            prefs.setInt(Preferences::RendererInstancingMode, mode);
            
#ifdef __APPLE__
            Controller::Command command(Controller::Command::InvalidateInstancedRenderers);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &command);
#endif
        }
        
        void GeneralPreferencePane::OnMouseSliderChanged(wxScrollEvent& event) {
            wxSlider* sender = static_cast<wxSlider*>(event.GetEventObject());
            float value = sender->GetValue() / 100.0f;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            switch (event.GetId()) {
                case CommandIds::GeneralPreferencePane::LookSpeedSliderId:
                    prefs.setFloat(Preferences::CameraLookSpeed, value);
                    break;
                case CommandIds::GeneralPreferencePane::PanSpeedSliderId:
                    prefs.setFloat(Preferences::CameraPanSpeed, value);
                    break;
                case CommandIds::GeneralPreferencePane::MoveSpeedSliderId:
                    prefs.setFloat(Preferences::CameraMoveSpeed, value);
                    break;
                default:
                    break;
            }
        }
        
        void GeneralPreferencePane::OnInvertAxisChanged(wxCommandEvent& event) {
            bool value = event.GetInt() != 0;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            switch (event.GetId()) {
                case CommandIds::GeneralPreferencePane::InvertLookXAxisCheckBoxId:
                    prefs.setBool(Preferences::CameraLookInvertX, value);
                    break;
                case CommandIds::GeneralPreferencePane::InvertLookYAxisCheckBoxId:
                    prefs.setBool(Preferences::CameraLookInvertY, value);
                    break;
                case CommandIds::GeneralPreferencePane::InvertPanXAxisCheckBoxId:
                    prefs.setBool(Preferences::CameraPanInvertX, value);
                    break;
                case CommandIds::GeneralPreferencePane::InvertPanYAxisCheckBoxId:
                    prefs.setBool(Preferences::CameraPanInvertY, value);
                    break;
                default:
                    break;
            }
        }

        void GeneralPreferencePane::OnEnableAltMoveChanged(wxCommandEvent& event) {
            bool value = event.GetInt() != 0;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            prefs.setBool(Preferences::CameraEnableAltMove, value);
        }
        
        void GeneralPreferencePane::OnMoveCameraInCursorDirChanged(wxCommandEvent& event) {
            bool value = event.GetInt() != 0;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            prefs.setBool(Preferences::CameraMoveInCursorDir, value);
        }
	}
}
