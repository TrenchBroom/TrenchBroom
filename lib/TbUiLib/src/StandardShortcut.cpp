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

#include "ui/StandardShortcut.h"

#include <QKeySequence>
#include <QtSystemDetection>

#include "base/Macros.h"
#include "ui/QKeySequenceUtils.h"

namespace tb::ui
{

KeySequence standardShortcut(const StandardShortcut shortcut)
{
  switch (shortcut)
  {
  case StandardShortcut::New:
    return fromQKeySequence(QKeySequence{QKeySequence::New});
  case StandardShortcut::Open:
    return fromQKeySequence(QKeySequence{QKeySequence::Open});
  case StandardShortcut::Save:
    return fromQKeySequence(QKeySequence{QKeySequence::Save});
  case StandardShortcut::SaveAs:
    return fromQKeySequence(QKeySequence{QKeySequence::SaveAs});
  case StandardShortcut::Close:
    return fromQKeySequence(QKeySequence{QKeySequence::Close});
  case StandardShortcut::Undo:
    return fromQKeySequence(QKeySequence{QKeySequence::Undo});
  case StandardShortcut::Redo:
    return fromQKeySequence(QKeySequence{QKeySequence::Redo});
  case StandardShortcut::Cut:
    return fromQKeySequence(QKeySequence{QKeySequence::Cut});
  case StandardShortcut::Copy:
    return fromQKeySequence(QKeySequence{QKeySequence::Copy});
  case StandardShortcut::Paste:
    return fromQKeySequence(QKeySequence{QKeySequence::Paste});
  case StandardShortcut::SelectAll:
    return fromQKeySequence(QKeySequence{QKeySequence::SelectAll});
  case StandardShortcut::Delete:
    // Qt's standard "Delete" key doesn't match what we want on macOS.
    return fromQKeySequence(QKeySequence{
#if defined(Q_OS_MACOS)
      Qt::Key_Backspace
#else
      QKeySequence::Delete
#endif
    });
  case StandardShortcut::Preferences:
    return fromQKeySequence(QKeySequence{QKeySequence::Preferences});
  case StandardShortcut::HelpContents:
    return fromQKeySequence(QKeySequence{QKeySequence::HelpContents});
    switchDefault();
  }
}

} // namespace tb::ui
