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
#include "View/wxUtils.h"

#include <QTextEdit>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TextOutputAdapter::TextOutputAdapter(QTextEdit* textEdit) :
        m_textEdit(textEdit),
        m_lastNewLine(0) {
            ensure(m_textEdit != nullptr, "textEdit is null");
        }

        void TextOutputAdapter::appendString(const QString& str) {
            const auto cStr = compressString(str);
            if (!str.isEmpty()) {
                DisableWindowUpdates disableUpdates(m_textEdit);

                auto l = 0;
                for (int i = 0; i < cStr.length(); ++i) {
                    const auto c = cStr[i];
                    const auto n = i < str.length() - 1 ? str[i + 1] : QChar(0);
                    if (c == '\r' && n == '\n') {
                        continue;
                    } else if (c == '\r') {
                        const auto from = m_lastNewLine;
                        const auto to = m_textEdit->textCursor().position();

                        QTextCursor cursor(m_textEdit->document());
                        cursor.clearSelection();
                        cursor.setPosition(from, QTextCursor::MoveAnchor);
                        cursor.setPosition(to, QTextCursor::KeepAnchor);
                        cursor.removeSelectedText();
                        l = i;
                    } else if (c == '\n') {
                        const auto text = str.mid(l, i - l + 1);
                        appendToTextEdit(text + '\n');
                    }
                }
                appendToTextEdit(str.mid(l));
            }
        }

        QString TextOutputAdapter::compressString(const QString& str) {
            QString fullStr = m_remainder + str;
            QString result;
            int chunkStart = 0;
            int previousChunkStart = 0;
            for (int i = 0; i < fullStr.length(); ++i) {
                const QChar c = fullStr[i];
                const QChar n = i < fullStr.length() - 1 ? fullStr[i+1] : QChar(0);
                if (c == '\r' && n == '\n') {
                    continue;
                } else if (c == '\r') {
                    previousChunkStart = chunkStart;
                    chunkStart = i;
                } else if (c == '\n') {
                    result += fullStr.mid(chunkStart, i - chunkStart + 1);
                    chunkStart = previousChunkStart = i+1;
                }
            }
            if (previousChunkStart < chunkStart) {
                const QString chunk = fullStr.mid(previousChunkStart, chunkStart - previousChunkStart);
                result += chunk;
            }
            m_remainder = fullStr.mid(chunkStart);
            return result;
        }

        void TextOutputAdapter::appendToTextEdit(const QString& str) {
            QTextCursor cursor(m_textEdit->document());
            cursor.clearSelection();
            cursor.movePosition(QTextCursor::MoveOperation::End);
            cursor.insertText(str);
            cursor.movePosition(QTextCursor::MoveOperation::End);
            m_textEdit->ensureCursorVisible();
        }
    }
}
