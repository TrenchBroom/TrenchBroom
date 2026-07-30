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

#include "gl/IndexRangeMap.h"
#include "gl/MockGl.h"
#include "gl/PrimType.h"
#include "gl/ShaderProgram.h"
#include "gl/TestUtils.h"
#include "gl/VboManager.h"
#include "gl/VertexArray.h"
#include "gl/VertexType.h"

#include "vm/vec.h"

#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{
namespace
{

using Primitive = std::tuple<PrimType, size_t, size_t>;

std::vector<Primitive> collectPrimitives(const IndexRangeMap& map)
{
  auto result = std::vector<Primitive>{};
  map.forEachPrimitive(
    [&](const PrimType primType, const size_t index, const size_t count) {
      result.emplace_back(primType, index, count);
    });
  return result;
}

} // namespace

TEST_CASE("IndexRangeMap")
{
  SECTION("a default-constructed map is empty")
  {
    CHECK(collectPrimitives(IndexRangeMap{}).empty());
  }

  SECTION("constructor with a single primitive")
  {
    const auto map = IndexRangeMap{PrimType::Triangles, 10, 3};
    CHECK(collectPrimitives(map) == std::vector<Primitive>{{PrimType::Triangles, 10, 3}});
  }

  SECTION("add(primType, index, count)")
  {
    SECTION("merges consecutive ranges of the same non-strip primitive type")
    {
      auto size = IndexRangeMap::Size{};
      size.inc(PrimType::Triangles, 2);

      auto map = IndexRangeMap{size};
      map.add(PrimType::Triangles, 0, 3);
      map.add(PrimType::Triangles, 3, 3); // contiguous, so it extends the first range

      CHECK(
        collectPrimitives(map) == std::vector<Primitive>{{PrimType::Triangles, 0, 6}});
    }

    SECTION("does not merge non-contiguous ranges of the same primitive type")
    {
      auto size = IndexRangeMap::Size{};
      size.inc(PrimType::Triangles, 2);

      auto map = IndexRangeMap{size};
      map.add(PrimType::Triangles, 0, 3);
      map.add(PrimType::Triangles, 10, 3); // not contiguous, so it is a separate range

      CHECK(
        collectPrimitives(map)
        == std::vector<Primitive>{
          {PrimType::Triangles, 0, 3}, {PrimType::Triangles, 10, 3}});
    }

    SECTION("never merges strip, fan, loop or polygon primitives")
    {
      auto size = IndexRangeMap::Size{};
      size.inc(PrimType::TriangleStrip, 2);

      auto map = IndexRangeMap{size};
      map.add(PrimType::TriangleStrip, 0, 3);
      // contiguous, but strips are never merged since each one is a separate strip
      map.add(PrimType::TriangleStrip, 3, 3);

      CHECK(
        collectPrimitives(map)
        == std::vector<Primitive>{
          {PrimType::TriangleStrip, 0, 3}, {PrimType::TriangleStrip, 3, 3}});
    }

    SECTION("with dynamic growth")
    {
      auto map = IndexRangeMap{};
      map.add(PrimType::Points, 0, 1);
      map.add(PrimType::Points, 5, 1);

      CHECK(
        collectPrimitives(map)
        == std::vector<Primitive>{{PrimType::Points, 0, 1}, {PrimType::Points, 5, 1}});
    }
  }

  SECTION("add(IndexRangeMap) merges another map's ranges into this one")
  {
    auto mapA = IndexRangeMap{};
    mapA.add(PrimType::Points, 0, 1);

    auto mapB = IndexRangeMap{};
    mapB.add(PrimType::Points, 5, 1);
    mapB.add(PrimType::Lines, 0, 2);

    mapA.add(mapB);

    CHECK(
      collectPrimitives(mapA)
      == std::vector<Primitive>{
        {PrimType::Points, 0, 1}, {PrimType::Points, 5, 1}, {PrimType::Lines, 0, 2}});
  }

  SECTION("Size::inc(other) accumulates sizes from another Size")
  {
    auto sizeA = IndexRangeMap::Size{};
    sizeA.inc(PrimType::Points, 1);

    auto sizeB = IndexRangeMap::Size{};
    sizeB.inc(PrimType::Points, 1);
    sizeB.inc(PrimType::Lines, 1);

    sizeA.inc(sizeB);

    // sizeA now reserves capacity for 2 Points ranges and 1 Lines range; adding
    // exactly that many non-contiguous ranges must not exceed the reserved capacity
    auto map = IndexRangeMap{sizeA};
    map.add(PrimType::Points, 0, 1);
    map.add(PrimType::Points, 10, 1);
    map.add(PrimType::Lines, 0, 2);

    CHECK(collectPrimitives(map).size() == 3);
  }

  SECTION("size() returns the capacity needed to reproduce the same ranges")
  {
    auto original = IndexRangeMap{};
    original.add(PrimType::Points, 0, 1);
    original.add(PrimType::Points, 10, 1); // not contiguous -> 2 separate ranges
    original.add(PrimType::Triangles, 0, 3);

    // constructing a new map from the recorded size and adding back the exact same
    // primitives must not exceed the reserved capacity
    auto copy = IndexRangeMap{original.size()};
    copy.add(PrimType::Points, 0, 1);
    copy.add(PrimType::Points, 10, 1);
    copy.add(PrimType::Triangles, 0, 3);

    CHECK(collectPrimitives(copy) == collectPrimitives(original));
  }

  SECTION("render draws every non-empty primitive type and skips the rest")
  {
    auto gl = MockGl{};
    installVboSupport(gl);
    gl.onVertexPointer = [](GLint, GLenum, GLsizei, const GLvoid*) {};
    gl.onEnableClientState = [](GLenum) {};
    gl.onDisableClientState = [](GLenum) {};

    auto vboManager = VboManager{};
    auto shaderProgram = ShaderProgram{"test", 1u};

    auto vertices = std::vector<VertexTypes::P3::Vertex>{
      VertexTypes::P3::Vertex{vm::vec3f{0, 0, 0}},
      VertexTypes::P3::Vertex{vm::vec3f{1, 0, 0}},
      VertexTypes::P3::Vertex{vm::vec3f{0, 1, 0}},
    };
    auto vertexArray = VertexArray::copy(vertices);
    vertexArray.prepare(gl, vboManager);
    REQUIRE(vertexArray.setup(gl, shaderProgram));

    auto map = IndexRangeMap{};
    map.add(PrimType::Triangles, 0, 3);
    map.add(PrimType::Points, 0, 1);

    auto drawnPrimTypes = std::vector<GLenum>{};
    gl.onMultiDrawArrays = [&](const GLenum mode, const GLint*, const GLsizei*, GLsizei) {
      drawnPrimTypes.push_back(mode);
    };

    map.render(gl, vertexArray);

    // only the two primitive types that were added are drawn, one multiDrawArrays
    // call each; every other (empty) primitive type is skipped
    CHECK(
      drawnPrimTypes
      == std::vector<GLenum>{toGL(PrimType::Points), toGL(PrimType::Triangles)});

    vertexArray.cleanup(gl, shaderProgram);
    vertexArray = VertexArray{};
    vboManager.destroyPendingVbos(gl);
  }
}

} // namespace tb::gl
