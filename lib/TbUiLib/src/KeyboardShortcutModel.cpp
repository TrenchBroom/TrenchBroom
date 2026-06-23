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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/KeyboardShortcutModel.h"

#include <QBrush>

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "ui/Action.h"
#include "ui/ActionContext.h"
#include "ui/ActionManager.h"
#include "ui/ActionMenu.h"
#include "ui/MapDocument.h"
#include "ui/QPathUtils.h"

#include "kd/contracts.h"
#include "kd/set_adapter.h"
#include "kd/vector_utils.h"

namespace tb::ui
{

KeyboardShortcutModel::KeyboardShortcutModel(
  ActionManager& actionManager, MapDocument* document, QObject* parent)
  : QAbstractTableModel{parent}
  , m_actionManager{actionManager}
  , m_document{document}
{
  initializeActions();
  updateConflicts();
}

void KeyboardShortcutModel::reset()
{
  m_actions.clear();
  initializeActions();
  updateConflicts();
  if (totalActionCount() > 0)
  {
    emit dataChanged(createIndex(0, 0), createIndex(totalActionCount() - 1, 3));
  }
}

int KeyboardShortcutModel::rowCount(const QModelIndex& /* parent */) const
{
  return totalActionCount();
}

int KeyboardShortcutModel::columnCount(const QModelIndex& /* parent */) const
{
  // Shortcut, Alternative, Context, Description
  return 4;
}

QVariant KeyboardShortcutModel::headerData(
  const int section, const Qt::Orientation orientation, const int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch (section)
    {
    case 0:
      return QString{"Shortcut"};
    case 1:
      return QString{"Alternative"};
    case 2:
      return QString{"Context"};
    case 3:
      return QString{"Description"};
    }
  }
  return QVariant{};
}

QVariant KeyboardShortcutModel::data(const QModelIndex& index, const int role) const
{
  if (!checkIndex(index))
  {
    return QVariant{};
  }

  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    const auto& actionInfo = this->actionInfo(index.row());

    auto& prefs = PreferenceManager::instance();
    const auto& keyboardShortcuts =
      prefs.getPendingValue(actionInfo.keyboardShortcutPreference());

    switch (index.column())
    {
    case 0:
      return !keyboardShortcuts.empty() ? keyboardShortcuts[0] : QKeySequence{};
    case 1:
      return keyboardShortcuts.size() > 1 ? keyboardShortcuts[1] : QKeySequence{};
    case 2:
      return QString::fromStdString(actionContextName(actionInfo.actionContext()));
    case 3:
      return QString::fromStdString(actionInfo.displayPath().generic_string());
    }
  }

  if (role == Qt::ForegroundRole && hasConflicts(index))
  {
    return QBrush{Qt::red};
  }

  return QVariant{};
}

bool KeyboardShortcutModel::setData(
  const QModelIndex& index, const QVariant& value, const int role)
{
  if (!checkIndex(index) || role != Qt::EditRole)
  {
    return false;
  }

  auto& prefs = PreferenceManager::instance();

  // We take a copy here on purpose in order to set the key further below.
  auto& actionInfo = this->actionInfo(index.row());
  auto keyboardShortcuts = prefs.getPendingValue(actionInfo.keyboardShortcutPreference());

  switch (index.column())
  {
  case 0:
    if (keyboardShortcuts.empty())
    {
      keyboardShortcuts.emplace_back();
    }
    keyboardShortcuts[0] = value.value<QKeySequence>();
    break;
  case 1:
    if (keyboardShortcuts.empty())
    {
      keyboardShortcuts.emplace_back();
    }
    if (keyboardShortcuts.size() == 1)
    {
      keyboardShortcuts.emplace_back();
    }
    keyboardShortcuts[1] = value.value<QKeySequence>();
    break;
  default:
    break;
  }

  prefs.set(actionInfo.keyboardShortcutPreference(), std::move(keyboardShortcuts));

  updateConflicts();

  emit dataChanged(index, index, {Qt::DisplayRole, role});
  return true;
}

