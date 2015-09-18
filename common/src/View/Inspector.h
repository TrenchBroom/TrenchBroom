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

#ifndef TrenchBroom_Inspector
#define TrenchBroom_Inspector

#include "View/ViewTypes.h"

#include <wx/panel.h>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class EntityInspector;
        class FaceInspector;
        class GLContextManager;
        class MapInspector;
        class TabBook;
        
        class Inspector : public wxPanel {
        public:
            typedef enum {
                InspectorPage_Map = 0,
                InspectorPage_Entity = 1,
                InspectorPage_Face = 2
            } InspectorPage;
        private:
            TabBook* m_tabBook;
            MapInspector* m_mapInspector;
            EntityInspector* m_entityInspector;
            FaceInspector* m_faceInspector;
        public:
            Inspector(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager);
            void connectTopWidgets(wxWindow* master);
            void switchToPage(InspectorPage page);
        private:
            void OnTopWidgetSize(wxSizeEvent& event);
        };
    }
}

#endif /* defined(TrenchBroom_Inspector) */
