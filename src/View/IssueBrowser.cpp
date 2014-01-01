/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Model/Issue.h"
#include "Model/IssueManager.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"

#include <wx/dataview.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/variant.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        class IssueBrowserDataModel : public wxDataViewModel {
        private:
            Model::IssueManager& m_issueManager;
        public:
            IssueBrowserDataModel(Model::IssueManager& issueManager) :
            m_issueManager(issueManager) {
                bindObservers();
            }
            
            ~IssueBrowserDataModel() {
                unbindObservers();
            }
            
            unsigned int GetColumnCount() const {
                return 1;
            }
            
            wxString GetColumnType(const unsigned int col) const {
                assert(col == 0);
                return _("string");
            }
            
            bool IsContainer(const wxDataViewItem& item) const {
                if (!item.IsOk())
                    return true;
                
                const void* data = item.GetID();
                assert(data != NULL);
                const Model::Issue* issue = reinterpret_cast<const Model::Issue*>(data);
                return issue->subIssueCount() > 0;
            }
            
            unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const {
                if (!item.IsOk()) {
                    Model::Issue* issue = m_issueManager.issues();
                    while (issue != NULL) {
                        if (!issue->ignore())
                            children.Add(wxDataViewItem(reinterpret_cast<void*>(issue)));
                        issue = issue->next();
                    }
                } else {
                    const void* data = item.GetID();
                    assert(data != NULL);
                    const Model::Issue* issue = reinterpret_cast<const Model::Issue*>(data);
                    Model::Issue* subIssue = issue->subIssues();
                    while (subIssue != NULL) {
                        if (!subIssue->ignore())
                            children.Add(wxDataViewItem(reinterpret_cast<void*>(subIssue)));
                        subIssue = subIssue->next();
                    }
                }
                
                return children.size();
            }
            
            wxDataViewItem GetParent(const wxDataViewItem& item) const {
                if (!item.IsOk())
                    return wxDataViewItem(NULL);
                
                const void* data = item.GetID();
                assert(data != NULL);
                
                const Model::Issue* issue = reinterpret_cast<const Model::Issue*>(data);
                Model::Issue* parent = issue->parent();
                return wxDataViewItem(reinterpret_cast<void*>(parent));
            }
            
            void GetValue(wxVariant& result, const wxDataViewItem& item, const unsigned int col) const {
                assert(col == 0);
                if (!item.IsOk()) {
                    result = wxVariant("Issues");
                } else {
                    const void* data = item.GetID();
                    assert(data != NULL);
                    const Model::Issue* issue = reinterpret_cast<const Model::Issue*>(data);
                    result = wxVariant(wxString(issue->description()));
                }
            }
            
            bool SetValue(const wxVariant& value, const wxDataViewItem& item, const unsigned int col) {
                assert(col == 0);
                return false;
            }
        private:
            void bindObservers() {
                m_issueManager.issueWasAddedNotifier.addObserver(this, &IssueBrowserDataModel::issueWasAdded);
                m_issueManager.issueWillBeRemovedNotifier.addObserver(this, &IssueBrowserDataModel::issueWillBeRemoved);
                m_issueManager.issueIgnoreChangedNotifier.addObserver(this, &IssueBrowserDataModel::issueIgnoreChanged);
                m_issueManager.issuesClearedNotifier.addObserver(this, &IssueBrowserDataModel::issuesCleared);
            }
            
            void unbindObservers() {
                m_issueManager.issueWasAddedNotifier.removeObserver(this, &IssueBrowserDataModel::issueWasAdded);
                m_issueManager.issueWillBeRemovedNotifier.removeObserver(this, &IssueBrowserDataModel::issueWillBeRemoved);
                m_issueManager.issueIgnoreChangedNotifier.removeObserver(this, &IssueBrowserDataModel::issueIgnoreChanged);
                m_issueManager.issuesClearedNotifier.removeObserver(this, &IssueBrowserDataModel::issuesCleared);
            }
            
            void issueWasAdded(Model::Issue* issue) {
                assert(issue != NULL);
                addIssue(issue);
            }
            
            void issueWillBeRemoved(Model::Issue* issue) {
                assert(issue != NULL);
                removeIssue(issue);
            }
            
            void issueIgnoreChanged(Model::Issue* issue) {
                assert(issue != NULL);
                if (issue->ignore())
                    removeIssue(issue);
                else
                    addIssue(issue);
            }
            
            void issuesCleared() {
                Cleared();
            }

            void addIssue(Model::Issue* issue) {
                if (!issue->ignore()) {
                    const bool success = ItemAdded(wxDataViewItem(NULL), wxDataViewItem(reinterpret_cast<void*>(issue)));
                    assert(success);
                    if (issue->subIssueCount() > 0) {
                        Model::Issue* subIssue = issue->subIssues();
                        assert(subIssue != NULL);
                        while (subIssue != NULL) {
                            if (!subIssue->ignore()) {
                                const bool success =
                                ItemAdded(wxDataViewItem(reinterpret_cast<void*>(issue)),
                                          wxDataViewItem(reinterpret_cast<void*>(subIssue)));
                                assert(success);
                            }
                            subIssue = subIssue->next();
                        }
                    }
                }
            }
            
            void removeIssue(Model::Issue* issue) {
                const bool success = ItemDeleted(wxDataViewItem(NULL), wxDataViewItem(reinterpret_cast<void*>(issue)));
                assert(success);
            }
        };
        
        IssueBrowser::IssueBrowser(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_tree(NULL) {
            m_tree = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_MULTIPLE | wxBORDER_SIMPLE);
            m_tree->AssociateModel(new IssueBrowserDataModel(lock(document)->issueManager()));
            m_tree->AppendTextColumn("Description", 0)->SetWidth(200);
            m_tree->Expand(wxDataViewItem(NULL));
            
            m_tree->Bind(wxEVT_SIZE, &IssueBrowser::OnTreeViewSize, this);
            m_tree->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &IssueBrowser::OnTreeViewContextMenu, this);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_tree, 1, wxEXPAND);
            SetSizerAndFit(sizer);
        }

        Model::QuickFix::List collectQuickFixes(const wxDataViewItemArray& selections) {
            assert(!selections.empty());
            Model::QuickFix::List result = reinterpret_cast<Model::Issue*>(selections[0].GetID())->quickFixes();
            for (size_t i = 1; i < selections.size(); ++i) {
                const Model::QuickFix::List quickFixes = reinterpret_cast<Model::Issue*>(selections[0].GetID())->quickFixes();
                
                Model::QuickFix::List::iterator it = result.begin();
                while (it != result.end()) {
                    Model::QuickFix& quickFix = *it;
                    if (!VectorUtils::contains(quickFixes, quickFix))
                        it = result.erase(it);
                    else
                        ++it;
                }
            }
            return result;
        }
        
        void IssueBrowser::OnTreeViewContextMenu(wxDataViewEvent& event) {
            const wxDataViewItem& item = event.GetItem();
            if (!item.IsOk())
                return;
            if (!m_tree->IsSelected(item))
                return;
            
            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            assert(!selections.empty());

            const Model::QuickFix::List quickFixes = collectQuickFixes(selections);

            wxMenu* quickFixMenu = new wxMenu();
            for (size_t i = 0; i < quickFixes.size(); ++i) {
                const Model::QuickFix& quickFix = quickFixes[i];
                quickFixMenu->Append(FixObjectsBaseId + i, quickFix.description());
            }
            
            wxString ignoreStr;
            ignoreStr << "Ignore " << (selections.size() == 1 ? "this issue" : "these issues");
            
            wxMenu popupMenu;
            popupMenu.Append(SelectObjectsCommandId, "Select");
            popupMenu.Append(IgnoreIssuesCommandId, ignoreStr);
            popupMenu.AppendSubMenu(quickFixMenu, "Fix");
            
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnSelectIssues, this, SelectObjectsCommandId);
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnIgnoreIssues, this, IgnoreIssuesCommandId);
            quickFixMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnApplyQuickFix, this, FixObjectsBaseId, FixObjectsBaseId + quickFixes.size());
            
            PopupMenu(&popupMenu);
        }
        
        void IssueBrowser::OnSelectIssues(wxCommandEvent& event) {
            View::ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("Select fixable objects");

            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            selectIssueObjects(selections, controller);
            
            controller->closeGroup();
        }
        
        void IssueBrowser::OnIgnoreIssues(wxCommandEvent& event) {
            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);

            MapDocumentSPtr document = lock(m_document);
            Model::IssueManager& issueManager = document->issueManager();
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                void* data = item.GetID();
                assert(data != NULL);
                Model::Issue* issue = reinterpret_cast<Model::Issue*>(data);
                issueManager.setIgnoreIssue(issue, true);
            }
            
            document->incModificationCount();
        }

        void IssueBrowser::OnApplyQuickFix(wxCommandEvent& event) {
            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            assert(!selections.empty());
            
            Model::QuickFix::List quickFixes = collectQuickFixes(selections);
            const size_t index = static_cast<size_t>(event.GetId()) - FixObjectsBaseId;
            assert(index < quickFixes.size());
            Model::QuickFix& quickFix = quickFixes[index];
            
            View::ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("");

            selectIssueObjects(selections, controller);
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                void* data = item.GetID();
                assert(data != NULL);
                Model::Issue* issue = reinterpret_cast<Model::Issue*>(data);
                
                issue->select(controller);
                quickFix.apply(*issue, controller);
            }
            
            controller->closeGroup();
        }
        
        void IssueBrowser::OnTreeViewSize(wxSizeEvent& event) {
            const int newWidth = std::max(20, m_tree->GetClientSize().x - 20);
            m_tree->GetColumn(0)->SetWidth(newWidth);
            event.Skip();
        }

        void IssueBrowser::selectIssueObjects(const wxDataViewItemArray& selections, View::ControllerSPtr controller) {
            controller->deselectAll();
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                void* data = item.GetID();
                assert(data != NULL);
                Model::Issue* issue = reinterpret_cast<Model::Issue*>(data);
                issue->select(controller);
            }
        }
    }
}
