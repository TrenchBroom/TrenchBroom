/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__SmartPropertyEditorManager__
#define __TrenchBroom__SmartPropertyEditorManager__

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <vector>

#include <wx/panel.h>

class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class SelectionResult;
    }
    
    namespace View {
        class SmartPropertyEditor;
        class SmartPropertyEditorMatcher;
        
        class SmartPropertyEditorManager : public wxPanel {
        private:
            typedef std::tr1::shared_ptr<SmartPropertyEditor> EditorPtr;
            typedef std::tr1::shared_ptr<SmartPropertyEditorMatcher> MatcherPtr;
            typedef std::pair<MatcherPtr, EditorPtr> MatcherEditorPair;
            typedef std::vector<MatcherEditorPair> EditorList;
            
            View::MapDocumentPtr m_document;
            View::ControllerPtr m_controller;
            
            EditorList m_editors;
            Model::PropertyKey m_key;
            EditorPtr m_activeEditor;
        public:
            SmartPropertyEditorManager(wxWindow* parent, View::MapDocumentPtr document, View::ControllerPtr controller);
            ~SmartPropertyEditorManager();
            
            void switchEditor(const Model::PropertyKey& key, const Model::EntityList& entities);
        private:
            void createEditors(View::MapDocumentPtr document, View::ControllerPtr controller);

            void bindObservers();
            void unbindObservers();
            
            void selectionDidChange(const Model::SelectionResult& result);
            void objectDidChange(Model::Object* object);

            EditorPtr selectEditor(const Model::PropertyKey& key, const Model::EntityList& entities) const;
            EditorPtr defaultEditor() const;
            
            void activateEditor(EditorPtr editor, const Model::PropertyKey& key);
            void deactivateEditor();
            void updateEditor();
        };
    }
}

#endif /* defined(__TrenchBroom__SmartPropertyEditorManager__) */
