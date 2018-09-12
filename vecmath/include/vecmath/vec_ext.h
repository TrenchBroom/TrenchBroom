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

#ifndef TRENCHBROOM_VEC_EXT_H
#define TRENCHBROOM_VEC_EXT_H

#include "vec.h"

#include <vector>

namespace vm {
    /**
 * Adds the given vector to each of the vectors in the given range.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the range of vectors
 * @param rhs the right hand vector
 * @return a range containing the sum of each of the vectors in the given range with the right hand vector
 */
    template <typename T, size_t S>
    std::vector<vec<T,S>> operator+(const std::vector<vec<T,S>>& lhs, const vec<T,S>& rhs) {
        std::vector<vec<T,S>> result;
        result.reserve(lhs.size());
        for (const auto& vec : lhs) {
            result.push_back(vec + rhs);
        }
        return result;
    }

    /**
     * Adds the given vector to each of the vectors in the given range.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param lhs the left hand vector
     * @param rhs the range of vectors
     * @return a range containing the sum of each of the vectors in the given range with the left hand vector
     */
    template <typename T, size_t S>
    std::vector<vec<T,S>> operator+(const vec<T,S>& lhs, const std::vector<vec<T,S>>& rhs) {
        return rhs + lhs;
    }

    /**
     * Multiplies each vector in the given range by the given scalar.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param lhs the range of vectors
     * @param rhs the scalar factor
     * @return a range containing the scalar product of each vector in the given range with the given scalar
     */
    template <typename T, size_t S>
    std::vector<vec<T,S>> operator*(const std::vector<vec<T,S>>& lhs, const T rhs) {
        std::vector<vec<T,S>> result;
        result.reserve(lhs.size());
        for (const auto& vec : lhs) {
            result.push_back(vec + rhs);
        }
        return result;
    }

    /**
     * Multiplies each vector in the given range by the given scalar.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param lhs the scalar factor
     * @param rhs the range of vectors
     * @return a range containing the scalar product of each vector in the given range with the given scalar
     */
    template <typename T, size_t S>
    std::vector<vec<T,S>> operator*(const T lhs, const std::vector<vec<T,S>>& rhs) {
        return rhs * lhs;
    }
}

#endif //TRENCHBROOM_VEC_EXT_H
