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

#include "IssueBrowserView.h"

#include "Model/CollectMatchingIssuesVisitor.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/World.h"
#include "View/MapDocument.h"
#include "View/wxUtils.h"

#include <wx/menu.h>
#include <wx/settings.h>

namespace TrenchBroom {
    namespace View {
        IssueBrowserView::IssueBrowserView(wxWindow* parent, MapDocumentWPtr document) :
        wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES | wxBORDER_NONE),
        m_document(document),
        m_hiddenGenerators(0),
        m_showHiddenIssues(false) {
            AppendColumn("Line");
            AppendColumn("Description");
            
            reset();
            bindEvents();
        }
        
        int IssueBrowserView::hiddenGenerators() const {
            return m_hiddenGenerators;
        }
        
        void IssueBrowserView::setHiddenGenerators(const int hiddenGenerators) {
            if (hiddenGenerators == m_hiddenGenerators)
                return;
            m_hiddenGenerators = hiddenGenerators;
            reset();
        }

        void IssueBrowserView::setShowHiddenIssues(const bool show) {
            m_showHiddenIssues = show;
            reset();
        }

        void IssueBrowserView::reset() {
            updateIssues();
            SetItemCount(static_cast<long>(m_issues.size()));
            Refresh();
        }

        void IssueBrowserView::OnSize(wxSizeEvent& event) {
            if (IsBeingDeleted()) return;

            const int newWidth = std::max(1, GetClientSize().x - GetColumnWidth(0));
            SetColumnWidth(1, newWidth);
            event.Skip();
        }
        
        void IssueBrowserView::OnItemRightClick(wxListEvent& event) {
            if (IsBeingDeleted()) return;

            if (GetSelectedItemCount() == 0 || event.GetIndex() < 0)
                return;
            
            wxMenu popupMenu;
            popupMenu.Append(ShowIssuesCommandId, "Show");
            popupMenu.Append(HideIssuesCommandId, "Hide");
            popupMenu.Bind(wxEVT_MENU, &IssueBrowserView::OnShowIssues, this, ShowIssuesCommandId);
            popupMenu.Bind(wxEVT_MENU, &IssueBrowserView::OnHideIssues, this, HideIssuesCommandId);
            
            const Model::IssueQuickFixList quickFixes = collectQuickFixes(getSelection());
            if (!quickFixes.empty()) {
                wxMenu* quickFixMenu = new wxMenu();
                
                for (size_t i = 0; i < quickFixes.size(); ++i) {
                    Model::IssueQuickFix* quickFix = quickFixes[i];
                    const int quickFixId = FixObjectsBaseId + static_cast<int>(i);
                    quickFixMenu->Append(quickFixId, quickFix->description());
                    
                    wxVariant* data = new wxVariant(reinterpret_cast<void*>(quickFix));
                    quickFixMenu->Bind(wxEVT_MENU, &IssueBrowserView::OnApplyQuickFix, this, quickFixId, quickFixId, data);
                }
                
                popupMenu.AppendSeparator();
                popupMenu.AppendSubMenu(quickFixMenu, "Fix");
            }

            PopupMenu(&popupMenu);
        }
        
        void IssueBrowserView::OnItemSelectionChanged(wxListEvent& event) {
            if (IsBeingDeleted()) return;

            updateSelection();
        }

        void IssueBrowserView::OnShowIssues(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            setIssueVisibility(true);
        }
        
        void IssueBrowserView::OnHideIssues(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            setIssueVisibility(false);
        }
        
        class IssueBrowserView::IssueVisible {
            int m_hiddenTypes;
            bool m_showHiddenIssues;
        public:
            IssueVisible(const int hiddenTypes, const bool showHiddenIssues) :
            m_hiddenTypes(hiddenTypes),
            m_showHiddenIssues(showHiddenIssues) {}
            
            bool operator()(const Model::Issue* issue) const {
                return m_showHiddenIssues || (!issue->hidden() && (issue->type() & m_hiddenTypes) == 0);
            }
        };
        
        class IssueBrowserView::IssueCmp {
        public:
            bool operator()(const Model::Issue* lhs, const Model::Issue* rhs) const {
                return lhs->seqId() > rhs->seqId();
            }
        };
        
