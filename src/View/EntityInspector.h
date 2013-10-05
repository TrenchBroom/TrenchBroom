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

#include "Controller/Command.h"
#include "View/ViewTypes.h"

#include <wx/grid.h>
#include <wx/panel.h>

class wxButton;

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
        class EntityPropertyGridTable;
        
        class EntityInspector : public wxPanel {
        private:
            MapDocumentPtr m_document;
            ControllerPtr m_controller;
            
            EntityPropertyGridTable* m_propertyTable;
            wxGrid* m_propertyGrid;
            wxGridCellCoords m_lastHoveredCell;
            wxButton* m_addPropertyButton;
            wxButton* m_removePropertiesButton;
            
            EntityBrowser* m_entityBrowser;
        public:
            EntityInspector(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller, Renderer::RenderResources& resources);
            ~EntityInspector();

            void OnPropertyGridSize(wxSizeEvent& event);
            void OnPropertyGridSelectCell(wxGridEvent& event);
            void OnPropertyGridTab(wxGridEvent& event);
            void OnPropertyGridMouseMove(wxMouseEvent& event);
            
            void OnAddPropertyPressed(wxCommandEvent& event);
            void OnRemovePropertiesPressed(wxCommandEvent& event);
            void OnUpdatePropertyViewOrAddPropertiesButton(wxUpdateUIEvent& event);
            void OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event);
        private:
            void bindObservers();
            void unbindObservers();

            void commandDoneOrUndone(Controller::Command::Ptr command);
            void documentWasNewedOrLoaded();
            void objectDidChange(Model::Object* object);
            void selectionDidChange(const Model::SelectionResult& result);
            
            void updatePropertyGrid();
            void updateEntityBrowser();
            wxWindow* createPropertyEditor(wxWindow* parent);
            wxWindow* createEntityBrowser(wxWindow* parent, Renderer::RenderResources& resources);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityInspector__) */
