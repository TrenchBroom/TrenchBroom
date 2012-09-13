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
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

#include "View/LayoutConstants.h"

namespace TrenchBroom {
    namespace View {
        namespace PreferencesDialogLayout {
            static const int MinimumLabelWidth = 100;
        }
        
        wxWindow* PreferencesDialog::createQuakePreferences() {
            wxStaticBox* quakeBox = new wxStaticBox(this, wxID_ANY, "Quake");
            
            wxStaticText* quakePathLabel = new wxStaticText(quakeBox, wxID_ANY, "Quake Path");
            wxStaticText* quakePathValue = new wxStaticText(quakeBox, wxID_ANY, "/Applications/Quake");
            wxButton* chooseQuakePathButton = new wxButton(quakeBox, wxID_ANY, "Choose...");
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(3, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(quakePathLabel);
            innerSizer->Add(quakePathValue, 0, wxEXPAND);
            innerSizer->Add(chooseQuakePathButton);
            innerSizer->SetItemMinSize(quakePathLabel, PreferencesDialogLayout::MinimumLabelWidth, quakePathLabel->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::StaticBoxInnerMargin);
            
            quakeBox->SetSizerAndFit(outerSizer);
            return quakeBox;
        }

        wxWindow* PreferencesDialog::createViewPreferences() {
            wxStaticBox* viewBox = new wxStaticBox(this, wxID_ANY, "View");
            
            wxStaticText* brightnessLabel = new wxStaticText(viewBox, wxID_ANY, "Brightness");
            wxSlider* brightnessSlider = new wxSlider(viewBox, wxID_ANY, 10, 1, 20, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_BOTTOM);
            
            wxStaticText* gridAlphaLabel = new wxStaticText(viewBox, wxID_ANY, "Grid Alpha");
            wxSlider* gridAlphaSlider = new wxSlider(viewBox, wxID_ANY, 10, 1, 20, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_BOTTOM);
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(brightnessLabel);
            innerSizer->Add(brightnessSlider, 0, wxEXPAND);
            innerSizer->Add(gridAlphaLabel);
            innerSizer->Add(gridAlphaSlider, 0, wxEXPAND);
            innerSizer->SetItemMinSize(brightnessLabel, PreferencesDialogLayout::MinimumLabelWidth, brightnessLabel->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::StaticBoxInnerMargin);
            
            viewBox->SetSizerAndFit(outerSizer);
            return viewBox;
        }
        
        wxWindow* PreferencesDialog::createMousePreferences() {
            wxStaticBox* mouseBox = new wxStaticBox(this, wxID_ANY, "Mouse");

            wxStaticText* lookSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, "Mouse Look");
            wxSlider* lookSpeedSlider = new wxSlider(mouseBox, wxID_ANY, 10, 1, 20, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_BOTTOM);

            wxStaticText* invertLookFakeLabel = new wxStaticText(mouseBox, wxID_ANY, "");
            wxCheckBox* invertLookXAxis = new wxCheckBox(mouseBox, wxID_ANY, "Invert X Axis");
            wxCheckBox* invertLookYAxis = new wxCheckBox(mouseBox, wxID_ANY, "Invert Y Axis");
            wxSizer* invertLookSizer = new wxBoxSizer(wxHORIZONTAL);
            invertLookSizer->Add(invertLookXAxis);
            invertLookSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            invertLookSizer->Add(invertLookYAxis);
            
            wxStaticText* panSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, "Mouse Pan");
            wxSlider* panSpeedSlider = new wxSlider(mouseBox, wxID_ANY, 10, 1, 20, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_BOTTOM);
            
            wxStaticText* invertPanFakeLabel = new wxStaticText(mouseBox, wxID_ANY, "");
            wxCheckBox* invertPanXAxis = new wxCheckBox(mouseBox, wxID_ANY, "Invert X Axis");
            wxCheckBox* invertPanYAxis = new wxCheckBox(mouseBox, wxID_ANY, "Invert Y Axis");
            wxSizer* invertPanSizer = new wxBoxSizer(wxHORIZONTAL);
            invertPanSizer->Add(invertPanXAxis);
            invertPanSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            invertPanSizer->Add(invertPanYAxis);
            
            wxStaticText* moveSpeedLabel = new wxStaticText(mouseBox, wxID_ANY, "Mouse Move");
            wxSlider* moveSpeedSlider = new wxSlider(mouseBox, wxID_ANY, 10, 1, 20, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_BOTTOM);
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(lookSpeedLabel);
            innerSizer->Add(lookSpeedSlider, 0, wxEXPAND);
            innerSizer->Add(invertLookFakeLabel);
            innerSizer->Add(invertLookSizer);
            innerSizer->Add(panSpeedLabel);
            innerSizer->Add(panSpeedSlider, 0, wxEXPAND);
            innerSizer->Add(invertPanFakeLabel);
            innerSizer->Add(invertPanSizer);
            innerSizer->Add(moveSpeedLabel);
            innerSizer->Add(moveSpeedSlider, 0, wxEXPAND);
            innerSizer->SetItemMinSize(lookSpeedLabel, PreferencesDialogLayout::MinimumLabelWidth, lookSpeedLabel->GetSize().y);

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::StaticBoxInnerMargin);

            mouseBox->SetSizerAndFit(outerSizer);
            return mouseBox;
        }

        PreferencesDialog::PreferencesDialog() :
        wxDialog(NULL, wxID_ANY, "Preferences") {
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
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, 10);
            outerSizer->SetItemMinSize(innerSizer, 600, innerSizer->GetSize().y);
            
            SetSizerAndFit(outerSizer);
        }
    }
}