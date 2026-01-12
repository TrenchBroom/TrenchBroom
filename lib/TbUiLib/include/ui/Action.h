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

#pragma once

#include <QKeySequence>
#include <QString>

#include "Macros.h"
#include "Preference.h"
#include "ui/ActionContext.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

namespace tb
{
namespace mdl
{
struct EntityDefinition;
class Map;
class SmartTag;
} // namespace mdl

namespace ui
{
class ActionExecutionContext;
class MapDocument;
class MapWindow;
class MapViewBase;

using ExecuteFn = std::function<void(ActionExecutionContext&)>;
using EnabledFn = std::function<bool(const ActionExecutionContext&)>;
using CheckedFn = std::function<bool(const ActionExecutionContext&)>;

class Action
{
private:
  QString m_label;
  ActionContext::Type m_actionContext;
  Preference<QKeySequence> m_shortcutPreference;

  ExecuteFn m_execute;
  EnabledFn m_enabled;
  std::optional<CheckedFn> m_checked;

  std::optional<std::filesystem::path> m_iconPath;
  std::optional<QString> m_statusTip;

  bool m_isMenuAction = false;

public:
  Action(
    std::filesystem::path preferencePath,
    QString label,
    ActionContext::Type actionContext,
    QKeySequence defaultShortcut,
    ExecuteFn execute,
    EnabledFn enabled,
    std::optional<CheckedFn> checked,
    std::optional<std::filesystem::path> iconPath = std::nullopt,
    std::optional<QString> statusTip = std::nullopt);

  Action(
    std::filesystem::path preferencePath,
    QString label,
    ActionContext::Type actionContext,
    QKeySequence defaultShortcut,
    ExecuteFn execute,
    EnabledFn enabled,
    std::optional<std::filesystem::path> iconPath = std::nullopt,
    std::optional<QString> statusTip = std::nullopt);

  Action(
    std::filesystem::path preferencePath,
    QString label,
    ActionContext::Type actionContext,
    ExecuteFn execute,
    EnabledFn enabled);

  const QString& label() const;
  ActionContext::Type actionContext() const;

  const Preference<QKeySequence>& preference() const;
  Preference<QKeySequence>& preference();

  void execute(ActionExecutionContext& context) const;
  bool enabled(const ActionExecutionContext& context) const;
  bool checkable() const;
  bool checked(const ActionExecutionContext& context) const;

  const std::optional<std::filesystem::path>& iconPath() const;

  const std::optional<QString>& statusTip() const;

  bool isMenuAction() const;
  void setIsMenuAction(bool isMenuAction);

  deleteCopy(Action);

  // cannot be noexcept because it will call QKeySequence's copy constructor
  Action(Action&& other) = default;
  Action& operator=(Action&& other) = default;
};

std::vector<size_t> findConflicts(const std::vector<const Action*>& actions);

} // namespace ui
} // namespace tb
