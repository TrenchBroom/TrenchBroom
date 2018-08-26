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


#ifndef TRENCHBROOM_VEC_FORWARD_H
#define TRENCHBROOM_VEC_FORWARD_H

#include <cstddef>

template <typename T, size_t S>
class vec;

using vec1f = vec<float,1>;
using vec1d = vec<double,1>;
using vec1i = vec<int,1>;
using vec1l = vec<long,1>;
using vec1s = vec<size_t,1>;
using vec1b = vec<bool,1>;

using vec2f = vec<float,2>;
using vec2d = vec<double,2>;
using vec2i = vec<int,2>;
using vec2l = vec<long,2>;
using vec2s = vec<size_t,2>;
using vec2b = vec<bool,2>;

using vec3f = vec<float,3>;
using vec3d = vec<double,3>;
using vec3i = vec<int,3>;
using vec3l = vec<long,3>;
using vec3s = vec<size_t,3>;
using vec3b = vec<bool,3>;

using vec4f = vec<float,4>;
using vec4d = vec<double,4>;
using vec4i = vec<int,4>;
using vec4l = vec<long,4>;
using vec4s = vec<size_t,4>;
using vec4b = vec<bool,4>;

#endif //TRENCHBROOM_VEC_FORWARD_H
