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

#include "PreferencesDialog.h"

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
        namespace PreferencesDialogLayout {
            static const int MinimumLabelWidth = 100;
        }
        
        BEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
        EVT_BUTTON(CommandIds::PreferencesDialog::ChooseQuakePathButtonId, PreferencesDialog::OnChooseQuakePathClicked)
        
        EVT_COMMAND_SCROLL(CommandIds::PreferencesDialog::BrightnessSliderId, PreferencesDialog::OnViewSliderChanged)
        EVT_COMMAND_SCROLL(CommandIds::PreferencesDialog::GridAlphaSliderId, PreferencesDialog::OnViewSliderChanged)

        EVT_COMMAND_SCROLL(CommandIds::PreferencesDialog::LookSpeedSliderId, PreferencesDialog::OnMouseSliderChanged)
        EVT_CHECKBOX(CommandIds::PreferencesDialog::InvertLookXAxisCheckBoxId, PreferencesDialog::OnInvertAxisChanged)
        EVT_CHECKBOX(CommandIds::PreferencesDialog::InvertLookYAxisCheckBoxId, PreferencesDialog::OnInvertAxisChanged)
        
        EVT_COMMAND_SCROLL(CommandIds::PreferencesDialog::PanSpeedSliderId, PreferencesDialog::OnMouseSliderChanged)
        EVT_CHECKBOX(CommandIds::PreferencesDialog::InvertPanXAxisCheckBoxId, PreferencesDialog::OnInvertAxisChanged)
        EVT_CHECKBOX(CommandIds::PreferencesDialog::InvertPanYAxisCheckBoxId, PreferencesDialog::OnInvertAxisChanged)

        EVT_COMMAND_SCROLL(CommandIds::PreferencesDialog::MoveSpeedSliderId, PreferencesDialog::OnMouseSliderChanged)

        EVT_BUTTON(wxID_OK, PreferencesDialog::OnOkClicked)
        EVT_BUTTON(wxID_CANCEL, PreferencesDialog::OnCancelClicked)
		EVT_CLOSE(PreferencesDialog::OnCloseDialog)
        EVT_MENU(wxID_CLOSE, PreferencesDialog::OnFileExit)
		END_EVENT_TABLE()

        void PreferencesDialog::updateControls() {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            m_quakePathValueLabel->SetLabel(prefs.getString(Preferences::QuakePath));
            
            m_brightnessSlider->SetValue(static_cast<int>(prefs.getFloat(Preferences::RendererBrightness) * 40.0f));
            m_gridAlphaSlider->SetValue(static_cast<int>(prefs.getColor(Preferences::GridColor).w * m_gridAlphaSlider->GetMax()));

            m_lookSpeedSlider->SetValue(static_cast<int>(prefs.getFloat(Preferences::CameraLookSpeed) * m_lookSpeedSlider->GetMax()));
            m_invertLookXAxisCheckBox->SetValue(prefs.getBool(Preferences::CameraLookInvertX));
            m_invertLookYAxisCheckBox->SetValue(prefs.getBool(Preferences::CameraLookInvertY));
            
            m_panSpeedSlider->SetValue(static_cast<int>(prefs.getFloat(Preferences::CameraPanSpeed) * m_panSpeedSlider->GetMax()));
            m_invertPanXAxisCheckBox->SetValue(prefs.getBool(Preferences::CameraPanInvertX));
            m_invertPanYAxisCheckBox->SetValue(prefs.getBool(Preferences::CameraPanInvertY));

            m_moveSpeedSlider->SetValue(static_cast<int>(prefs.getFloat(Preferences::CameraMoveSpeed) * m_moveSpeedSlider->GetMax()));
        }

        wxWindow* PreferencesDialog::createQuakePreferences() {
            wxStaticBox* quakeBox = new wxStaticBox(this, wxID_ANY, wxT("Quake"));
            
            wxStaticText* quakePathLabel = new wxStaticText(quakeBox, wxID_ANY, wxT("Quake Path"));
            m_quakePathValueLabel = new wxStaticText(quakeBox, wxID_ANY, wxT(""));
            wxButton* chooseQuakePathButton = new wxButton(quakeBox, CommandIds::PreferencesDialog::ChooseQuakePathButtonId, wxT("Choose..."));
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(3, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(quakePathLabel);
            innerSizer->Add(m_quakePathValueLabel, 0, wxEXPAND);
            innerSizer->Add(chooseQuakePathButton);
            innerSizer->SetItemMinSize(quakePathLabel, PreferencesDialogLayout::MinimumLabelWidth, quakePathLabel->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::StaticBoxInnerMargin);
            
            quakeBox->SetSizerAndFit(outerSizer);
            return quakeBox;
        }

        wxWindow* PreferencesDialog::createViewPreferences() {
            wxStaticBox* viewBox = new wxStaticBox(this, wxID_ANY, wxT("View"));
            
            wxStaticText* brightnessLabel = new wxStaticText(viewBox, wxID_ANY, wxT("Brightness"));
            m_brightnessSlider = new wxSlider(viewBox, CommandIds::PreferencesDialog::BrightnessSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* gridAlphaLabel = new wxStaticText(viewBox, wxID_ANY, wxT("Grid Alpha"));
            m_gridAlphaSlider = new wxSlider(viewBox, CommandIds::PreferencesDialog::GridAlphaSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(brightnessLabel);
            innerSizer->Add(m_brightnessSlider, 0, wxEXPAND);
            innerSizer->Add(gridAlphaLabel);
            innerSizer->Add(m_gridAlphaSlider, 0, wxEXPAND);
            innerSizer->SetItemMinSize(brightnessLabel, PreferencesDialogLayout::MinimumLabelWidth, brightnessLabel->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::StaticBoxInnerMargin);
            
            viewBox->SetSizerAndFit(outerSizer);
            return viewBox;
        }
        
        wxWindow* PreferencesDialog::createMousePreferences() {
            wxStaticBox* mouseBox = new wxStaticBox(this, wxID_ANY, wxT("Mouse"));

            wxStaticText* lookSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, wxT("Mouse Look"));
            m_lookSpeedSlider = new wxSlider(mouseBox, CommandIds::PreferencesDialog::LookSpeedSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);

            wxStaticText* invertLookFakeLabel = new wxStaticText(mouseBox, wxID_ANY, wxT(""));
            m_invertLookXAxisCheckBox = new wxCheckBox(mouseBox, CommandIds::PreferencesDialog::InvertLookXAxisCheckBoxId, wxT("Invert X Axis"));
            m_invertLookYAxisCheckBox = new wxCheckBox(mouseBox, CommandIds::PreferencesDialog::InvertLookYAxisCheckBoxId, wxT("Invert Y Axis"));
            wxSizer* invertLookSizer = new wxBoxSizer(wxHORIZONTAL);
            invertLookSizer->Add(m_invertLookXAxisCheckBox);
            invertLookSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            invertLookSizer->Add(m_invertLookYAxisCheckBox);
            
            wxStaticText* panSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, wxT("Mouse Pan"));
            m_panSpeedSlider = new wxSlider(mouseBox, CommandIds::PreferencesDialog::PanSpeedSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* invertPanFakeLabel = new wxStaticText(mouseBox, wxID_ANY, "");
            m_invertPanXAxisCheckBox = new wxCheckBox(mouseBox, CommandIds::PreferencesDialog::InvertPanXAxisCheckBoxId, wxT("Invert X Axis"));
            m_invertPanYAxisCheckBox = new wxCheckBox(mouseBox, CommandIds::PreferencesDialog::InvertPanYAxisCheckBoxId, wxT("Invert Y Axis"));
            wxSizer* invertPanSizer = new wxBoxSizer(wxHORIZONTAL);
            invertPanSizer->Add(m_invertPanXAxisCheckBox);
            invertPanSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            invertPanSizer->Add(m_invertPanYAxisCheckBox);
            
            wxStaticText* moveSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, "Mouse Move");
            m_moveSpeedSlider = new wxSlider(mouseBox, CommandIds::PreferencesDialog::MoveSpeedSliderId, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
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
            innerSizer->SetItemMinSize(lookSpeedLabel, PreferencesDialogLayout::MinimumLabelWidth, lookSpeedLabel->GetSize().y);

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::StaticBoxInnerMargin);

            mouseBox->SetSizerAndFit(outerSizer);
            return mouseBox;
        }

        PreferencesDialog::PreferencesDialog() :
        wxDialog(NULL, wxID_ANY, wxT("Preferences")) {
            wxWindow* quakePreferences = createQuakePreferences();
            wxWindow* viewPreferences = createViewPreferences();
            wxWindow* mousePreferences = createMousePreferences();
            
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(quakePreferences, 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(viewPreferences, 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(mousePreferences, 0, wxEXPAND);

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);

#ifndef __APPLE__
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::DialogOuterMargin);
            wxSizer* buttonSizer = CreateButtonSizer(wxOK | wxCANCEL);
            outerSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 7);
#else
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::DialogOuterMargin);
#endif
            
            outerSizer->SetItemMinSize(innerSizer, 600, innerSizer->GetSize().y);
            SetSizerAndFit(outerSizer);
            
