/*
 Copyright 2021 Kristian Duske

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

#include <vecmath/bezier_surface.h>
#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <array>
#include <tuple>

#include <catch2/catch.hpp>

namespace vm {
TEST_CASE("evaluate_quadratic_bezier_surface") {
  using T = std::tuple<std::array<vec3d, 9>, double, double, vec3d>;

  // clang-format off
  const auto 
  [ points,                                             u,   v,   expected       ] = GENERATE(values<T>({
  // the surface reproduces the control points in the corners
  { { vec3d{0, 0, 0}, vec3d{1, 0, 1}, vec3d{2, 0, 0},
      vec3d{0, 1, 1}, vec3d{1, 1, 2}, vec3d{2, 1, 1},
      vec3d{0, 2, 0}, vec3d{1, 2, 1}, vec3d{2, 2, 0} }, 0.0, 0.0, vec3d{0, 0, 0} },
  { { vec3d{0, 0, 0}, vec3d{1, 0, 1}, vec3d{2, 0, 0},
      vec3d{0, 1, 1}, vec3d{1, 1, 2}, vec3d{2, 1, 1},
      vec3d{0, 2, 0}, vec3d{1, 2, 1}, vec3d{2, 2, 0} }, 1.0, 0.0, vec3d{2, 0, 0} },
  { { vec3d{0, 0, 0}, vec3d{1, 0, 1}, vec3d{2, 0, 0},
      vec3d{0, 1, 1}, vec3d{1, 1, 2}, vec3d{2, 1, 1},
      vec3d{0, 2, 0}, vec3d{1, 2, 1}, vec3d{2, 2, 0} }, 0.0, 1.0, vec3d{0, 2, 0} },
  { { vec3d{0, 0, 0}, vec3d{1, 0, 1}, vec3d{2, 0, 0},
      vec3d{0, 1, 1}, vec3d{1, 1, 2}, vec3d{2, 1, 1},
      vec3d{0, 2, 0}, vec3d{1, 2, 1}, vec3d{2, 2, 0} }, 1.0, 1.0, vec3d{2, 2, 0} },

  // sample the remaining control points
  { { vec3d{0, 0, 0}, vec3d{1, 0, 1}, vec3d{2, 0, 0},
      vec3d{0, 1, 1}, vec3d{1, 1, 2}, vec3d{2, 1, 1},
      vec3d{0, 2, 0}, vec3d{1, 2, 1}, vec3d{2, 2, 0} }, 0.5, 0.0, vec3d{1, 0, 0.5} },
  { { vec3d{0, 0, 0}, vec3d{1, 0, 1}, vec3d{2, 0, 0},
      vec3d{0, 1, 1}, vec3d{1, 1, 2}, vec3d{2, 1, 1},
      vec3d{0, 2, 0}, vec3d{1, 2, 1}, vec3d{2, 2, 0} }, 0.0, 0.5, vec3d{0, 1, 0.5} },
  { { vec3d{0, 0, 0}, vec3d{1, 0, 1}, vec3d{2, 0, 0},
      vec3d{0, 1, 1}, vec3d{1, 1, 2}, vec3d{2, 1, 1},
      vec3d{0, 2, 0}, vec3d{1, 2, 1}, vec3d{2, 2, 0} }, 1.0, 0.5, vec3d{2, 1, 0.5} },
  { { vec3d{0, 0, 0}, vec3d{1, 0, 1}, vec3d{2, 0, 0},
      vec3d{0, 1, 1}, vec3d{1, 1, 2}, vec3d{2, 1, 1},
      vec3d{0, 2, 0}, vec3d{1, 2, 1}, vec3d{2, 2, 0} }, 0.5, 1.0, vec3d{1, 2, 0.5} },
  { { vec3d{0, 0, 0}, vec3d{1, 0, 1}, vec3d{2, 0, 0},
      vec3d{0, 1, 1}, vec3d{1, 1, 2}, vec3d{2, 1, 1},
      vec3d{0, 2, 0}, vec3d{1, 2, 1}, vec3d{2, 2, 0} }, 0.5, 0.5, vec3d{1, 1, 1} },
  }));
  // clang-format on

  CAPTURE(points, u, v);

  // clang-format off
  const auto controlPoints = std::array<std::array<vm::vec3d, 3>, 3>{
    std::array<vm::vec3d, 3>{ points[0], points[1], points[2] },
    std::array<vm::vec3d, 3>{ points[3], points[4], points[5] },
    std::array<vm::vec3d, 3>{ points[6], points[7], points[8] },
  };
  // clang-format on

  CHECK(evaluate_quadratic_bezier_surface(controlPoints, u, v) == expected);
}
} // namespace vm
