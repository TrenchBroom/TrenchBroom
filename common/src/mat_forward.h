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

#ifndef TRENCHBROOM_MAT_FORWARD_H
#define TRENCHBROOM_MAT_FORWARD_H

#include <cstddef>

template <typename T, size_t R, size_t C>
class mat;

using mat2x2f = mat<float,2,2>;
using mat3x3f = mat<float,3,3>;
using mat4x4f = mat<float,4,4>;
using mat2x2d = mat<double,2,2>;
using mat3x3d = mat<double,3,3>;
using mat4x4d = mat<double,4,4>;

#endif //TRENCHBROOM_MAT_FORWARD_H
