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

#ifndef __TrenchBroom__MapInspector__
#define __TrenchBroom__MapInspector__

#include "View/GLContextHolder.h"
#include "View/TabBook.h"
#include "View/ViewTypes.h"

class wxCollapsiblePaneEvent;
class wxWindow;

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class MapTreeView;
        class ModEditor;
        
        class MapInspector : public TabBookPage {
        public:
            MapInspector(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document);

            void OnPaneChanged(wxCollapsiblePaneEvent& event);
        private:
            void createGui(GLContextHolder::Ptr sharedContext, MapDocumentWPtr document);
            wxWindow* createLayerEditor(wxWindow* parent, MapDocumentWPtr document);
            wxWindow* createModEditor(wxWindow* parent, MapDocumentWPtr document);
        };
    }
}

#endif /* defined(__TrenchBroom__MapInspector__) */
