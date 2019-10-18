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

#ifndef TrenchBroom_VectorUtilsMinimal_h
#define TrenchBroom_VectorUtilsMinimal_h

#include <vector>
// NOTE: No other includes allowed

namespace VectorUtils {
    template <typename T>
    void clearToZero(std::vector<T>& vec) {
        vec.clear();
        vec.shrink_to_fit();
    }

    template <typename T1, typename T2>
    void append(std::vector<T1>& vec1, const std::vector<T2>& vec2) {
        vec1.reserve(vec1.size() + vec2.size());
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
    }
}

#endif
