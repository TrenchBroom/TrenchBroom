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

#include "Controller/Command.h"
#include "View/EditorView.h"
#include "View/LayoutConstants.h"
#include "View/ViewOptions.h"

namespace TrenchBroom {
    namespace View {
        static const unsigned int ShowEntitiesCheckBoxId            = wxID_HIGHEST + 1;
        static const unsigned int ShowEntityModelsCheckBoxId        = wxID_HIGHEST + 2;
        static const unsigned int ShowEntityBoundsCheckBoxId        = wxID_HIGHEST + 3;
        static const unsigned int ShowEntityClassnamesCheckBoxId    = wxID_HIGHEST + 4;
        static const unsigned int ShowBrushesCheckBoxId             = wxID_HIGHEST + 5;
        static const unsigned int ShowClipBrushesCheckBoxId         = wxID_HIGHEST + 6;
        static const unsigned int ShowSkipBrushesCheckBoxId         = wxID_HIGHEST + 7;
        static const unsigned int FaceRenderModeChoiceId            = wxID_HIGHEST + 8;
        static const unsigned int RenderEdgesCheckBoxId             = wxID_HIGHEST + 9;
        
        BEGIN_EVENT_TABLE(ViewInspector, wxPanel)
        EVT_TEXT(wxID_ANY, ViewInspector::OnFilterPatternChanged)
        EVT_CHECKBOX(ShowEntitiesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(ShowEntityModelsCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(ShowEntityBoundsCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(ShowEntityClassnamesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(ShowBrushesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(ShowClipBrushesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(ShowSkipBrushesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHOICE(FaceRenderModeChoiceId, ViewInspector::OnRenderFaceModeSelected)
        EVT_CHECKBOX(RenderEdgesCheckBoxId, ViewInspector::OnRenderEdgesChanged)
        END_EVENT_TABLE()
        
        void ViewInspector::updateControls() {
            ViewOptions& viewOptions = m_editorView.viewOptions();
            
            m_searchBox->ChangeValue(viewOptions.filterPattern());
            m_toggleEntities->SetValue(viewOptions.showEntities());
            m_toggleEntityModels->SetValue(viewOptions.showEntityModels());
            m_toggleEntityBounds->SetValue(viewOptions.showEntityBounds());
            m_toggleEntityClassnames->SetValue(viewOptions.showEntityClassnames());
            m_toggleBrushes->SetValue(viewOptions.showBrushes());
            m_toggleClipBrushes->SetValue(viewOptions.showClipBrushes());
            m_toggleSkipBrushes->SetValue(viewOptions.showSkipBrushes());

            m_toggleEntityModels->Enable(viewOptions.showEntities());
            m_toggleEntityBounds->Enable(viewOptions.showEntities());
            m_toggleEntityClassnames->Enable(viewOptions.showEntities());
            m_toggleClipBrushes->Enable(viewOptions.showBrushes());
            m_toggleSkipBrushes->Enable(viewOptions.showBrushes());
            
            m_faceRenderModeChoice->SetSelection(viewOptions.faceRenderMode());
            m_toggleRenderEdges->SetValue(viewOptions.renderEdges());
        }

        wxWindow* ViewInspector::createFilterBox(ViewOptions& viewOptions) {
            wxStaticBox* filterBox = new wxStaticBox(this, wxID_ANY, "Filter");
            wxPanel* searchPanel = new wxPanel(filterBox);
            {
                wxStaticText* searchLabel = new wxStaticText(searchPanel, wxID_ANY, "Show objects matching");
                m_searchBox = new wxSearchCtrl(searchPanel, wxID_ANY);
                
                wxSizer* searchPanelSizer = new wxBoxSizer(wxHORIZONTAL);
                searchPanelSizer->Add(searchLabel, 0, wxEXPAND | wxTOP, 2);
                searchPanelSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
                searchPanelSizer->Add(m_searchBox, 1, wxEXPAND);
                searchPanel->SetSizerAndFit(searchPanelSizer);
            }
            
            wxPanel* togglePanel = new wxPanel(filterBox);
            {
                wxPanel* entityPanel = new wxPanel(togglePanel);
                {
                    m_toggleEntities = new wxCheckBox(entityPanel, ShowEntitiesCheckBoxId, "Entities");
                    m_toggleEntityModels = new wxCheckBox(entityPanel, ShowEntityModelsCheckBoxId, "Models");
                    m_toggleEntityBounds = new wxCheckBox(entityPanel, ShowEntityBoundsCheckBoxId, "Bounds");
                    m_toggleEntityClassnames = new wxCheckBox(entityPanel, ShowEntityClassnamesCheckBoxId, "Classnames");
                    
                    wxSizer* entityPanelSizer = new wxBoxSizer(wxVERTICAL);
                    entityPanelSizer->Add(m_toggleEntities, 0, wxEXPAND);
                    entityPanelSizer->AddSpacer(LayoutConstants::CheckBoxVerticalMargin);
                    entityPanelSizer->Add(m_toggleEntityModels, 0, wxEXPAND | wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
                    entityPanelSizer->AddSpacer(LayoutConstants::CheckBoxVerticalMargin);
                    entityPanelSizer->Add(m_toggleEntityBounds, 0, wxEXPAND | wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
                    entityPanelSizer->AddSpacer(LayoutConstants::CheckBoxVerticalMargin);
                    entityPanelSizer->Add(m_toggleEntityClassnames, 0, wxEXPAND | wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
                    entityPanel->SetSizerAndFit(entityPanelSizer);
                }
                wxPanel* brushPanel = new wxPanel(togglePanel);
                {
                    m_toggleBrushes = new wxCheckBox(brushPanel, ShowBrushesCheckBoxId, "Brushes");
                    m_toggleClipBrushes = new wxCheckBox(brushPanel, ShowClipBrushesCheckBoxId, "Clip brushes");
                    m_toggleSkipBrushes = new wxCheckBox(brushPanel, ShowSkipBrushesCheckBoxId, "Skip brushes");

                    wxSizer* brushPanelSizer = new wxBoxSizer(wxVERTICAL);
                    brushPanelSizer->Add(m_toggleBrushes, 0, wxEXPAND);
                    brushPanelSizer->AddSpacer(LayoutConstants::CheckBoxVerticalMargin);
                    brushPanelSizer->Add(m_toggleClipBrushes, 0, wxEXPAND | wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
                    brushPanelSizer->AddSpacer(LayoutConstants::CheckBoxVerticalMargin);
                    brushPanelSizer->Add(m_toggleSkipBrushes, 0, wxEXPAND | wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
                    brushPanel->SetSizerAndFit(brushPanelSizer);
                }
                
                wxSizer* togglePanelSizer = new wxBoxSizer(wxHORIZONTAL);
                togglePanelSizer->Add(entityPanel, 0, wxEXPAND);
                togglePanelSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
                togglePanelSizer->Add(brushPanel, 1, wxEXPAND);
                togglePanel->SetSizerAndFit(togglePanelSizer);
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
            
            filterBox->SetSizerAndFit(outerSizer);
            return filterBox;
        }

        wxWindow* ViewInspector::createRenderModeSelector(ViewOptions& viewOptions) {
            wxStaticBox* renderModeBox = new wxStaticBox(this, wxID_ANY, "Render mode");
            
            wxStaticText* faceRenderModeLabel = new wxStaticText(renderModeBox, wxID_ANY, "Faces");
            wxString faceRenderModes[3] = {"Render with textures", "Render flat", "Don't render"};
            m_faceRenderModeChoice = new wxChoice(renderModeBox, FaceRenderModeChoiceId, wxDefaultPosition, wxDefaultSize, 3, faceRenderModes);
            
            wxStaticText* toggleRenderEdgesLabel = new wxStaticText(renderModeBox, wxID_ANY, "");
            m_toggleRenderEdges = new wxCheckBox(renderModeBox, RenderEdgesCheckBoxId, "Render edges");
            
            // layout of the contained controls
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(faceRenderModeLabel);
            innerSizer->Add(m_faceRenderModeChoice);
            innerSizer->Add(toggleRenderEdgesLabel);
            innerSizer->Add(m_toggleRenderEdges);
            
            // creates 5 pixel border inside the static box
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::StaticBoxInnerMargin);
            
            renderModeBox->SetSizerAndFit(outerSizer);
            return renderModeBox;
        }

        ViewInspector::ViewInspector(wxWindow* parent, EditorView& editorView) :
        wxPanel(parent),
        m_editorView(editorView) {
            
            // layout of the contained controls
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(createFilterBox(m_editorView.viewOptions()), 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(createRenderModeSelector(m_editorView.viewOptions()), 0, wxEXPAND | wxBOTTOM, 2);
            
            // creates 5 pixel border inside the page
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 1, wxEXPAND | wxALL, 5);
            SetSizerAndFit(outerSizer);
            
            updateControls();
        }

        void ViewInspector::OnFilterPatternChanged(wxCommandEvent& event) {
            m_editorView.viewOptions().setFilterPattern(m_searchBox->GetValue().ToStdString());
            Controller::Command command(Controller::Command::InvalidateRendererState, m_editorView.MapDocument(), false, "Change filter text");
            m_editorView.OnUpdate(NULL, &command);
        }

        void ViewInspector::OnFilterOptionChanged(wxCommandEvent& event){
            switch (event.GetId()) {
                case ShowEntitiesCheckBoxId:
                    m_editorView.viewOptions().setShowEntities(event.GetInt() != 0);
                    m_editorView.OnUpdate(NULL); // will just trigger a refresh
                    break;
                case ShowEntityModelsCheckBoxId:
                    m_editorView.viewOptions().setShowEntityModels(event.GetInt() != 0);
                    m_editorView.OnUpdate(NULL); // will just trigger a refresh
                    break;
                case ShowEntityBoundsCheckBoxId:
                    m_editorView.viewOptions().setShowEntityBounds(event.GetInt() != 0);
                    m_editorView.OnUpdate(NULL); // will just trigger a refresh
                    break;
                case ShowEntityClassnamesCheckBoxId:
                    m_editorView.viewOptions().setShowEntityClassnames(event.GetInt() != 0);
                    m_editorView.OnUpdate(NULL); // will just trigger a refresh
                    break;
                case ShowBrushesCheckBoxId:
                    m_editorView.viewOptions().setShowBrushes(event.GetInt() != 0);
                    m_editorView.OnUpdate(NULL); // will just trigger a refresh
                    break;
                case ShowClipBrushesCheckBoxId: {
                    m_editorView.viewOptions().setShowClipBrushes(event.GetInt() != 0);
                    Controller::Command command(Controller::Command::InvalidateRendererBrushState, m_editorView.MapDocument(), false, "Change brush state");
                    m_editorView.OnUpdate(NULL, &command);
                    break;
                }
                case ShowSkipBrushesCheckBoxId: {
                    m_editorView.viewOptions().setShowSkipBrushes(event.GetInt() != 0);
                    Controller::Command command(Controller::Command::InvalidateRendererBrushState, m_editorView.MapDocument(), false, "Change brush state");
                    m_editorView.OnUpdate(NULL, &command);
                    break;
                }
            }
            updateControls();
        }

        void ViewInspector::OnRenderFaceModeSelected(wxCommandEvent& event) {
            ViewOptions::FaceRenderMode mode;
            if (m_faceRenderModeChoice->GetSelection() == 1)
                mode = ViewOptions::Flat;
            else if (m_faceRenderModeChoice->GetSelection() == 2)
                mode = ViewOptions::Discard;
            else
                mode = ViewOptions::Textured;
            m_editorView.viewOptions().setFaceRenderMode(mode);
            m_editorView.OnUpdate(NULL); // will just trigger a refresh
            updateControls(); // if something went wrong, set the choice selection to the default value ("Textured")
        }
        
        void ViewInspector::OnRenderEdgesChanged(wxCommandEvent& event) {
            m_editorView.viewOptions().setRenderEdges(event.GetInt() != 0);
            m_editorView.OnUpdate(NULL); // will just trigger a refresh
        }
    }
}
