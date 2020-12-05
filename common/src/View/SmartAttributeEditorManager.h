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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <QWidget>

class QStackedLayout;

namespace TrenchBroom {
    namespace Model {
        class AttributableNode;
        class Node;
    }

    namespace View {
        class MapDocument;
        class Selection;
        class SmartAttributeEditor;
        class SmartAttributeEditorMatcher;

        class SmartAttributeEditorManager : public QWidget {
        private:
            using EditorPtr = SmartAttributeEditor*;
            using MatcherPtr = std::shared_ptr<SmartAttributeEditorMatcher>;
            using MatcherEditorPair = std::pair<MatcherPtr, EditorPtr>;
            using EditorList = std::vector<MatcherEditorPair>;

            std::weak_ptr<MapDocument> m_document;

            EditorList m_editors;
            std::string m_name;
            QStackedLayout* m_stackedLayout;
        public:
            explicit SmartAttributeEditorManager(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            ~SmartAttributeEditorManager();

            void switchEditor(const std::string& name, const std::vector<Model::AttributableNode*>& attributables);
            bool isDefaultEditorActive() const;
        private:
            SmartAttributeEditor* activeEditor() const;
            void createEditors();

            void bindObservers();
            void unbindObservers();

            void selectionDidChange(const Selection& selection);
            void nodesDidChange(const std::vector<Model::Node*>& nodes);

            EditorPtr selectEditor(const std::string& name, const std::vector<Model::AttributableNode*>& attributables) const;
            EditorPtr defaultEditor() const;

            void activateEditor(EditorPtr editor, const std::string& name);
            void deactivateEditor();
            void updateEditor();
        };
    }
}

