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

#include "SmartAttributeEditorManager.h"

#include "CollectionUtils.h"
#include "View/MapDocument.h"
#include "View/SmartChoiceEditor.h"
#include "View/SmartChoiceEditorMatcher.h"
#include "View/SmartColorEditor.h"
#include "View/SmartDefaultAttributeEditor.h"
#include "View/SmartAttributeEditor.h"
#include "View/SmartAttributeEditorMatcher.h"
#include "View/SmartSpawnflagsEditor.h"

#include <QWidget>
#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        SmartAttributeEditorManager::SmartAttributeEditorManager(QWidget* parent, View::MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document),
        m_name(""),
        m_activeEditor(nullptr) {
            createEditors();
            activateEditor(defaultEditor(), "");
            bindObservers();
        }
        
        SmartAttributeEditorManager::~SmartAttributeEditorManager() {
            unbindObservers();
            deactivateEditor();
        }

        void SmartAttributeEditorManager::switchEditor(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) {
            EditorPtr editor = selectEditor(name, attributables);
            activateEditor(editor, name);
            updateEditor();
        }

        void SmartAttributeEditorManager::createEditors() {
            // NOTE: the SmartAttributeEditor subclasses are created here with their parent set to `this`
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartAttributeEditorKeyMatcher("spawnflags")),
                                                  new SmartSpawnflagsEditor(this, m_document)));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartAttributeEditorKeyMatcher({ "*_color", "*_color2", "*_colour" })),
                                                  new SmartColorEditor(this, m_document)));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartChoiceEditorMatcher()),
                                                  new SmartChoiceEditor(this, m_document)));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartAttributeEditorDefaultMatcher()),
                                                  new SmartDefaultAttributeEditor(this, m_document)));
        }
        
        void SmartAttributeEditorManager::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &SmartAttributeEditorManager::selectionDidChange);
            document->nodesDidChangeNotifier.addObserver(this, &SmartAttributeEditorManager::nodesDidChange);
        }
        
        void SmartAttributeEditorManager::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &SmartAttributeEditorManager::selectionDidChange);
                document->nodesDidChangeNotifier.removeObserver(this, &SmartAttributeEditorManager::nodesDidChange);
            }
        }

        void SmartAttributeEditorManager::selectionDidChange(const Selection& selection) {
            MapDocumentSPtr document = lock(m_document);
            switchEditor(m_name, document->allSelectedAttributableNodes());
        }
        
        void SmartAttributeEditorManager::nodesDidChange(const Model::NodeList& nodes) {
            MapDocumentSPtr document = lock(m_document);
            switchEditor(m_name, document->allSelectedAttributableNodes());
        }
        
        SmartAttributeEditorManager::EditorPtr SmartAttributeEditorManager::selectEditor(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const {
            for (const auto& entry : m_editors) {
                const MatcherPtr matcher = entry.first;
                if (matcher->matches(name, attributables))
                    return entry.second;
            }
            
            // should never happen
            assert(false);
            return defaultEditor();
        }
    

        SmartAttributeEditorManager::EditorPtr SmartAttributeEditorManager::defaultEditor() const {
            return m_editors.back().second;
        }
        
        void SmartAttributeEditorManager::activateEditor(EditorPtr editor, const Model::AttributeName& name) {
            if (m_activeEditor != editor || !m_activeEditor->usesName(name)) {
                deactivateEditor();
                m_activeEditor = editor;
                m_name = name;
                QWidget* window = m_activeEditor->activate(this, m_name);
                
                auto* sizer = new QVBoxLayout();
                sizer->addWidget(window, 1);
                setLayout(sizer);
            }
        }
        
        void SmartAttributeEditorManager::deactivateEditor() {
            if (m_activeEditor != nullptr) {
                m_activeEditor->deactivate();
                m_activeEditor = nullptr;
                m_name = "";
            }
        }

        void SmartAttributeEditorManager::updateEditor() {
            if (m_activeEditor != nullptr) {
                MapDocumentSPtr document = lock(m_document);
                m_activeEditor->update(document->allSelectedAttributableNodes());
            }
        }
    }
}
