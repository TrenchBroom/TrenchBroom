/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_EntityAttributeEditor
#define TrenchBroom_EntityAttributeEditor

#include "View/ViewTypes.h"

#include <wx/panel.h>

namespace TrenchBroom {
    namespace View {
        class EntityAttributeGrid;
        class EntityAttributeSelectedCommand;
        class SmartAttributeEditorManager;
        
        class EntityAttributeEditor : public wxPanel {
        private:
            View::MapDocumentWPtr m_document;
            EntityAttributeGrid* m_attributeGrid;
            SmartAttributeEditorManager* m_smartEditorManager;
        public:
            EntityAttributeEditor(wxWindow* parent, MapDocumentWPtr document);
            
            void OnEntityAttributeSelected(EntityAttributeSelectedCommand& command);
        private:
            void createGui(wxWindow* parent, MapDocumentWPtr document);
        };
    }
}

#endif /* defined(TrenchBroom_EntityAttributeEditor) */
