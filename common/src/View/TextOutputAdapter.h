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

#include "StringUtils.h"

#include <QString>

#include <iostream>

class QTextEdit;

namespace TrenchBroom {
    namespace View {
        /**
         * Adapts a QTextEdit to the requirements of displaying the output of a command line tool, specifically
         * interpreting selected control characters.
         */
        class TextOutputAdapter {
        private:
            QTextEdit* m_textEdit;
            int m_lastNewLine;
            String m_remainder;
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
                StringStream str;
                str << t;
                appendString(str.str());
                return *this;
            }
        private:
            /**
             * Appends the given string. The string is first compressed, then the remainder is interpreted again in case
             * any control characters could not be interpreted without considering the previously appended strings. In
             * such a case, the contents of the text control are updated according to the control characters, and the
             * string itself is appended to the text widget.
             *
             * @param str the string to append
             */
            void appendString(const String& str);

            /**
             * Interprets some control characters in the given string line by line. If the string ends with an
             * unterminated line portion, then that remainder is stored in the member varable m_remainder. The next
             * time this function is invoked, the remainder is preprended to the given string.
             *
             * @param str the string to compress
             * @return the compressed string
             */
            String compressString(const String& str);

            /**
             * Appends the given string to the contents of the QTextEdit widget.
             *
             * @param str the string to append
             */
            void appendToTextEdit(const String& str);
        };
    }
}

#endif /* TextCtrlOutputAdapter_h */
