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
#include "Model/IssueManager.h"
#include "View/ControllerFacade.h"
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
        IssueBrowser::IssueBrowser(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        TabBookPage(parent),
        m_document(document),
        m_controller(controller),
        m_view(new IssueBrowserView(this, m_document, m_controller)),
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
            m_filterEditor->Bind(EVT_FLAG_CHANGED_EVENT,
                                 EVT_FLAG_CHANGED_HANDLER(IssueBrowser::OnFilterChanged),
                                 this);
            
            wxBoxSizer* barPageSizer = new wxBoxSizer(wxHORIZONTAL);
            barPageSizer->Add(m_showHiddenIssuesCheckBox, 0, wxALIGN_CENTER_VERTICAL);
            barPageSizer->AddSpacer(LayoutConstants::MediumHMargin);
            barPageSizer->Add(m_filterEditor, 0, wxALIGN_CENTER_VERTICAL);
            barPage->SetSizer(barPageSizer);
            
            return barPage;
        }

        /*
        Model::QuickFix::List collectQuickFixes(const wxDataViewItemArray& selections);
        Model::QuickFix::List collectQuickFixes(const wxDataViewItemArray& selections) {
            assert(!selections.empty());
            const Model::Issue* issue = reinterpret_cast<const Model::Issue*>(selections[0].GetID());
            const Model::IssueType type = issue->type();
            Model::QuickFix::List result = issue->quickFixes();
            
            for (size_t i = 1; i < selections.size(); ++i) {
                issue = reinterpret_cast<const Model::Issue*>(selections[i].GetID());
                if (issue->type() != type)
                    return Model::QuickFix::List(0);
            }
            return result;
        }
         */
        
        void IssueBrowser::OnShowHiddenIssuesChanged(wxCommandEvent& event) {
            // m_model->setShowHiddenIssues(m_showHiddenIssuesCheckBox->IsChecked());
        }

        void IssueBrowser::OnFilterChanged(FlagChangedCommand& command) {
            // m_model->setHiddenGenerators(~command.flagSetValue());
        }

        /*
        void IssueBrowser::OnTreeViewContextMenu(wxDataViewEvent& event) {
            const wxDataViewItem& item = event.GetItem();
            if (!item.IsOk())
                return;
            if (!m_tree->IsSelected(item))
                return;
            
            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            assert(!selections.empty());

            wxMenu popupMenu;
            popupMenu.Append(SelectObjectsCommandId, "Select");
            popupMenu.Append(ShowIssuesCommandId, "Show");
            popupMenu.Append(HideIssuesCommandId, "Hide");
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnSelectIssues, this, SelectObjectsCommandId);
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnShowIssues, this, ShowIssuesCommandId);
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnHideIssues, this, HideIssuesCommandId);

            const Model::QuickFix::List quickFixes = collectQuickFixes(selections);
            if (!quickFixes.empty()) {
                wxMenu* quickFixMenu = new wxMenu();
                for (size_t i = 0; i < quickFixes.size(); ++i) {
                    const Model::QuickFix* quickFix = quickFixes[i];
                    const int quickFixId = FixObjectsBaseId + static_cast<int>(i);
                    quickFixMenu->Append(quickFixId, quickFix->description());
                }
                popupMenu.AppendSubMenu(quickFixMenu, "Fix");
                
                const int firstId = FixObjectsBaseId;
                const int lastId = firstId + static_cast<int>(quickFixes.size());
                quickFixMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnApplyQuickFix, this, firstId, lastId);
            }
            
            PopupMenu(&popupMenu);
        }
         */
        
        void IssueBrowser::OnSelectIssues(wxCommandEvent& event) {
            /*
            View::ControllerSPtr controller = lock(m_controller);
            const UndoableCommandGroup commandGroup(controller);

            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            selectIssueObjects(selections, controller);
             */
        }
        
        void IssueBrowser::OnShowIssues(wxCommandEvent& event) {
            setIssueVisibility(true);
        }
        
        void IssueBrowser::OnHideIssues(wxCommandEvent& event) {
            setIssueVisibility(false);
        }

        void IssueBrowser::OnApplyQuickFix(wxCommandEvent& event) {
            /*
            wxWindowUpdateLocker locker(m_tree);

            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            assert(!selections.empty());
            
            Model::QuickFix::List quickFixes = collectQuickFixes(selections);
            const size_t index = static_cast<size_t>(event.GetId()) - FixObjectsBaseId;
            assert(index < quickFixes.size());
            const Model::QuickFix* quickFix = quickFixes[index];
            
            View::ControllerSPtr controller = lock(m_controller);
            const UndoableCommandGroup commandGroup(controller);

            selectIssueObjects(selections, controller);
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                void* data = item.GetID();
                assert(data != NULL);
                Model::Issue* issue = reinterpret_cast<Model::Issue*>(data);
                issue->applyQuickFix(quickFix, controller);
            }
            
            m_tree->UnselectAll();
            m_tree->SetSelections(selections);
             */
        }

        /*
        void IssueBrowser::selectIssueObjects(const wxDataViewItemArray& selections, View::ControllerSPtr controller) {
            wxWindowUpdateLocker locker(m_tree);

            controller->deselectAll();
            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                void* data = item.GetID();
                assert(data != NULL);
                Model::Issue* issue = reinterpret_cast<Model::Issue*>(data);
                issue->select(controller);
            }
        }
         */

        void IssueBrowser::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasSavedNotifier.addObserver(this, &IssueBrowser::documentWasSaved);
            document->documentWasNewedNotifier.addObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &IssueBrowser::documentWasNewedOrLoaded);

            Model::IssueManager& issueManager = document->issueManager();
            issueManager.issueIgnoreChangedNotifier.addObserver(this, &IssueBrowser::issueIgnoreChanged);
        }
        
        void IssueBrowser::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasSavedNotifier.removeObserver(this, &IssueBrowser::documentWasSaved);
                document->documentWasNewedNotifier.removeObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &IssueBrowser::documentWasNewedOrLoaded);

                Model::IssueManager& issueManager = document->issueManager();
                issueManager.issueIgnoreChangedNotifier.removeObserver(this, &IssueBrowser::issueIgnoreChanged);
            }
        }
        
        void IssueBrowser::documentWasNewedOrLoaded() {
            updateFilterFlags();
        }

        void IssueBrowser::documentWasSaved() {
            m_view->Refresh();
        }
        
        void IssueBrowser::issueIgnoreChanged(Model::Issue* issue) {
            m_view->Refresh();
        }
        
        void IssueBrowser::updateFilterFlags() {
            MapDocumentSPtr document = lock(m_document);
            Model::IssueManager& issueManager = document->issueManager();
            
            wxArrayInt flags;
            wxArrayString labels;
            
            const Model::IssueManager::GeneratorList& generators = issueManager.registeredGenerators();
            Model::IssueManager::GeneratorList::const_iterator it, end;
            for (it = generators.begin(), end = generators.end(); it != end; ++it) {
                const Model::IssueGenerator* generator = *it;
                const Model::IssueType flag = generator->type();
                const String& description = generator->description();
                
                flags.push_back(flag);
                labels.push_back(description);
            }

            /*
            wxWindowUpdateLocker locker(m_tree);
            m_filterEditor->setFlags(flags, labels);
            m_model->setHiddenGenerators(issueManager.defaultHiddenGenerators());
            m_filterEditor->setFlagValue(~issueManager.defaultHiddenGenerators());
             */
        }

        void IssueBrowser::setIssueVisibility(const bool show) {
            /*
            wxWindowUpdateLocker locker(m_tree);

            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            
            MapDocumentSPtr document = lock(m_document);
            Model::IssueManager& issueManager = document->issueManager();
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                void* data = item.GetID();
                assert(data != NULL);
                Model::Issue* issue = reinterpret_cast<Model::Issue*>(data);
                issueManager.setIssueHidden(issue, !show);
            }
            
            document->incModificationCount();
            m_tree->UnselectAll();
             */
        }
    }
}
