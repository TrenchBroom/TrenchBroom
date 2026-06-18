/*
 Copyright (C) 2026 Artsiom Trubchyk
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

#pragma once

#include <QKeyEvent>

#include "PreferenceManager.h"
#include "ui/Action.h"

#include "kd/reflection_decl.h"

#include <algorithm>
#include <ranges>
#include <variant>

class QKeySequence;

namespace tb::ui
{
class ActionExecutionContext;

bool eventMatchesPhysicalKey(const QKeyEvent& event, const QKeySequence& shortcut);

bool shouldUsePhysicalShortcutFallback(const QKeyEvent& event);

bool eventMatchesPhysicalShortcutFallback(
  const QKeyEvent& event, const QKeySequence& shortcut);

struct NoFallbackActionMatch
{
  kdl_reflect_decl_empty(NoFallbackActionMatch);
};

struct UniqueFallbackActionMatch
{
  const Action& action;

  kdl_reflect_decl(UniqueFallbackActionMatch, action);
};

struct AmbiguousFallbackActionMatch
{
  const Action& action;

  kdl_reflect_decl(AmbiguousFallbackActionMatch, action);
};

using FallbackActionMatch = std::
  variant<NoFallbackActionMatch, UniqueFallbackActionMatch, AmbiguousFallbackActionMatch>;

std::ostream& operator<<(std::ostream& lhs, const FallbackActionMatch& rhs);

template <std::ranges::range R>
FallbackActionMatch findFallbackAction(
  const QKeyEvent& event, const ActionExecutionContext& context, R&& actions)
{
  const auto actionMatches = [&](const auto* action) {
    return action->enabled(context)
           && eventMatchesPhysicalShortcutFallback(event, pref(action->preference()));
  };

  const auto iFirstMatch = std::ranges::find_if(actions, actionMatches);
  return iFirstMatch == actions.end() ? FallbackActionMatch{NoFallbackActionMatch{}}
         : std::ranges::none_of(std::next(iFirstMatch), actions.end(), actionMatches)
           ? FallbackActionMatch{UniqueFallbackActionMatch{**iFirstMatch}}
           : FallbackActionMatch{AmbiguousFallbackActionMatch{**iFirstMatch}};
}

template <std::ranges::range R, typename F>
bool triggerFallbackAction(
  QKeyEvent& event,
  ActionExecutionContext& context,
  R&& actions,
  const F& triggerAmbiguousAction)
{
  return std::visit(
    kdl::overload(
      [&](const NoFallbackActionMatch&) { return false; },
      [&](const UniqueFallbackActionMatch& match) {
        match.action.execute(context);
        event.accept();
        return true;
      },
      [&](const AmbiguousFallbackActionMatch& match) {
        return triggerAmbiguousAction(match.action);
      }),
    findFallbackAction(event, context, actions));
}

} // namespace tb::ui
