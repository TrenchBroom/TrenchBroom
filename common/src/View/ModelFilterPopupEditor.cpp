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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ModelFilterPopupEditor.h"

#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionGroup.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/Game.h"
#include "Model/ModelFilter.h"
#include "Renderer/RenderConfig.h"
#include "View/BorderPanel.h"
#include "View/MapDocument.h"
#include "View/PopupButton.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        EntityDefinitionCheckBoxList::EntityDefinitionCheckBoxList(wxWindow* parent, Assets::EntityDefinitionManager& entityDefinitionManager, Model::ModelFilter& filter) :
        wxPanel(parent),
        m_entityDefinitionManager(entityDefinitionManager),
        m_filter(filter) {
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
                    const bool firstHidden = m_filter.entityDefinitionHidden(definitions[0]);
                    bool mixed = false;
                    for (size_t j = 0; j < definitions.size(); ++j) {
                        const bool hidden = m_filter.entityDefinitionHidden(definitions[j]);
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
            const wxVariant* variant = static_cast<wxVariant*>(event.GetEventUserData());
            const size_t groupIndex = static_cast<size_t>(variant->GetLong());

            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            assert(groupIndex < m_entityDefinitionManager.groups().size());
            const Assets::EntityDefinitionGroup& group = groups[groupIndex];
            
            const Assets::EntityDefinitionList& definitions = group.definitions();
            for (size_t i = 0; i < definitions.size(); ++i) {
                const Assets::EntityDefinition* definition = definitions[i];
                m_filter.setEntityDefinitionHidden(definition, !event.IsChecked());
            }
            
            refresh();
        }
        
        void EntityDefinitionCheckBoxList::OnDefCheckBoxChanged(wxCommandEvent& event) {
            const wxVariant* variant = static_cast<wxVariant*>(event.GetEventUserData());
            const Assets::EntityDefinition* definition = reinterpret_cast<const Assets::EntityDefinition*>(variant->GetVoidPtr());
            m_filter.setEntityDefinitionHidden(definition, !event.IsChecked());
            refresh();
        }

        void EntityDefinitionCheckBoxList::OnShowAllClicked(wxCommandEvent& event) {
            hideAll(false);
        }
        
        void EntityDefinitionCheckBoxList::OnHideAllClicked(wxCommandEvent& event) {
            hideAll(true);
        }

        void EntityDefinitionCheckBoxList::hideAll(const bool hidden) {
            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const Assets::EntityDefinitionList& definitions = group.definitions();
                for (size_t j = 0; j < definitions.size(); ++j) {
                    const Assets::EntityDefinition* definition = definitions[j];
                    m_filter.setEntityDefinitionHidden(definition, hidden);
                }
            }
        }

        void EntityDefinitionCheckBoxList::createGui() {
            wxScrolledWindow* scrollWindow = new wxScrolledWindow(this);
            
            wxSizer* scrollWindowSizer = new wxBoxSizer(wxVERTICAL);
            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const Assets::EntityDefinitionList& definitions = group.definitions();
                const String& groupName = group.displayName();
                
                wxCheckBox* groupCB = new wxCheckBox(scrollWindow, wxID_ANY, groupName, wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
                groupCB->SetFont(groupCB->GetFont().Bold());
                groupCB->Bind(wxEVT_CHECKBOX, &EntityDefinitionCheckBoxList::OnGroupCheckBoxChanged, this, wxID_ANY, wxID_ANY, new wxVariant(static_cast<long>(i)));
                m_groupCheckBoxes.push_back(groupCB);
                
                scrollWindowSizer->Add(groupCB);
                
                Assets::EntityDefinitionList::const_iterator defIt, defEnd;
                for (defIt = definitions.begin(), defEnd = definitions.end(); defIt != defEnd; ++defIt) {
                    Assets::EntityDefinition* definition = *defIt;
                    const String defName = definition->name();
                    
                    wxCheckBox* defCB = new wxCheckBox(scrollWindow, wxID_ANY, defName);
                    defCB->Bind(wxEVT_CHECKBOX, &EntityDefinitionCheckBoxList::OnDefCheckBoxChanged, this, wxID_ANY, wxID_ANY, new wxVariant(reinterpret_cast<void*>(definition)));
                    
                    m_defCheckBoxes.push_back(defCB);
                    scrollWindowSizer->Add(defCB, 0, wxLEFT, 10);
                }
            }
            
            scrollWindow->SetSizer(scrollWindowSizer);
            scrollWindow->SetScrollRate(1, 1);
            
            wxButton* showAllButton = new wxButton(this, wxID_ANY, "Show all", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            showAllButton->SetFont(showAllButton->GetFont().Bold());
            wxButton* hideAllButton = new wxButton(this, wxID_ANY, "Hide all", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            hideAllButton->SetFont(hideAllButton->GetFont().Bold());
            
            showAllButton->Bind(wxEVT_BUTTON, &EntityDefinitionCheckBoxList::OnShowAllClicked, this);
            hideAllButton->Bind(wxEVT_BUTTON, &EntityDefinitionCheckBoxList::OnHideAllClicked, this);

            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            buttonSizer->Add(showAllButton);
            buttonSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            buttonSizer->Add(hideAllButton);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(scrollWindow, 1, wxEXPAND
#ifdef __APPLE__
                            | wxTOP | wxRIGHT, 1
#endif
                            );
            outerSizer->Add(buttonSizer, 0, wxTOP | wxBOTTOM, 1);
            
            SetSizer(outerSizer);
        }
        
        ModelFilterEditor::ModelFilterEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            bindObservers();
        }
        
        ModelFilterEditor::~ModelFilterEditor() {
            unbindObservers();
        }
        
        void ModelFilterEditor::OnShowEntityClassnamesChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Renderer::RenderConfig& config = document->renderConfig();
            config.setShowEntityClassnames(event.IsChecked());
        }
        
        void ModelFilterEditor::OnShowEntityBoundsChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Renderer::RenderConfig& config = document->renderConfig();
            config.setShowEntityBounds(event.IsChecked());
        }
        
        void ModelFilterEditor::OnShowPointEntitiesChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Model::ModelFilter& filter = document->filter();
            filter.setShowPointEntities(event.IsChecked());
        }
        
        void ModelFilterEditor::OnShowPointEntityModelsChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Renderer::RenderConfig& config = document->renderConfig();
            config.setShowPointEntityModels(event.IsChecked());
        }
        
        void ModelFilterEditor::OnShowBrushesChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Model::ModelFilter& filter = document->filter();
            filter.setShowBrushes(event.IsChecked());
        }
        
        void ModelFilterEditor::OnShowBrushContentTypeChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Model::GamePtr game = document->game();
            
            if (game != NULL) {
                Model::BrushContentType::FlagType hiddenFlags = 0;
                const Model::BrushContentType::List& contentTypes = game->brushContentTypes();
                
                for (size_t i = 0; i < contentTypes.size(); ++i) {
                    const Model::BrushContentType& contentType = contentTypes[i];
                    wxCheckBox* checkBox = m_brushContentTypeCheckBoxes[i];
                    if (!checkBox->GetValue())
                        hiddenFlags |= contentType.flagValue();
                }
                
                Model::ModelFilter& filter = document->filter();
                filter.setHiddenBrushContentTypes(hiddenFlags);
            }
        }
        
        void ModelFilterEditor::OnFaceRenderModeChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Renderer::RenderConfig& config = document->renderConfig();
            
            switch (event.GetSelection()) {
                case 1:
                    config.setFaceRenderMode(Renderer::RenderConfig::FaceRenderMode_Flat);
                    break;
                case 2:
                    config.setFaceRenderMode(Renderer::RenderConfig::FaceRenderMode_Skip);
                    break;
                default:
                    config.setFaceRenderMode(Renderer::RenderConfig::FaceRenderMode_Textured);
                    break;
            }
        }
        
        void ModelFilterEditor::OnShadeFacesChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Renderer::RenderConfig& config = document->renderConfig();
            config.setShadeFaces(event.IsChecked());
        }
        
        void ModelFilterEditor::OnUseFogChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Renderer::RenderConfig& config = document->renderConfig();
            config.setUseFog(event.IsChecked());
        }
        
        void ModelFilterEditor::OnShowEdgesChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Renderer::RenderConfig& config = document->renderConfig();
            config.setShowEdges(event.IsChecked());
        }
        
        void ModelFilterEditor::OnEntityLinkModeChanged(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Model::ModelFilter& filter = document->filter();
            
            switch (event.GetSelection()) {
                case 0:
                    filter.setEntityLinkMode(Model::ModelFilter::EntityLinkMode_All);
                    break;
                case 1:
                    filter.setEntityLinkMode(Model::ModelFilter::EntityLinkMode_Transitive);
                    break;
                case 2:
                    filter.setEntityLinkMode(Model::ModelFilter::EntityLinkMode_Direct);
                    break;
                default:
                    filter.setEntityLinkMode(Model::ModelFilter::EntityLinkMode_None);
                    break;
            }
        }

        void ModelFilterEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &ModelFilterEditor::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &ModelFilterEditor::documentWasNewedOrLoaded);
            document->modelFilterDidChangeNotifier.addObserver(this, &ModelFilterEditor::modelFilterDidChange);
            document->renderConfigDidChangeNotifier.addObserver(this, &ModelFilterEditor::renderConfigDidChange);
        }
        
        void ModelFilterEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &ModelFilterEditor::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &ModelFilterEditor::documentWasNewedOrLoaded);
                document->modelFilterDidChangeNotifier.removeObserver(this, &ModelFilterEditor::modelFilterDidChange);
                document->renderConfigDidChangeNotifier.removeObserver(this, &ModelFilterEditor::renderConfigDidChange);
            }
        }
        
        void ModelFilterEditor::documentWasNewedOrLoaded() {
            createGui();
            refreshGui();
        }
        
        void ModelFilterEditor::modelFilterDidChange() {
            refreshGui();
        }
        
        void ModelFilterEditor::renderConfigDidChange() {
            refreshGui();
        }
        
        void ModelFilterEditor::createGui() {
            DestroyChildren();
            
            wxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
            rightSizer->Add(createEntitiesPanel(), 0, wxEXPAND);
            rightSizer->AddSpacer(LayoutConstants::WideVMargin);
            rightSizer->Add(createBrushesPanel(), 0, wxEXPAND);
            rightSizer->AddSpacer(LayoutConstants::WideVMargin);
            rightSizer->Add(createRendererPanel(), 0, wxEXPAND);
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(createEntityDefinitionsPanel(), 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::WideHMargin);
            outerSizer->Add(rightSizer);
            
            SetSizerAndFit(outerSizer);
            
            GetParent()->Fit();
            GetParent()->GetParent()->Fit();
        }
        
        wxWindow* ModelFilterEditor::createEntityDefinitionsPanel() {
            TitledPanel* panel = new TitledPanel(this, "Entity Definitions", false);
            
            BorderPanel* container = new BorderPanel(panel->getPanel(), wxALL);
            container->SetBackgroundColour(*wxWHITE);
            
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& entityDefinitionManager = document->entityDefinitionManager();
            Model::ModelFilter& filter = document->filter();
            m_entityDefinitionCheckBoxList = new EntityDefinitionCheckBoxList(container, entityDefinitionManager, filter);
            
            wxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_entityDefinitionCheckBoxList, 1, wxEXPAND);
            containerSizer->SetItemMinSize(m_entityDefinitionCheckBoxList, 200, wxDefaultCoord);
            container->SetSizer(containerSizer);
            
            wxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
            panelSizer->Add(container, 1, wxEXPAND);
            
            panel->getPanel()->SetSizer(panelSizer);
            return panel;
        }
        
        wxWindow* ModelFilterEditor::createEntitiesPanel() {
            TitledPanel* panel = new TitledPanel(this, "Entities", false);
            
            m_showEntityClassnamesCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show entity classnames");
            m_showEntityBoundsCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show entity bounds");
            m_showPointEntitiesCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show point entities");
            m_showPointEntityModelsCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show point entity models");
            
            m_showEntityClassnamesCheckBox->Bind(wxEVT_CHECKBOX, &ModelFilterEditor::OnShowEntityClassnamesChanged, this);
            m_showEntityBoundsCheckBox->Bind(wxEVT_CHECKBOX, &ModelFilterEditor::OnShowEntityBoundsChanged, this);
            m_showPointEntitiesCheckBox->Bind(wxEVT_CHECKBOX, &ModelFilterEditor::OnShowPointEntitiesChanged, this);
            m_showPointEntityModelsCheckBox->Bind(wxEVT_CHECKBOX, &ModelFilterEditor::OnShowPointEntityModelsChanged, this);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_showEntityClassnamesCheckBox);
            sizer->Add(m_showEntityBoundsCheckBox);
            sizer->Add(m_showPointEntitiesCheckBox);
            sizer->Add(m_showPointEntityModelsCheckBox);
            
            panel->getPanel()->SetSizerAndFit(sizer);
            return panel;
        }
        
        wxWindow* ModelFilterEditor::createBrushesPanel() {
            TitledPanel* panel = new TitledPanel(this, "Brushes", false);
            wxWindow* inner = panel->getPanel();
            createBrushContentTypeFilter(inner);
            
            m_showBrushesCheckBox = new wxCheckBox(panel->getPanel(), wxID_ANY, "Show brushes");
            m_showBrushesCheckBox->Bind(wxEVT_CHECKBOX, &ModelFilterEditor::OnShowBrushesChanged, this);
            
            assert(inner->GetSizer() != NULL);
            inner->GetSizer()->Prepend(m_showBrushesCheckBox);
            
            return panel;
        }
        
        void ModelFilterEditor::createBrushContentTypeFilter(wxWindow* parent) {
            m_brushContentTypeCheckBoxes.clear();
            
            MapDocumentSPtr document = lock(m_document);
            Model::GamePtr game = document->game();
            if (game == NULL) {
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
        
        void ModelFilterEditor::createEmptyBrushContentTypeFilter(wxWindow* parent) {
            wxStaticText* msg = new wxStaticText(parent, wxID_ANY, "No brush content types found");
            msg->SetForegroundColour(*wxLIGHT_GREY);
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->AddStretchSpacer();
            sizer->AddSpacer(LayoutConstants::WideHMargin);
            sizer->Add(msg, 0, wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            sizer->AddSpacer(LayoutConstants::WideHMargin);
            sizer->AddStretchSpacer();
            
            parent->SetSizerAndFit(sizer);
        }
        
        void ModelFilterEditor::createBrushContentTypeFilter(wxWindow* parent, const Model::BrushContentType::List& contentTypes) {
            assert(!contentTypes.empty());
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            for (size_t i = 0; i < contentTypes.size(); ++i) {
                const Model::BrushContentType& contentType = contentTypes[i];
                
                wxString label = "Show ";
                label << StringUtils::toLower(contentType.name());
                
                wxCheckBox* checkBox = new wxCheckBox(parent, wxID_ANY, label);
                m_brushContentTypeCheckBoxes.push_back(checkBox);
                
                sizer->Add(checkBox);
                checkBox->Bind(wxEVT_CHECKBOX, &ModelFilterEditor::OnShowBrushContentTypeChanged, this);
            }
            parent->SetSizerAndFit(sizer);
        }
        
        wxWindow* ModelFilterEditor::createRendererPanel() {
            TitledPanel* panel = new TitledPanel(this, "Renderer", false);
            wxWindow* inner = panel->getPanel();
            
            static const wxString FaceRenderModes[] = { "Textured", "Flat", "Hidden" };
            m_faceRenderModeChoice = new wxChoice(inner, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, FaceRenderModes);
            m_shadeFacesCheckBox = new wxCheckBox(inner, wxID_ANY, "Shade faces");
            m_useFogCheckBox = new wxCheckBox(inner, wxID_ANY, "Use fog");
            m_showEdgesCheckBox = new wxCheckBox(inner, wxID_ANY, "Show edges");
            
            static const wxString EntityLinkModes[] = { "All", "Transitive selected", "Direct selected", "No" };
            m_entityLinkModeChoice = new wxChoice(inner, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, EntityLinkModes);
            
            m_faceRenderModeChoice->Bind(wxEVT_CHOICE, &ModelFilterEditor::OnFaceRenderModeChanged, this);
            m_shadeFacesCheckBox->Bind(wxEVT_CHECKBOX, &ModelFilterEditor::OnShadeFacesChanged, this);
            m_useFogCheckBox->Bind(wxEVT_CHECKBOX, &ModelFilterEditor::OnUseFogChanged, this);
            m_showEdgesCheckBox->Bind(wxEVT_CHECKBOX, &ModelFilterEditor::OnShowEdgesChanged, this);
            m_entityLinkModeChoice->Bind(wxEVT_CHOICE, &ModelFilterEditor::OnEntityLinkModeChanged, this);
            
            wxSizer* faceRenderModeSizer = new wxBoxSizer(wxHORIZONTAL);
            faceRenderModeSizer->AddSpacer(LayoutConstants::ChoiceLeftMargin);
            faceRenderModeSizer->Add(m_faceRenderModeChoice);
            faceRenderModeSizer->Add(new wxStaticText(inner, wxID_ANY, "faces"), 0, wxALIGN_CENTER_VERTICAL);
            
            wxSizer* entityLinkModeSizer = new wxBoxSizer(wxHORIZONTAL);
            entityLinkModeSizer->AddSpacer(LayoutConstants::ChoiceLeftMargin);
            entityLinkModeSizer->Add(m_entityLinkModeChoice);
            entityLinkModeSizer->Add(new wxStaticText(inner, wxID_ANY, "entity links"), 0, wxALIGN_CENTER_VERTICAL);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->AddSpacer(LayoutConstants::ChoiceTopMargin);
            sizer->Add(faceRenderModeSizer);
            sizer->AddSpacer(LayoutConstants::ChoiceSizeDelta);
            sizer->AddSpacer(LayoutConstants::NarrowVMargin);
            sizer->Add(m_shadeFacesCheckBox);
            sizer->Add(m_useFogCheckBox);
            sizer->Add(m_showEdgesCheckBox);
            sizer->AddSpacer(LayoutConstants::ChoiceTopMargin);
            sizer->Add(entityLinkModeSizer);
            sizer->AddSpacer(LayoutConstants::ChoiceSizeDelta);
            sizer->AddSpacer(LayoutConstants::NarrowVMargin);
            
            inner->SetSizerAndFit(sizer);
            return panel;
        }
        
        void ModelFilterEditor::refreshGui() {
            refreshEntityDefinitionsPanel();
            refreshEntitiesPanel();
            refreshBrushesPanel();
            refreshRendererPanel();
        }
        
        void ModelFilterEditor::refreshEntityDefinitionsPanel() {
            m_entityDefinitionCheckBoxList->refresh();
        }
        
        void ModelFilterEditor::refreshEntitiesPanel() {
            MapDocumentSPtr document = lock(m_document);
            const Renderer::RenderConfig& config = document->renderConfig();
            
            m_showEntityClassnamesCheckBox->SetValue(config.showEntityClassnames());
            m_showEntityBoundsCheckBox->SetValue(config.showEntityBounds());
            m_showPointEntitiesCheckBox->SetValue(config.showPointEntities());
            m_showPointEntityModelsCheckBox->SetValue(config.showPointEntityModels());
        }
        
        void ModelFilterEditor::refreshBrushesPanel() {
            MapDocumentSPtr document = lock(m_document);
            
            const Renderer::RenderConfig& config = document->renderConfig();
            m_showBrushesCheckBox->SetValue(config.showBrushes());
            
            const Model::ModelFilter& filter = document->filter();
            const Model::BrushContentType::FlagType hiddenFlags = filter.hiddenBrushContentTypes();
            
            Model::GamePtr game = document->game();
            if (game != NULL) {
                const Model::BrushContentType::List& contentTypes = game->brushContentTypes();
                for (size_t i = 0; i < contentTypes.size(); ++i) {
                    const Model::BrushContentType& contentType = contentTypes[i];
                    wxCheckBox* checkBox = m_brushContentTypeCheckBoxes[i];
                    checkBox->SetValue((contentType.flagValue() & hiddenFlags) == 0);
                }
            }
        }
        
        void ModelFilterEditor::refreshRendererPanel() {
            MapDocumentSPtr document = lock(m_document);
            const Renderer::RenderConfig& config = document->renderConfig();
            const Model::ModelFilter& filter = document->filter();
            
            m_faceRenderModeChoice->SetSelection(config.faceRenderMode());
            m_shadeFacesCheckBox->SetValue(config.shadeFaces());
            m_useFogCheckBox->SetValue(config.useFog());
            m_showEdgesCheckBox->SetValue(config.showEdges());
            m_entityLinkModeChoice->SetSelection(filter.entityLinkMode());
        }
        
        ModelFilterPopupEditor::ModelFilterPopupEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_button(NULL),
        m_editor(NULL) {
            m_button = new PopupButton(this, "View");
            m_button->SetToolTip("Click to edit view settings");
            
            BorderPanel* editorContainer = new BorderPanel(m_button->GetPopupWindow(), wxALL);
            m_editor = new ModelFilterEditor(editorContainer, document);
            
            wxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_editor, 1, wxEXPAND | wxALL, LayoutConstants::DialogOuterMargin);
            editorContainer->SetSizer(containerSizer);
            
            wxSizer* popupSizer = new wxBoxSizer(wxVERTICAL);
            popupSizer->Add(editorContainer, 1, wxEXPAND);
            m_button->GetPopupWindow()->SetSizer(popupSizer);
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_button, 0, wxALIGN_CENTER_VERTICAL);
            SetSizerAndFit(sizer);
        }
    }
}
