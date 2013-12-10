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

#include "SmartPropertyEditorManager.h"

#include "CollectionUtils.h"
#include "View/DefaultPropertyEditor.h"
#include "View/SmartPropertyEditor.h"

#include <wx/panel.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        SmartPropertyEditorManager::SmartPropertyEditorManager(wxWindow* parent, View::MapDocumentPtr document, View::ControllerPtr controller) :
        wxPanel(parent),
        m_defaultEditor(NULL),
        m_activeEditor(NULL) {
            createEditors(document, controller);
            switchEditor("");
        }
        
        SmartPropertyEditorManager::~SmartPropertyEditorManager() {
            destroyEditors();
        }

        void SmartPropertyEditorManager::switchEditor(const Model::PropertyKey& key) {
            SmartPropertyEditor* editor = selectEditor(key);
            activateEditor(editor, key);
        }

        void SmartPropertyEditorManager::createEditors(View::MapDocumentPtr document, View::ControllerPtr controller) {
            m_defaultEditor = new DefaultPropertyEditor(document, controller);
        }
        
        void SmartPropertyEditorManager::destroyEditors() {
            deactivateEditor();
            MapUtils::clearAndDelete(m_editors);
            delete m_defaultEditor;
            m_defaultEditor = NULL;
        }

        SmartPropertyEditor* SmartPropertyEditorManager::selectEditor(const Model::PropertyKey& key) const {
            EditorMap::const_iterator it = m_editors.find(key);
            if (it == m_editors.end())
                return m_defaultEditor;
            return it->second;
        }
    

        void SmartPropertyEditorManager::activateEditor(SmartPropertyEditor* editor, const Model::PropertyKey& key) {
            deactivateEditor();
            m_activeEditor = editor;
            wxWindow* window = m_activeEditor->activate(this, key);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(window, 1, wxEXPAND);
            SetSizer(sizer);
        }
        
        void SmartPropertyEditorManager::deactivateEditor() {
            if (m_activeEditor != NULL) {
                m_activeEditor->deactivate();
                m_activeEditor = NULL;
            }
        }
    }
}
