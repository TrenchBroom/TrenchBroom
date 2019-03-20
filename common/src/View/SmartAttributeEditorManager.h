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

#ifndef TrenchBroom_SmartAttributeEditorManager
#define TrenchBroom_SmartAttributeEditorManager

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <vector>

#include <QWidget>

class QStackedLayout;

namespace TrenchBroom {
    namespace View {
        class Selection;
        class SmartAttributeEditor;
        class SmartAttributeEditorMatcher;

        class SmartAttributeEditorManager : public QWidget {
        private:
            using EditorPtr = SmartAttributeEditor*;
            using MatcherPtr = std::shared_ptr<SmartAttributeEditorMatcher>;
            using MatcherEditorPair = std::pair<MatcherPtr, EditorPtr>;
            using EditorList = std::vector<MatcherEditorPair>;

            View::MapDocumentWPtr m_document;

            EditorList m_editors;
            Model::AttributeName m_name;
            QStackedLayout* m_stackedLayout;
        public:
            SmartAttributeEditorManager(QWidget* parent, View::MapDocumentWPtr document);
            ~SmartAttributeEditorManager();

            void switchEditor(const Model::AttributeName& name, const Model::AttributableNodeList& attributables);
            bool isDefaultEditorActive() const;
        private:
            SmartAttributeEditor* activeEditor() const;
            void createEditors();

            void bindObservers();
            void unbindObservers();

            void selectionDidChange(const Selection& selection);
            void nodesDidChange(const Model::NodeList& nodes);

            EditorPtr selectEditor(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const;
            EditorPtr defaultEditor() const;

            void activateEditor(EditorPtr editor, const Model::AttributeName& name);
            void deactivateEditor();
            void updateEditor();
        };
    }
}

#endif /* defined(TrenchBroom_SmartAttributeEditorManager) */
