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

#ifndef __TrenchBroom__EntityInspector__
#define __TrenchBroom__EntityInspector__

#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxButton;
class wxCollapsiblePaneEvent;

namespace TrenchBroom {
    namespace Model {
        class Object;
        class SelectionResult;
    }
    
    namespace Renderer {
        class RenderResources;
    }
    
    namespace View {
        class EntityBrowser;
        class EntityDefinitionFileChooser;
        class EntityPropertyEditor;
        
        class EntityInspector : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;

            EntityPropertyEditor* m_propertyEditor;
            EntityBrowser* m_entityBrowser;
            EntityDefinitionFileChooser* m_entityDefinitionFileChooser;
        public:
            EntityInspector(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, Renderer::RenderResources& resources);

            void OnEntityDefinitionFileChooserPaneChanged(wxCollapsiblePaneEvent& event);
        private:
            void createGui(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, Renderer::RenderResources& resources);
            wxWindow* createPropertyEditor(wxWindow* parent);
            wxWindow* createEntityBrowser(wxWindow* parent, Renderer::RenderResources& resources);
            wxWindow* createEntityDefinitionFileChooser(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityInspector__) */
