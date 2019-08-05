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

#ifndef TrenchBroom_KeyboardShortcut
#define TrenchBroom_KeyboardShortcut

#include <QKeySequence>
#include <optional-lite/optional.hpp>

class QKeyEvent;

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut {
        private:
            int m_qtKey;
        public:
            explicit KeyboardShortcut(int qtKey = 0);
            explicit KeyboardShortcut(const QKeySequence& keySequence);

            friend bool operator==(const KeyboardShortcut& lhs, const KeyboardShortcut& rhs);

            QKeySequence keySequence() const;

            static nonstd::optional<KeyboardShortcut> fromV1Settings(const QString& string);
            QString toV1Settings() const;
        };

        int wxKeyToQt(int wxKey);
        int qtKeyToWx(int qtKey);
    }
}

#endif /* defined(TrenchBroom_KeyboardShortcut) */
