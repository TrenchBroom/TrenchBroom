/*
 Copyright (C) 2024 Kristian Duske

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

#include "Ensure.h"
#include "ui/Actions.h"

#include "kdl/vector_utils.h"

#include <sstream>
#include <string>
#include <vector>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

auto collectMenuActions(const ActionManager& actionManager)
{
  auto actions = std::vector<const Action*>{};
  actionManager.visitMainMenu(kdl::overload(
    [](const MenuSeparator&) {},
    [&](const MenuAction& actionItem) { actions.push_back(&actionItem.action); },
    [](const auto& thisLambda, const Menu& menu) { menu.visitEntries(thisLambda); }));
  return actions;
}

auto collectViewActions(const ActionManager& actionManager)
{
  auto actions = std::vector<const Action*>{};
  actionManager.visitMapViewActions(
    [&](const Action& action) { actions.push_back(&action); });
  return actions;
}

auto collectAllActions(const ActionManager& actionManager)
{
  return kdl::vec_concat(
    collectMenuActions(actionManager), collectViewActions(actionManager));
}

using ActionConflict = std::tuple<const Action*, const Action*>;

auto getActionConflicts(
  const std::vector<const Action*>& actions, const std::vector<size_t>& conflicts)
{
  ensure(conflicts.size() % 2 == 0, "Conflicts must be pairs of indices");

  auto conflictingActions = std::vector<ActionConflict>{};
  for (size_t i = 0; i < conflicts.size(); i += 2)
  {
    const auto* action1 = actions[conflicts[i + 0]];
    const auto* action2 = actions[conflicts[i + 1]];
    conflictingActions.emplace_back(action1, action2);
  }
  return conflictingActions;
}

} // namespace

TEST_CASE("Actions")
{
  SECTION("Default actions have no conflicts")
  {
    const auto& actionManager = ActionManager::instance();
    const auto allActions = collectAllActions(actionManager);

    const auto conflicts = getActionConflicts(allActions, findConflicts(allActions));
    CHECK(conflicts == std::vector<ActionConflict>{});
  }
}

} // namespace tb::ui

namespace Catch
{

template <>
struct StringMaker<tb::ui::ActionConflict>
{
  static std::string convert(const tb::ui::ActionConflict& value)
  {
    const auto& [action1, action2] = value;
    auto str = std::stringstream{};
    str << action1->preferencePath() << " conflicts with " << action2->preferencePath();
    return str.str();
  }
};

} // namespace Catch
