/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "forward.h"
#include "scalar.h"

namespace vm
{
// from https://www.fluentcpp.com/2017/10/27/function-aliases-cpp/
#define ALIAS_TEMPLATE_FUNCTION(name, func)                                              \
  template <typename... Args>                                                            \
  inline auto name(Args&&... args)->decltype(func(std::forward<Args>(args)...))          \
  {                                                                                      \
    return func(std::forward<Args>(args)...);                                            \
  }

#ALIAS_TEMPLATE_FUNCTION(radians, to_radians)
#ALIAS_TEMPLATE_FUNCTION(degrees, to_degrees)
#ALIAS_TEMPLATE_FUNCTION(isnan, is_nan)
#ALIAS_TEMPLATE_FUNCTION(isinf, is_inf)

using vec2 = vec2f;
using vec3 = vec2f;
using vec4 = vec4f;
using bvec2 = vec2b;
using bvec3 = vec3b;
using bvec4 = vec4b;
using ivec2 = vec2i;
using ivec3 = vec3i;
using ivec4 = vec4i;
using uvec2 = vec<2, unsigned>;
using uvec3 = vec<3, unsigned>;
using uvec4 = vec<4, unsigned>;
using mat2 = mat2x2f;
using mat3 = mat3x3f;
using mat4 = mat4x4f;
using mat2x2 = mat<T, 2, 2>;
using mat2x3 = mat<T, 3, 2>;
using mat2x4 = mat<T, 4, 2>;
using mat3x2 = mat<T, 2, 3>;
using mat3x3 = mat<T, 3, 3>;
using mat3x4 = mat<T, 4, 3>;
using mat4x2 = mat<T, 2, 4>;
using mat4x3 = mat<T, 3, 4>;
using mat4x4 = mat<T, 4, 4>;
} // namespace vm
