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

#ifndef __TrenchBroom__EntityPropertyEditor__
#define __TrenchBroom__EntityPropertyEditor__

#include "View/ViewTypes.h"

#include <wx/panel.h>

namespace TrenchBroom {
    namespace View {
        class EntityPropertyGrid;
        class EntityPropertySelectedCommand;
        class SmartPropertyEditorManager;
        
        class EntityPropertyEditor : public wxPanel {
        private:
            View::MapDocumentWPtr m_document;
            EntityPropertyGrid* m_propertyGrid;
            SmartPropertyEditorManager* m_smartEditorManager;
        public:
            EntityPropertyEditor(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);
            
            void OnEntityPropertySelected(EntityPropertySelectedCommand& command);
        private:
            void createGui(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyEditor__) */
