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

#include "Macros.h"
#include "Assets/AttributeDefinition.h"
#include "View/MapDocument.h"
#include "View/SmartChoiceEditor.h"
#include "View/SmartTypeEditorMatcher.h"
#include "View/SmartColorEditor.h"
#include "View/SmartDefaultAttributeEditor.h"
#include "View/SmartAttributeEditor.h"
#include "View/SmartAttributeEditorMatcher.h"
#include "View/SmartFlagsEditor.h"

#include <kdl/memory_utils.h>

#include <QWidget>
#include <QStackedLayout>

namespace TrenchBroom {
    namespace View {
        SmartAttributeEditorManager::SmartAttributeEditorManager(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(document),
        m_name(""),
        m_stackedLayout(nullptr) {
            createEditors();
            activateEditor(defaultEditor(), "");
            bindObservers();
        }

        SmartAttributeEditorManager::~SmartAttributeEditorManager() {
            unbindObservers();
        }

        void SmartAttributeEditorManager::switchEditor(const std::string& name, const std::vector<Model::AttributableNode*>& attributables) {
            EditorPtr editor = selectEditor(name, attributables);
            activateEditor(editor, name);
            updateEditor();
        }

        SmartAttributeEditor* SmartAttributeEditorManager::activeEditor() const {
            return static_cast<SmartAttributeEditor *>(m_stackedLayout->currentWidget());
        }

        bool SmartAttributeEditorManager::isDefaultEditorActive() const {
            return activeEditor() == defaultEditor();
        }

        void SmartAttributeEditorManager::createEditors() {
            assert(m_editors.empty());

            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartTypeEditorMatcher(Assets::AttributeDefinitionType::FlagsAttribute)),
                                                  new SmartFlagsEditor(m_document)));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartAttributeEditorKeyMatcher({ "*_color", "*_color2", "*_colour" })),
                                                  new SmartColorEditor(m_document)));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartTypeWithSameDefinitionEditorMatcher(Assets::AttributeDefinitionType::ChoiceAttribute)),
                                                  new SmartChoiceEditor(m_document)));
            m_editors.push_back(MatcherEditorPair(MatcherPtr(new SmartAttributeEditorDefaultMatcher()),
                                                  new SmartDefaultAttributeEditor(m_document)));

            m_stackedLayout = new QStackedLayout();
            for (auto& [matcherPtr, editor] : m_editors) {
                unused(matcherPtr);
                m_stackedLayout->addWidget(editor);
            }
            setLayout(m_stackedLayout);
        }

        void SmartAttributeEditorManager::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &SmartAttributeEditorManager::selectionDidChange);
            document->nodesDidChangeNotifier.addObserver(this, &SmartAttributeEditorManager::nodesDidChange);
        }

        void SmartAttributeEditorManager::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &SmartAttributeEditorManager::selectionDidChange);
                document->nodesDidChangeNotifier.removeObserver(this, &SmartAttributeEditorManager::nodesDidChange);
            }
        }

        void SmartAttributeEditorManager::selectionDidChange(const Selection&) {
            auto document = kdl::mem_lock(m_document);
            switchEditor(m_name, document->allSelectedAttributableNodes());
        }

        void SmartAttributeEditorManager::nodesDidChange(const std::vector<Model::Node*>&) {
            auto document = kdl::mem_lock(m_document);
            switchEditor(m_name, document->allSelectedAttributableNodes());
        }

        SmartAttributeEditorManager::EditorPtr SmartAttributeEditorManager::selectEditor(const std::string& name, const std::vector<Model::AttributableNode*>& attributables) const {
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

        void SmartAttributeEditorManager::activateEditor(EditorPtr editor, const std::string& name) {
            if (m_stackedLayout->currentWidget() != editor || !activeEditor()->usesName(name)) {
                deactivateEditor();

                m_name = name;
                m_stackedLayout->setCurrentWidget(editor);
                editor->activate(m_name);
            }
        }

        void SmartAttributeEditorManager::deactivateEditor() {
            if (activeEditor() != nullptr) {
                activeEditor()->deactivate();
                m_stackedLayout->setCurrentIndex(-1);
                m_name = "";
            }
        }

        void SmartAttributeEditorManager::updateEditor() {
            if (activeEditor() != nullptr) {
                auto document = kdl::mem_lock(m_document);
                activeEditor()->update(document->allSelectedAttributableNodes());
            }
        }
    }
}
