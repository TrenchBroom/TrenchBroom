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

#include "base/Notifier.h"

namespace tb::ui
{
class Tool
{
private:
  bool m_active = false;
  bool m_gestureInProgress = false;

public:
  Notifier<Tool&> toolActivatedNotifier;
  Notifier<Tool&> toolDeactivatedNotifier;
  Notifier<Tool&> refreshViewsNotifier;
  Notifier<Tool&> toolHandleSelectionChangedNotifier;

protected:
  explicit Tool(bool initiallyActive);

public:
  virtual ~Tool();

  bool active() const;
  bool activate();
  bool deactivate();

  /**
   * Indicates whether a gesture tracker created by one of this tool's controllers is
   * active, e.g. because the user is dragging one of this tool's handles.
   *
   * The tool's controllers set this when they accept a mouse drag or gesture, and reset
   * it before the gesture tracker is notified that the gesture has ended or was
   * cancelled.
   */
  bool gestureInProgress() const;
  void setGestureInProgress(bool gestureInProgress);

  void refreshViews();
  void notifyToolHandleSelectionChanged();

  virtual bool ownsSelection() const;

private:
  virtual bool doActivate();
  virtual bool doDeactivate();
};

} // namespace tb::ui
