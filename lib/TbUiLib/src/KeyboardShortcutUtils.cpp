/*
 Copyright (C) 2026 Kristian Duske

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

#include "ui/KeyboardShortcutUtils.h"

#include <QKeySequence>

#include <algorithm>
#include <ranges>

namespace tb::ui
{
namespace
{

bool isSupportedShortcutKey(const Qt::Key key)
{
  return key != Qt::Key_CapsLock && key != Qt::Key_NumLock && key != Qt::Key_ScrollLock;
}

} // namespace

bool isSupportedShortcut(const QKeySequence& keySequence)
{
  return std::ranges::all_of(
    std::views::iota(0u, uint(keySequence.count())),
    [&](const auto i) { return isSupportedShortcutKey(keySequence[i].key()); });
}

} // namespace tb::ui
