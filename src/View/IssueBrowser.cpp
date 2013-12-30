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
#include "View/MapDocument.h"

#include <wx/dataview.h>
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
                        children.Add(wxDataViewItem(reinterpret_cast<void*>(issue)));
                        issue = issue->next();
                    }
                } else {
                    const void* data = item.GetID();
                    assert(data != NULL);
                    const Model::Issue* issue = reinterpret_cast<const Model::Issue*>(data);
                    Model::Issue* subIssue = issue->subIssues();
                    while (subIssue != NULL) {
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
                    result = wxVariant(wxString(issue->asString()));
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
                m_issueManager.issuesClearedNotifier.addObserver(this, &IssueBrowserDataModel::issuesCleared);
            }
            
            void unbindObservers() {
                m_issueManager.issueWasAddedNotifier.removeObserver(this, &IssueBrowserDataModel::issueWasAdded);
                m_issueManager.issueWillBeRemovedNotifier.removeObserver(this, &IssueBrowserDataModel::issueWillBeRemoved);
                m_issueManager.issuesClearedNotifier.removeObserver(this, &IssueBrowserDataModel::issuesCleared);
            }
            
            void issueWasAdded(Model::Issue* issue) {
                assert(issue != NULL);
                
                ItemAdded(wxDataViewItem(NULL), wxDataViewItem(reinterpret_cast<void*>(issue)));
                if (issue->subIssueCount() > 0) {
                    Model::Issue* subIssue = issue->subIssues();
                    assert(subIssue != NULL);
                    while (subIssue != NULL) {
                        ItemAdded(wxDataViewItem(reinterpret_cast<void*>(issue)),
                                  wxDataViewItem(reinterpret_cast<void*>(subIssue)));
                        subIssue = subIssue->next();
                    }
                }
            }
            
            void issueWillBeRemoved(Model::Issue* issue) {
                assert(issue != NULL);
                ItemDeleted(wxDataViewItem(NULL), wxDataViewItem(reinterpret_cast<void*>(issue)));
            }
            
            void issuesCleared() {
                Cleared();
            }
        };
        
        IssueBrowser::IssueBrowser(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_tree(NULL) {
            m_tree = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_MULTIPLE | wxBORDER_SIMPLE);
            m_tree->AssociateModel(new IssueBrowserDataModel(lock(document)->issueManager()));
            m_tree->AppendTextColumn("Description", 0)->SetWidth(200);
            m_tree->Expand(wxDataViewItem(NULL));
            
            m_tree->Bind(wxEVT_SIZE, &IssueBrowser::OnTreeViewSize, this);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_tree, 1, wxEXPAND);
            SetSizerAndFit(sizer);
        }

        void IssueBrowser::OnTreeViewSize(wxSizeEvent& event) {
            int newWidth = std::max(20, m_tree->GetClientSize().x - 20);
            m_tree->GetColumn(0)->SetWidth(newWidth);
            event.Skip();
        }
    }
}
