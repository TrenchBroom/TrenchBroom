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

#include "ui/Action.h"

#include "ui/ActionExecutionContext.h"

#include "kd/const_overload.h"

namespace tb::ui
{

Action::Action(
  std::filesystem::path preferencePath,
  QString label,
  const ActionContext::Type actionContext,
  QKeySequence defaultShortcut,
  ExecuteFn execute,
  EnabledFn enabled,
  std::optional<CheckedFn> checked,
  std::optional<std::filesystem::path> iconPath,
  std::optional<QString> statusTip)
  : m_label{std::move(label)}
  , m_actionContext{actionContext}
  , m_shortcutPreference{std::move(preferencePath), defaultShortcut}
  , m_execute{std::move(execute)}
  , m_enabled{std::move(enabled)}
  , m_checked{std::move(checked)}
  , m_iconPath{std::move(iconPath)}
  , m_statusTip{std::move(statusTip)}
{
}

Action::Action(
  std::filesystem::path preferencePath,
  QString label,
  const ActionContext::Type actionContext,
  QKeySequence defaultShortcut,
  ExecuteFn execute,
  EnabledFn enabled,
  std::optional<std::filesystem::path> iconPath,
  std::optional<QString> statusTip)
  : Action{
      std::move(preferencePath),
      std::move(label),
      actionContext,
      defaultShortcut,
      std::move(execute),
      std::move(enabled),
      std::nullopt,
      std::move(iconPath),
      std::move(statusTip)}
{
}

Action::Action(
  std::filesystem::path preferencePath,
  QString label,
  ActionContext::Type actionContext,
  ExecuteFn execute,
  EnabledFn enabled)
  : Action{
      std::move(preferencePath),
      std::move(label),
      actionContext,
      QKeySequence{},
      std::move(execute),
      std::move(enabled),
      std::nullopt,
      std::nullopt,
      std::nullopt}
{
}

const QString& Action::label() const
{
  return m_label;
}

ActionContext::Type Action::actionContext() const
{
  return m_actionContext;
}

const Preference<QKeySequence>& Action::preference() const
{
  return m_shortcutPreference;
}

Preference<QKeySequence>& Action::preference()
{
  return KDL_CONST_OVERLOAD(preference());
}

void Action::execute(ActionExecutionContext& context) const
{
  if (enabled(context))
  {
    m_execute(context);
  }
}

bool Action::enabled(const ActionExecutionContext& context) const
{
  return context.hasActionContext(m_actionContext) && m_enabled(context);
}

bool Action::checkable() const
{
  return m_checked != std::nullopt;
}

bool Action::checked(const ActionExecutionContext& context) const
{
  return m_checked && (*m_checked)(context);
}

const std::optional<std::filesystem::path>& Action::iconPath() const
{
  return m_iconPath;
}

const std::optional<QString>& Action::statusTip() const
{
  return m_statusTip;
}

bool Action::isMenuAction() const
{
  return m_isMenuAction;
}

void Action::setIsMenuAction(const bool isMenuAction)
{
  m_isMenuAction = isMenuAction;
}

} // namespace tb::ui
