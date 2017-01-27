/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include <wx/persist.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityAttributeEditor::EntityAttributeEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui(this, document);
        }

        void EntityAttributeEditor::OnIdle(wxIdleEvent& event) {
            if (IsBeingDeleted()) return;

            const String& attributeName = m_attributeGrid->selectedRowName();
            if (!attributeName.empty() && attributeName != m_lastSelectedAttributeName) {
                MapDocumentSPtr document = lock(m_document);
                m_smartEditorManager->switchEditor(attributeName, document->allSelectedAttributableNodes());
                m_lastSelectedAttributeName = attributeName;
            }
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
            
            wxPersistenceManager::Get().RegisterAndRestore(splitter);
            
            Bind(wxEVT_IDLE, &EntityAttributeEditor::OnIdle, this);
        }
    }
}
