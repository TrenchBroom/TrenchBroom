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

#include "base/KeySequence.h"
#include "ui/ActionInfo.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::ui
{
using Catch::Matchers::UnorderedEquals;

namespace
{

auto makeActionInfo(
  const Preference<std::vector<KeySequence>>& preference,
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
      const auto preference1 =
        Preference<std::vector<KeySequence>>{"Action 1", {KeySequence{}}};
      const auto preference2 =
        Preference<std::vector<KeySequence>>{"Action 2", {KeySequence{}}};

      CHECK_THAT(
        findConflicts({makeActionInfo(preference1), makeActionInfo(preference2)}),
        UnorderedEquals(std::vector<size_t>{}));
    }

    SECTION("Ignores distinct shortcuts")
    {
      const auto preference1 =
        Preference<std::vector<KeySequence>>{"Action 1", {KeySequence{"A"}}};
      const auto preference2 =
        Preference<std::vector<KeySequence>>{"Action 2", {KeySequence{"B"}}};

      CHECK_THAT(
        findConflicts({makeActionInfo(preference1), makeActionInfo(preference2)}),
        UnorderedEquals(std::vector<size_t>{}));
    }

    SECTION("Ignores matching shortcuts in disjoint action contexts")
    {
      const auto preference1 =
        Preference<std::vector<KeySequence>>{"Action 1", {KeySequence{"A"}}};
      const auto preference2 =
        Preference<std::vector<KeySequence>>{"Action 2", {KeySequence{"A"}}};

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
      const auto preference1 =
        Preference<std::vector<KeySequence>>{"Action 1", {KeySequence{"A"}}};
      const auto preference2 =
        Preference<std::vector<KeySequence>>{"Action 2", {KeySequence{"A"}}};

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
      const auto preference1 =
        Preference<std::vector<KeySequence>>{"Action 1", {KeySequence{"A"}}};
      const auto preference2 =
        Preference<std::vector<KeySequence>>{"Action 2", {KeySequence{"A"}}};
      const auto preference3 =
        Preference<std::vector<KeySequence>>{"Action 3", {KeySequence{"A"}}};

      CHECK_THAT(
        findConflicts({
          makeActionInfo(preference1),
          makeActionInfo(preference2),
          makeActionInfo(preference3),
        }),
        UnorderedEquals(std::vector<size_t>{0, 1, 0, 2}));
    }

    SECTION("Reports matching shortcuts in multi-shortcut preferences")
    {
      const auto preference1 = Preference<std::vector<KeySequence>>{
        "Action 1", {KeySequence{"A"}, KeySequence{"B"}}};

      const auto preference2 = GENERATE(
        Preference<std::vector<KeySequence>>{
          "Action 2", {KeySequence{"C"}, KeySequence{"B"}}},
        Preference<std::vector<KeySequence>>{
          "Action 2", {KeySequence{"B"}, KeySequence{"C"}}});

      CAPTURE(preference2);

      CHECK_THAT(
        findConflicts({
          makeActionInfo(preference1),
          makeActionInfo(preference2),
        }),
        UnorderedEquals(std::vector<size_t>{0, 1}));
    }

    SECTION("Ignores duplicate shortcuts in the same multi-shortcut preference")
    {
      const auto preference = Preference<std::vector<KeySequence>>{
        "Action", {KeySequence{"A"}, KeySequence{"A"}}};

      CHECK_THAT(
        findConflicts({
          makeActionInfo(preference),
        }),
        UnorderedEquals(std::vector<size_t>{}));
    }

    SECTION(
      "Reports duplicate shortcuts in the same preference only against other actions")
    {
      const auto preference1 = Preference<std::vector<KeySequence>>{
        "Action 1", {KeySequence{"A"}, KeySequence{"A"}}};
      const auto preference2 =
        Preference<std::vector<KeySequence>>{"Action 2", {KeySequence{"A"}}};

      CHECK_THAT(
        findConflicts({
          makeActionInfo(preference1),
          makeActionInfo(preference2),
        }),
        UnorderedEquals(std::vector<size_t>{0, 1}));
    }
  }
}

} // namespace tb::ui
