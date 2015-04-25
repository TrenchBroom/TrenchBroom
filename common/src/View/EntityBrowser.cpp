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

#include "EntityBrowser.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinitionManager.h"
#include "View/EntityBrowserView.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"

#include <wx/choice.h>
#include <wx/event.h>
#include <wx/tglbtn.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityBrowser::EntityBrowser(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        wxPanel(parent),
        m_document(document) {
            createGui(contextManager);
            bindObservers();
        }
        
        EntityBrowser::~EntityBrowser() {
            unbindObservers();
        }
        
        void EntityBrowser::reload() {
            if (m_view != NULL) {
                m_view->clear();
                m_view->reload();
            }
        }
        
        void EntityBrowser::OnSortOrderChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const Assets::EntityDefinition::SortOrder sortOrder = event.GetSelection() == 0 ? Assets::EntityDefinition::Name : Assets::EntityDefinition::Usage;
            m_view->setSortOrder(sortOrder);
        }
        
        void EntityBrowser::OnGroupButtonToggled(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_view->setGroup(m_groupButton->GetValue());
        }
        
        void EntityBrowser::OnUsedButtonToggled(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_view->setHideUnused(m_usedButton->GetValue());
        }
        
        void EntityBrowser::OnFilterPatternChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_view->setFilterText(m_filterBox->GetValue().ToStdString());
        }

        void EntityBrowser::createGui(GLContextManager& contextManager) {
            wxPanel* browserPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_scrollBar = new wxScrollBar(browserPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
            
            MapDocumentSPtr document = lock(m_document);
            m_view = new EntityBrowserView(browserPanel, m_scrollBar,
                                           contextManager,
                                           document->entityDefinitionManager(),
                                           document->entityModelManager(),
                                           *document);
            
            wxSizer* browserPanelSizer = new wxBoxSizer(wxHORIZONTAL);
            browserPanelSizer->Add(m_view, 1, wxEXPAND);
            browserPanelSizer->Add(m_scrollBar, 0, wxEXPAND);
            browserPanel->SetSizerAndFit(browserPanelSizer);
            
            const wxString sortOrders[2] = { "Name", "Usage" };
            m_sortOrderChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, sortOrders);
            m_sortOrderChoice->SetSelection(0);
            m_sortOrderChoice->SetToolTip("Select ordering criterion");

            m_groupButton = new wxToggleButton(this, wxID_ANY, "Group", wxDefaultPosition, wxDefaultSize, LayoutConstants::ToggleButtonStyle | wxBU_EXACTFIT);
            m_groupButton->SetToolTip("Group entity definitions by category");
            
            m_usedButton = new wxToggleButton(this, wxID_ANY, "Used", wxDefaultPosition, wxDefaultSize, LayoutConstants::ToggleButtonStyle | wxBU_EXACTFIT);
            m_usedButton->SetToolTip("Only show entity definitions currently in use");
            
            m_filterBox = new wxSearchCtrl(this, wxID_ANY);
            m_filterBox->ShowCancelButton(true);
            
            m_sortOrderChoice->Bind(wxEVT_CHOICE, &EntityBrowser::OnSortOrderChanged, this);
            m_groupButton->Bind(wxEVT_TOGGLEBUTTON, &EntityBrowser::OnGroupButtonToggled, this);
            m_usedButton->Bind(wxEVT_TOGGLEBUTTON, &EntityBrowser::OnUsedButtonToggled, this);
            m_filterBox->Bind(wxEVT_TEXT, &EntityBrowser::OnFilterPatternChanged, this);
            
            wxSizer* controlSizer = new wxBoxSizer(wxHORIZONTAL);
            controlSizer->AddSpacer(LayoutConstants::ChoiceLeftMargin);
            controlSizer->Add(m_sortOrderChoice, 0, wxTOP, LayoutConstants::ChoiceTopMargin);
            controlSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            controlSizer->Add(m_groupButton, 0);
            controlSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            controlSizer->Add(m_usedButton, 0);
            controlSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            controlSizer->Add(m_filterBox, 1, wxEXPAND);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(browserPanel, 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            outerSizer->Add(controlSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            outerSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            
            SetBackgroundColour(*wxWHITE);
            SetSizer(outerSizer);
        }
        
        void EntityBrowser::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &EntityBrowser::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &EntityBrowser::documentWasLoaded);
            document->modsDidChangeNotifier.addObserver(this, &EntityBrowser::modsDidChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &EntityBrowser::entityDefinitionsDidChange);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &EntityBrowser::preferenceDidChange);
        }
        
        void EntityBrowser::unbindObservers() {
            if (!expired(m_document)) {
            MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &EntityBrowser::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &EntityBrowser::documentWasLoaded);
                document->modsDidChangeNotifier.removeObserver(this, &EntityBrowser::modsDidChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &EntityBrowser::entityDefinitionsDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &EntityBrowser::preferenceDidChange);
        }

        void EntityBrowser::documentWasNewed(MapDocument* document) {
            reload();
        }
        
        void EntityBrowser::documentWasLoaded(MapDocument* document) {
            reload();
        }

        void EntityBrowser::modsDidChange() {
            reload();
        }
        
        void EntityBrowser::entityDefinitionsDidChange() {
            reload();
        }

        void EntityBrowser::preferenceDidChange(const IO::Path& path) {
            MapDocumentSPtr document = lock(m_document);
            if (document->isGamePathPreference(path))
                reload();
            else
                m_view->Refresh();
        }
    }
}
