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

#include "ui/LimitedKeySequenceEdit.h"

#include <QKeyEvent>
#include <QKeySequence>

#include "ui/KeyboardShortcutUtils.h"

#include <algorithm>

namespace tb::ui
{

LimitedKeySequenceEdit::LimitedKeySequenceEdit(QWidget* parent)
  : QKeySequenceEdit{parent}
{
  connect(
    this, &QKeySequenceEdit::editingFinished, this, &LimitedKeySequenceEdit::resetCount);
}

void LimitedKeySequenceEdit::setMaxCount(size_t maxCount)
{
  m_maxCount = std::min(maxCount, MaxCount);
}

void LimitedKeySequenceEdit::cancel()
{
  setKeySequence(QKeySequence{});
  emit keySequenceChanged(QKeySequence{});
  emit editingFinished();
}

void LimitedKeySequenceEdit::keyPressEvent(QKeyEvent* event)
{
  if (!isSupportedShortcut(QKeySequence{event->keyCombination()}))
  {
    event->accept();
    return;
  }

  QKeySequenceEdit::keyPressEvent(event);
  if (event->modifiers() == Qt::NoModifier)
  {
    ++m_count;
    if (m_count == m_maxCount)
    {
      // will call resetState and thereby clear the timer
      setKeySequence(keySequence());
      emit keySequenceChanged(keySequence());
      emit editingFinished();
    }
  }
}

void LimitedKeySequenceEdit::resetCount()
{
  m_count = 0;
}

} // namespace tb::ui