        void IssueBrowserView::updateSelection() {
            const IndexList selection = getSelection();
            
            Model::NodeList nodes;
            for (size_t i = 0; i < selection.size(); ++i) {
                Model::Issue* issue = m_issues[selection[i]];
                issue->addSelectableNodes(nodes);
            }
            
            MapDocumentSPtr document = lock(m_document);
            document->deselectAll();
            document->select(nodes);
        }

        void IssueBrowserView::updateIssues() {
            m_issues.clear();
            
            MapDocumentSPtr document = lock(m_document);
            Model::World* world = document->world();
            if (world != NULL) {
                const Model::IssueGeneratorList& issueGenerators = world->registeredIssueGenerators();
                Model::CollectMatchingIssuesVisitor<IssueVisible> visitor(issueGenerators, IssueVisible(m_hiddenGenerators, m_showHiddenIssues));
                world->acceptAndRecurse(visitor);
                m_issues = visitor.issues();
                VectorUtils::sort(m_issues, IssueCmp());
            }
        }

        void IssueBrowserView::OnApplyQuickFix(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const wxVariant* data = static_cast<wxVariant*>(event.GetEventUserData());
            assert(data != NULL);
            
            const Model::IssueQuickFix* quickFix = reinterpret_cast<const Model::IssueQuickFix*>(data->GetVoidPtr());
            assert(quickFix != NULL);

            MapDocumentSPtr document = lock(m_document);
            const Model::IssueList issues = collectIssues(getSelection());

            const Transaction transaction(document, "Apply Quick Fix (" + quickFix->description() + ")");
            updateSelection();
            quickFix->apply(document.get(), issues);
        }
        
        Model::IssueList IssueBrowserView::collectIssues(const IndexList& indices) const {
            Model::IssueList result;
            for (size_t i = 0; i < indices.size(); ++i)
                result.push_back(m_issues[indices[i]]);
            return result;
        }

        Model::IssueQuickFixList IssueBrowserView::collectQuickFixes(const IndexList& indices) const {
            if (indices.empty())
                return Model::IssueQuickFixList(0);
            
            Model::IssueType issueTypes = ~0;
            for (size_t i = 0; i < indices.size(); ++i) {
                const Model::Issue* issue = m_issues[indices[i]];
                issueTypes &= issue->type();
            }
            
            MapDocumentSPtr document = lock(m_document);
            const Model::World* world = document->world();
            return world->quickFixes(issueTypes);
        }
        
        Model::IssueType IssueBrowserView::issueTypeMask() const {
            Model::IssueType result = ~static_cast<Model::IssueType>(0);
            const IndexList selection = getSelection();
            for (size_t i = 0; i < selection.size(); ++i) {
                Model::Issue* issue = m_issues[selection[i]];
                result &= issue->type();
            }
            return result;
        }

        void IssueBrowserView::setIssueVisibility(const bool show) {
            const IndexList selection = getSelection();
            
            MapDocumentSPtr document = lock(m_document);
            for (size_t i = 0; i < selection.size(); ++i) {
                Model::Issue* issue = m_issues[selection[i]];
                document->setIssueHidden(issue, !show);
            }

            reset();
        }
        
        IssueBrowserView::IndexList IssueBrowserView::getSelection() const {
            return getListCtrlSelection(this);
        }
        
        wxListItemAttr* IssueBrowserView::OnGetItemAttr(const long item) const {
            assert(item >= 0 && static_cast<size_t>(item) < m_issues.size());

            static wxListItemAttr attr;
            
            Model::Issue* issue = m_issues[static_cast<size_t>(item)];
            if (issue->hidden()) {
                attr.SetFont(GetFont().Italic());
                return &attr;
            }
            
            return NULL;
        }

        wxString IssueBrowserView::OnGetItemText(const long item, const long column) const {
            assert(item >= 0 && static_cast<size_t>(item) < m_issues.size());
            assert(column >= 0 && column < 2);
            
            Model::Issue* issue = m_issues[static_cast<size_t>(item)];
            if (column == 0) {
                wxString result;
                result << issue->lineNumber();
                return result;
            } else {
                return issue->description();
            }
        }
        
        void IssueBrowserView::bindEvents() {
            Bind(wxEVT_SIZE, &IssueBrowserView::OnSize, this);
            Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &IssueBrowserView::OnItemRightClick, this);
            Bind(wxEVT_LIST_ITEM_SELECTED, &IssueBrowserView::OnItemSelectionChanged, this);
            Bind(wxEVT_LIST_ITEM_DESELECTED, &IssueBrowserView::OnItemSelectionChanged, this);
        }
    }
}
