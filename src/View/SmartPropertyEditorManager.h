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

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <map>

#include <wx/panel.h>

class wxWindow;

namespace TrenchBroom {
    namespace View {
        class SmartPropertyEditor;
        
        class SmartPropertyEditorManager : public wxPanel {
        private:
            typedef std::map<Model::PropertyKey, SmartPropertyEditor*> EditorMap;
            
            EditorMap m_editors;
            SmartPropertyEditor* m_defaultEditor;
            SmartPropertyEditor* m_activeEditor;
        public:
            SmartPropertyEditorManager(wxWindow* parent, View::MapDocumentPtr document, View::ControllerPtr controller);
            ~SmartPropertyEditorManager();
            
            void switchEditor(const Model::PropertyKey& key);
        private:
            void createEditors(View::MapDocumentPtr document, View::ControllerPtr controller);
            void destroyEditors();
            
            SmartPropertyEditor* selectEditor(const Model::PropertyKey& key) const;
            
            void activateEditor(SmartPropertyEditor* editor, const Model::PropertyKey& key);
            void deactivateEditor();
        };
    }
}

#endif /* defined(__TrenchBroom__SmartPropertyEditorManager__) */
