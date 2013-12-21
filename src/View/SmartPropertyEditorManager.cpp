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
#include "View/MapDocument.h"
#include "View/SmartChoiceEditor.h"
#include "View/SmartChoiceEditorMatcher.h"
#include "View/SmartColorEditor.h"
#include "View/SmartDefaultPropertyEditor.h"
#include "View/SmartPropertyEditor.h"
#include "View/SmartPropertyEditorMatcher.h"
#include "View/SmartSpawnFlagsEditor.h"

#include <wx/panel.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        SmartPropertyEditorManager::SmartPropertyEditorManager(wxWindow* parent, View::MapDocumentWPtr document, View::ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_key("") {
            createEditors(m_document, m_controller);
            activateEditor(defaultEditor(), "");
            bindObservers();
        }
        
        SmartPropertyEditorManager::~SmartPropertyEditorManager() {
            unbindObservers();
            deactivateEditor();
        }

        void SmartPropertyEditorManager::switchEditor(const Model::PropertyKey& key, const Model::EntityList& entities) {
            EditorPtr editor = selectEditor(key, entities);
            activateEditor(editor, key);
            updateEditor();
        }

        void SmartPropertyEditorManager::createEditors(View::MapDocumentWPtr document, View::ControllerWPtr controller) {
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartPropertyEditorKeyMatcher("spawnflags")),
                                                  EditorPtr(new SmartSpawnflagsEditor(document, controller))));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartPropertyEditorKeyMatcher("_color", "_sunlight_color", "_sunlight_color2")),
                                                  EditorPtr(new SmartColorEditor(document, controller))));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartChoiceEditorMatcher()),
                                                  EditorPtr(new SmartChoiceEditor(document, controller))));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartPropertyEditorDefaultMatcher()),
                                                  EditorPtr(new SmartDefaultPropertyEditor(document, controller))));
        }
        
        void SmartPropertyEditorManager::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &SmartPropertyEditorManager::selectionDidChange);
            document->objectDidChangeNotifier.addObserver(this, &SmartPropertyEditorManager::objectDidChange);
        }
        
        void SmartPropertyEditorManager::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &SmartPropertyEditorManager::selectionDidChange);
                document->objectDidChangeNotifier.removeObserver(this, &SmartPropertyEditorManager::objectDidChange);
            }
        }

        void SmartPropertyEditorManager::selectionDidChange(const Model::SelectionResult& result) {
            MapDocumentSPtr document = lock(m_document);
            switchEditor(m_key, document->allSelectedEntities());
        }
        
        void SmartPropertyEditorManager::objectDidChange(Model::Object* object) {
            MapDocumentSPtr document = lock(m_document);
            switchEditor(m_key, document->allSelectedEntities());
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
        
        void SmartPropertyEditorManager::activateEditor(EditorPtr editor, const Model::PropertyKey& key) {
            if (m_activeEditor != editor || !m_activeEditor->usesKey(key)) {
                deactivateEditor();
                m_activeEditor = editor;
                m_key = key;
                wxWindow* window = m_activeEditor->activate(this, key);
                
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
                m_key = "";
            }
        }

        void SmartPropertyEditorManager::updateEditor() {
            if (m_activeEditor != NULL) {
                MapDocumentSPtr document = lock(m_document);
                m_activeEditor->update(document->allSelectedEntities());
            }
        }
    }
}
