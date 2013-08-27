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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Inspector__
#define __TrenchBroom__Inspector__

#include "Controller/Command.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxNotebook;

namespace TrenchBroom {
    namespace Renderer {
        class RenderResources;
    }
    namespace View {
        class EntityInspector;
        class FaceInspector;
        class ViewInspector;
        
        class Inspector : public wxPanel {
        private:
            wxNotebook* m_notebook;
            EntityInspector* m_entityInspector;
            FaceInspector* m_faceInspector;
            ViewInspector* m_viewInspector;
        public:
            Inspector(wxWindow* parent, MapDocumentPtr document, ControllerFacade& controller, Renderer::RenderResources& resources);

            void update(Controller::Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__Inspector__) */
