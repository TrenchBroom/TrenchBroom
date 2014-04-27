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

#ifndef __TrenchBroom__Inspector__
#define __TrenchBroom__Inspector__

#include "Controller/Command.h"
#include "View/GLContextHolder.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxNotebook;

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class EntityInspector;
        class FaceInspector;
        class MapInspector;
        class ViewInspector;
        
        class Inspector : public wxPanel {
        private:
            wxNotebook* m_notebook;
            MapInspector* m_mapInspector;
            EntityInspector* m_entityInspector;
            FaceInspector* m_faceInspector;
            ViewInspector* m_viewInspector;
        public:
            Inspector(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera);
            void switchToPage(InspectorPage page);
        };
    }
}

#endif /* defined(__TrenchBroom__Inspector__) */
