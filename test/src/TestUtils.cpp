/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "TestUtils.h"

#include <gmock/gmock.h>

namespace TrenchBroom {
    bool texCoordsEqual(const Vec2f& tc1, const Vec2f& tc2) {
        for (size_t i = 0; i < 2; ++i) {
            const float d1 = tc1[i] - Math::roundDownToMultiple(tc1[i], 1.0f);
            const float c1 = d1 < 0.0f ? 1.0f + d1 : d1;
            const float d2 = tc2[i] - Math::roundDownToMultiple(tc2[i], 1.0f);
            const float c2 = d2 < 0.0f ? 1.0f + d2 : d2;
            if (!Math::eq(c1, c2))
                return false;
        }
        return true;
    }
}
