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

#ifndef TrenchBroom_IssueBrowserView
#define TrenchBroom_IssueBrowserView

#include "View/ViewTypes.h"

#include "Model/Issue.h"
#include "Model/ModelTypes.h"

#include <wx/listctrl.h>

#include <vector>

class wxWindow;

namespace TrenchBroom {
    namespace View {
        class IssueBrowserView : public wxListCtrl {
        private:
            static const int ShowIssuesCommandId = 1;
            static const int HideIssuesCommandId = 2;
            static const int FixObjectsBaseId = 3;
            
            typedef std::vector<size_t> IndexList;
            
            MapDocumentWPtr m_document;
            Model::IssueList m_issues;
            
            Model::IssueType m_hiddenGenerators;
            bool m_showHiddenIssues;
        public:
            IssueBrowserView(wxWindow* parent, MapDocumentWPtr document);
            
            int hiddenGenerators() const;
            void setHiddenGenerators(int hiddenGenerators);
            void setShowHiddenIssues(bool show);
            void reset();
            
            void OnSize(wxSizeEvent& event);
            
            void OnItemRightClick(wxListEvent& event);
            void OnItemSelectionChanged(wxListEvent& event);
            void OnShowIssues(wxCommandEvent& event);
            void OnHideIssues(wxCommandEvent& event);
            void OnApplyQuickFix(wxCommandEvent& event);
        private:
            class IssueVisible;
            class IssueCmp;
            
            void updateIssues();
            
            Model::IssueList collectIssues(const IndexList& indices) const;
            Model::IssueQuickFixList collectQuickFixes(const IndexList& indices) const;
            Model::IssueType issueTypeMask() const;
            
            void setIssueVisibility(bool show);
            
            void updateSelection();
            IndexList getSelection() const;
            
            wxListItemAttr* OnGetItemAttr(long item) const;
            wxString OnGetItemText(long item, long column) const;
            
            void bindEvents();
        };
    }
}

#endif /* defined(TrenchBroom_IssueBrowserView) */