Qt::ItemFlags KeyboardShortcutModel::flags(const QModelIndex& index) const
{
  if (!checkIndex(index))
  {
    return Qt::ItemIsEnabled;
  }

  switch (index.column())
  {
  case 0:
  case 1:
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
  default:
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
}

size_t KeyboardShortcutModel::maxKeySequenceCount(const QModelIndex& index) const
{
  if (checkIndex(index))
  {
    switch (actionInfo(index.row()).type())
    {
    case ActionInfoType::Menu:
    case ActionInfoType::View:
    case ActionInfoType::Tag:
    case ActionInfoType::EntityDefinition:
      return 4;
    case ActionInfoType::Key:
      return 1;
      switchDefault();
    }
  }

  return 0;
}

bool KeyboardShortcutModel::hasConflicts() const
{
  return !m_conflicts.empty();
}

bool KeyboardShortcutModel::hasConflicts(const QModelIndex& index) const
{
  if (!checkIndex(index))
  {
    return false;
  }

  return kdl::wrap_set(m_conflicts).count(index.row()) > 0u;
}

void KeyboardShortcutModel::initializeActions()
{
  initializeMenuActions();
  initializeViewActions();
  initializeKeys();
  if (m_document)
  {
    initializeTagActions();
    initializeEntityDefinitionActions();
  }

  std::ranges::stable_sort(m_actions);
}

void KeyboardShortcutModel::initializeMenuActions()
{
  auto currentPath = std::filesystem::path{};
  m_actionManager.visitMainMenu(kdl::overload(
    [](const MenuSeparator&) {},
    [&](const MenuAction& actionItem) {
      m_actions.emplace_back(
        ActionInfoType::Menu,
        currentPath / pathFromQString(actionItem.action.label()),
        actionItem.action.actionContext(),
        actionItem.action.preference());
    },
    [&](const auto& thisLambda, const Menu& menu) {
      currentPath = currentPath / menu.name;
      menu.visitEntries(thisLambda);
      currentPath = currentPath.parent_path();
    }));
}

void KeyboardShortcutModel::initializeViewActions()
{
  m_actionManager.visitMapViewActions([&](Action& action) {
    m_actions.emplace_back(
      ActionInfoType::View,
      "Map View" / pathFromQString(action.label()),
      action.actionContext(),
      action.preference());
  });
}

void KeyboardShortcutModel::initializeKeys()
{
  m_actions.emplace_back(
    ActionInfoType::Key,
    std::filesystem::path{"Map View"} / "Fly Forward",
    ActionContext::FlyMode,
    Preferences::CameraFlyForward);
  m_actions.emplace_back(
    ActionInfoType::Key,
    std::filesystem::path{"Map View"} / "Fly Left",
    ActionContext::FlyMode,
    Preferences::CameraFlyLeft);
  m_actions.emplace_back(
    ActionInfoType::Key,
    std::filesystem::path{"Map View"} / "Fly Backward",
    ActionContext::FlyMode,
    Preferences::CameraFlyBackward);
  m_actions.emplace_back(
    ActionInfoType::Key,
    std::filesystem::path{"Map View"} / "Fly Right",
    ActionContext::FlyMode,
    Preferences::CameraFlyRight);
  m_actions.emplace_back(
    ActionInfoType::Key,
    std::filesystem::path{"Map View"} / "Fly Up",
    ActionContext::FlyMode,
    Preferences::CameraFlyUp);
  m_actions.emplace_back(
    ActionInfoType::Key,
    std::filesystem::path{"Map View"} / "Fly Down",
    ActionContext::FlyMode,
    Preferences::CameraFlyDown);
}

void KeyboardShortcutModel::initializeTagActions()
{
  contract_pre(m_document);

  m_document->visitTagActions(m_actionManager, [&](Action& action) {
    m_actions.emplace_back(
      ActionInfoType::Tag,
      "Tags" / pathFromQString(action.label()),
      action.actionContext(),
      action.preference());
  });
}

void KeyboardShortcutModel::initializeEntityDefinitionActions()
{
  contract_pre(m_document);

  m_document->visitEntityDefinitionActions(m_actionManager, [&](Action& action) {
    m_actions.emplace_back(
      ActionInfoType::EntityDefinition,
      "Entity Definitions" / pathFromQString(action.label()),
      action.actionContext(),
      action.preference());
  });
}

void KeyboardShortcutModel::updateConflicts()
{
  m_conflicts = kdl::vec_static_cast<int>(findConflicts(m_actions));
  for (const auto& row : m_conflicts)
  {
    const auto index = createIndex(row, 0);
    emit dataChanged(index, index, {Qt::DisplayRole});
  }
}

const ActionInfo& KeyboardShortcutModel::actionInfo(const int index) const
{
  contract_pre(index < totalActionCount());

  return m_actions[static_cast<size_t>(index)];
}

int KeyboardShortcutModel::totalActionCount() const
{
  return static_cast<int>(m_actions.size());
}

bool KeyboardShortcutModel::checkIndex(const QModelIndex& index) const
{
  return index.isValid() && index.column() < 4 && index.row() < totalActionCount();
}

} // namespace tb::ui
