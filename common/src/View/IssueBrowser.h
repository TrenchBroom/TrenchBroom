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

#ifndef __TrenchBroom__IssueBrowser__
#define __TrenchBroom__IssueBrowser__

#include "View/ViewTypes.h"
#include "View/TabBook.h"

class wxCheckBox;
class wxCommandEvent;
class wxDataViewCtrl;
class wxDataViewEvent;
class wxDataViewItemArray;
class wxMouseEvent;
class wxSimplebook;
class wxSizeEvent;
class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class Issue;
    }
    
    namespace View {
        class FlagChangedCommand;
        class FlagsPopupEditor;
        class IssueBrowserDataModel;
        
        class IssueBrowser : public TabBookPage {
        private:
            static const int SelectObjectsCommandId = 1;
            static const int ShowIssuesCommandId = 2;
            static const int HideIssuesCommandId = 3;
            static const int FixObjectsBaseId = 4;

            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            IssueBrowserDataModel* m_model;
            wxDataViewCtrl* m_tree;
            wxCheckBox* m_showHiddenIssuesCheckBox;
            FlagsPopupEditor* m_filterEditor;
        public:
            IssueBrowser(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);
            ~IssueBrowser();

            wxWindow* createTabBarPage(wxWindow* parent);
            
            void OnShowHiddenIssuesChanged(wxCommandEvent& event);
            void OnFilterChanged(FlagChangedCommand& command);
            void OnTreeViewContextMenu(wxDataViewEvent& event);
            void OnSelectIssues(wxCommandEvent& event);
            void OnShowIssues(wxCommandEvent& event);
            void OnHideIssues(wxCommandEvent& event);
            void OnApplyQuickFix(wxCommandEvent& event);
            void OnTreeViewSize(wxSizeEvent& event);
        private:
            void bindObservers();
            void unbindObservers();
            void documentWasNewedOrLoaded();
            void documentWasSaved();

            void updateFilterFlags();
            
            void selectIssueObjects(const wxDataViewItemArray& selections, View::ControllerSPtr controller);
            void setIssueVisibility(bool show);
        };
    }
}

#endif /* defined(__TrenchBroom__IssueBrowser__) */
