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

#include <QTextCursor>

#include <sstream>
#include <string>

class QTextEdit;
class QString;

namespace TrenchBroom {
    namespace View {
        /**
         * Helper for displaying the output of a command line tool in QTextEdit.
         *
         * - Interprets CR and LF control characters.
         * - Scroll bar follows output, unless it's manually raised.
         */
        class TextOutputAdapter {
        private:
            QTextEdit* m_textEdit;
            QTextCursor m_insertionCursor;
        public:
            explicit TextOutputAdapter(QTextEdit* textEdit);

            /**
             * Appends the given value to the text widget.
             * Objects are formatted using std::stringstream.
             * 8-bit to Unicode conversion is performed with QString::fromLocal8Bit.
             */
            template <typename T>
            TextOutputAdapter& operator<<(const T& t) {
                append(t);
                return *this;
            }
        private:
            template <typename T>
            void append(const T& t) {
                std::stringstream s;
                s << t;
                appendStdString(s.str());
            }

            void appendStdString(const std::string& string);
            void appendString(const QString& string);
        };
    }
}

#endif /* TextCtrlOutputAdapter_h */
