/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "LimitedKeySequenceEdit.h"

#include <QKeyEvent>

namespace TrenchBroom
{
namespace View
{
LimitedKeySequenceEdit::LimitedKeySequenceEdit(QWidget* parent)
  : LimitedKeySequenceEdit(MaxCount, parent)
{
}

LimitedKeySequenceEdit::LimitedKeySequenceEdit(const size_t maxCount, QWidget* parent)
  : QKeySequenceEdit(parent)
  , m_maxCount(maxCount)
  , m_count(0)
{
  Q_ASSERT(m_maxCount <= MaxCount);
  connect(
    this, &QKeySequenceEdit::editingFinished, this, &LimitedKeySequenceEdit::resetCount);
}

void LimitedKeySequenceEdit::keyPressEvent(QKeyEvent* event)
{
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
} // namespace View
} // namespace TrenchBroom
