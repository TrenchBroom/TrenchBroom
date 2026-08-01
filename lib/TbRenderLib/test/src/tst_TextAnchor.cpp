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

#include "gl/OrthographicCamera.h"
#include "render/TextAnchor.h"

#include "vm/vec.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

TEST_CASE("SimpleTextAnchor")
{
  auto camera = gl::OrthographicCamera{};
  const auto position = vm::vec3f{10, 20, 30};
  const auto size = vm::vec2f{40, 16};
  const auto base = camera.project(position);

  SECTION("position")
  {
    const auto anchor = SimpleTextAnchor{position, TextAlignment::Center};
    CHECK(anchor.position(camera) == position);
  }

  SECTION("offset")
  {
    SECTION("Center alignment (no horizontal or vertical bit set) centers the text")
    {
      const auto anchor = SimpleTextAnchor{position, TextAlignment::Center};
      const auto expected = base + vm::vec3f{vm::round(-size / 2.0f), 0.0f};
      CHECK(anchor.offset(camera, size) == expected);
    }

    SECTION("Left | Top anchors the text's left and top edges to the position")
    {
      const auto anchor =
        SimpleTextAnchor{position, TextAlignment::Left | TextAlignment::Top};
      const auto expected = base + vm::vec3f{0.0f, -size.y(), 0.0f};
      CHECK(anchor.offset(camera, size) == expected);
    }

    SECTION("Right | Bottom anchors the text's right and bottom edges to the position")
    {
      const auto anchor =
        SimpleTextAnchor{position, TextAlignment::Right | TextAlignment::Bottom};
      const auto expected = base + vm::vec3f{-size.x(), 0.0f, 0.0f};
      CHECK(anchor.offset(camera, size) == expected);
    }

    SECTION(
      "an axis with neither of its two bits set is centered on that axis, just like "
      "Center, even when the other axis is aligned")
    {
      const auto anchor = SimpleTextAnchor{position, TextAlignment::Left};
      const auto expected = base + vm::vec3f{0.0f, vm::round(-size.y() / 2.0f), 0.0f};
      CHECK(anchor.offset(camera, size) == expected);
    }

    SECTION("applies extraOffsets on top of the alignment")
    {
      const auto extra = vm::vec2f{3, -7};
      const auto anchor = SimpleTextAnchor{position, TextAlignment::Center, extra};
      const auto expected = base + vm::vec3f{vm::round(-size / 2.0f + extra), 0.0f};
      CHECK(anchor.offset(camera, size) == expected);
    }
  }
}

} // namespace tb::render