#ifdef __APPLE__
            // allow the dialog to be closed using CMD+W
            wxAcceleratorEntry acceleratorEntries[1];
            acceleratorEntries[0].Set(wxACCEL_CMD, static_cast<int>('W'), wxID_CLOSE);
            wxAcceleratorTable accceleratorTable(4, acceleratorEntries);
            SetAcceleratorTable(accceleratorTable);
#endif
            
            updateControls();
        }

        void PreferencesDialog::OnChooseQuakePathClicked(wxCommandEvent& event) {
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

        void PreferencesDialog::OnViewSliderChanged(wxScrollEvent& event) {
            wxSlider* sender = static_cast<wxSlider*>(event.GetEventObject());
            int value = sender->GetValue();
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            switch (event.GetId()) {
                case CommandIds::PreferencesDialog::BrightnessSliderId:
                    prefs.setFloat(Preferences::RendererBrightness, value / 40.0f);
                    break;
                case CommandIds::PreferencesDialog::GridAlphaSliderId: {
                    int max = sender->GetMax();
                    float floatValue = static_cast<float>(value) / static_cast<float>(max);

                    Color gridColor = prefs.getColor(Preferences::GridColor);
                    gridColor.w = floatValue;
                    prefs.setColor(Preferences::GridColor, gridColor);
                    break;
                }
                default:
                    break;
            }
            
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews();
        }
        
        void PreferencesDialog::OnMouseSliderChanged(wxScrollEvent& event) {
            wxSlider* sender = static_cast<wxSlider*>(event.GetEventObject());
            float value = sender->GetValue() / 100.0f;

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            switch (event.GetId()) {
                case CommandIds::PreferencesDialog::LookSpeedSliderId:
                    prefs.setFloat(Preferences::CameraLookSpeed, value);
                    break;
                case CommandIds::PreferencesDialog::PanSpeedSliderId:
                    prefs.setFloat(Preferences::CameraPanSpeed, value);
                    break;
                case CommandIds::PreferencesDialog::MoveSpeedSliderId:
                    prefs.setFloat(Preferences::CameraMoveSpeed, value);
                    break;
                default:
                    break;
            }
        }
        
        void PreferencesDialog::OnInvertAxisChanged(wxCommandEvent& event) {
            bool value = event.GetInt() != 0;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            switch (event.GetId()) {
                case CommandIds::PreferencesDialog::InvertLookXAxisCheckBoxId:
                    prefs.setBool(Preferences::CameraLookInvertX, value);
                    break;
                case CommandIds::PreferencesDialog::InvertLookYAxisCheckBoxId:
                    prefs.setBool(Preferences::CameraLookInvertY, value);
                    break;
                case CommandIds::PreferencesDialog::InvertPanXAxisCheckBoxId:
                    prefs.setBool(Preferences::CameraPanInvertX, value);
                    break;
                case CommandIds::PreferencesDialog::InvertPanYAxisCheckBoxId:
                    prefs.setBool(Preferences::CameraPanInvertY, value);
                    break;
                default:
                    break;
            }
        }

        void PreferencesDialog::OnOkClicked(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
			prefs.save();

			Controller::Command command(Controller::Command::InvalidateEntityModelRendererCache);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &command);

			EndModal(wxID_OK);
		}

		void PreferencesDialog::OnCancelClicked(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
			prefs.discardChanges();
			EndModal(wxID_CANCEL);
		}

		void PreferencesDialog::OnCloseDialog(wxCloseEvent& event) {
#ifndef __APPLE__
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
			prefs.discardChanges();

			Controller::Command command(Controller::Command::InvalidateEntityModelRendererCache);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &command);
#endif
            event.Skip();
		}

        void PreferencesDialog::OnFileExit(wxCommandEvent& event) {
            Close();
        }
	}
}
