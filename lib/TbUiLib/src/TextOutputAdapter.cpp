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

#include "ui/TextOutputAdapter.h"

#include <QByteArray>
#include <QScrollBar>
#include <QString>
#include <QTextEdit>

#include "kd/contracts.h"

namespace tb::ui
{

TextOutputAdapter::TextOutputAdapter(QTextEdit* textEdit)
  : m_textEdit{textEdit}
  , m_insertionCursor{m_textEdit->document()}
{
  contract_pre(m_textEdit != nullptr);

  // Create our own private cursor, separate from the UI cursor
  // so user selections don't interfere with our text insertions
  m_insertionCursor.movePosition(QTextCursor::End);
}

void TextOutputAdapter::appendString(const QString& string)
{
  auto* scrollBar = m_textEdit->verticalScrollBar();
  const auto wasAtBottom = (scrollBar->value() >= scrollBar->maximum());

  const auto size = string.size();
  for (int i = 0; i < size; ++i)
  {
    const auto c = string[i];
    const auto n = (i + 1) < size ? string[i + 1] : static_cast<QChar>(0);

    // Handle CRLF by advancing to the LF, which is handled below
    if (c == '\r' && n == '\n')
    {
      continue;
    }
    // Handle LF
    if (c == '\n')
    {
      m_insertionCursor.movePosition(QTextCursor::End);
      m_insertionCursor.insertBlock();
      continue;
    }
    // Handle CR, next character not LF
    if (c == '\r')
    {
      m_insertionCursor.movePosition(QTextCursor::StartOfLine);
      continue;
    }

    // Insert characters from index i, up to but excluding the next
    // CR or LF, as a literal string
    auto lastToInsert = i;
    for (int j = i; j < size; ++j)
    {
      const auto charJ = string[j];
      if (charJ == '\r' || charJ == '\n')
      {
        break;
      }
      lastToInsert = j;
    }

    const auto insertionSize = lastToInsert - i + 1;
    const auto substring = string.mid(i, insertionSize);
    if (!m_insertionCursor.atEnd())
    {
      // This means a CR was previously used. We need to select
      // the same number of characters as we're inserting, so the
      // text is overwritten.
      m_insertionCursor.movePosition(
        QTextCursor::NextCharacter, QTextCursor::KeepAnchor, insertionSize);
    }
    m_insertionCursor.insertText(substring);
    i = lastToInsert;
  }

  if (wasAtBottom)
  {
    m_textEdit->verticalScrollBar()->setValue(m_textEdit->verticalScrollBar()->maximum());
  }
}

} // namespace tb::ui
