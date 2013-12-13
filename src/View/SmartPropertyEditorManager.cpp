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
#include "View/SmartPropertyEditorMatcher.h"
#include "View/SmartSpawnFlagsEditor.h"

#include <wx/panel.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        SmartPropertyEditorManager::SmartPropertyEditorManager(wxWindow* parent, View::MapDocumentPtr document, View::ControllerPtr controller) :
        wxPanel(parent) {
            createEditors(document, controller);
            activateEditor(defaultEditor(), "", Model::EmptyEntityList);
        }
        
        SmartPropertyEditorManager::~SmartPropertyEditorManager() {
            deactivateEditor();
        }

        void SmartPropertyEditorManager::switchEditor(const Model::PropertyKey& key, const Model::EntityList& entities) {
            EditorPtr editor = selectEditor(key, entities);
            activateEditor(editor, key, entities);
        }

        void SmartPropertyEditorManager::createEditors(View::MapDocumentPtr document, View::ControllerPtr controller) {
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartPropertyEditorKeyMatcher("spawnflags")),
                                                  EditorPtr(new SmartSpawnflagsEditor(document, controller))));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartPropertyEditorDefaultMatcher()),
                                                  EditorPtr(new DefaultPropertyEditor(document, controller))));
        }
        
        SmartPropertyEditorManager::EditorPtr SmartPropertyEditorManager::selectEditor(const Model::PropertyKey& key, const Model::EntityList& entities) const {
            EditorList::const_iterator it, end;
            for (it = m_editors.begin(), end = m_editors.end(); it != end; ++it) {
                const MatcherEditorPair& pair = *it;
                const MatcherPtr matcher = pair.first;
                if (matcher->matches(key, entities))
                    return pair.second;
            }
            
            // should never happen
            assert(false);
            return defaultEditor();
        }
    

        SmartPropertyEditorManager::EditorPtr SmartPropertyEditorManager::defaultEditor() const {
            return m_editors.back().second;
        }
        
        void SmartPropertyEditorManager::activateEditor(EditorPtr editor, const Model::PropertyKey& key, const Model::EntityList& entities) {
            if (m_activeEditor != editor || !m_activeEditor->usesKey(key)) {
                deactivateEditor();
                m_activeEditor = editor;
                wxWindow* window = m_activeEditor->activate(this, key, entities);
                
                wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
                sizer->Add(window, 1, wxEXPAND);
                SetSizer(sizer);
                Layout();
            }
        }
        
        void SmartPropertyEditorManager::deactivateEditor() {
            if (m_activeEditor != NULL) {
                m_activeEditor->deactivate();
                m_activeEditor = EditorPtr();
            }
        }
    }
}
