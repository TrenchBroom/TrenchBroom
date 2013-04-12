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

#include "Controller/Command.h"
#include "View/CommandIds.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "View/LayoutConstants.h"
#include "View/ViewOptions.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {

        BEGIN_EVENT_TABLE(ViewInspector, wxPanel)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowEntitiesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowEntityModelsCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowEntityBoundsCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowEntityClassnamesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowBrushesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowClipBrushesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowSkipBrushesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHOICE(CommandIds::ViewInspector::FaceRenderModeChoiceId, ViewInspector::OnRenderFaceModeSelected)
        EVT_CHECKBOX(CommandIds::ViewInspector::RenderEdgesCheckBoxId, ViewInspector::OnRenderEdgesChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::FaceShadingCheckBoxId, ViewInspector::OnFaceShadingChanged)
        // EVT_CHECKBOX(CommandIds::ViewInspector::FogCheckBoxId, ViewInspector::OnFogChanged)
        EVT_CHOICE(CommandIds::ViewInspector::LinkDisplayModeChoiceId, ViewInspector::OnLinkDisplayModeSelected)
        END_EVENT_TABLE()

        void ViewInspector::updateControls() {
            if (!m_documentViewHolder.valid())
                return;

            EditorView& editorView = m_documentViewHolder.view();
            ViewOptions& viewOptions = editorView.viewOptions();

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
            m_toggleFaceShading->SetValue(viewOptions.shadeFaces());
//            m_toggleFog->SetValue(viewOptions.useFog());
            m_linkDisplayModeChoice->SetSelection(viewOptions.linkDisplayMode());
        }

        wxWindow* ViewInspector::createFilterBox() {
            wxStaticBox* filterBox = new wxStaticBox(this, wxID_ANY, wxT("Filter"));
            
            m_toggleEntities = new wxCheckBox(filterBox, CommandIds::ViewInspector::ShowEntitiesCheckBoxId, wxT("Entities"));
            m_toggleEntityModels = new wxCheckBox(filterBox, CommandIds::ViewInspector::ShowEntityModelsCheckBoxId, wxT("Models"));
            m_toggleEntityBounds = new wxCheckBox(filterBox, CommandIds::ViewInspector::ShowEntityBoundsCheckBoxId, wxT("Bounds"));
            m_toggleEntityClassnames = new wxCheckBox(filterBox, CommandIds::ViewInspector::ShowEntityClassnamesCheckBoxId, wxT("Classnames"));
            
            m_toggleBrushes = new wxCheckBox(filterBox, CommandIds::ViewInspector::ShowBrushesCheckBoxId, wxT("Brushes"));
            m_toggleClipBrushes = new wxCheckBox(filterBox, CommandIds::ViewInspector::ShowClipBrushesCheckBoxId, wxT("Clip brushes"));
            m_toggleSkipBrushes = new wxCheckBox(filterBox, CommandIds::ViewInspector::ShowSkipBrushesCheckBoxId, wxT("Skip brushes"));
            
            wxGridBagSizer* filterPanelSizer = new wxGridBagSizer(LayoutConstants::CheckBoxVerticalMargin, LayoutConstants::ControlHorizontalMargin);
            filterPanelSizer->Add(m_toggleEntities, wxGBPosition(0, 0));
            filterPanelSizer->Add(m_toggleEntityModels, wxGBPosition(1, 0), wxDefaultSpan, wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
            filterPanelSizer->Add(m_toggleEntityBounds, wxGBPosition(2, 0), wxDefaultSpan, wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
            filterPanelSizer->Add(m_toggleEntityClassnames, wxGBPosition(3, 0), wxDefaultSpan, wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
            
            filterPanelSizer->Add(m_toggleBrushes, wxGBPosition(0, 1));
            filterPanelSizer->Add(m_toggleClipBrushes, wxGBPosition(1, 1), wxDefaultSpan, wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
            filterPanelSizer->Add(m_toggleSkipBrushes, wxGBPosition(2, 1), wxDefaultSpan, wxLEFT, LayoutConstants::CheckBoxHierarchyLeftMargin);
            filterPanelSizer->AddGrowableCol(1);
            
            // layout of the contained controls
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            outerSizer->Add(filterPanelSizer, 1, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);

            filterBox->SetSizerAndFit(outerSizer);
            return filterBox;
        }

        wxWindow* ViewInspector::createRenderModeSelector() {
            wxStaticBox* renderModeBox = new wxStaticBox(this, wxID_ANY, wxT("Render mode"));

            wxStaticText* faceRenderModeLabel = new wxStaticText(renderModeBox, wxID_ANY, wxT("Faces"));
            wxString faceRenderModes[3] = {wxT("Render with textures"), wxT("Render flat"), wxT("Don't render")};
            m_faceRenderModeChoice = new wxChoice(renderModeBox, CommandIds::ViewInspector::FaceRenderModeChoiceId, wxDefaultPosition, wxDefaultSize, 3, faceRenderModes);

            wxStaticText* toggleRenderEdgesLabel = new wxStaticText(renderModeBox, wxID_ANY, wxT(""));
            m_toggleRenderEdges = new wxCheckBox(renderModeBox, CommandIds::ViewInspector::RenderEdgesCheckBoxId, wxT("Render edges"));
            
            wxStaticText* toggleFaceShadingLabel = new wxStaticText( renderModeBox, wxID_ANY, wxT(""));
            m_toggleFaceShading = new wxCheckBox( renderModeBox, CommandIds::ViewInspector::FaceShadingCheckBoxId, wxT("Shade faces"));
            
            /*
            wxStaticText* toggleFogLabel = new wxStaticText( renderModeBox, wxID_ANY, wxT(""));
            m_toggleFog = new wxCheckBox( renderModeBox, CommandIds::ViewInspector::FogCheckBoxId, wxT("Apply fog"));
             */

            wxStaticText* linkDisplayModeLabel = new wxStaticText(renderModeBox, wxID_ANY, wxT("Links"));
            wxString linkDisplayModes[4] = {wxT("Context"), wxT("Local"), wxT("All"), wxT("Don't show")};
            m_linkDisplayModeChoice = new wxChoice(renderModeBox, CommandIds::ViewInspector::LinkDisplayModeChoiceId, wxDefaultPosition, wxDefaultSize, 4, linkDisplayModes);
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, 0, LayoutConstants::ControlHorizontalMargin);
            innerSizer->Add(faceRenderModeLabel);
            innerSizer->Add(m_faceRenderModeChoice);
            innerSizer->Add(toggleRenderEdgesLabel, 0, wxTOP, LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(m_toggleRenderEdges, 0, wxTOP, LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(toggleFaceShadingLabel, 0, wxTOP, LayoutConstants::CheckBoxVerticalMargin);
            innerSizer->Add(m_toggleFaceShading, 0, wxTOP, LayoutConstants::CheckBoxVerticalMargin);
            innerSizer->Add(linkDisplayModeLabel, 0, wxTOP, LayoutConstants::CheckBoxVerticalMargin);
            innerSizer->Add(m_linkDisplayModeChoice, 0, wxTOP, LayoutConstants::CheckBoxVerticalMargin);

            // creates 5 pixel border inside the static box
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);

            renderModeBox->SetSizerAndFit(outerSizer);
            return renderModeBox;
        }

        ViewInspector::ViewInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder) :
        wxPanel(parent),
        m_documentViewHolder(documentViewHolder) {

            // layout of the contained controls
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->Add(createFilterBox(), 0, wxEXPAND);
            innerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(createRenderModeSelector(), 0, wxEXPAND | wxBOTTOM, LayoutConstants::NotebookPageExtraBottomMargin);

            // creates 5 pixel border inside the page
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 1, wxEXPAND | wxALL, LayoutConstants::NotebookPageInnerMargin);
            SetSizerAndFit(outerSizer);

            updateControls();
        }

        void ViewInspector::OnFilterOptionChanged(wxCommandEvent& event){
            if (!m_documentViewHolder.valid())
                return;

            EditorView& editorView = m_documentViewHolder.view();
            switch (event.GetId()) {
                case CommandIds::ViewInspector::ShowEntitiesCheckBoxId:
                    editorView.viewOptions().setShowEntities(event.GetInt() != 0);
                    editorView.OnUpdate(NULL); // will just trigger a refresh
                    break;
                case CommandIds::ViewInspector::ShowEntityModelsCheckBoxId:
                    editorView.viewOptions().setShowEntityModels(event.GetInt() != 0);
                    editorView.OnUpdate(NULL); // will just trigger a refresh
                    break;
                case CommandIds::ViewInspector::ShowEntityBoundsCheckBoxId:
                    editorView.viewOptions().setShowEntityBounds(event.GetInt() != 0);
                    editorView.OnUpdate(NULL); // will just trigger a refresh
                    break;
                case CommandIds::ViewInspector::ShowEntityClassnamesCheckBoxId:
                    editorView.viewOptions().setShowEntityClassnames(event.GetInt() != 0);
                    editorView.OnUpdate(NULL); // will just trigger a refresh
                    break;
                case CommandIds::ViewInspector::ShowBrushesCheckBoxId: {
                    editorView.viewOptions().setShowBrushes(event.GetInt() != 0);
                    Controller::Command command(Controller::Command::InvalidateRendererBrushState);
                    editorView.OnUpdate(NULL, &command);
                    break;
                }
                case CommandIds::ViewInspector::ShowClipBrushesCheckBoxId: {
                    editorView.viewOptions().setShowClipBrushes(event.GetInt() != 0);
                    Controller::Command command(Controller::Command::InvalidateRendererBrushState);
                    editorView.OnUpdate(NULL, &command);
                    break;
                }
                case CommandIds::ViewInspector::ShowSkipBrushesCheckBoxId: {
                    editorView.viewOptions().setShowSkipBrushes(event.GetInt() != 0);
                    Controller::Command command(Controller::Command::InvalidateRendererBrushState);
                    editorView.OnUpdate(NULL, &command);
                    break;
                }
            }
            updateControls();
        }

        void ViewInspector::OnRenderFaceModeSelected(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;

            EditorView& editorView = m_documentViewHolder.view();
            ViewOptions::FaceRenderMode mode;
            if (m_faceRenderModeChoice->GetSelection() == 1)
                mode = ViewOptions::Flat;
            else if (m_faceRenderModeChoice->GetSelection() == 2)
                mode = ViewOptions::Discard;
            else
                mode = ViewOptions::Textured;
            editorView.viewOptions().setFaceRenderMode(mode);
            editorView.OnUpdate(NULL); // will just trigger a refresh
            updateControls(); // if something went wrong, set the choice selection to the default value ("Textured")
        }

        void ViewInspector::OnRenderEdgesChanged(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;

            EditorView& editorView = m_documentViewHolder.view();
            editorView.viewOptions().setRenderEdges(event.GetInt() != 0);
            editorView.OnUpdate(NULL); // will just trigger a refresh
        }

        void ViewInspector::OnFaceShadingChanged( wxCommandEvent& event ) {
            if( !m_documentViewHolder.valid() ) 
                return;

            EditorView& editorView = m_documentViewHolder.view();
            editorView.viewOptions().setShadeFaces(event.GetInt() != 0);
            editorView.OnUpdate(NULL); // will just trigger a refresh
        }

        /*
        void ViewInspector::OnFogChanged( wxCommandEvent& event ) {
            if( !m_documentViewHolder.valid() ) 
                return;

            EditorView& editorView = m_documentViewHolder.view();
            editorView.viewOptions().setUseFog(event.GetInt() != 0);
            editorView.OnUpdate(NULL); // will just trigger a refresh
        }
         */

        void ViewInspector::OnLinkDisplayModeSelected(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;

            EditorView& editorView = m_documentViewHolder.view();
            ViewOptions::LinkDisplayMode mode;

            switch (m_linkDisplayModeChoice->GetSelection()) {
            case 1:
                mode = ViewOptions::LinkDisplayLocal;
                break;
            case 2:
                mode = ViewOptions::LinkDisplayAll;
                break;
            case 3:
                mode = ViewOptions::LinkDisplayNone;
                break;
            default:
                mode = ViewOptions::LinkDisplayContext;
            }

            editorView.viewOptions().setLinkDisplayMode(mode);
            Controller::Command command(Controller::Command::InvalidateRendererEntityState);
            editorView.OnUpdate(NULL, &command); // invalidate entities to invalidate decorators
            updateControls(); // if something went wrong, set the choice selection to the default value ("Textured")
        }
    }
}
