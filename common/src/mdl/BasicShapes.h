/*
 Copyright (C) 2010 Kristian Duske

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

#pragma once

#include "kd/contracts.h"

#include "vm/bbox.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <array>
#include <map>
#include <utility>
#include <vector>

namespace tb::mdl
{

void coordinateSystemVerticesX(
  const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end);
void coordinateSystemVerticesY(
  const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end);
void coordinateSystemVerticesZ(
  const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end);

std::vector<vm::vec2f> circle2D(float radius, size_t segments);
std::vector<vm::vec2f> circle2D(
  float radius, float startAngle, float angleLength, size_t segments);
std::vector<vm::vec3f> circle2D(
  float radius,
  vm::axis::type axis,
  float startAngle,
  float angleLength,
  size_t segments);
std::pair<float, float> startAngleAndLength(
  vm::axis::type axis, const vm::vec3f& startAxis, const vm::vec3f& endAxis);

size_t roundedRect2DVertexCount(size_t cornerSegments);
std::vector<vm::vec2f> roundedRect2D(
  const vm::vec2f& size, float cornerRadius, size_t cornerSegments);
std::vector<vm::vec2f> roundedRect2D(
  float width, float height, float cornerRadius, size_t cornerSegments);

using Triangle = std::array<size_t, 3>;
using MidPointIndex = std::tuple<size_t, size_t>;
using MidPointCache = std::map<MidPointIndex, size_t>;

template <typename T>
size_t midPoint(
  std::vector<vm::vec<T, 3>>& vertices,
  MidPointCache& cache,
  size_t index1,
  size_t index2)
{
  if (auto it = cache.find(MidPointIndex{index1, index2}); it != cache.end())
  {
    return it->second;
  }

  const auto& vertex1 = vertices[index1];
  const auto& vertex2 = vertices[index2];
  auto midPoint = (vertex1 + vertex2) / T(2);
  vertices.push_back(vm::normalize(midPoint));

  auto midPointIndex = vertices.size() - 1;
  cache[MidPointIndex{index1, index2}] = midPointIndex;
  cache[MidPointIndex{index2, index1}] = midPointIndex;
  return midPointIndex;
}

template <typename T>
std::tuple<std::vector<vm::vec<T, 3>>, std::vector<Triangle>> sphereMesh(
  const size_t iterations)
{
  contract_pre(iterations > 0);

  auto vertices = std::vector<vm::vec<T, 3>>{};
  auto triangles = std::vector<Triangle>{};

  // build initial icosahedron
  const auto t = float((T(1) + vm::sqrt(T(5))) / T(2));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{-1, t, 0}));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{1, t, 0}));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{-1, -t, 0}));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{1, -t, 0}));

  vertices.push_back(vm::normalize(vm::vec<T, 3>{0, -1, t}));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{0, 1, t}));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{0, -1, -t}));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{0, 1, -t}));

  vertices.push_back(vm::normalize(vm::vec<T, 3>{t, 0, -1}));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{t, 0, 1}));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{-t, 0, -1}));
  vertices.push_back(vm::normalize(vm::vec<T, 3>{-t, 0, 1}));

  // 5 triangles around point 0
  triangles.push_back(Triangle{0, 5, 11});
  triangles.push_back(Triangle{0, 1, 5});
  triangles.push_back(Triangle{0, 7, 1});
  triangles.push_back(Triangle{0, 10, 7});
  triangles.push_back(Triangle{0, 11, 10});

  // 5 adjacent faces
  triangles.push_back(Triangle{4, 11, 5});
  triangles.push_back(Triangle{9, 5, 1});
  triangles.push_back(Triangle{8, 1, 7});
  triangles.push_back(Triangle{6, 7, 10});
  triangles.push_back(Triangle{2, 10, 11});

  // 5 faces around point 3
  triangles.push_back(Triangle{3, 2, 4});
  triangles.push_back(Triangle{3, 6, 2});
  triangles.push_back(Triangle{3, 8, 6});
  triangles.push_back(Triangle{3, 9, 8});
  triangles.push_back(Triangle{3, 4, 9});

  // 5 adjacent faces
  triangles.push_back(Triangle{11, 4, 2});
  triangles.push_back(Triangle{10, 2, 6});
  triangles.push_back(Triangle{7, 6, 8});
  triangles.push_back(Triangle{1, 8, 9});
  triangles.push_back(Triangle{5, 9, 4});

  // subdivide the icosahedron
  auto cache = MidPointCache{};
  for (size_t i = 0; i < iterations; ++i)
  {
    auto newTriangles = std::vector<Triangle>{};
    newTriangles.reserve(triangles.size() * 4);

    for (const auto& triangle : triangles)
    {
      const auto index1 = midPoint(vertices, cache, triangle[0], triangle[1]);
      const auto index2 = midPoint(vertices, cache, triangle[1], triangle[2]);
      const auto index3 = midPoint(vertices, cache, triangle[2], triangle[0]);
      newTriangles.push_back(Triangle{triangle[0], index1, index3});
      newTriangles.push_back(Triangle{triangle[1], index2, index1});
      newTriangles.push_back(Triangle{triangle[2], index3, index2});
      newTriangles.push_back(Triangle{index1, index2, index3});
    }
    triangles = std::move(newTriangles);
  }

  return {std::move(vertices), std::move(triangles)};
}

std::vector<vm::vec3f> sphere(float radius, size_t iterations);

struct VertsAndNormals
{
  std::vector<vm::vec3f> vertices;
  std::vector<vm::vec3f> normals;
};

VertsAndNormals circle3D(float radius, size_t segments);
VertsAndNormals cylinder(float radius, float length, size_t segments);
VertsAndNormals cone(float radius, float length, size_t segments);

} // namespace tb::mdl
