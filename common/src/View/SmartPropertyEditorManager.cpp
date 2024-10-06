/*
 Copyright (C) 2010 Kristian Duske

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

#include <QStackedLayout>
#include <QWidget>

#include "Macros.h"
#include "View/MapDocument.h"
#include "View/SmartChoiceEditor.h"
#include "View/SmartColorEditor.h"
#include "View/SmartDefaultPropertyEditor.h"
#include "View/SmartFlagsEditor.h"
#include "View/SmartPropertyEditor.h"
#include "View/SmartWadEditor.h"
#include "asset/PropertyDefinition.h"
#include "mdl/EntityNodeBase.h"

#include "kdl/memory_utils.h"
#include "kdl/string_compare.h"

namespace tb::View
{
namespace
{

/**
 * Matches if all of the nodes have a property definition for the give property key that
 * is of the type passed to the constructor.
 */
SmartPropertyEditorMatcher makeSmartTypeEditorMatcher(
  const asset::PropertyDefinitionType type)
{
  return [=](const auto& propertyKey, const auto& nodes) {
    return !nodes.empty() && std::ranges::all_of(nodes, [&](const auto* node) {
      const auto* propDef = mdl::propertyDefinition(node, propertyKey);
      return propDef && propDef->type() == type;
    });
  };
}

/**
 * Matches if all of the nodes have a property definition for the give property key that
 * is of the type passed to the constructor, and these property definitions are all equal.
 */
SmartPropertyEditorMatcher makeSmartTypeWithSameDefinitionEditorMatcher(
  const asset::PropertyDefinitionType type)
{
  return [=](const auto& propertyKey, const auto& nodes) {
    const auto* propDef = mdl::selectPropertyDefinition(propertyKey, nodes);
    return propDef && propDef->type() == type;
  };
}

SmartPropertyEditorMatcher makeSmartPropertyEditorKeyMatcher(
  std::vector<std::string> patterns_)
{
  return [patterns = std::move(patterns_)](const auto& propertyKey, const auto& nodes) {
    return !nodes.empty() && std::ranges::any_of(patterns, [&](const auto& pattern) {
      return kdl::cs::str_matches_glob(propertyKey, pattern);
    });
  };
}

} // namespace

SmartPropertyEditorManager::SmartPropertyEditorManager(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createEditors();
  activateEditor(defaultEditor(), "");
  connectObservers();
}

void SmartPropertyEditorManager::switchEditor(
  const std::string& propertyKey, const std::vector<mdl::EntityNodeBase*>& nodes)
{
  auto* editor = selectEditor(propertyKey, nodes);
  activateEditor(editor, propertyKey);
  updateEditor();
}

SmartPropertyEditor* SmartPropertyEditorManager::activeEditor() const
{
  return static_cast<SmartPropertyEditor*>(m_stackedLayout->currentWidget());
}

bool SmartPropertyEditorManager::isDefaultEditorActive() const
{
  return activeEditor() == defaultEditor();
}

void SmartPropertyEditorManager::createEditors()
{
  assert(m_editors.empty());

  m_editors.emplace_back(
    makeSmartTypeEditorMatcher(asset::PropertyDefinitionType::FlagsProperty),
    new SmartFlagsEditor{m_document});
  m_editors.emplace_back(
    makeSmartPropertyEditorKeyMatcher({"color", "*_color", "*_color2", "*_colour"}),
    new SmartColorEditor{m_document});
  m_editors.emplace_back(
    makeSmartTypeWithSameDefinitionEditorMatcher(
      asset::PropertyDefinitionType::ChoiceProperty),
    new SmartChoiceEditor{m_document});
  m_editors.emplace_back(
    [&](const auto& propertyKey, const auto& nodes) {
      return propertyKey
               == kdl::mem_lock(m_document)->game()->config().materialConfig.property
             && nodes.size() == 1
             && nodes.front()->entity().classname()
                  == mdl::EntityPropertyValues::WorldspawnClassname;
    },
    new SmartWadEditor{m_document});
  m_editors.emplace_back(
    [](const auto&, const auto&) { return true; },
    new SmartDefaultPropertyEditor{m_document});

  m_stackedLayout = new QStackedLayout{};
  for (auto& [matcher, editor] : m_editors)
  {
    unused(matcher);
    m_stackedLayout->addWidget(editor);
  }
  setLayout(m_stackedLayout);
}

void SmartPropertyEditorManager::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->selectionDidChangeNotifier.connect(
    this, &SmartPropertyEditorManager::selectionDidChange);
  m_notifierConnection += document->nodesDidChangeNotifier.connect(
    this, &SmartPropertyEditorManager::nodesDidChange);
}

void SmartPropertyEditorManager::selectionDidChange(const Selection&)
{
  auto document = kdl::mem_lock(m_document);
  switchEditor(m_propertyKey, document->allSelectedEntityNodes());
}

void SmartPropertyEditorManager::nodesDidChange(const std::vector<mdl::Node*>&)
{
  auto document = kdl::mem_lock(m_document);
  switchEditor(m_propertyKey, document->allSelectedEntityNodes());
}

SmartPropertyEditor* SmartPropertyEditorManager::selectEditor(
  const std::string& propertyKey, const std::vector<mdl::EntityNodeBase*>& nodes) const
{
  for (const auto& [matcher, editor] : m_editors)
  {
    if (matcher(propertyKey, nodes))
    {
      return editor;
    }
  }

  // should never happen
  assert(false);
  return defaultEditor();
}

SmartPropertyEditor* SmartPropertyEditorManager::defaultEditor() const
{
  return std::get<1>(m_editors.back());
}

void SmartPropertyEditorManager::activateEditor(
  SmartPropertyEditor* editor, const std::string& propertyKey)
{
  if (
    m_stackedLayout->currentWidget() != editor
    || !activeEditor()->usesPropertyKey(propertyKey))
  {
    deactivateEditor();

    m_propertyKey = propertyKey;
    m_stackedLayout->setCurrentWidget(editor);
    editor->activate(m_propertyKey);
  }
}

void SmartPropertyEditorManager::deactivateEditor()
{
  if (activeEditor())
  {
    activeEditor()->deactivate();
    m_stackedLayout->setCurrentIndex(-1);
    m_propertyKey = "";
  }
}

void SmartPropertyEditorManager::updateEditor()
{
  if (activeEditor())
  {
    auto document = kdl::mem_lock(m_document);
    activeEditor()->update(document->allSelectedEntityNodes());
  }
}

} // namespace tb::View
