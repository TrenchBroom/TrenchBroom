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
#include <wx/log.h>

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

                // collapse the splitter if needed
                if        (!m_documentationText->IsEmpty() && !m_smartEditorManager->isDefaultEditorActive()) {
                    // show both
                    m_documentationSplitter->restore();
                    wxLogDebug("showing both");

                } else if ( m_documentationText->IsEmpty() && !m_smartEditorManager->isDefaultEditorActive()) {
                    // show only the smart editor
                    m_documentationSplitter->maximize(m_smartEditorManager);
                    wxLogDebug("showing only the smart editor");

                } else if (!m_documentationText->IsEmpty() &&  m_smartEditorManager->isDefaultEditorActive()) {
                    // show only the documentation panel
                    m_documentationSplitter->maximize(m_documentationText);
                    wxLogDebug("showing only the smart editor");

                } else {
                    // nothing to display
                    m_documentationSplitter->maximize(m_documentationText);
                    wxLogDebug("nothing to display");

                }

                m_lastSelectedAttributeName = attributeName;
            }
        }

        void EntityAttributeEditor::updateAttributeDocumentation(const String& attributeName) {
            MapDocumentSPtr document = lock(m_document);
            const Assets::EntityDefinition* entityDefinition = Model::AttributableNode::selectEntityDefinition(document->allSelectedAttributableNodes());

            m_documentationText->Clear();
            if (entityDefinition != nullptr) {
                // attribute
                if (const Assets::AttributeDefinition* attributeDefinition = entityDefinition->attributeDefinition(attributeName); attributeDefinition != nullptr) {

                    const long start = m_documentationText->GetLastPosition();
                    m_documentationText->AppendText(attributeDefinition->shortDescription());
                    const long end = m_documentationText->GetLastPosition();

                    // Make the shortDescription() bold
                    wxTextAttr boldAttr;
                    boldAttr.SetFontWeight(wxFONTWEIGHT_BOLD);
                    m_documentationText->SetStyle(start, end, boldAttr);

                    if (!attributeDefinition->longDescription().empty()) {
                        m_documentationText->AppendText("\n\n");
                        m_documentationText->AppendText(attributeDefinition->longDescription());
                    }
                }

                m_documentationText->AppendText("\n\n");
                m_documentationText->AppendText(entityDefinition->description());

                // Scroll to the top
                m_documentationText->ShowPosition(0);
            }
        }
        
        void EntityAttributeEditor::createGui(wxWindow* parent, MapDocumentWPtr document) {
            SplitterWindow2* splitter = new SplitterWindow2(parent);
            splitter->setSashGravity(1.0);
            splitter->SetName("EntityAttributeEditorSplitter");
            
            m_attributeGrid = new EntityAttributeGrid(splitter, document);

            m_documentationSplitter = new SplitterWindow2(splitter);
            m_documentationSplitter->setSashGravity(1.0);
            m_documentationSplitter->SetName("AttributeDocumentationSplitter");

            m_documentationText = new wxTextCtrl(m_documentationSplitter, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP | wxBORDER_NONE);
            m_smartEditorManager = new SmartAttributeEditorManager(m_documentationSplitter, document);
            m_documentationSplitter->splitHorizontally(m_smartEditorManager,
                                                       m_documentationText,
                                                       wxSize(100, 50), wxSize(100, 100));

            splitter->splitHorizontally(m_attributeGrid,
                                        m_documentationSplitter,
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
