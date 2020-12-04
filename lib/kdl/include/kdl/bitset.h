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

#pragma once

#include <vector>

namespace kdl {
    class bitset {
    private:
        std::vector<bool> m_bits;
    public:
        bitset(const size_t initialSize = 64) :
        m_bits(initialSize, false) {}

        bool operator[](const size_t index) const {
            if (index >= m_bits.size())
                return false;
            return m_bits[index];
        }

        std::vector<bool>::reference operator[](const size_t index) {
            if (index >= m_bits.size()) {
                m_bits.insert(std::end(m_bits), index - m_bits.size() + 1, false);
            }
            return m_bits[index];
        }

        void reset() {
            m_bits = std::vector<bool>(64, false);
        }
    };
}

#endif
