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

#include "ViewInspector.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "View/LayoutConstants.h"

namespace TrenchBroom {
    namespace View {
        
        wxWindow* ViewInspector::createFilterBox() {
            wxStaticBox* filterBox = new wxStaticBox(this, wxID_ANY, "Filter");
            wxPanel* searchPanel = new wxPanel(filterBox);
            {
                wxStaticText* searchLabel = new wxStaticText(searchPanel, wxID_ANY, "Show objects matching");
                wxSearchCtrl* searchBox = new wxSearchCtrl(searchPanel, wxID_ANY);
                
                wxSizer* searchPanelSizer = new wxBoxSizer(wxHORIZONTAL);
                searchPanelSizer->Add(searchLabel, 0, wxEXPAND | wxTOP, 2);
                searchPanelSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
                searchPanelSizer->Add(searchBox, 1, wxEXPAND);
                searchPanel->SetSizer(searchPanelSizer);
            }
            
            wxPanel* togglePanel = new wxPanel(filterBox);
            {
                wxPanel* entityPanel = new wxPanel(togglePanel);
                {
                    wxCheckBox* toggleEntities = new wxCheckBox(entityPanel, wxID_ANY, "Entities");
                    wxCheckBox* toggleEntityModels = new wxCheckBox(entityPanel, wxID_ANY, "Models");
                    wxCheckBox* toggleEntityBounds = new wxCheckBox(entityPanel, wxID_ANY, "Bounds");
                    wxCheckBox* toggleEntityClassnames = new wxCheckBox(entityPanel, wxID_ANY, "Classnames");
                    
                    wxSizer* entityPanelSizer = new wxBoxSizer(wxVERTICAL);
                    entityPanelSizer->Add(toggleEntities, 0, wxEXPAND);
                    entityPanelSizer->AddSpacer(LayoutConstants::CheckBoxVerticalMargin);
                    entityPanelSizer->Add(toggleEntityModels, 0, wxEXPAND | wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
                    entityPanelSizer->AddSpacer(LayoutConstants::CheckBoxVerticalMargin);
                    entityPanelSizer->Add(toggleEntityBounds, 0, wxEXPAND | wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
                    entityPanelSizer->AddSpacer(LayoutConstants::CheckBoxVerticalMargin);
                    entityPanelSizer->Add(toggleEntityClassnames, 0, wxEXPAND | wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
                    entityPanel->SetSizer(entityPanelSizer);
                }
                wxPanel* brushPanel = new wxPanel(togglePanel);
                {
                    wxCheckBox* toggleBrushes = new wxCheckBox(brushPanel, wxID_ANY, "Brushes");
                    wxCheckBox* toggleClipBrushes = new wxCheckBox(brushPanel, wxID_ANY, "Clip brushes");

                    wxSizer* brushPanelSizer = new wxBoxSizer(wxVERTICAL);
                    brushPanelSizer->Add(toggleBrushes, 0, wxEXPAND);
                    brushPanelSizer->AddSpacer(LayoutConstants::CheckBoxVerticalMargin);
                    brushPanelSizer->Add(toggleClipBrushes, 0, wxEXPAND | wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
                    brushPanel->SetSizer(brushPanelSizer);
                }
                
                wxSizer* togglePanelSizer = new wxBoxSizer(wxHORIZONTAL);
                togglePanelSizer->Add(entityPanel, 0, wxEXPAND);
                togglePanelSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
                togglePanelSizer->Add(brushPanel, 1, wxEXPAND);
                togglePanel->SetSizer(togglePanelSizer);
            }
            
            // layout of the contained controls
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(searchPanel, 0, wxEXPAND | wxTOP, LayoutConstants::StaticBoxTopMargin);
            innerSizer->AddSpacer(LayoutConstants::DefaultVerticalMargin);
            innerSizer->Add(new wxStaticLine(filterBox), 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::DefaultVerticalMargin);
            innerSizer->Add(togglePanel, 1, wxEXPAND);
            
            // creates 5 pixel border inside the static box
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::StaticBoxInnerMargin);
            
            filterBox->SetSizer(outerSizer);
            return filterBox;
        }

        wxWindow* ViewInspector::createRenderModeSelector() {
            wxPanel* renderModePanel = new wxPanel(this);
            
            // label
            wxStaticText* renderModeLabel = new wxStaticText(renderModePanel, wxID_ANY, "Render mode");
            wxFont font = renderModeLabel->GetFont();
            font.SetWeight(wxFONTWEIGHT_BOLD);
            renderModeLabel->SetFont(font);
            
            // choice drop down
            wxString renderModes[3] = {"Textured", "Flat", "Wireframe"};
            wxChoice* renderModeChoice = new wxChoice(renderModePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, renderModes);
            
            // put both next to each other, allowing the dropdown to expand
            wxSizer* renderModePanelSizer = new wxBoxSizer(wxHORIZONTAL);
            renderModePanelSizer->Add(renderModeLabel, 0, wxEXPAND | wxTOP, 1);
            renderModePanelSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            renderModePanelSizer->Add(renderModeChoice, 1, wxEXPAND);
            renderModePanel->SetSizer(renderModePanelSizer);
            
            return renderModePanel;
        }

        ViewInspector::ViewInspector(wxWindow* parent) :
        wxPanel(parent) {
            
            // layout of the contained controls
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(createFilterBox(), 1, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(createRenderModeSelector(), 0, wxEXPAND | wxBOTTOM, 2);
            
            // creates 5 pixel border inside the page
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 1, wxEXPAND | wxALL, 5);
            SetSizer(outerSizer);
        }
    }
}
