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

#include "EntityAttributeEditor.h"

#include "View/BorderLine.h"
#include "View/EntityAttributeGrid.h"
#include "View/EntityAttributeSelectedCommand.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SmartAttributeEditorManager.h"
#include "View/SplitterWindow2.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityAttributeEditor::EntityAttributeEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui(this, document);
        }

        void EntityAttributeEditor::OnEntityAttributeSelected(EntityAttributeSelectedCommand& command) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);

            const String& name = command.name();
            const Model::AttributableNodeList& attributables = document->allSelectedAttributableNodes();
            m_smartEditorManager->switchEditor(name, attributables);
        }

        void EntityAttributeEditor::createGui(wxWindow* parent, MapDocumentWPtr document) {
            SplitterWindow2* splitter = new SplitterWindow2(parent);
            splitter->setSashGravity(1.0);
            splitter->SetName("EntityAttributeEditorSplitter");
            
            m_attributeGrid = new EntityAttributeGrid(splitter, document);
            m_smartEditorManager = new SmartAttributeEditorManager(splitter, document);
            
            splitter->splitHorizontally(m_attributeGrid,
                                        m_smartEditorManager,
                                        wxSize(100, 50), wxSize(100, 50));

            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(splitter, 1, wxEXPAND);
            sizer->SetItemMinSize(m_smartEditorManager, 500, 100);
            SetSizer(sizer);
            
            m_attributeGrid->Bind(ENTITY_ATTRIBUTE_SELECTED_EVENT, &EntityAttributeEditor::OnEntityAttributeSelected, this);
        }
    }
}
