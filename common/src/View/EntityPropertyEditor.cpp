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

#include "EntityPropertyEditor.h"

#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "Model/EntityNodeBase.h"
#include "View/EntityPropertyGrid.h"
#include "View/MapDocument.h"
#include "View/SmartPropertyEditorManager.h"
#include "View/Splitter.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <algorithm>

#include <QVBoxLayout>
#include <QChar>
#include <QStringBuilder>
#include <QTextStream>
#include <QTextEdit>

namespace TrenchBroom {
    namespace View {
        EntityPropertyEditor::EntityPropertyEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(document),
        m_splitter(nullptr),
        m_propertyGrid(nullptr),
        m_smartEditorManager(nullptr),
        m_documentationText(nullptr),
        m_currentDefinition(nullptr) {
        createGui(document);
        connectObservers();
        }

        EntityPropertyEditor::~EntityPropertyEditor() {
            saveWindowState(m_splitter);
        }

        void EntityPropertyEditor::OnCurrentRowChanged() {
            updateDocumentationAndSmartEditor();
        }

        void EntityPropertyEditor::connectObservers() {
            auto document = kdl::mem_lock(m_document);
            m_notifierConnection += document->selectionDidChangeNotifier.connect(this, &EntityPropertyEditor::selectionDidChange);
            m_notifierConnection += document->nodesDidChangeNotifier.connect(this, &EntityPropertyEditor::nodesDidChange);
        }

        void EntityPropertyEditor::selectionDidChange(const Selection&) {
            updateIfSelectedEntityDefinitionChanged();
        }

        void EntityPropertyEditor::nodesDidChange(const std::vector<Model::Node*>&) {
            updateIfSelectedEntityDefinitionChanged();
        }

        void EntityPropertyEditor::updateIfSelectedEntityDefinitionChanged() {
            auto document = kdl::mem_lock(m_document);
            const Assets::EntityDefinition* entityDefinition = Model::selectEntityDefinition(
                document->allSelectedEntityNodes());

            if (entityDefinition != m_currentDefinition) {
                m_currentDefinition = entityDefinition;
                updateDocumentationAndSmartEditor();
            }
        }

        void EntityPropertyEditor::updateDocumentationAndSmartEditor() {
            auto document = kdl::mem_lock(m_document);
            const auto& propertyKey = m_propertyGrid->selectedRowName();

            m_smartEditorManager->switchEditor(propertyKey, document->allSelectedEntityNodes());

            updateDocumentation(propertyKey);

            // collapse the splitter if needed
            m_documentationText->setHidden(m_documentationText->document()->isEmpty());
            m_smartEditorManager->setHidden(m_smartEditorManager->isDefaultEditorActive());

            updateMinimumSize();
        }

