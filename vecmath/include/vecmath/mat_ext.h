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

#ifndef TRENCHBROOM_MAT_EXT_H
#define TRENCHBROOM_MAT_EXT_H

#include "vec.h"
#include "mat.h"

#include <vector>

namespace vm {

    /**
     * Multiplies the given list of vectors with the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the list of vectors
     * @param rhs the matrix
     * @return a list of the the products of the given vectors and the given matrix
     */
    template <typename T, size_t R, size_t C>
    std::vector<vec<T,R>> operator*(const std::vector<vec<T,R>>& lhs, const mat<T,R,C>& rhs) {
        std::vector<vec<T,R>> result;
        result.reserve(lhs.size());
        std::transform(std::begin(lhs), std::end(lhs), std::back_inserter(result), [rhs](const vec<T,R>& elem) { return elem * rhs; });
        return result;
    }

    /**
     * Multiplies the given list of vectors with the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the list of vectors
     * @param rhs the matrix
     * @return a list of the the products of the given vectors and the given matrix
     */
    template <typename T, size_t R, size_t C>
    std::vector<vec<T,R-1>> operator*(const std::vector<vec<T,R-1>>& lhs, const mat<T,R,C>& rhs) {
        std::vector<vec<T,R-1>> result;
        result.reserve(lhs.size());
        std::transform(std::begin(lhs), std::end(lhs), std::back_inserter(result), [rhs](const vec<T,R-1>& elem) { return elem * rhs; });
        return result;
    }

    /**
     * Multiplies the given list of vectors with the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the list of vectors
     * @param rhs the matrix
     * @return a list of the the products of the given vectors and the given matrix
     */
    template <typename T, size_t R, size_t C>
    std::vector<vec<T,C>> operator*(const mat<T,R,C>& lhs, const std::vector<vec<T,C>>& rhs) {
        std::vector<vec<T,C>> result;
        result.reserve(rhs.size());
        std::transform(std::begin(rhs), std::end(rhs), std::back_inserter(result), [lhs](const vec<T,C>& elem) { return lhs * elem; });
        return result;
    }

    /**
     * Multiplies the given list of vectors with the given matrix.
     *
     * @tparam T the component type
     * @tparam R the number of rows
     * @tparam C the number of columns
     * @param lhs the list of vectors
     * @param rhs the matrix
     * @return a list of the the products of the given vectors and the given matrix
     */
    template <typename T, size_t R, size_t C>
    std::vector<vec<T,C-1>> operator*(const mat<T,R,C>& lhs, const std::vector<vec<T,C-1>>& rhs) {
        std::vector<vec<T,C-1>> result;
        result.reserve(rhs.size());
        std::transform(std::begin(rhs), std::end(rhs), std::back_inserter(result), [lhs](const vec<T,C-1>& elem) { return lhs * elem; });
        return result;
    }}

#endif //TRENCHBROOM_MAT_EXT_H
