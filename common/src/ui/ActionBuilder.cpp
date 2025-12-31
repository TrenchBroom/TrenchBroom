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

#include "ActionBuilder.h"

#include <QMenuBar>
#include <QToolBar>

#include "PreferenceManager.h"
#include "ui/Action.h"
#include "ui/ActionManager.h"
#include "ui/ActionMenu.h"
#include "ui/ImageUtils.h"

#include "kd/contracts.h"

namespace tb::ui
{

void updateActionKeySequence(QAction& qAction, const Action& tAction)
{
  const auto& shortcutPreference = pref(tAction.preference());
  if (!shortcutPreference.isEmpty())
  {
    const auto tooltip = QObject::tr("%1 (%2)")
                           .arg(tAction.label())
                           .arg(shortcutPreference.toString(QKeySequence::NativeText));
    qAction.setToolTip(tooltip);
  }
  else
  {
    qAction.setToolTip(tAction.label());
  }

  qAction.setShortcut(shortcutPreference);
}

namespace
{

QAction& findOrCreateQtAction(
  std::unordered_map<const Action*, QAction*>& actionMap,
  const Action& tbAction,
  const TriggerFn& triggerFn)
{
  if (const auto it = actionMap.find(&tbAction); it != actionMap.end())
  {
    return *it->second;
  }

  auto& qtAction =
    *actionMap.emplace(&tbAction, new QAction{tbAction.label()}).first->second;

  qtAction.setCheckable(tbAction.checkable());
  if (const auto& iconPath = tbAction.iconPath())
  {
    qtAction.setIcon(loadSVGIcon(*iconPath));
  }
  if (const auto& statusTip = tbAction.statusTip())
  {
    qtAction.setStatusTip(*statusTip);
  }
  updateActionKeySequence(qtAction, tbAction);

  QObject::connect(
    &qtAction, &QAction::triggered, [triggerFn, &tbAction]() { triggerFn(tbAction); });

  return qtAction;
}
} // namespace

PopulateMenuResult populateMenuBar(
  ActionManager& actionManager,
  QMenuBar& qtMenuBar,
  std::unordered_map<const Action*, QAction*>& actionMap,
  const TriggerFn& triggerFn)
{
  auto result = PopulateMenuResult{};
  QMenu* currentMenu = nullptr;

  actionManager.visitMainMenu(kdl::overload(
    [&](const MenuSeparator&) {
      contract_assert(currentMenu != nullptr);

      currentMenu->addSeparator();
    },
    [&](const MenuAction& actionItem) {
      contract_assert(currentMenu);

      auto& qtAction = findOrCreateQtAction(actionMap, actionItem.action, triggerFn);
      currentMenu->addAction(&qtAction);

      if (actionItem.entryType == MenuEntryType::Undo)
      {
        result.undoAction = &qtAction;
      }
      else if (actionItem.entryType == MenuEntryType::Redo)
      {
        result.redoAction = &qtAction;
      }
      else if (actionItem.entryType == MenuEntryType::Paste)
      {
        result.pasteAction = &qtAction;
      }
      else if (actionItem.entryType == MenuEntryType::PasteAtOriginalPosition)
      {
        result.pasteAtOriginalPositionAction = &qtAction;
      }
    },
    [&](const auto& thisLambda, const Menu& menu) {
      auto* parentMenu = currentMenu;
      if (!currentMenu)
      {
        // top level menu
        currentMenu = qtMenuBar.addMenu(QString::fromStdString(menu.name));
      }
      else
      {
        currentMenu = currentMenu->addMenu(QString::fromStdString(menu.name));
      }

      if (menu.entryType == MenuEntryType::RecentDocuments)
      {
        result.recentDocumentsMenu = currentMenu;
      }

      menu.visitEntries(thisLambda);
      currentMenu = parentMenu;
    }));

  return result;
}

void populateToolBar(
  ActionManager& actionManager,
  QToolBar& qtToolBar,
  std::unordered_map<const Action*, QAction*>& actionMap,
  const TriggerFn& triggerFn)
{
  actionManager.visitToolBar(kdl::overload(
    [&](const MenuSeparator&) { qtToolBar.addSeparator(); },
    [&](const MenuAction& actionItem) {
      auto& qtAction = findOrCreateQtAction(actionMap, actionItem.action, triggerFn);
      qtToolBar.addAction(&qtAction);
    },
    [](const auto& thisLambda, const Menu& menu) { menu.visitEntries(thisLambda); }));
}

} // namespace tb::ui
