/*
 Copyright (C) 2026 Kristian Duske

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

#include "gl/MockGl.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{

TEST_CASE("MockGl")
{
  auto gl = MockGl{};

  SECTION("a call with no slot assigned throws MockGlUnexpectedCall")
  {
    CHECK_THROWS_AS(gl.clear(0), MockGlUnexpectedCall);
  }

  SECTION("a call with a slot assigned delegates to it")
  {
    auto capturedMask = GLbitfield{0};
    gl.onClear = [&](const GLbitfield mask) { capturedMask = mask; };

    gl.clear(0x1234);
    CHECK(capturedMask == 0x1234u);
  }

  SECTION("a slot's return value is passed through")
  {
    gl.onCreateProgram = []() { return GLuint{42}; };
    CHECK(gl.createProgram() == 42u);
  }

  SECTION("each function is backed by its own independent slot")
  {
    gl.onClear = [](GLbitfield) {};

    // onClearColor was never assigned, so clearColor must still throw even though
    // clear (a different function) has a slot
    CHECK_THROWS_AS(gl.clearColor(0.0f, 0.0f, 0.0f, 0.0f), MockGlUnexpectedCall);
  }
}

} // namespace tb::gl
