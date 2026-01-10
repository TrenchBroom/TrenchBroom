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

#include "ui/ActionMenu.h"

#include <QKeySequence>
#include <QString>

#include "ui/MapViewBase.h"
#include "ui/MapWindow.h"

#include <string>

namespace tb::ui
{

void Menu::addSeparator()
{
  entries.emplace_back(MenuSeparator{});
}

const Action& Menu::addItem(Action& action, const MenuEntryType entryType_)
{
  entries.emplace_back(MenuAction{action, entryType_});
  action.setIsMenuAction(true);
  return action;
}

Menu& Menu::addMenu(std::string name_, const MenuEntryType entryType_)
{
  return std::get<Menu>(entries.emplace_back(Menu{std::move(name_), entryType_, {}}));
}

} // namespace tb::ui
