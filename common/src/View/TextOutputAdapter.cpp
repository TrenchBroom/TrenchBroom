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

#include <QTextEdit>

namespace TrenchBroom {
    namespace View {
        TextOutputAdapter::TextOutputAdapter(QTextEdit* textEdit) :
        m_textEdit(textEdit),
        m_lastNewLine(0) {
            ensure(m_textEdit != nullptr, "textEdit is null");
        }

        void TextOutputAdapter::appendString(const String& str) {
            const auto cStr = compressString(str);
            if (!cStr.empty()) {
                DisableWindowUpdates disableUpdates(m_textEdit);

                size_t l = 0;
                for (size_t i = 0; i < cStr.length(); ++i) {
                    const auto c = cStr[i];
                    const auto n = i < cStr.length() - 1 ? cStr[i + 1] : 0;
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
                        const auto text = cStr.substr(l, i - l + 1);
                        appendToTextEdit(text);
                        m_lastNewLine = m_textEdit->textCursor().position();
                        l = i+1;
                    }
                }
                appendToTextEdit(cStr.substr(l));
            }
        }

        String TextOutputAdapter::compressString(const String& str) {
            String fullStr = m_remainder + str;
            StringStream result;
            size_t chunkStart = 0;
            size_t previousChunkStart = 0;
            for (size_t i = 0; i < fullStr.length(); ++i) {
                const auto c = fullStr[i];
                const auto n = i < fullStr.length() - 1 ? fullStr[i+1] : 0;
                if (c == '\r' && n == '\n') {
                    continue;
                } else if (c == '\r') {
                    previousChunkStart = chunkStart;
                    chunkStart = i;
                } else if (c == '\n') {
                    result << fullStr.substr(chunkStart, i - chunkStart + 1);
                    chunkStart = previousChunkStart = i+1;
                }
            }
            if (previousChunkStart < chunkStart) {
                const auto chunk = fullStr.substr(previousChunkStart, chunkStart - previousChunkStart);
                result << chunk;
            }
            m_remainder = fullStr.substr(chunkStart);
            return result.str();
        }

        void TextOutputAdapter::appendToTextEdit(const String& str) {
            QTextCursor cursor(m_textEdit->document());
            cursor.clearSelection();
            cursor.movePosition(QTextCursor::MoveOperation::End);
            cursor.insertText(QString::fromStdString(str));
            cursor.movePosition(QTextCursor::MoveOperation::End);
            m_textEdit->ensureCursorVisible();
        }
    }
}
