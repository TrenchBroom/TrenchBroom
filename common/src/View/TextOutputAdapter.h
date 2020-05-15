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

#ifndef TextCtrlOutputAdapter_h
#define TextCtrlOutputAdapter_h

#include <kdl/string_utils.h>

#include <QTextCursor>
#include <QString>
#include <QByteArray>

#include <string>

class QTextEdit;
class QStringRef;

namespace TrenchBroom {
    namespace View {
        /**
         * Adapts a QTextEdit to the requirements of displaying the output of a command line tool, specifically
         * interpreting selected control characters.
         */
        class TextOutputAdapter {
        private:
            QTextEdit* m_textEdit;
            QTextDocument* m_textDocument;
            QTextCursor m_insertionCursor;
        public:
            explicit TextOutputAdapter(QTextEdit* textEdit);

            /**
             * Appends the given value to the text widget.
             *
             * @tparam T the type of the value to append
             * @param t the value to append
             * @return a reference to this output adapter
             */
            template <typename T>
            TextOutputAdapter& operator<<(const T& t) {
                return append(t);
            }

            /**
             * Appends the given value to the text widget.
             *
             * @tparam T the type of the value to append
             * @param t the value to append
             * @return a reference to this output adapter
             */
            template <typename T>
            TextOutputAdapter& append(const T& t) {
                appendString(QString::fromLocal8Bit(QByteArray::fromStdString(kdl::str_to_string(t))));
                return *this;
            }
        private:
            void appendString(const QString& str);
        };
    }
}

#endif /* TextCtrlOutputAdapter_h */
