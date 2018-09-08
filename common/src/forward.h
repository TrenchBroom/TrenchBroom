/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation,either version 3 of the License,or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not,see <http://www.gnu.org/licenses/>.
 */

#ifndef TRENCHBROOM_FORWARD_H
#define TRENCHBROOM_FORWARD_H

#include <cstddef>

namespace vm {
    template<typename T, size_t S>
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

    template<typename T, size_t R, size_t C>
    class mat;

    using mat2x2f = mat<float,2,2>;
    using mat3x3f = mat<float,3,3>;
    using mat4x4f = mat<float,4,4>;
    using mat2x2d = mat<double,2,2>;
    using mat3x3d = mat<double,3,3>;
    using mat4x4d = mat<double,4,4>;

    template<typename T>
    class quat;

    using quatf = quat<float>;
    using quatd = quat<double>;

    template<typename T, size_t S>
    class bbox;

    using bbox1f = bbox<float,1>;
    using bbox1d = bbox<double,1>;
    using bbox2f = bbox<float,2>;
    using bbox2d = bbox<double,2>;
    using bbox3f = bbox<float,3>;
    using bbox3d = bbox<double,3>;

    template<typename T, size_t S>
    class line;

    using line3f = line<float,3>;
    using line3d = line<double,3>;

    template<typename T, size_t S>
    class plane;

    using plane3f = plane<float,3>;
    using plane3d = plane<double,3>;

    template<typename T, size_t S>
    class ray;

    using ray3f = ray<float,3>;
    using ray3d = ray<double,3>;

    template<typename T, size_t S>
    class segment;

    using segment3d = segment<double,3>;
    using segment3f = segment<float,3>;
    using segment2d = segment<double,2>;
    using segment2f = segment<float,2>;

    template<typename T, size_t S>
    class polygon;

    using polygon2f = polygon<float,2>;
    using polygon2d = polygon<double,2>;
    using polygon3f = polygon<float,3>;
    using polygon3d = polygon<double,3>;
}

#endif //TRENCHBROOM_FORWARD_H
