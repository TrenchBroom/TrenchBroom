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

#pragma once

#include <QKeySequence>

#include "ui/Action.h"

#include "kd/overload.h"

#include <string>
#include <variant>
#include <vector>

namespace tb::ui
{

enum class MenuEntryType
{
  RecentDocuments,
  Undo,
  Redo,
  Cut,
  Copy,
  Paste,
  PasteAtOriginalPosition,
  None
};

struct MenuSeparator
{
};

struct MenuAction
{
  Action& action;
  MenuEntryType entryType;
};

struct Menu;

using MenuEntry = std::variant<MenuSeparator, MenuAction, Menu>;

struct Menu
{
  std::string name;
  MenuEntryType entryType;
  std::vector<MenuEntry> entries;

  void addSeparator();
  const Action& addItem(Action& action, MenuEntryType entryType = MenuEntryType::None);
  Menu& addMenu(std::string name, MenuEntryType entryType = MenuEntryType::None);

  template <typename Visitor>
  void visitEntries(const Visitor& visitor) const
  {
    for (const auto& entry : entries)
    {
      std::visit(
        kdl::overload(
          [&](const MenuSeparator& separatorItem) { visitor(separatorItem); },
          [&](const MenuAction& actionItem) { visitor(actionItem); },
          [&](const Menu& menu) { visitor(visitor, menu); }),
        entry);
    }
  }
};

} // namespace tb::ui