        QString EntityPropertyEditor::optionDescriptions(const Assets::PropertyDefinition& definition) {
            const QString bullet = QString(" ") % QChar(0x2022) % QString(" ");

            switch (definition.type()) {
                case Assets::PropertyDefinitionType::ChoiceProperty: {
                    const auto& choiceDef = dynamic_cast<const Assets::ChoicePropertyDefinition&>(definition);

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
                case Assets::PropertyDefinitionType::FlagsProperty: {
                    const auto& flagsDef = dynamic_cast<const Assets::FlagsPropertyDefinition&>(definition);

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
                    for (const auto& [value, description] : flagDescriptors) {
                        stream << description << "\n";
                    }
                    return result;
                }
                case Assets::PropertyDefinitionType::StringProperty:
                case Assets::PropertyDefinitionType::BooleanProperty:
                case Assets::PropertyDefinitionType::IntegerProperty:
                case Assets::PropertyDefinitionType::FloatProperty:
                case Assets::PropertyDefinitionType::TargetSourceProperty:
                case Assets::PropertyDefinitionType::TargetDestinationProperty:
                    return QString();
                switchDefault()
            }
        }

        void EntityPropertyEditor::updateDocumentation(const std::string& propertyKey) {
            auto document = kdl::mem_lock(m_document);
            const Assets::EntityDefinition* entityDefinition = Model::selectEntityDefinition(
                document->allSelectedEntityNodes());

            m_documentationText->clear();

            QTextCharFormat boldFormat;
            boldFormat.setFontWeight(QFont::Bold);

            QTextCharFormat normalFormat;

            if (entityDefinition != nullptr) {
                // add property documentation, if available
                const Assets::PropertyDefinition* propertyDefinition = entityDefinition->propertyDefinition(
                    propertyKey);
                if (propertyDefinition != nullptr) {
                    const QString optionsDescription = optionDescriptions(*propertyDefinition);

                    const bool propertyHasDocs = !propertyDefinition->longDescription().empty()
                                                 || !propertyDefinition->shortDescription().empty()
                                                 || !optionsDescription.isEmpty();

                    if (propertyHasDocs) {
                        // e.g. "Property "delay" (Attenuation formula)", in bold
                        {
                            QString title = tr("Property \"%1\"")
                                .arg(QString::fromStdString(propertyDefinition->key()));
                            if (!propertyDefinition->shortDescription().empty()) {
                                title += tr(" (%1)").arg(QString::fromStdString(propertyDefinition->shortDescription()));
                            }

                            m_documentationText->setCurrentCharFormat(boldFormat);
                            m_documentationText->append(title);
                            m_documentationText->setCurrentCharFormat(normalFormat);
                        }

                        if (!propertyDefinition->longDescription().empty()) {
                            m_documentationText->append("");
                            m_documentationText->append(propertyDefinition->longDescription().c_str());
                        }

                        if (!optionsDescription.isEmpty()) {
                            m_documentationText->append("");
                            m_documentationText->append("Options:");
                            m_documentationText->append(optionsDescription);
                        }
                    }
                }

                // add class description, if available
                if (!entityDefinition->description().empty()) {
                    // add space after property text
                    if (!m_documentationText->document()->isEmpty()) {
                        m_documentationText->append("");
                    }

                    // e.g. "Class "func_door"", in bold
                    {
                        m_documentationText->setCurrentCharFormat(boldFormat);
                        m_documentationText->append(tr("Class \"%1\"")
                            .arg(QString::fromStdString(entityDefinition->name())));
                        m_documentationText->setCurrentCharFormat(normalFormat);
                    }

                    m_documentationText->append("");
                    m_documentationText->append(entityDefinition->description().c_str());
                    m_documentationText->append("");
                }
            }

            // Scroll to the top
            m_documentationText->moveCursor(QTextCursor::MoveOperation::Start);
        }

        void EntityPropertyEditor::createGui(std::weak_ptr<MapDocument> document) {
            m_splitter = new Splitter(Qt::Vertical);

            // This class has since been renamed, but we leave the old name so as not to reset the users' view settings.
            m_splitter->setObjectName("EntityAttributeEditor_Splitter");

            m_propertyGrid = new EntityPropertyGrid(document);
            m_smartEditorManager = new SmartPropertyEditorManager(document);
            m_documentationText = new QTextEdit();
            m_documentationText->setReadOnly(true);

            m_splitter->addWidget(m_propertyGrid);
            m_splitter->addWidget(m_smartEditorManager);
            m_splitter->addWidget(m_documentationText);

            // give most space to the property grid
            m_splitter->setSizes(QList<int>{1'000'000, 1, 1});

            // NOTE: this should be done before setChildrenCollapsible() and setMinimumSize()
            // otherwise it can override them.
            restoreWindowState(m_splitter);

            m_propertyGrid->setMinimumSize(100, 100); // should have enough vertical space for at least one row
            m_documentationText->setMinimumSize(100, 50);
            updateMinimumSize();

            // don't allow the user to collapse the panels, it's hard to see them
            m_splitter->setChildrenCollapsible(false);

            // resize only the property grid when the container resizes
            m_splitter->setStretchFactor(0, 1);
            m_splitter->setStretchFactor(1, 0);
            m_splitter->setStretchFactor(2, 0);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(m_splitter, 1);
            setLayout(layout);

            connect(m_propertyGrid, &EntityPropertyGrid::currentRowChanged, this, &EntityPropertyEditor::OnCurrentRowChanged);
        }

        void EntityPropertyEditor::updateMinimumSize() {
            QSize size;
            size.setWidth(m_propertyGrid->minimumWidth());
            size.setHeight(m_propertyGrid->minimumHeight());

            size.setWidth(std::max(size.width(), m_smartEditorManager->minimumSizeHint().width()));
            size.setHeight(size.height() + m_smartEditorManager->minimumSizeHint().height());

            size.setWidth(std::max(size.width(), m_documentationText->minimumSizeHint().width()));
            size.setHeight(size.height() + m_documentationText->minimumSizeHint().height());

            setMinimumSize(size);
            updateGeometry();
        }
    }
}
