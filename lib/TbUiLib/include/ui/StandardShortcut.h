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

#include "base/KeySequence.h"

namespace tb::ui
{

/**
 * Keyboard shortcuts whose platform-conventional binding is resolved by the UI
 * toolkit at runtime (e.g. Cmd+Z vs. Ctrl+Z for Undo).
 */
enum class StandardShortcut
{
  New,
  Open,
  Save,
  SaveAs,
  Close,
  Undo,
  Redo,
  Cut,
  Copy,
  Paste,
  SelectAll,
  Delete,
  Preferences,
  HelpContents,
};

KeySequence standardShortcut(StandardShortcut shortcut);

} // namespace tb::ui
