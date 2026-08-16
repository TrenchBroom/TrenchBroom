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

#include "TestPreferenceStore.h"
#include "base/KeySequence.h"
#include "base/PreferenceManager.h"
#include "ui/ActionInfo.h"

#include "kd/k.h"

#include <unordered_set>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ui
{
using Catch::Matchers::UnorderedRangeEquals;

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

/**
 * Temporarily replaces the global PreferenceManager instance with one that doesn't save
 * instantly, so that PreferenceManager::set() only produces a pending (unsaved) value.
 * Restores an instance equivalent to the one RunAllTests.cpp sets up when it goes out of
 * scope, so later tests in the same binary still see a live instance.
 */
struct ScopedPendingPreferenceManager
{
  ScopedPendingPreferenceManager()
  {
    PreferenceManager::createInstance(
      std::make_unique<TestPreferenceStore>(), !K(saveInstantly));
  }

  ~ScopedPendingPreferenceManager()
  {
    PreferenceManager::createInstance(
      std::make_unique<TestPreferenceStore>(), K(saveInstantly));
  }
};

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
        UnorderedRangeEquals(std::unordered_set<size_t>{}));
    }

    SECTION("Ignores distinct shortcuts")
    {
      const auto preference1 =
        Preference<std::vector<KeySequence>>{"Action 1", {KeySequence{"A"}}};
      const auto preference2 =
        Preference<std::vector<KeySequence>>{"Action 2", {KeySequence{"B"}}};

      CHECK_THAT(
        findConflicts({makeActionInfo(preference1), makeActionInfo(preference2)}),
        UnorderedRangeEquals(std::unordered_set<size_t>{}));
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
        UnorderedRangeEquals(std::unordered_set<size_t>{}));
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
        UnorderedRangeEquals(std::unordered_set<size_t>{0, 1}));
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
        UnorderedRangeEquals(std::unordered_set<size_t>{0, 1, 2}));
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
        UnorderedRangeEquals(std::unordered_set<size_t>{0, 1}));
    }

    SECTION("Ignores duplicate shortcuts in the same multi-shortcut preference")
    {
      const auto preference = Preference<std::vector<KeySequence>>{
        "Action", {KeySequence{"A"}, KeySequence{"A"}}};

      CHECK_THAT(
        findConflicts({
          makeActionInfo(preference),
        }),
        UnorderedRangeEquals(std::unordered_set<size_t>{}));
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
        UnorderedRangeEquals(std::unordered_set<size_t>{0, 1}));
    }

    SECTION("Reports conflicts introduced by a pending, unsaved preference change")
    {
      const auto scopedPreferenceManager = ScopedPendingPreferenceManager{};

      const auto preference1 =
        Preference<std::vector<KeySequence>>{"Action 1", {KeySequence{"A"}}};
      const auto preference2 =
        Preference<std::vector<KeySequence>>{"Action 2", {KeySequence{"B"}}};

      REQUIRE_THAT(
        findConflicts({makeActionInfo(preference1), makeActionInfo(preference2)}),
        UnorderedRangeEquals(std::unordered_set<size_t>{}));

      // Not saved, only pending -- getPendingValue() must be consulted for this to be
      // picked up by findConflicts()
      PreferenceManager::instance().set(
        preference2, std::vector<KeySequence>{KeySequence{"A"}});

      CHECK_THAT(
        findConflicts({makeActionInfo(preference1), makeActionInfo(preference2)}),
        UnorderedRangeEquals(std::unordered_set<size_t>{0, 1}));
    }
  }
}

} // namespace tb::ui
