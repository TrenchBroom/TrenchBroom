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

#ifndef __TrenchBroom__IssueBrowser__
#define __TrenchBroom__IssueBrowser__

#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxCommandEvent;
class wxDataViewCtrl;
class wxDataViewEvent;
class wxDataViewItemArray;
class wxMouseEvent;
class wxSizeEvent;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class IssueBrowser : public wxPanel {
        private:
            static const int SelectObjectsCommandId = 1;
            static const int IgnoreIssuesCommandId = 2;
            static const int FixObjectsBaseId = 3;

            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            wxDataViewCtrl* m_tree;
        public:
            IssueBrowser(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);

            void OnTreeViewContextMenu(wxDataViewEvent& event);
            void OnSelectIssues(wxCommandEvent& event);
            void OnIgnoreIssues(wxCommandEvent& event);
            void OnApplyQuickFix(wxCommandEvent& event);
            void OnTreeViewSize(wxSizeEvent& event);
        private:
            void selectIssueObjects(const wxDataViewItemArray& selections, View::ControllerSPtr controller);
        };
    }
}

#endif /* defined(__TrenchBroom__IssueBrowser__) */
