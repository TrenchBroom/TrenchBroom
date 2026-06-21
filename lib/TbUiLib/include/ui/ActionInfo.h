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

#pragma once

#include <QKeySequence>

#include "Preference.h"
#include "ui/ActionContext.h"

#include <compare>
#include <filesystem>
#include <vector>

class QObject;

namespace tb::ui
{
class Action;
class ActionManager;
class MapDocument;

enum class ActionInfoType
{
  Menu,
  View,
  Key,
  Tag,
  EntityDefinition,
};

class ActionInfo
{
private:
  ActionInfoType m_type;

  /**
   * Path displayed to the user, unrelated to the preference path.
   */
  std::filesystem::path m_displayPath;
  ActionContext::Type m_actionContext;
  const Preference<QKeySequence>* m_keyboardShortcutPreference;

public:
  ActionInfo(
    ActionInfoType type,
    std::filesystem::path displayPath,
    ActionContext::Type actionContext,
    const Preference<QKeySequence>& keyboardShortcutPreference);

  const std::filesystem::path& displayPath() const;

  ActionContext::Type actionContext() const;
  const Preference<QKeySequence>& keyboardShortcutPreference() const;

  std::strong_ordering operator<=>(const ActionInfo& other) const;
  bool operator==(const ActionInfo& other) const;
};

std::vector<size_t> findConflicts(const std::vector<ActionInfo>& actionInfos);

} // namespace tb::ui
