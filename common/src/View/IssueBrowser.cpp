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

#include "IssueBrowser.h"

#include "Macros.h"
#include "Model/Issue.h"
#include "Model/IssueGenerator.h"
#include "Model/World.h"
#include "View/FlagChangedCommand.h"
#include "View/FlagsPopupEditor.h"
#include "View/IssueBrowserView.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <wx/checkbox.h>
#include <wx/menu.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        IssueBrowser::IssueBrowser(wxWindow* parent, MapDocumentWPtr document) :
        TabBookPage(parent),
        m_document(document),
        m_view(new IssueBrowserView(this, m_document)),
        m_showHiddenIssuesCheckBox(NULL),
        m_filterEditor(NULL) {
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_view, 1, wxEXPAND);
            SetSizerAndFit(sizer);
            
            bindObservers();
        }
        
        IssueBrowser::~IssueBrowser() {
            unbindObservers();
        }

        wxWindow* IssueBrowser::createTabBarPage(wxWindow* parent) {
            wxPanel* barPage = new wxPanel(parent);
            m_showHiddenIssuesCheckBox = new wxCheckBox(barPage, wxID_ANY, "Show hidden issues");
            m_showHiddenIssuesCheckBox->Bind(wxEVT_CHECKBOX, &IssueBrowser::OnShowHiddenIssuesChanged, this);
            
            m_filterEditor = new FlagsPopupEditor(barPage, 1, "Filter", false);
            m_filterEditor->Bind(FLAG_CHANGED_EVENT, &IssueBrowser::OnFilterChanged, this);
            
            wxBoxSizer* barPageSizer = new wxBoxSizer(wxHORIZONTAL);
            barPageSizer->Add(m_showHiddenIssuesCheckBox, 0, wxALIGN_CENTER_VERTICAL);
            barPageSizer->AddSpacer(LayoutConstants::MediumHMargin);
            barPageSizer->Add(m_filterEditor, 0, wxALIGN_CENTER_VERTICAL);
            barPage->SetSizer(barPageSizer);
            
            return barPage;
        }

        void IssueBrowser::OnShowHiddenIssuesChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_view->setShowHiddenIssues(m_showHiddenIssuesCheckBox->IsChecked());
        }

        void IssueBrowser::OnFilterChanged(FlagChangedCommand& command) {
            if (IsBeingDeleted()) return;

            m_view->setHiddenGenerators(~command.flagSetValue());
        }

        void IssueBrowser::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasSavedNotifier.addObserver(this, &IssueBrowser::documentWasSaved);
            document->documentWasNewedNotifier.addObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
            document->nodesWereAddedNotifier.addObserver(this, &IssueBrowser::nodesWereAdded);
            document->nodesWereRemovedNotifier.addObserver(this, &IssueBrowser::nodesWereRemoved);
            document->nodesDidChangeNotifier.addObserver(this, &IssueBrowser::nodesDidChange);
            document->brushFacesDidChangeNotifier.addObserver(this, &IssueBrowser::brushFacesDidChange);
        }
        
        void IssueBrowser::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasSavedNotifier.removeObserver(this, &IssueBrowser::documentWasSaved);
                document->documentWasNewedNotifier.removeObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
                document->nodesWereAddedNotifier.removeObserver(this, &IssueBrowser::nodesWereAdded);
                document->nodesWereRemovedNotifier.removeObserver(this, &IssueBrowser::nodesWereRemoved);
                document->nodesDidChangeNotifier.removeObserver(this, &IssueBrowser::nodesDidChange);
                document->brushFacesDidChangeNotifier.removeObserver(this, &IssueBrowser::brushFacesDidChange);
            }
        }
        
        void IssueBrowser::documentWasNewedOrLoaded(MapDocument* document) {
            updateFilterFlags();
            m_view->reset();
        }

        void IssueBrowser::documentWasSaved(MapDocument* document) {
            m_view->Refresh();
        }
        
        void IssueBrowser::nodesWereAdded(const Model::NodeList& nodes) {
            m_view->reset();
        }
        
        void IssueBrowser::nodesWereRemoved(const Model::NodeList& nodes) {
            m_view->reset();
        }
        
        void IssueBrowser::nodesDidChange(const Model::NodeList& nodes) {
            m_view->reset();
        }
        
        void IssueBrowser::brushFacesDidChange(const Model::BrushFaceList& faces) {
            m_view->reset();
        }

        void IssueBrowser::issueIgnoreChanged(Model::Issue* issue) {
            m_view->Refresh();
        }
        
        void IssueBrowser::updateFilterFlags() {
            MapDocumentSPtr document = lock(m_document);
            const Model::World* world = document->world();
            const Model::IssueGeneratorList& generators = world->registeredIssueGenerators();
            
            wxArrayInt flags;
            wxArrayString labels;
            
            Model::IssueGeneratorList::const_iterator it, end;
            for (it = generators.begin(), end = generators.end(); it != end; ++it) {
                const Model::IssueGenerator* generator = *it;
                const Model::IssueType flag = generator->type();
                const String& description = generator->description();
                
                flags.push_back(flag);
                labels.push_back(description);
            }

            m_filterEditor->setFlags(flags, labels);
            m_view->setHiddenGenerators(0);
            m_filterEditor->setFlagValue(~0);
        }
    }
}
