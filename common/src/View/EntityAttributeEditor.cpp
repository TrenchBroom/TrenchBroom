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

#include "View/EntityAttributeGrid.h"
#include "View/EntityAttributeSelectedCommand.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SmartAttributeEditorManager.h"
#include "View/SplitterWindow2.h"
#include "Model/AttributableNode.h"
#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"

#include <wx/persist.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        EntityAttributeEditor::EntityAttributeEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui(this, document);
        }

        void EntityAttributeEditor::OnIdle(wxIdleEvent& event) {
            if (IsBeingDeleted()) return;

            // FIXME: This won't notice the case when the current row name stays the same, but you select a different
            // entity with a different entity description
            const String& attributeName = m_attributeGrid->selectedRowName();
            if (!attributeName.empty() && attributeName != m_lastSelectedAttributeName) {
                MapDocumentSPtr document = lock(m_document);

                m_smartEditorManager->switchEditor(attributeName, document->allSelectedAttributableNodes());

                updateAttributeDocumentation(attributeName);

                m_lastSelectedAttributeName = attributeName;
            }
        }

        void EntityAttributeEditor::updateAttributeDocumentation(const String& attributeName) {
            MapDocumentSPtr document = lock(m_document);
            const Assets::EntityDefinition* entityDefinition = Model::AttributableNode::selectEntityDefinition(document->allSelectedAttributableNodes());

            m_attributeDocumentation->Clear();
            if (entityDefinition != nullptr) {
                if (const Assets::AttributeDefinition* attributeDefinition = entityDefinition->attributeDefinition(attributeName); attributeDefinition != nullptr) {

                    const long start = m_attributeDocumentation->GetLastPosition();
                    m_attributeDocumentation->AppendText(attributeDefinition->shortDescription());
                    const long end = m_attributeDocumentation->GetLastPosition();

                    // Make the shortDescription() bold
                    wxTextAttr boldAttr;
                    boldAttr.SetFontWeight(wxFONTWEIGHT_BOLD);
                    m_attributeDocumentation->SetStyle(start, end, boldAttr);

                    if (!attributeDefinition->longDescription().empty()) {
                        m_attributeDocumentation->AppendText("\n\n");
                        m_attributeDocumentation->AppendText(attributeDefinition->longDescription());
                    }

                    // Scroll to the top
                    m_attributeDocumentation->ShowPosition(0);
                }
            }
        }
        
        void EntityAttributeEditor::createGui(wxWindow* parent, MapDocumentWPtr document) {
            SplitterWindow2* splitter = new SplitterWindow2(parent);
            splitter->setSashGravity(1.0);
            splitter->SetName("EntityAttributeEditorSplitter");
            
            m_attributeGrid = new EntityAttributeGrid(splitter, document);

            SplitterWindow2* docsSplitter = new SplitterWindow2(splitter);
            docsSplitter->setSashGravity(1.0);
            docsSplitter->SetName("AttributeDocumentationSplitter");

            m_attributeDocumentation = new wxTextCtrl(docsSplitter, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP | wxBORDER_NONE);
            m_smartEditorManager = new SmartAttributeEditorManager(docsSplitter, document);
            docsSplitter->splitHorizontally(m_attributeDocumentation,
                                            m_smartEditorManager,
                                            wxSize(100, 50), wxSize(100, 100));

            splitter->splitHorizontally(m_attributeGrid,
                                        docsSplitter,
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
