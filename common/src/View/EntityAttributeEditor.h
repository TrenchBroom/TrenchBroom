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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <QWidget>

class QTextEdit;
class QSplitter;

namespace TrenchBroom {
    namespace Assets {
        class AttributeDefinition;
        class EntityDefinition;
    }

    namespace Model {
        class Node;
    }

    namespace View {
        class EntityAttributeGrid;
        class MapDocument;
        class Selection;
        class SmartAttributeEditorManager;

        /**
         * Panel containing the EntityAttributeGrid (the key/value editor table),
         * smart editor, and documentation text view.
         */
        class EntityAttributeEditor : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
            QSplitter* m_splitter;
            EntityAttributeGrid* m_attributeGrid;
            SmartAttributeEditorManager* m_smartEditorManager;
            QTextEdit* m_documentationText;
            const Assets::EntityDefinition* m_currentDefinition;
        public:
            explicit EntityAttributeEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            ~EntityAttributeEditor() override;
        private:
            void OnCurrentRowChanged();

            void bindObservers();
            void unbindObservers();

            void selectionDidChange(const Selection& selection);
            void nodesDidChange(const std::vector<Model::Node*>& nodes);

            void updateIfSelectedEntityDefinitionChanged();
            void updateDocumentationAndSmartEditor();

            /**
             * Returns a description of the options for ChoiceAttributeOption and FlagsAttributeDefinition,
             * other subclasses return an empty string.
             */
            static QString optionDescriptions(const Assets::AttributeDefinition& definition);

            void updateDocumentation(const std::string& attributeName);
            void createGui(std::weak_ptr<MapDocument> document);

            void updateMinimumSize();
        };
    }
}


