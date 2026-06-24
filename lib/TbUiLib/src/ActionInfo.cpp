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

#include "ui/ActionInfo.h"

#include <QKeySequence>

#include "PreferenceManager.h"
#include "ui/ActionContext.h"

#include "kd/ranges/enumerate_view.h"

#include <map>
#include <ranges>

namespace tb::ui
{

namespace
{

struct ActionConflictCmp
{
  bool operator()(const ActionInfo& lhs, const ActionInfo& rhs) const
  {
    if (actionContextMatches(lhs.actionContext(), rhs.actionContext()))
    {
      // if the two have the same sequence, they would be in conflict, so we compare the
      // sequences
      const auto& lhsKeySequence = pref(lhs.keyboardShortcutPreference());
      const auto& rhsKeySequence = pref(rhs.keyboardShortcutPreference());
      return lhsKeySequence < rhsKeySequence;
    }
    // otherwise, we just compare by the action context
    return lhs.actionContext() < rhs.actionContext();
  }
};

} // namespace

ActionInfo::ActionInfo(
  const ActionInfoType type,
  std::filesystem::path displayPath,
  const ActionContext::Type actionContext,
  const Preference<QKeySequence>& keyboardShortcutPreference)
  : m_type{type}
  , m_displayPath{std::move(displayPath)}
  , m_actionContext{actionContext}
  , m_keyboardShortcutPreference{&keyboardShortcutPreference}
{
}

const std::filesystem::path& ActionInfo::displayPath() const
{
  return m_displayPath;
}

ActionInfoType ActionInfo::type() const
{
  return m_type;
}

ActionContext::Type ActionInfo::actionContext() const
{
  return m_actionContext;
}

const Preference<QKeySequence>& ActionInfo::keyboardShortcutPreference() const
{
  return *m_keyboardShortcutPreference;
}

std::strong_ordering ActionInfo::operator<=>(const ActionInfo& other) const
{
  const auto typeResult = m_type <=> other.m_type;
  if (typeResult != std::strong_ordering::equal || m_type == ActionInfoType::Menu)
  {
    return typeResult;
  }

  return m_displayPath <=> other.m_displayPath;
}

bool ActionInfo::operator==(const ActionInfo& other) const
{
  return m_type == other.m_type && m_displayPath == other.m_displayPath;
}

std::vector<size_t> findConflicts(const std::vector<ActionInfo>& actionInfos)
{
  auto entries = std::map<ActionInfo, size_t, ActionConflictCmp>{};
  auto conflicts = std::vector<size_t>{};

  for (const auto& [index, actionInfo] : actionInfos | kdl::views::enumerate)
  {
    const auto& keySequence = pref(actionInfo.keyboardShortcutPreference());
    if (keySequence.count() > 0)
    {
      const auto [it, noConflict] = entries.emplace(actionInfo, index);
      if (!noConflict)
      {
        // found a duplicate, so there are conflicts
        const auto otherIndex = it->second;
        conflicts.emplace_back(otherIndex);
        conflicts.emplace_back(index);
      }
    }
  }

  return conflicts;
}

} // namespace tb::ui
