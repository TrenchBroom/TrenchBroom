/*
 Copyright (C) 2010-2016 Kristian Duske

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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ViewEditor.h"

#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionGroup.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/EditorContext.h"
#include "Model/Game.h"
#include "View/BorderLine.h"
#include "View/BorderPanel.h"
#include "View/MapDocument.h"
#include "View/MapViewConfig.h"
#include "View/PopupButton.h"
#include "View/RadioGroup.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/gbsizer.h>
#include <wx/scrolwin.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        EntityDefinitionCheckBoxList::EntityDefinitionCheckBoxList(wxWindow* parent, Assets::EntityDefinitionManager& entityDefinitionManager, Model::EditorContext& editorContext) :
        wxPanel(parent),
        m_entityDefinitionManager(entityDefinitionManager),
        m_editorContext(editorContext) {
            createGui();
            refresh();
        }

        void EntityDefinitionCheckBoxList::refresh() {
            size_t defIndex = 0;
            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const Assets::EntityDefinitionList& definitions = group.definitions();

                if (!definitions.empty()) {
                    const bool firstHidden = m_editorContext.entityDefinitionHidden(definitions[0]);
                    bool mixed = false;
                    for (size_t j = 0; j < definitions.size(); ++j) {
                        const bool hidden = m_editorContext.entityDefinitionHidden(definitions[j]);
                        mixed |= (hidden != firstHidden);
                        m_defCheckBoxes[defIndex++]->SetValue(!hidden);
                    }

                    if (mixed)
                        m_groupCheckBoxes[i]->Set3StateValue(wxCHK_UNDETERMINED);
                    else
                        m_groupCheckBoxes[i]->SetValue(!firstHidden);
                    m_groupCheckBoxes[i]->Enable();
                } else {
                    m_groupCheckBoxes[i]->SetValue(true);
                    m_groupCheckBoxes[i]->Disable();
                }
            }
        }

        void EntityDefinitionCheckBoxList::OnGroupCheckBoxChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const wxVariant* variant = static_cast<wxVariant*>(event.GetEventUserData());
            const size_t groupIndex = static_cast<size_t>(variant->GetLong());

            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            ensure(groupIndex < m_entityDefinitionManager.groups().size(), "index out of range");
            const Assets::EntityDefinitionGroup& group = groups[groupIndex];

            const Assets::EntityDefinitionList& definitions = group.definitions();
            for (size_t i = 0; i < definitions.size(); ++i) {
                const Assets::EntityDefinition* definition = definitions[i];
                m_editorContext.setEntityDefinitionHidden(definition, !event.IsChecked());
            }

            refresh();
        }

        void EntityDefinitionCheckBoxList::OnDefCheckBoxChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const wxVariant* variant = static_cast<wxVariant*>(event.GetEventUserData());
            const Assets::EntityDefinition* definition = reinterpret_cast<const Assets::EntityDefinition*>(variant->GetVoidPtr());
            m_editorContext.setEntityDefinitionHidden(definition, !event.IsChecked());
            refresh();
        }

        void EntityDefinitionCheckBoxList::OnShowAllClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            hideAll(false);
        }

        void EntityDefinitionCheckBoxList::OnHideAllClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            hideAll(true);
        }

        void EntityDefinitionCheckBoxList::hideAll(const bool hidden) {
            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const Assets::EntityDefinitionList& definitions = group.definitions();
                for (size_t j = 0; j < definitions.size(); ++j) {
                    const Assets::EntityDefinition* definition = definitions[j];
                    m_editorContext.setEntityDefinitionHidden(definition, hidden);
                }
            }
        }

        void EntityDefinitionCheckBoxList::createGui() {
            BorderPanel* border = new BorderPanel(this);
            border->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));

            wxScrolledWindow* scrollWindow = new wxScrolledWindow(border);
            int checkBoxHeight = 1;

            wxSizer* scrollWindowSizer = new wxBoxSizer(wxVERTICAL);
            scrollWindowSizer->AddSpacer(1);
            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const Assets::EntityDefinitionList& definitions = group.definitions();
                const String& groupName = group.displayName();

                wxCheckBox* groupCB = new wxCheckBox(scrollWindow, wxID_ANY, groupName, wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
                groupCB->SetFont(groupCB->GetFont().Bold());
                groupCB->Bind(wxEVT_CHECKBOX, &EntityDefinitionCheckBoxList::OnGroupCheckBoxChanged, this, wxID_ANY, wxID_ANY, new wxVariant(static_cast<long>(i)));
                m_groupCheckBoxes.push_back(groupCB);

                scrollWindowSizer->Add(groupCB, 0, wxLEFT, 1);
                checkBoxHeight = groupCB->GetSize().y;

                Assets::EntityDefinitionList::const_iterator defIt, defEnd;
                for (defIt = definitions.begin(), defEnd = definitions.end(); defIt != defEnd; ++defIt) {
                    Assets::EntityDefinition* definition = *defIt;
                    const String defName = definition->name();

                    wxCheckBox* defCB = new wxCheckBox(scrollWindow, wxID_ANY, defName);
                    defCB->Bind(wxEVT_CHECKBOX, &EntityDefinitionCheckBoxList::OnDefCheckBoxChanged, this, wxID_ANY, wxID_ANY, new wxVariant(reinterpret_cast<void*>(definition)));

                    m_defCheckBoxes.push_back(defCB);
                    scrollWindowSizer->Add(defCB, wxSizerFlags().Border(wxLEFT, 11));
                }
            }

            scrollWindowSizer->AddSpacer(1);
            scrollWindow->SetSizer(scrollWindowSizer);
            scrollWindow->SetScrollRate(1, checkBoxHeight);

            wxSizer* borderSizer = new wxBoxSizer(wxVERTICAL);
            borderSizer->Add(scrollWindow, wxSizerFlags().Border(wxALL, 1).Expand().Proportion(1));
            border->SetSizer(borderSizer);

            wxButton* showAllButton = new wxButton(this, wxID_ANY, "Show all", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            showAllButton->SetFont(showAllButton->GetFont().Bold());
            wxButton* hideAllButton = new wxButton(this, wxID_ANY, "Hide all", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            hideAllButton->SetFont(hideAllButton->GetFont().Bold());

            showAllButton->Bind(wxEVT_BUTTON, &EntityDefinitionCheckBoxList::OnShowAllClicked, this);
            hideAllButton->Bind(wxEVT_BUTTON, &EntityDefinitionCheckBoxList::OnHideAllClicked, this);

            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->AddStretchSpacer();
            buttonSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            buttonSizer->Add(showAllButton);
            buttonSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            buttonSizer->Add(hideAllButton);
            buttonSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            buttonSizer->AddStretchSpacer();

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(border, wxSizerFlags().Expand().Proportion(1));
            outerSizer->Add(buttonSizer, wxSizerFlags().Border(wxTOP | wxBOTTOM, 1).Expand());

            SetSizer(outerSizer);
        }

        ViewEditor::ViewEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            bindObservers();
        }

        ViewEditor::~ViewEditor() {
            unbindObservers();
        }

        void ViewEditor::OnShowEntityClassnamesChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowEntityClassnames(event.IsChecked());
        }

        void ViewEditor::OnShowEntityBoundsChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowEntityBounds(event.IsChecked());
        }

        void ViewEditor::OnShowPointEntitiesChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setShowPointEntities(event.IsChecked());
        }

        void ViewEditor::OnShowPointEntityModelsChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowPointEntityModels(event.IsChecked());
        }

        void ViewEditor::OnShowBrushesChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setShowBrushes(event.IsChecked());
        }

        void ViewEditor::OnShowBrushContentTypeChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::GamePtr game = document->game();

            if (game.get() != NULL) {
                Model::BrushContentType::FlagType hiddenFlags = 0;
                const Model::BrushContentType::List& contentTypes = game->brushContentTypes();

                for (size_t i = 0; i < contentTypes.size(); ++i) {
                    const Model::BrushContentType& contentType = contentTypes[i];
                    wxCheckBox* checkBox = m_brushContentTypeCheckBoxes[i];
                    if (!checkBox->GetValue())
                        hiddenFlags |= contentType.flagValue();
                }

                Model::EditorContext& editorContext = document->editorContext();
                editorContext.setHiddenBrushContentTypes(hiddenFlags);
            }
        }

        void ViewEditor::OnFaceRenderModeChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();

            switch (event.GetSelection()) {
                case 1:
                    config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Flat);
                    break;
                case 2:
                    config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Skip);
                    break;
                default:
                    config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Textured);
                    break;
            }
        }

        void ViewEditor::OnShadeFacesChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShadeFaces(event.IsChecked());
        }

        void ViewEditor::OnShowFogChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowFog(event.IsChecked());
        }

        void ViewEditor::OnShowEdgesChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowEdges(event.IsChecked());
        }

        void ViewEditor::OnEntityLinkModeChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();

            switch (event.GetSelection()) {
                case 0:
                    editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_All);
                    break;
                case 1:
                    editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_Transitive);
                    break;
                case 2:
                    editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_Direct);
                    break;
                default:
                    editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_None);
                    break;
            }
        }

        void ViewEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &ViewEditor::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &ViewEditor::documentWasNewedOrLoaded);
            document->editorContextDidChangeNotifier.addObserver(this, &ViewEditor::editorContextDidChange);
            document->mapViewConfigDidChangeNotifier.addObserver(this, &ViewEditor::mapViewConfigDidChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &ViewEditor::entityDefinitionsDidChange);
        }

        void ViewEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &ViewEditor::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &ViewEditor::documentWasNewedOrLoaded);
                document->editorContextDidChangeNotifier.removeObserver(this, &ViewEditor::editorContextDidChange);
                document->mapViewConfigDidChangeNotifier.removeObserver(this, &ViewEditor::mapViewConfigDidChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &ViewEditor::entityDefinitionsDidChange);
            }
        }

        void ViewEditor::documentWasNewedOrLoaded(MapDocument* document) {
            createGui();
            refreshGui();
        }

        void ViewEditor::editorContextDidChange() {
            refreshGui();
        }

        void ViewEditor::mapViewConfigDidChange() {
            refreshGui();
        }

        void ViewEditor::entityDefinitionsDidChange() {
            createGui();
            refreshGui();
        }

        void ViewEditor::createGui() {
			SetSizer(NULL);
            DestroyChildren();

            wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::WideVMargin, LayoutConstants::WideHMargin);
            sizer->Add(createEntityDefinitionsPanel(this), wxGBPosition(0,0), wxGBSpan(3,1), wxEXPAND);
            sizer->Add(createEntitiesPanel(this),          wxGBPosition(0,1), wxDefaultSpan);
            sizer->Add(createBrushesPanel(this),           wxGBPosition(1,1), wxDefaultSpan);
            sizer->Add(createRendererPanel(this),          wxGBPosition(2,1), wxDefaultSpan);

			SetSizerAndFit(sizer);
			Layout();
			GetParent()->GetParent()->Fit();

#ifdef __WXGTK20__
            // For some reason, the popup window is too small on GTK, so we add a few pixels.
            wxSize size = GetParent()->GetParent()->GetSize();
            size.IncBy(0, 25);
            GetParent()->GetParent()->SetSize(size);
#endif
        }

        wxWindow* ViewEditor::createEntityDefinitionsPanel(wxWindow* parent) {
            TitledPanel* panel = new TitledPanel(parent, "Entity Definitions", false);

            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& entityDefinitionManager = document->entityDefinitionManager();

            Model::EditorContext& editorContext = document->editorContext();
            m_entityDefinitionCheckBoxList = new EntityDefinitionCheckBoxList(panel->getPanel(), entityDefinitionManager, editorContext);

            wxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
            panelSizer->Add(m_entityDefinitionCheckBoxList, wxSizerFlags().Expand().Proportion(1));
            panelSizer->SetItemMinSize(m_entityDefinitionCheckBoxList, 250, wxDefaultCoord);
            panel->getPanel()->SetSizer(panelSizer);

            return panel;
        }

        wxWindow* ViewEditor::createEntitiesPanel(wxWindow* parent) {
            TitledPanel* panel = new TitledPanel(parent, "Entities", false);

            m_showEntityClassnamesCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show entity classnames");
            m_showEntityBoundsCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show entity bounds");
            m_showPointEntitiesCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show point entities");
            m_showPointEntityModelsCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show point entity models");

            m_showEntityClassnamesCheckBox->Bind(wxEVT_CHECKBOX, &ViewEditor::OnShowEntityClassnamesChanged, this);
            m_showEntityBoundsCheckBox->Bind(wxEVT_CHECKBOX, &ViewEditor::OnShowEntityBoundsChanged, this);
            m_showPointEntitiesCheckBox->Bind(wxEVT_CHECKBOX, &ViewEditor::OnShowPointEntitiesChanged, this);
            m_showPointEntityModelsCheckBox->Bind(wxEVT_CHECKBOX, &ViewEditor::OnShowPointEntityModelsChanged, this);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_showEntityClassnamesCheckBox);
            sizer->Add(m_showEntityBoundsCheckBox);
            sizer->Add(m_showPointEntitiesCheckBox);
            sizer->Add(m_showPointEntityModelsCheckBox);

            panel->getPanel()->SetSizerAndFit(sizer);
            return panel;
        }

        wxWindow* ViewEditor::createBrushesPanel(wxWindow* parent) {
            TitledPanel* panel = new TitledPanel(parent, "Brushes", false);
            wxWindow* inner = panel->getPanel();
            createBrushContentTypeFilter(inner);

            m_showBrushesCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show brushes");
            m_showBrushesCheckBox->Bind(wxEVT_CHECKBOX, &ViewEditor::OnShowBrushesChanged, this);

            assert(inner->GetSizer() != NULL);
            inner->GetSizer()->Prepend(m_showBrushesCheckBox);

            return panel;
        }

        void ViewEditor::createBrushContentTypeFilter(wxWindow* parent) {
            m_brushContentTypeCheckBoxes.clear();

            MapDocumentSPtr document = lock(m_document);
            Model::GamePtr game = document->game();
            if (game.get() == NULL) {
                createEmptyBrushContentTypeFilter(parent);
            } else {
                const Model::BrushContentType::List& contentTypes = game->brushContentTypes();
                if (contentTypes.empty()) {
                    createEmptyBrushContentTypeFilter(parent);
                } else {
                    createBrushContentTypeFilter(parent, contentTypes);
                }
            }
        }

        void ViewEditor::createEmptyBrushContentTypeFilter(wxWindow* parent) {
            wxStaticText* msg = new wxStaticText(parent, wxID_ANY, "No brush content types found");
            msg->SetForegroundColour(*wxLIGHT_GREY);

            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->AddSpacer(LayoutConstants::WideHMargin);
            sizer->Add(msg, wxSizerFlags().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            sizer->AddSpacer(LayoutConstants::WideHMargin);

            parent->SetSizerAndFit(sizer);
        }

        void ViewEditor::createBrushContentTypeFilter(wxWindow* parent, const Model::BrushContentType::List& contentTypes) {
            assert(!contentTypes.empty());

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            for (size_t i = 0; i < contentTypes.size(); ++i) {
                const Model::BrushContentType& contentType = contentTypes[i];

                wxString label = "Show ";
                label << StringUtils::toLower(contentType.name());

                wxCheckBox* checkBox = new wxCheckBox(parent, wxID_ANY, label);
                m_brushContentTypeCheckBoxes.push_back(checkBox);

                sizer->Add(checkBox);
                checkBox->Bind(wxEVT_CHECKBOX, &ViewEditor::OnShowBrushContentTypeChanged, this);
            }
            parent->SetSizerAndFit(sizer);
        }

        wxWindow* ViewEditor::createRendererPanel(wxWindow* parent) {
            TitledPanel* panel = new TitledPanel(parent, "Renderer", false);
            wxWindow* inner = panel->getPanel();

            static const wxString FaceRenderModes[] = { "Show textures", "Hide textures", "Hide faces" };
            m_renderModeRadioGroup = new RadioGroup(inner, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, FaceRenderModes);

            m_shadeFacesCheckBox = new wxCheckBox(inner, wxID_ANY, "Shade faces");
            m_showFogCheckBox = new wxCheckBox(inner, wxID_ANY, "Use fog");
            m_showEdgesCheckBox = new wxCheckBox(inner, wxID_ANY, "Show edges");

            static const wxString EntityLinkModes[] = { "Show all entity links", "Show transitively selected entity links", "Show directly selected entity links", "Hide entity links" };
            m_entityLinkRadioGroup = new RadioGroup(inner, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, EntityLinkModes);

            m_renderModeRadioGroup->Bind(wxEVT_RADIOGROUP, &ViewEditor::OnFaceRenderModeChanged, this);
            m_shadeFacesCheckBox->Bind(wxEVT_CHECKBOX, &ViewEditor::OnShadeFacesChanged, this);
            m_showFogCheckBox->Bind(wxEVT_CHECKBOX, &ViewEditor::OnShowFogChanged, this);
            m_showEdgesCheckBox->Bind(wxEVT_CHECKBOX, &ViewEditor::OnShowEdgesChanged, this);
            m_entityLinkRadioGroup->Bind(wxEVT_RADIOGROUP, &ViewEditor::OnEntityLinkModeChanged, this);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_renderModeRadioGroup);
            sizer->Add(m_shadeFacesCheckBox);
            sizer->Add(m_showFogCheckBox);
            sizer->Add(m_showEdgesCheckBox);
            sizer->Add(m_entityLinkRadioGroup);

            inner->SetSizerAndFit(sizer);
            return panel;
        }

        void ViewEditor::refreshGui() {
            refreshEntityDefinitionsPanel();
            refreshEntitiesPanel();
            refreshBrushesPanel();
            refreshRendererPanel();
        }

        void ViewEditor::refreshEntityDefinitionsPanel() {
            m_entityDefinitionCheckBoxList->refresh();
        }

        void ViewEditor::refreshEntitiesPanel() {
            MapDocumentSPtr document = lock(m_document);
            const MapViewConfig& config = document->mapViewConfig();

            m_showEntityClassnamesCheckBox->SetValue(config.showEntityClassnames());
            m_showEntityBoundsCheckBox->SetValue(config.showEntityBounds());
            m_showPointEntitiesCheckBox->SetValue(config.showPointEntities());
            m_showPointEntityModelsCheckBox->SetValue(config.showPointEntityModels());
        }

        void ViewEditor::refreshBrushesPanel() {
            MapDocumentSPtr document = lock(m_document);

            const MapViewConfig& config = document->mapViewConfig();
            m_showBrushesCheckBox->SetValue(config.showBrushes());

            Model::EditorContext& editorContext = document->editorContext();
            const Model::BrushContentType::FlagType hiddenFlags = editorContext.hiddenBrushContentTypes();

            Model::GamePtr game = document->game();
            if (game.get() != NULL) {
                const Model::BrushContentType::List& contentTypes = game->brushContentTypes();
                for (size_t i = 0; i < contentTypes.size(); ++i) {
                    const Model::BrushContentType& contentType = contentTypes[i];
                    wxCheckBox* checkBox = m_brushContentTypeCheckBoxes[i];
                    checkBox->SetValue((contentType.flagValue() & hiddenFlags) == 0);
                }
            }
        }

        void ViewEditor::refreshRendererPanel() {
            MapDocumentSPtr document = lock(m_document);
            const MapViewConfig& config = document->mapViewConfig();
            Model::EditorContext& editorContext = document->editorContext();

            m_renderModeRadioGroup->SetSelection(config.faceRenderMode());
            m_shadeFacesCheckBox->SetValue(config.shadeFaces());
            m_showFogCheckBox->SetValue(config.showFog());
            m_showEdgesCheckBox->SetValue(config.showEdges());
            m_entityLinkRadioGroup->SetSelection(editorContext.entityLinkMode());
        }

        ViewPopupEditor::ViewPopupEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_button(NULL),
        m_editor(NULL) {
            m_button = new PopupButton(this, "View");
            m_button->SetToolTip("Click to edit view settings");

            BorderPanel* editorContainer = new BorderPanel(m_button->GetPopupWindow(), wxALL);
            m_editor = new ViewEditor(editorContainer, document);

            wxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_editor, wxSizerFlags().Border(wxALL, LayoutConstants::DialogOuterMargin));
            editorContainer->SetSizer(containerSizer);

            wxSizer* popupSizer = new wxBoxSizer(wxVERTICAL);
            popupSizer->Add(editorContainer, wxSizerFlags());
            m_button->GetPopupWindow()->SetSizer(popupSizer);

            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_button, wxSizerFlags().CenterVertical());
            SetSizerAndFit(sizer);
        }
    }
}
