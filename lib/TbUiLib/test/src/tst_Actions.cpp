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

#include "prefs/Preferences.h"
#include "ui/Action.h"
#include "ui/ActionInfo.h"
#include "ui/ActionManager.h"
#include "ui/ActionMenu.h"
#include "ui/CatchConfig.h"

#include "kd/contracts.h"
#include "kd/ranges/concat_view.h"
#include "kd/ranges/to.h"

#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

auto collectMenuActionInfos(const ActionManager& actionManager)
{
  auto actions = std::vector<const Action*>{};
  actionManager.visitMainMenu(kdl::overload(
    [](const MenuSeparator&) {},
    [&](const MenuAction& actionItem) { actions.push_back(&actionItem.action); },
    [](const auto& thisLambda, const Menu& menu) { menu.visitEntries(thisLambda); }));

  return actions | std::views::transform([](const auto* action) {
           return ActionInfo{
             ActionInfoType::Menu,
             action->preference().path,
             action->actionContext(),
             action->preference(),
           };
         })
         | kdl::ranges::to<std::vector>();
}

auto collectViewActionInfos(const ActionManager& actionManager)
{
  auto actions = std::vector<const Action*>{};
  actionManager.visitMapViewActions(
    [&](const Action& action) { actions.push_back(&action); });

  return actions | std::views::transform([](const auto* action) {
           return ActionInfo{
             ActionInfoType::View,
             action->preference().path,
             action->actionContext(),
             action->preference(),
           };
         })
         | kdl::ranges::to<std::vector>();
}

auto collectKeyActionInfos()
{
  return Preferences::keyPreferences()
         | std::views::transform([=](const auto* preference) {
             return ActionInfo{
               ActionInfoType::Key,
               preference->path,
               ActionContext::FlyMode,
               *preference,
             };
           })
         | kdl::ranges::to<std::vector>();
}

auto collectAllActionInfos(const ActionManager& actionManager)
{
  return kdl::views::concat(
           collectMenuActionInfos(actionManager),
           collectViewActionInfos(actionManager),
           collectKeyActionInfos())
         | kdl::ranges::to<std::vector>();
}

using ActionConflict = std::tuple<ActionInfo, ActionInfo>;

auto getActionConflicts(
  const std::vector<ActionInfo>& actions, const std::vector<size_t>& conflicts)
{
  contract_pre(conflicts.size() % 2 == 0);

  auto conflictingActions = std::vector<ActionConflict>{};
  for (size_t i = 0; i < conflicts.size(); i += 2)
  {
    const auto& actionInfo1 = actions[conflicts[i + 0]];
    const auto& actionInfo2 = actions[conflicts[i + 1]];
    conflictingActions.emplace_back(actionInfo1, actionInfo2);
  }
  return conflictingActions;
}

} // namespace

TEST_CASE("Actions")
{
  const auto actionManager = ActionManager{};

  SECTION("Default actions have no conflicts")
  {
    const auto allActionInfos = collectAllActionInfos(actionManager);

    const auto conflicts =
      getActionConflicts(allActionInfos, findConflicts(allActionInfos));
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
    const auto& [actionInfo1, actionInfo2] = value;
    auto str = std::stringstream{};
    str << actionInfo1.displayPath() << " conflicts with " << actionInfo2.displayPath();
    return str.str();
  }
};

} // namespace Catch
