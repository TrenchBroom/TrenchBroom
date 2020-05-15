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

#include "TextOutputAdapter.h"

#include "Ensure.h"
#include "View/QtUtils.h"

#include <sstream>
#include <string>

#include <QTextEdit>
#include <QScrollBar>

namespace TrenchBroom {
    namespace View {
        TextOutputAdapter::TextOutputAdapter(QTextEdit* textEdit) {
            ensure(textEdit != nullptr, "textEdit is null");
            m_textEdit = textEdit;
            m_textDocument = textEdit->document();

            m_insertionCursor = QTextCursor(m_textDocument);
            m_insertionCursor.movePosition(QTextCursor::End);
        }
        
        void TextOutputAdapter::appendString(const QString& string) {
            QScrollBar* bar = m_textEdit->verticalScrollBar();
            const bool wasAtBottom = (bar->value() >= bar->maximum());
            qDebug() << "was at bot " << wasAtBottom;

            const int size = string.size();

            for (int i = 0; i < size; ++i) {
                const QChar c = string[i];
                const QChar n = (i + 1) < size ? string[i + 1] : static_cast<QChar>(0);

                // handle CRLF by advancing to the LF, which is handled below
                if (c == '\r' && n == '\n') {                    
                    continue;
                }
                // handle LF
                if (c == '\n') {
                    m_insertionCursor.movePosition(QTextCursor::End);
                    m_insertionCursor.insertBlock();
                    continue;
                }
                // handle CR, next character not LF
                if (c == '\r') {
                    m_insertionCursor.movePosition(QTextCursor::StartOfLine);
                    continue;
                }

                // insert a literal string
                int last = i;
                for (int j = i; j < size; ++j) {
                    const QChar charJ = string[j];
                    if (charJ != '\r' && charJ != '\n') {
                        last = j;
                    } else {
                        break;
                    }
                }
                const int insertionSize = last - i + 1;
                const QString substr = string.mid(i, insertionSize);
                if (!m_insertionCursor.atEnd()) {
                    m_insertionCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, insertionSize);
                }
                m_insertionCursor.insertText(substr);
                i = last;
            }

            if (wasAtBottom) {
                m_textEdit->verticalScrollBar()->setValue(m_textEdit->verticalScrollBar()->maximum());
                //m_textEdit->moveCursor(QTextCursor::End);
                //m_textEdit->ensureCursorVisible();
            }
        }
    }
}
