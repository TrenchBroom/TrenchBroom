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

#include "KeyboardShortcutModel.h"

#include <QBrush>

#include "PreferenceManager.h"
#include "ui/Action.h"
#include "ui/ActionContext.h"
#include "ui/ActionManager.h"
#include "ui/ActionMenu.h"
#include "ui/MapDocument.h"
#include "ui/QPathUtils.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"
#include "kd/set_adapter.h"
#include "kd/vector_utils.h"

#include <ranges>

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
    emit dataChanged(createIndex(0, 0), createIndex(totalActionCount() - 1, 2));
  }
}

int KeyboardShortcutModel::rowCount(const QModelIndex& /* parent */) const
{
  return totalActionCount();
}

int KeyboardShortcutModel::columnCount(const QModelIndex& /* parent */) const
{
  // Shortcut, Context, Description
  return 3;
}

QVariant KeyboardShortcutModel::headerData(
  const int section, const Qt::Orientation orientation, const int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    return section == 0   ? QString{"Shortcut"}
           : section == 1 ? QString{"Context"}
                          : QString{"Description"};
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
    if (index.column() == 0)
    {
      return pref(actionInfo.action.preference());
    }
    if (index.column() == 1)
    {
      return QString::fromStdString(actionContextName(actionInfo.action.actionContext()));
    }
    return QString::fromStdString(actionInfo.displayPath.generic_string());
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
  prefs.set(actionInfo.action.preference(), value.value<QKeySequence>());

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

  return index.column() == 0
           ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
           : Qt::ItemIsEnabled | Qt::ItemIsSelectable;
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
  if (m_document)
  {
    initializeTagActions();
    initializeEntityDefinitionActions();
  }
}

void KeyboardShortcutModel::initializeMenuActions()
{
  auto currentPath = std::filesystem::path{};
  m_actionManager.visitMainMenu(kdl::overload(
    [](const MenuSeparator&) {},
    [&](const MenuAction& actionItem) {
      m_actions.push_back(ActionInfo{
        currentPath / pathFromQString(actionItem.action.label()), actionItem.action});
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
    m_actions.push_back(ActionInfo{"Map View" / pathFromQString(action.label()), action});
  });
}

void KeyboardShortcutModel::initializeTagActions()
{
  contract_pre(m_document);

  m_document->visitTagActions(m_actionManager, [&](Action& action) {
    m_actions.push_back(ActionInfo{"Tags" / pathFromQString(action.label()), action});
  });
}

void KeyboardShortcutModel::initializeEntityDefinitionActions()
{
  contract_pre(m_document);

  m_document->visitEntityDefinitionActions(m_actionManager, [&](Action& action) {
    m_actions.push_back(
      ActionInfo{"Entity Definitions" / pathFromQString(action.label()), action});
  });
}

void KeyboardShortcutModel::updateConflicts()
{
  const auto allActions = m_actions | std::views::transform([](const auto& actionInfo) {
                            return const_cast<const Action*>(&actionInfo.action);
                          })
                          | kdl::ranges::to<std::vector>();

  m_conflicts = kdl::vec_static_cast<int>(findConflicts(allActions));
  for (const auto& row : m_conflicts)
  {
    const auto index = createIndex(row, 0);
    emit dataChanged(index, index, {Qt::DisplayRole});
  }
}

const KeyboardShortcutModel::ActionInfo& KeyboardShortcutModel::actionInfo(
  const int index) const
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
  return index.isValid() && index.column() < 3 && index.row() < totalActionCount();
}

} // namespace tb::ui
