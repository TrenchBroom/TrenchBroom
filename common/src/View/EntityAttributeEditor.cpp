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
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SmartAttributeEditorManager.h"
#include "Model/AttributableNode.h"
#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QChar>
#include <QStringBuilder>
#include <QTextStream>
#include <QTextEdit>

namespace TrenchBroom {
    namespace View {
        EntityAttributeEditor::EntityAttributeEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document),
        m_currentDefinition(nullptr) {
            createGui(this, document);
            bindObservers();
        }

        EntityAttributeEditor::~EntityAttributeEditor() {
            unbindObservers();
        }

        void EntityAttributeEditor::OnCurrentRowChanged() {
            updateDocumentationAndSmartEditor();
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
            m_documentationText->setHidden(m_documentationText->document()->isEmpty());
            m_smartEditorManager->setHidden(m_smartEditorManager->isDefaultEditorActive());
        }

        QString EntityAttributeEditor::optionDescriptions(const Assets::AttributeDefinition& definition) {
            const QString bullet = QString(" ") % QChar(0x2022) % QString(" ");

            switch (definition.type()) {
                case Assets::AttributeDefinition::Type_ChoiceAttribute: {
                    const auto& choiceDef = dynamic_cast<const Assets::ChoiceAttributeDefinition&>(definition);

                    QString result;
                    QTextStream stream(&result);
                    for (auto& option : choiceDef.options()) {
                        stream << bullet << option.value().c_str();
                        if (!option.description().empty()) {
                            stream << " (" << option.description().c_str() << ")";
                        }
                        stream << "\n";
                    }
                    return result;
                }
                case Assets::AttributeDefinition::Type_FlagsAttribute: {
                    const auto& flagsDef = dynamic_cast<const Assets::FlagsAttributeDefinition&>(definition);

                    // The options are not necessarily sorted by value, so we sort the descriptions here by inserting
                    // into a map sorted by the flag value.
                    std::map<int, QString> flagDescriptors;
                    for (auto& option : flagsDef.options()) {
                        QString line;
                        QTextStream stream(&line);                       
                        stream << bullet << option.value() << " = " << option.shortDescription().c_str();
                        if (!option.longDescription().empty()) {
                            stream << " (" << option.longDescription().c_str() << ")";
                        }
                        flagDescriptors[option.value()] = line;
                    }

                    // Concatenate the flag descriptions and return.
                    QString result;
                    QTextStream stream(&result);
                    for (const auto& flagDescriptor : flagDescriptors) {
                        stream << flagDescriptor.second << "\n";
                    }
                    return result;
                }
                case Assets::AttributeDefinition::Type_StringAttribute:
                case Assets::AttributeDefinition::Type_BooleanAttribute:
                case Assets::AttributeDefinition::Type_IntegerAttribute:
                case Assets::AttributeDefinition::Type_FloatAttribute:
                case Assets::AttributeDefinition::Type_TargetSourceAttribute:
                case Assets::AttributeDefinition::Type_TargetDestinationAttribute:
                    return QString();
                switchDefault()
            }
        }

        void EntityAttributeEditor::updateDocumentation(const String &attributeName) {
            MapDocumentSPtr document = lock(m_document);
            const Assets::EntityDefinition* entityDefinition = Model::AttributableNode::selectEntityDefinition(document->allSelectedAttributableNodes());

            m_documentationText->clear();

            QTextCharFormat boldFormat;
            boldFormat.setFontWeight(QFont::Bold);

            QTextCharFormat normalFormat;

            if (entityDefinition != nullptr) {
                // add attribute documentation, if available
                const Assets::AttributeDefinition* attributeDefinition = entityDefinition->attributeDefinition(attributeName);
                if (attributeDefinition != nullptr) {
                    const QString optionsDescription = optionDescriptions(*attributeDefinition);

                    const bool attributeHasDocs = !attributeDefinition->longDescription().empty()
                                               || !attributeDefinition->shortDescription().empty()
                                               || !optionsDescription.isEmpty();

                    if (attributeHasDocs) {
                        // e.g. "Attribute "delay" (Attenuation formula)", in bold
                        {
                            m_documentationText->setCurrentCharFormat(boldFormat);
                            m_documentationText->append("Attribute \"");
                            m_documentationText->append(attributeDefinition->name().c_str());
                            m_documentationText->append("\"");
                            if (!attributeDefinition->shortDescription().empty()) {
                                m_documentationText->append(" (");
                                m_documentationText->append(attributeDefinition->shortDescription().c_str());
                                m_documentationText->append(")");
                            }
                            m_documentationText->append("\n");
                            m_documentationText->setCurrentCharFormat(normalFormat);
                        }

                        if (!attributeDefinition->longDescription().empty()) {
                            m_documentationText->append("\n");
                            m_documentationText->append(attributeDefinition->longDescription().c_str());
                            m_documentationText->append("\n");
                        }

                        if (!optionsDescription.isEmpty()) {
                            m_documentationText->append("\nOptions:\n");
                            m_documentationText->append(optionsDescription); // ends with a newline
                        }
                    }
                }

                // add class description, if available
                if (!entityDefinition->description().empty()) {
                    // add space after attribute text
                    if (!m_documentationText->document()->isEmpty()) {
                        m_documentationText->append("\n");
                    }

                    // e.g. "Class "func_door"", in bold
                    {
                        m_documentationText->setCurrentCharFormat(boldFormat);
                        m_documentationText->append("Class \"");
                        m_documentationText->append(entityDefinition->name().c_str());
                        m_documentationText->append("\"\n");
                        m_documentationText->setCurrentCharFormat(normalFormat);
                    }

                    m_documentationText->append("\n");
                    m_documentationText->append(entityDefinition->description().c_str());
                    m_documentationText->append("\n");
                }
            }

            // Scroll to the top
            m_documentationText->moveCursor(QTextCursor::MoveOperation::Start);
        }

        void EntityAttributeEditor::createGui(QWidget* parent, MapDocumentWPtr document) {
            auto* splitter = new QSplitter(Qt::Vertical);

            // Configure the sash gravity so the first widget gets most of the space
            splitter->setSizes(QList<int>{1'000'000, 1});
            //splitter->SetName("EntityAttributeEditorSplitter");

            m_attributeGrid = new EntityAttributeGrid(nullptr, document);
            m_smartEditorManager = new SmartAttributeEditorManager(nullptr, document);
            m_documentationText = new QTextEdit();
            m_documentationText->setReadOnly(true);

            splitter->addWidget(m_attributeGrid);
            splitter->addWidget(m_smartEditorManager);
            splitter->addWidget(m_documentationText);

            m_attributeGrid->setMinimumSize(100, 50);
            m_smartEditorManager->setMinimumSize(100, 50);
            m_documentationText->setMinimumSize(100, 50);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(splitter, 1);
            setLayout(sizer);

            connect(m_attributeGrid, &EntityAttributeGrid::selectedRow, this, &EntityAttributeEditor::OnCurrentRowChanged);

            // FIXME:
            //wxPersistenceManager::Get().RegisterAndRestore(splitter);
        }
    }
}
