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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_SmartAttributeEditorManager
#define TrenchBroom_SmartAttributeEditorManager

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <vector>

#include <wx/panel.h>

class wxWindow;

namespace TrenchBroom {
    namespace View {
        class Selection;
        class SmartAttributeEditor;
        class SmartAttributeEditorMatcher;
        
        class SmartAttributeEditorManager : public wxPanel {
        private:
            typedef std::tr1::shared_ptr<SmartAttributeEditor> EditorPtr;
            typedef std::tr1::shared_ptr<SmartAttributeEditorMatcher> MatcherPtr;
            typedef std::pair<MatcherPtr, EditorPtr> MatcherEditorPair;
            typedef std::vector<MatcherEditorPair> EditorList;
            
            View::MapDocumentWPtr m_document;
            
            EditorList m_editors;
            Model::AttributeName m_name;
            EditorPtr m_activeEditor;
        public:
            SmartAttributeEditorManager(wxWindow* parent, View::MapDocumentWPtr document);
            ~SmartAttributeEditorManager();
            
            void switchEditor(const Model::AttributeName& name, const Model::AttributableNodeList& attributables);
        private:
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
