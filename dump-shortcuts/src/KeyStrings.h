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

#ifndef wxKeyStrings_h
#define wxKeyStrings_h

#include <Qt>
#include <QString>

#include <ostream>
#include <unordered_map>

namespace TrenchBroom {
    namespace View {
        class KeyStrings {
        private:
            using KeyMap = std::unordered_map<Qt::Key, QString>;
        public:
            using const_iterator = typename KeyMap::const_iterator;
        private:
            KeyMap m_keys;
            QString m_separator;
        public:
            KeyStrings();

            const_iterator begin() const;
            const_iterator end() const;
        private:
            void putKey(Qt::Key key);
        };
    }
}

#endif /* wxKeyStrings_h */
