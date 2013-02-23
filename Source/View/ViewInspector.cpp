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
#include "View/CommandIds.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "View/LayoutConstants.h"
#include "View/ViewOptions.h"

namespace TrenchBroom {
    namespace View {

        BEGIN_EVENT_TABLE(ViewInspector, wxPanel)
        EVT_TEXT(wxID_ANY, ViewInspector::OnFilterPatternChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowEntitiesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowEntityModelsCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowEntityBoundsCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowEntityClassnamesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowBrushesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowClipBrushesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHECKBOX(CommandIds::ViewInspector::ShowSkipBrushesCheckBoxId, ViewInspector::OnFilterOptionChanged)
        EVT_CHOICE(CommandIds::ViewInspector::FaceRenderModeChoiceId, ViewInspector::OnRenderFaceModeSelected)
        EVT_CHECKBOX(CommandIds::ViewInspector::RenderEdgesCheckBoxId, ViewInspector::OnRenderEdgesChanged)
        END_EVENT_TABLE()

        void ViewInspector::updateControls() {
            if (!m_documentViewHolder.valid())
                return;

            EditorView& editorView = m_documentViewHolder.view();
            ViewOptions& viewOptions = editorView.viewOptions();

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

        wxWindow* ViewInspector::createFilterBox() {
            wxStaticBox* filterBox = new wxStaticBox(this, wxID_ANY, wxT("Filter"));
            wxPanel* searchPanel = new wxPanel(filterBox);
            {
                wxStaticText* searchLabel = new wxStaticText(searchPanel, wxID_ANY, wxT("Show objects matching"));
                m_searchBox = new wxSearchCtrl(searchPanel, wxID_ANY);
                m_searchBox->ShowCancelButton(true);

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
                    m_toggleEntities = new wxCheckBox(entityPanel, CommandIds::ViewInspector::ShowEntitiesCheckBoxId, wxT("Entities"));
                    m_toggleEntityModels = new wxCheckBox(entityPanel, CommandIds::ViewInspector::ShowEntityModelsCheckBoxId, wxT("Models"));
                    m_toggleEntityBounds = new wxCheckBox(entityPanel, CommandIds::ViewInspector::ShowEntityBoundsCheckBoxId, wxT("Bounds"));
                    m_toggleEntityClassnames = new wxCheckBox(entityPanel, CommandIds::ViewInspector::ShowEntityClassnamesCheckBoxId, wxT("Classnames"));

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
                    m_toggleBrushes = new wxCheckBox(brushPanel, CommandIds::ViewInspector::ShowBrushesCheckBoxId, wxT("Brushes"));
                    m_toggleClipBrushes = new wxCheckBox(brushPanel, CommandIds::ViewInspector::ShowClipBrushesCheckBoxId, wxT("Clip brushes"));
                    m_toggleSkipBrushes = new wxCheckBox(brushPanel, CommandIds::ViewInspector::ShowSkipBrushesCheckBoxId, wxT("Skip brushes"));

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
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            outerSizer->Add(searchPanel, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            outerSizer->AddSpacer(LayoutConstants::DefaultVerticalMargin);
            outerSizer->Add(new wxStaticLine(filterBox), 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            outerSizer->AddSpacer(LayoutConstants::DefaultVerticalMargin);
            outerSizer->Add(togglePanel, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, LayoutConstants::StaticBoxInnerMargin);
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

            // layout of the contained controls
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(faceRenderModeLabel);
            innerSizer->Add(m_faceRenderModeChoice);
            innerSizer->Add(toggleRenderEdgesLabel);
            innerSizer->Add(m_toggleRenderEdges);

            // creates 5 pixel border inside the static box
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxALL, LayoutConstants::StaticBoxInnerMargin);
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

        void ViewInspector::OnFilterPatternChanged(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;

            EditorView& editorView = m_documentViewHolder.view();
            editorView.viewOptions().setFilterPattern(m_searchBox->GetValue().ToStdString());
            Controller::Command command(Controller::Command::InvalidateRendererState);
            editorView.OnUpdate(NULL, &command);
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
    }
}
