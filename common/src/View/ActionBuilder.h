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

#include <functional>

class QAction;
class QMenu;
class QMenuBar;
class QToolBar;

namespace TrenchBroom::View
{
class Action;

using TriggerFn = std::function<void(const Action&)>;

void updateActionKeySequence(QAction& qAction, const Action& tAction);

struct PopulateMenuResult
{
  QMenu* recentDocumentsMenu = nullptr;
  QAction* undoAction = nullptr;
  QAction* redoAction = nullptr;
  QAction* pasteAction = nullptr;
  QAction* pasteAtOriginalPositionAction = nullptr;
};

PopulateMenuResult populateMenuBar(
  QMenuBar& qtMenuBar,
  std::unordered_map<const Action*, QAction*>& actionMap,
  const TriggerFn& triggerFn);

void populateToolBar(
  QToolBar& qtToolBar,
  std::unordered_map<const Action*, QAction*>& actionMap,
  const TriggerFn& triggerFn);

} // namespace TrenchBroom::View
