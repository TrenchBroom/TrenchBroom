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

#ifndef __TrenchBroom__MapTreeView__
#define __TrenchBroom__MapTreeView__

#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxDataViewCtrl;
class wxDataViewEvent;
class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class SelectionResult;
    }
    
    namespace View {
        class MapTreeView : public wxPanel {
        private:
            MapDocumentPtr m_document;
            ControllerPtr m_controller;
            wxDataViewCtrl* m_tree;
            bool m_ignoreTreeSelection;
            bool m_ignoreDocumentSelection;
        public:
            MapTreeView(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller);
            ~MapTreeView();
            
            void OnTreeViewSize(wxSizeEvent& event);
            void OnTreeViewSelectionChanged(wxDataViewEvent& event);
        private:
            void bindObservers();
            void unbindObservers();
            void selectionDidChange(const Model::SelectionResult& result);
        };
    }
}

#endif /* defined(__TrenchBroom__MapTreeView__) */
