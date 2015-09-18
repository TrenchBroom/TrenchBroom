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

#ifndef TrenchBroom_IssueBrowser
#define TrenchBroom_IssueBrowser

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"
#include "View/TabBook.h"

class wxCheckBox;
class wxCommandEvent;
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
        class IssueBrowserView;
        
        class IssueBrowser : public TabBookPage {
        private:
            static const int SelectObjectsCommandId = 1;
            static const int ShowIssuesCommandId = 2;
            static const int HideIssuesCommandId = 3;
            static const int FixObjectsBaseId = 4;

            MapDocumentWPtr m_document;
            IssueBrowserView* m_view;
            wxCheckBox* m_showHiddenIssuesCheckBox;
            FlagsPopupEditor* m_filterEditor;
        public:
            IssueBrowser(wxWindow* parent, MapDocumentWPtr document);
            ~IssueBrowser();

            wxWindow* createTabBarPage(wxWindow* parent);
            
            void OnShowHiddenIssuesChanged(wxCommandEvent& event);
            void OnFilterChanged(FlagChangedCommand& command);
        private:
            void bindObservers();
            void unbindObservers();
            void documentWasNewedOrLoaded(MapDocument* document);
            void documentWasSaved(MapDocument* document);
            void nodesWereAdded(const Model::NodeList& nodes);
            void nodesWereRemoved(const Model::NodeList& nodes);
            void nodesDidChange(const Model::NodeList& nodes);
            void brushFacesDidChange(const Model::BrushFaceList& faces);
            void issueIgnoreChanged(Model::Issue* issue);

            void updateFilterFlags();
        };
    }
}

#endif /* defined(TrenchBroom_IssueBrowser) */
