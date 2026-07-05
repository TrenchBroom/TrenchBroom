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

#include <QKeySequence>

#include "ui/CatchConfig.h"
#include "ui/KeyboardShortcutUtils.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("KeyboardShortcutUtils")
{
  SECTION("Accepts supported shortcuts")
  {
    CHECK(isSupportedShortcut(QKeySequence{Qt::Key_A}));
    CHECK(isSupportedShortcut(QKeySequence{Qt::CTRL | Qt::Key_Return}));
  }

  SECTION("Rejects lock state keys")
  {
    CHECK(!isSupportedShortcut(QKeySequence{Qt::Key_CapsLock}));
    CHECK(!isSupportedShortcut(QKeySequence{Qt::CTRL | Qt::Key_CapsLock}));
    CHECK(!isSupportedShortcut(QKeySequence{Qt::Key_NumLock}));
    CHECK(!isSupportedShortcut(QKeySequence{Qt::Key_ScrollLock}));
  }
}

} // namespace tb::ui
