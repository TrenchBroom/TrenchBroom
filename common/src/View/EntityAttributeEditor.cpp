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
        m_document(document),
        m_currentDefinition(nullptr) {
            createGui(this, document);
            bindObservers();
        }

        EntityAttributeEditor::~EntityAttributeEditor() {
            unbindObservers();
        }

        void EntityAttributeEditor::OnIdle(wxIdleEvent& event) {
            if (IsBeingDeleted()) return;

            // Hacky way of listening for changes to the current row of the attribute grid
            const String attributeName = m_attributeGrid->selectedRowName();
            if (!attributeName.empty() && attributeName != m_lastSelectedAttributeName) {
                m_lastSelectedAttributeName = attributeName;

                updateDocumentationAndSmartEditor();
            }
        }

        void EntityAttributeEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &EntityAttributeEditor::selectionDidChange);
            document->nodesDidChangeNotifier.addObserver(this, &EntityAttributeEditor::nodesDidChange);
        }

        void EntityAttributeEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &EntityAttributeEditor::selectionDidChange);
                document->nodesDidChangeNotifier.removeObserver(this, &EntityAttributeEditor::nodesDidChange);
            }
        }

        void EntityAttributeEditor::selectionDidChange(const Selection& selection) {
            updateIfSelectedEntityDefinitionChanged();
        }

        void EntityAttributeEditor::nodesDidChange(const Model::NodeList& nodes) {
            updateIfSelectedEntityDefinitionChanged();
        }

        void EntityAttributeEditor::updateIfSelectedEntityDefinitionChanged() {
            MapDocumentSPtr document = lock(m_document);
            const Assets::EntityDefinition* entityDefinition = Model::AttributableNode::selectEntityDefinition(document->allSelectedAttributableNodes());

            if (entityDefinition != m_currentDefinition) {
                m_currentDefinition = entityDefinition;

                updateDocumentationAndSmartEditor();
            }
        }

        void EntityAttributeEditor::updateDocumentationAndSmartEditor() {
            MapDocumentSPtr document = lock(m_document);
            const String& attributeName = m_attributeGrid->selectedRowName();

            m_smartEditorManager->switchEditor(attributeName, document->allSelectedAttributableNodes());

            updateDocumentation(attributeName);

            // collapse the splitter if needed
            if (!m_documentationText->IsEmpty() && !m_smartEditorManager->isDefaultEditorActive()) {
                // show both
                m_documentationSplitter->restore();
            } else if (m_documentationText->IsEmpty() && !m_smartEditorManager->isDefaultEditorActive()) {
                // show only the smart editor
                m_documentationSplitter->maximize(m_smartEditorManager);
            } else if (!m_documentationText->IsEmpty() && m_smartEditorManager->isDefaultEditorActive()) {
                // show only the documentation panel
                m_documentationSplitter->maximize(m_documentationText);
            } else {
                // nothing to display
                m_documentationSplitter->maximize(m_documentationText);
            }
        }

        wxString EntityAttributeEditor::optionDescriptions(const Assets::AttributeDefinition& definition) {
            const auto bullet = wxString(" ") << wxUniChar(0x2022) << wxString(" ");

            switch (definition.type()) {
                case Assets::AttributeDefinition::Type_ChoiceAttribute: {
                    const auto& choiceDef = dynamic_cast<const Assets::ChoiceAttributeDefinition&>(definition);

                    wxString stream;
                    for (auto& option : choiceDef.options()) {
                        stream << bullet << option.value();
                        if (!option.description().empty()) {
                            stream << " (" << option.description() << ")";
                        }
                        stream << "\n";
                    }
                    return stream;
                }
                case Assets::AttributeDefinition::Type_FlagsAttribute: {
                    const auto& flagsDef = dynamic_cast<const Assets::FlagsAttributeDefinition&>(definition);

                    // The options are not necessarily sorted by value, so we sort the descriptions here by inserting
                    // into a map sorted by the flag value.
                    std::map<int, wxString> flagDescriptors;
                    for (auto& option : flagsDef.options()) {
                        auto& stream = flagDescriptors[option.value()];
                        stream << bullet << option.value() << " = " << option.shortDescription();
                        if (!option.longDescription().empty()) {
                            stream << " (" << option.longDescription() << ")";
                        }
                    }

                    // Concatenate the flag descriptions and return.
                    wxString stream;
                    for (const auto& flagDescriptor : flagDescriptors) {
                        stream << flagDescriptor.second << "\n";
                    }
                    return stream;
                }
                case Assets::AttributeDefinition::Type_StringAttribute:
                case Assets::AttributeDefinition::Type_BooleanAttribute:
                case Assets::AttributeDefinition::Type_IntegerAttribute:
                case Assets::AttributeDefinition::Type_FloatAttribute:
                case Assets::AttributeDefinition::Type_TargetSourceAttribute:
                case Assets::AttributeDefinition::Type_TargetDestinationAttribute:
                    return wxString();
                switchDefault()
            }
        }

        void EntityAttributeEditor::updateDocumentation(const String &attributeName) {
            MapDocumentSPtr document = lock(m_document);
            const Assets::EntityDefinition* entityDefinition = Model::AttributableNode::selectEntityDefinition(document->allSelectedAttributableNodes());

            m_documentationText->Clear();

            wxTextAttr boldAttr;
            boldAttr.SetFontWeight(wxFONTWEIGHT_BOLD);

            if (entityDefinition != nullptr) {
                // add attribute documentation, if available
                const Assets::AttributeDefinition* attributeDefinition = entityDefinition->attributeDefinition(attributeName);
                if (attributeDefinition != nullptr) {
                    const wxString optionsDescription = optionDescriptions(*attributeDefinition);

                    const bool attributeHasDocs = !attributeDefinition->longDescription().empty()
                                               || !attributeDefinition->shortDescription().empty()
                                               || !optionsDescription.empty();

                    if (attributeHasDocs) {
                        // e.g. "Attribute "delay" (Attenuation formula)", in bold
                        {
                            const long start = m_documentationText->GetLastPosition();
                            m_documentationText->AppendText("Attribute \"");
                            m_documentationText->AppendText(attributeDefinition->name());
                            m_documentationText->AppendText("\"");
                            if (!attributeDefinition->shortDescription().empty()) {
                                m_documentationText->AppendText(" (");
                                m_documentationText->AppendText(attributeDefinition->shortDescription());
                                m_documentationText->AppendText(")");
                            }
                            m_documentationText->AppendText("\n");
                            const long end = m_documentationText->GetLastPosition();
                            m_documentationText->SetStyle(start, end, boldAttr);
                        }

                        if (!attributeDefinition->longDescription().empty()) {
                            m_documentationText->AppendText("\n");
                            m_documentationText->AppendText(attributeDefinition->longDescription());
                            m_documentationText->AppendText("\n");
                        }

                        if (!optionsDescription.empty()) {
                            m_documentationText->AppendText("\nOptions:\n");
                            m_documentationText->AppendText(optionsDescription); // ends with a newline
                        }
                    }
                }

                // add class description, if available
                if (!entityDefinition->description().empty()) {
                    // add space after attribute text
                    if (!m_documentationText->IsEmpty()) {
                        m_documentationText->AppendText("\n");
                    }

                    // e.g. "Class "func_door"", in bold
                    {
                        const long start = m_documentationText->GetLastPosition();
                        m_documentationText->AppendText("Class \"");
                        m_documentationText->AppendText(entityDefinition->name());
                        m_documentationText->AppendText("\"\n");
                        const long end = m_documentationText->GetLastPosition();
                        m_documentationText->SetStyle(start, end, boldAttr);
                    }

                    m_documentationText->AppendText("\n");
                    m_documentationText->AppendText(entityDefinition->description());
                    m_documentationText->AppendText("\n");
                }
            }

            // Scroll to the top
            m_documentationText->ShowPosition(0);
        }

        void EntityAttributeEditor::createGui(wxWindow* parent, MapDocumentWPtr document) {
            SplitterWindow2* splitter = new SplitterWindow2(parent);
            splitter->setSashGravity(1.0);
            splitter->SetName("EntityAttributeEditorSplitter");

            m_attributeGrid = new EntityAttributeGrid(splitter, document);

            m_documentationSplitter = new SplitterWindow2(splitter);
            m_documentationSplitter->setSashGravity(1.0);
            m_documentationSplitter->SetName("EntityDocumentationSplitter");

            m_documentationText = new wxTextCtrl(m_documentationSplitter, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP | wxBORDER_NONE);
            m_smartEditorManager = new SmartAttributeEditorManager(m_documentationSplitter, document);
            m_documentationSplitter->splitHorizontally(m_smartEditorManager,
                                                       m_documentationText,
                                                       wxSize(100, 50), wxSize(100, 50));

            splitter->splitHorizontally(m_attributeGrid,
                                        m_documentationSplitter,
                                        wxSize(100, 50), wxSize(100, 100));

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(splitter, 1, wxEXPAND);
            sizer->SetItemMinSize(m_smartEditorManager, 500, 50);
            SetSizer(sizer);

            wxPersistenceManager::Get().RegisterAndRestore(splitter);
            wxPersistenceManager::Get().RegisterAndRestore(m_documentationSplitter);

            Bind(wxEVT_IDLE, &EntityAttributeEditor::OnIdle, this);
        }
    }
}
