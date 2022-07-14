/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <cstddef>

namespace vm {
enum class side;
enum class direction;
enum class rotation_axis;
enum class plane_status;

template <typename T, size_t S> class vec;

using vec1f = vec<float, 1>;
using vec1d = vec<double, 1>;
using vec1i = vec<int, 1>;
using vec1l = vec<long, 1>;
using vec1s = vec<size_t, 1>;
using vec1b = vec<bool, 1>;

using vec2f = vec<float, 2>;
using vec2d = vec<double, 2>;
using vec2i = vec<int, 2>;
using vec2l = vec<long, 2>;
using vec2s = vec<size_t, 2>;
using vec2b = vec<bool, 2>;

using vec3f = vec<float, 3>;
using vec3d = vec<double, 3>;
using vec3i = vec<int, 3>;
using vec3l = vec<long, 3>;
using vec3s = vec<size_t, 3>;
using vec3b = vec<bool, 3>;

using vec4f = vec<float, 4>;
using vec4d = vec<double, 4>;
using vec4i = vec<int, 4>;
using vec4l = vec<long, 4>;
using vec4s = vec<size_t, 4>;
using vec4b = vec<bool, 4>;

template <typename T, size_t R, size_t C> class mat;

using mat2x2f = mat<float, 2, 2>;
using mat3x3f = mat<float, 3, 3>;
using mat4x4f = mat<float, 4, 4>;
using mat2x2d = mat<double, 2, 2>;
using mat3x3d = mat<double, 3, 3>;
using mat4x4d = mat<double, 4, 4>;

template <typename T> class quat;

using quatf = quat<float>;
using quatd = quat<double>;

template <typename T, size_t S> class bbox;

using bbox1f = bbox<float, 1>;
using bbox1d = bbox<double, 1>;
using bbox2f = bbox<float, 2>;
using bbox2d = bbox<double, 2>;
using bbox3f = bbox<float, 3>;
using bbox3d = bbox<double, 3>;

template <typename T, size_t S> class line;

using line3f = line<float, 3>;
using line3d = line<double, 3>;

template <typename T, size_t S> class plane;

using plane3f = plane<float, 3>;
using plane3d = plane<double, 3>;

template <typename T, size_t S> class ray;

using ray3f = ray<float, 3>;
using ray3d = ray<double, 3>;

template <typename T, size_t S> class segment;

using segment3d = segment<double, 3>;
using segment3f = segment<float, 3>;
using segment2d = segment<double, 2>;
using segment2f = segment<float, 2>;

template <typename T, size_t S> class polygon;

using polygon2f = polygon<float, 2>;
using polygon2d = polygon<double, 2>;
using polygon3f = polygon<float, 3>;
using polygon3d = polygon<double, 3>;
} // namespace vm
