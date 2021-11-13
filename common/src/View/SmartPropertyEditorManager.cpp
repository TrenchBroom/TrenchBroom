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

#include "SmartPropertyEditorManager.h"

#include "Assets/PropertyDefinition.h"
#include "Macros.h"
#include "View/MapDocument.h"
#include "View/SmartChoiceEditor.h"
#include "View/SmartColorEditor.h"
#include "View/SmartDefaultPropertyEditor.h"
#include "View/SmartFlagsEditor.h"
#include "View/SmartPropertyEditor.h"
#include "View/SmartPropertyEditorMatcher.h"
#include "View/SmartTypeEditorMatcher.h"

#include <kdl/memory_utils.h>

#include <QStackedLayout>
#include <QWidget>

namespace TrenchBroom {
namespace View {
SmartPropertyEditorManager::SmartPropertyEditorManager(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget(parent)
  , m_document(document)
  , m_propertyKey("")
  , m_stackedLayout(nullptr) {
  createEditors();
  activateEditor(defaultEditor(), "");
  connectObservers();
}

void SmartPropertyEditorManager::switchEditor(
  const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes) {
  EditorPtr editor = selectEditor(propertyKey, nodes);
  activateEditor(editor, propertyKey);
  updateEditor();
}

SmartPropertyEditor* SmartPropertyEditorManager::activeEditor() const {
  return static_cast<SmartPropertyEditor*>(m_stackedLayout->currentWidget());
}

bool SmartPropertyEditorManager::isDefaultEditorActive() const {
  return activeEditor() == defaultEditor();
}

void SmartPropertyEditorManager::createEditors() {
  assert(m_editors.empty());

  m_editors.push_back(MatcherEditorPair(
    MatcherPtr(new SmartTypeEditorMatcher(Assets::PropertyDefinitionType::FlagsProperty)),
    new SmartFlagsEditor(m_document)));
  m_editors.push_back(MatcherEditorPair(
    MatcherPtr(new SmartPropertyEditorKeyMatcher({"*_color", "*_color2", "*_colour"})),
    new SmartColorEditor(m_document)));
  m_editors.push_back(MatcherEditorPair(
    MatcherPtr(
      new SmartTypeWithSameDefinitionEditorMatcher(Assets::PropertyDefinitionType::ChoiceProperty)),
    new SmartChoiceEditor(m_document)));
  m_editors.push_back(MatcherEditorPair(
    MatcherPtr(new SmartPropertyEditorDefaultMatcher()),
    new SmartDefaultPropertyEditor(m_document)));

  m_stackedLayout = new QStackedLayout();
  for (auto& [matcherPtr, editor] : m_editors) {
    unused(matcherPtr);
    m_stackedLayout->addWidget(editor);
  }
  setLayout(m_stackedLayout);
}

void SmartPropertyEditorManager::connectObservers() {
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->selectionDidChangeNotifier.connect(
    this, &SmartPropertyEditorManager::selectionDidChange);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &SmartPropertyEditorManager::nodesDidChange);
}

void SmartPropertyEditorManager::selectionDidChange(const Selection&) {
  auto document = kdl::mem_lock(m_document);
  switchEditor(m_propertyKey, document->allSelectedEntityNodes());
}

void SmartPropertyEditorManager::nodesDidChange(const std::vector<Model::Node*>&) {
  auto document = kdl::mem_lock(m_document);
  switchEditor(m_propertyKey, document->allSelectedEntityNodes());
}

SmartPropertyEditorManager::EditorPtr SmartPropertyEditorManager::selectEditor(
  const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes) const {
  for (const auto& [matcher, editor] : m_editors) {
    if (matcher->matches(propertyKey, nodes))
      return editor;
  }

  // should never happen
  assert(false);
  return defaultEditor();
}

SmartPropertyEditorManager::EditorPtr SmartPropertyEditorManager::defaultEditor() const {
  return m_editors.back().second;
}

void SmartPropertyEditorManager::activateEditor(EditorPtr editor, const std::string& propertyKey) {
  if (m_stackedLayout->currentWidget() != editor || !activeEditor()->usesPropertyKey(propertyKey)) {
    deactivateEditor();

    m_propertyKey = propertyKey;
    m_stackedLayout->setCurrentWidget(editor);
    editor->activate(m_propertyKey);
  }
}

void SmartPropertyEditorManager::deactivateEditor() {
  if (activeEditor() != nullptr) {
    activeEditor()->deactivate();
    m_stackedLayout->setCurrentIndex(-1);
    m_propertyKey = "";
  }
}

void SmartPropertyEditorManager::updateEditor() {
  if (activeEditor() != nullptr) {
    auto document = kdl::mem_lock(m_document);
    activeEditor()->update(document->allSelectedEntityNodes());
  }
}
} // namespace View
} // namespace TrenchBroom
