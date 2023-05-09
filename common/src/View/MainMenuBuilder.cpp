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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MainMenuBuilder.h"

#include <QMenuBar>

#include "IO/ResourceUtils.h"
#include "View/MapFrame.h"

namespace TrenchBroom
{
namespace View
{
MenuBuilderBase::MenuBuilderBase(
  MenuBuilderBase::ActionMap& actions, const TriggerFn& triggerFn)
  : m_actions(actions)
  , m_triggerFn(triggerFn)
{
}

MenuBuilderBase::~MenuBuilderBase() = default;

void MenuBuilderBase::updateActionKeySeqeunce(QAction* qAction, const Action* tAction)
{
  if (!tAction->keySequence().isEmpty())
  {
    const QString tooltip =
      QObject::tr("%1 (%2)")
        .arg(tAction->label())
        .arg(tAction->keySequence().toString(QKeySequence::NativeText));
    qAction->setToolTip(tooltip);
  }
  else
  {
    qAction->setToolTip(tAction->label());
  }

  qAction->setShortcut(tAction->keySequence());
}

QAction* MenuBuilderBase::findOrCreateQAction(const Action* tAction)
{
  // Check if it already exists
  {
    auto it = m_actions.find(tAction);
    if (it != m_actions.end())
    {
      return it->second;
    }
  }

  auto* qAction = new QAction(tAction->label());
  qAction->setCheckable(tAction->checkable());
  if (tAction->hasIcon())
  {
    qAction->setIcon(IO::loadSVGIcon(tAction->iconPath()));
  }
  qAction->setStatusTip(tAction->statusTip());
  updateActionKeySeqeunce(qAction, tAction);

  const auto& triggerFn = m_triggerFn;
  QObject::connect(qAction, &QAction::triggered, [=]() { triggerFn(*tAction); });

  m_actions[tAction] = qAction;
  return qAction;
}

MainMenuBuilder::MainMenuBuilder(
  QMenuBar& menuBar, ActionMap& actions, const TriggerFn& triggerFn)
  : MenuBuilderBase(actions, triggerFn)
  , m_menuBar(menuBar)
  , m_currentMenu(nullptr)
  , recentDocumentsMenu(nullptr)
  , undoAction(nullptr)
  , redoAction(nullptr)
  , pasteAction(nullptr)
  , pasteAtOriginalPositionAction(nullptr)
{
}

void MainMenuBuilder::visit(const Menu& menu)
{
  auto* parentMenu = m_currentMenu;
  if (m_currentMenu == nullptr)
  {
    // top level menu
    m_currentMenu = m_menuBar.addMenu(QString::fromStdString(menu.name()));
  }
  else
  {
    m_currentMenu = m_currentMenu->addMenu(QString::fromStdString(menu.name()));
  }

  if (menu.entryType() == MenuEntryType::Menu_RecentDocuments)
  {
    recentDocumentsMenu = m_currentMenu;
  }

  menu.visitEntries(*this);
  m_currentMenu = parentMenu;
}

void MainMenuBuilder::visit(const MenuSeparatorItem&)
{
  assert(m_currentMenu != nullptr);
  m_currentMenu->addSeparator();
}

void MainMenuBuilder::visit(const MenuActionItem& item)
{
  assert(m_currentMenu != nullptr);
  const auto& tAction = item.action();
  QAction* qAction = findOrCreateQAction(&tAction);
  m_currentMenu->addAction(qAction);

  if (item.entryType() == MenuEntryType::Menu_Undo)
  {
    undoAction = qAction;
  }
  else if (item.entryType() == MenuEntryType::Menu_Redo)
  {
    redoAction = qAction;
  }
  else if (item.entryType() == MenuEntryType::Menu_Paste)
  {
    pasteAction = qAction;
  }
  else if (item.entryType() == MenuEntryType::Menu_PasteAtOriginalPosition)
  {
    pasteAtOriginalPositionAction = qAction;
  }
}
} // namespace View
} // namespace TrenchBroom
