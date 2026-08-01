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

#include "render/BrushRenderer.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

TEST_CASE("BrushRenderer")
{
  SECTION("triIndicesCountForPolygon")
  {
    SECTION("a triangle needs 3 indices (1 triangle)")
    {
      CHECK(triIndicesCountForPolygon(3) == 3u);
    }

    SECTION("a quad needs 6 indices (2 triangles)")
    {
      CHECK(triIndicesCountForPolygon(4) == 6u);
    }

    SECTION("a pentagon needs 9 indices (3 triangles)")
    {
      CHECK(triIndicesCountForPolygon(5) == 9u);
    }
  }

  SECTION("addTriIndicesForPolygon")
  {
    SECTION("a triangle produces a single triangle referencing all three vertices")
    {
      auto dest = std::vector<GLuint>(triIndicesCountForPolygon(3));
      addTriIndicesForPolygon(dest.data(), 0, 3);
      CHECK(dest == std::vector<GLuint>{0, 1, 2});
    }

    SECTION("a quad produces a fan of two triangles sharing the base vertex")
    {
      auto dest = std::vector<GLuint>(triIndicesCountForPolygon(4));
      addTriIndicesForPolygon(dest.data(), 0, 4);
      CHECK(dest == std::vector<GLuint>{0, 1, 2, 0, 2, 3});
    }

    SECTION("a pentagon produces a fan of three triangles sharing the base vertex")
    {
      auto dest = std::vector<GLuint>(triIndicesCountForPolygon(5));
      addTriIndicesForPolygon(dest.data(), 0, 5);
      CHECK(dest == std::vector<GLuint>{0, 1, 2, 0, 2, 3, 0, 3, 4});
    }

    SECTION("indices are offset by the given base index")
    {
      auto dest = std::vector<GLuint>(triIndicesCountForPolygon(4));
      addTriIndicesForPolygon(dest.data(), 10, 4);
      CHECK(dest == std::vector<GLuint>{10, 11, 12, 10, 12, 13});
    }
  }
}

} // namespace tb::render
