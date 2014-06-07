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

#include "EntityPropertyEditor.h"

#include "View/BorderLine.h"
#include "View/EntityPropertyGrid.h"
#include "View/EntityPropertySelectedCommand.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SmartPropertyEditorManager.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityPropertyEditor::EntityPropertyEditor(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document) {
            createGui(this, document, controller);
        }

        void EntityPropertyEditor::OnEntityPropertySelected(EntityPropertySelectedCommand& command) {
            MapDocumentSPtr document = lock(m_document);

            const String& key = command.key();
            const Model::EntityList& entities = document->allSelectedEntities();
            m_smartEditorManager->switchEditor(key, entities);
        }

        void EntityPropertyEditor::createGui(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) {
            m_propertyGrid = new EntityPropertyGrid(parent, document, controller);
            m_smartEditorManager = new SmartPropertyEditorManager(parent, document, controller);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_propertyGrid, 1, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(m_smartEditorManager, 0, wxEXPAND);
            sizer->SetItemMinSize(m_propertyGrid, 100, 200);
            sizer->SetItemMinSize(m_smartEditorManager, 100, 120);
            SetSizer(sizer);
            
            m_propertyGrid->Bind(EVT_ENTITY_PROPERTY_SELECTED_EVENT, EVT_ENTITY_PROPERTY_SELECTED_HANDLER(EntityPropertyEditor::OnEntityPropertySelected), this);
        }
    }
}
