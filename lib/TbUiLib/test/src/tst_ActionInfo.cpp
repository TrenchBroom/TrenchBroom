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

#include "ui/ActionInfo.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::ui
{
using Catch::Matchers::UnorderedEquals;

namespace
{

auto makeActionInfo(
  const Preference<QKeySequence>& preference,
  const ActionContext::Type actionContext = ActionContext::Any)
{
  return ActionInfo{
    ActionInfoType::Menu,
    preference.path,
    actionContext,
    preference,
  };
}

} // namespace

TEST_CASE("ActionInfo")
{
  SECTION("findConflicts")
  {
    SECTION("Ignores empty shortcuts")
    {
      const auto preference1 = Preference<QKeySequence>{"Action 1", QKeySequence{}};
      const auto preference2 = Preference<QKeySequence>{"Action 2", QKeySequence{}};

      CHECK_THAT(
        findConflicts({makeActionInfo(preference1), makeActionInfo(preference2)}),
        UnorderedEquals(std::vector<size_t>{}));
    }

    SECTION("Ignores distinct shortcuts")
    {
      const auto preference1 = Preference<QKeySequence>{"Action 1", QKeySequence{'A'}};
      const auto preference2 = Preference<QKeySequence>{"Action 2", QKeySequence{'B'}};

      CHECK_THAT(
        findConflicts({makeActionInfo(preference1), makeActionInfo(preference2)}),
        UnorderedEquals(std::vector<size_t>{}));
    }

    SECTION("Ignores matching shortcuts in disjoint action contexts")
    {
      const auto preference1 = Preference<QKeySequence>{"Action 1", QKeySequence{'A'}};
      const auto preference2 = Preference<QKeySequence>{"Action 2", QKeySequence{'A'}};

      CHECK_THAT(
        findConflicts({
          makeActionInfo(
            preference1,
            ActionContext::View2D | ActionContext::NoSelection | ActionContext::NoTool),
          makeActionInfo(
            preference2,
            ActionContext::View3D | ActionContext::NoSelection | ActionContext::NoTool),
        }),
        UnorderedEquals(std::vector<size_t>{}));
    }

    SECTION("Reports matching shortcuts in overlapping action contexts")
    {
      const auto preference1 = Preference<QKeySequence>{"Action 1", QKeySequence{'A'}};
      const auto preference2 = Preference<QKeySequence>{"Action 2", QKeySequence{'A'}};

      CHECK_THAT(
        findConflicts({
          makeActionInfo(
            preference1,
            ActionContext::View2D | ActionContext::NoSelection | ActionContext::NoTool),
          makeActionInfo(
            preference2,
            ActionContext::AnyView | ActionContext::NoSelection | ActionContext::NoTool),
        }),
        UnorderedEquals(std::vector<size_t>{0, 1}));
    }

    SECTION("Reports later duplicates against the first matching shortcut")
    {
      const auto preference1 = Preference<QKeySequence>{"Action 1", QKeySequence{'A'}};
      const auto preference2 = Preference<QKeySequence>{"Action 2", QKeySequence{'A'}};
      const auto preference3 = Preference<QKeySequence>{"Action 3", QKeySequence{'A'}};

      CHECK_THAT(
        findConflicts({
          makeActionInfo(preference1),
          makeActionInfo(preference2),
          makeActionInfo(preference3),
        }),
        UnorderedEquals(std::vector<size_t>{0, 1, 0, 2}));
    }
  }
}

} // namespace tb::ui
