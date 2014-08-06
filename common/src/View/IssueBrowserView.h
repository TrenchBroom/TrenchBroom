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

#ifndef __TrenchBroom__IssueBrowserView__
#define __TrenchBroom__IssueBrowserView__

#include "View/ViewTypes.h"

#include "Model/Issue.h"

#include <wx/listctrl.h>

#include <vector>

class wxWindow;

namespace TrenchBroom {
    namespace View {
        class IssueBrowserView : public wxListCtrl {
        private:
            static const int SelectObjectsCommandId = 1;
            static const int ShowIssuesCommandId = 2;
            static const int HideIssuesCommandId = 3;
            static const int FixObjectsBaseId = 4;
            
            typedef std::vector<size_t> IndexList;
            
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
        public:
            IssueBrowserView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);
            ~IssueBrowserView();
            
            void OnSize(wxSizeEvent& event);
            void OnItemRightClick(wxListEvent& event);
            void OnSelectIssues(wxCommandEvent& event);
            void OnShowIssues(wxCommandEvent& event);
            void OnHideIssues(wxCommandEvent& event);
            void OnApplyQuickFix(wxCommandEvent& event);
        private:
            Model::QuickFix::List collectQuickFixes(const IndexList& selection) const;
            void setIssueVisibility(bool show);
            void selectIssueObjects(const IndexList& selection, View::ControllerSPtr controller);
            
            IndexList getSelection() const;
            void select(const IndexList& indices);
            void deselectAll();
            
            wxString OnGetItemText(long item, long column) const;
            
            void bindObservers();
            void unbindObservers();
            void issueCountDidChange(size_t count);
            
            void bindEvents();
        };
    }
}

#endif /* defined(__TrenchBroom__IssueBrowserView__) */
