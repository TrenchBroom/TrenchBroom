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

#include "RenderUtils.h"

#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Renderer/GL.h"

#include "vm/bbox.h"
#include "vm/forward.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <map>

namespace TrenchBroom::Renderer
{
namespace
{
constexpr auto EdgeOffset = 0.0001;
}

vm::vec3f gridColorForMaterial(const Assets::Material* material)
{
  if (const auto* texture = getTexture(material))
  {
    const auto& averageColor = texture->averageColor();
    if ((averageColor.r() + averageColor.g() + averageColor.b()) / 3.0f > 0.50f)
    {
      // bright material grid color
      return vm::vec3f{0, 0, 0};
    }
  }

  // dark material grid color
  return vm::vec3f{1, 1, 1};
}

void glSetEdgeOffset(const double f)
{
  glAssert(glDepthRange(0.0, 1.0 - EdgeOffset * f));
}

void glResetEdgeOffset()
{
  glAssert(glDepthRange(EdgeOffset, 1.0));
}

void coordinateSystemVerticesX(const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end)
{
  const auto center = bounds.center();
  start = vm::vec3f{bounds.min.x(), center.y(), center.z()};
  end = vm::vec3f{bounds.max.x(), center.y(), center.z()};
}

void coordinateSystemVerticesY(const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end)
{
  const auto center = bounds.center();
  start = vm::vec3f{center.x(), bounds.min.y(), center.z()};
  end = vm::vec3f{center.x(), bounds.max.y(), center.z()};
}

void coordinateSystemVerticesZ(const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end)
{
  const auto center = bounds.center();
  start = vm::vec3f{center.x(), center.y(), bounds.min.z()};
  end = vm::vec3f{center.x(), center.y(), bounds.max.z()};
}

MaterialRenderFunc::~MaterialRenderFunc() = default;
void MaterialRenderFunc::before(const Assets::Material* /* material */) {}
void MaterialRenderFunc::after(const Assets::Material* /* material */) {}

void DefaultMaterialRenderFunc::before(const Assets::Material* material)
{
  if (material)
  {
    material->activate();
  }
}

void DefaultMaterialRenderFunc::after(const Assets::Material* material)
{
  if (material)
  {
    material->deactivate();
  }
}

std::vector<vm::vec2f> circle2D(const float radius, const size_t segments)
{
  auto vertices = circle2D(radius, 0.0f, vm::Cf::two_pi(), segments);
  vertices.push_back(vm::vec2f::zero());
  return vertices;
}

std::vector<vm::vec2f> circle2D(
  const float radius,
  const float startAngle,
  const float angleLength,
  const size_t segments)
{
  assert(radius > 0.0f);
  assert(segments > 0);
  if (angleLength == 0.0f)
  {
    return {};
  }

  auto vertices = std::vector<vm::vec2f>{};
  vertices.reserve(segments + 1);

  const auto d = angleLength / float(segments);
  auto a = startAngle;
  for (size_t i = 0; i <= segments; ++i)
  {
    vertices.emplace_back(radius * std::sin(a), radius * std::cos(a));
    a += d;
  }

  return vertices;
}

std::vector<vm::vec3f> circle2D(
  const float radius,
  const vm::axis::type axis,
  const float startAngle,
  const float angleLength,
  const size_t segments)
{
  assert(radius > 0.0f);
  assert(segments > 0);
  if (angleLength == 0.0f)
  {
    return {};
  }

  size_t x, y, z;
  switch (axis)
  {
  case vm::axis::x:
    x = 1;
    y = 2;
    z = 0;
    break;
  case vm::axis::y:
    x = 2;
    y = 0;
    z = 1;
    break;
  default:
    x = 0;
    y = 1;
    z = 2;
    break;
  }

  auto vertices = std::vector<vm::vec3f>{segments + 1};

  const auto d = angleLength / float(segments);
  auto a = startAngle;
  for (size_t i = 0; i <= segments; ++i)
  {
    vertices[i][x] = radius * std::cos(a);
    vertices[i][y] = radius * std::sin(a);
    vertices[i][z] = 0.0f;
    a += d;
  }

  return vertices;
}

std::pair<float, float> startAngleAndLength(
  const vm::axis::type axis, const vm::vec3f& startAxis, const vm::vec3f& endAxis)
{
  float angle1, angle2, angleLength;
  switch (axis)
  {
  case vm::axis::x:
    angle1 = vm::measure_angle(startAxis, vm::vec3f::pos_y(), vm::vec3f::pos_x());
    angle2 = vm::measure_angle(endAxis, vm::vec3f::pos_y(), vm::vec3f::pos_x());
    angleLength = vm::min(
      vm::measure_angle(startAxis, endAxis, vm::vec3f::pos_x()),
      vm::measure_angle(endAxis, startAxis, vm::vec3f::pos_x()));
    break;
  case vm::axis::y:
    angle1 = vm::measure_angle(startAxis, vm::vec3f::pos_z(), vm::vec3f::pos_y());
    angle2 = vm::measure_angle(endAxis, vm::vec3f::pos_z(), vm::vec3f::pos_y());
    angleLength = vm::min(
      vm::measure_angle(startAxis, endAxis, vm::vec3f::pos_y()),
      vm::measure_angle(endAxis, startAxis, vm::vec3f::pos_y()));
    break;
  default:
    angle1 = vm::measure_angle(startAxis, vm::vec3f::pos_x(), vm::vec3f::pos_z());
    angle2 = vm::measure_angle(endAxis, vm::vec3f::pos_x(), vm::vec3f::pos_z());
    angleLength = vm::min(
      vm::measure_angle(startAxis, endAxis, vm::vec3f::pos_z()),
      vm::measure_angle(endAxis, startAxis, vm::vec3f::pos_z()));
    break;
  }

  const auto minAngle = vm::min(angle1, angle2);
  const auto maxAngle = std::max(angle1, angle2);
  const auto startAngle = (maxAngle - minAngle <= vm::Cf::pi() ? minAngle : maxAngle);

  return {startAngle, angleLength};
}

size_t roundedRect2DVertexCount(const size_t cornerSegments)
{
  return 4 * (3 * cornerSegments + 3);
}

std::vector<vm::vec2f> roundedRect2D(
  const vm::vec2f& size, const float cornerRadius, const size_t cornerSegments)
{
  return roundedRect2D(size.x(), size.y(), cornerRadius, cornerSegments);
}

std::vector<vm::vec2f> roundedRect2D(
  const float width,
  const float height,
  const float cornerRadius,
  const size_t cornerSegments)
{
  assert(cornerSegments > 0);
  assert(cornerRadius <= width / 2.0f && cornerRadius <= height / 2.0f);

  auto vertices = std::vector<vm::vec2f>{};
  vertices.reserve(roundedRect2DVertexCount(cornerSegments));

  const auto angle = vm::Cf::half_pi() / float(cornerSegments);
  const auto center = vm::vec2f{0.0f, 0.0f};
  auto translation = vm::vec2f{};

  auto curAngle = 0.0f;
  auto x = std::cos(curAngle) * cornerRadius;
  auto y = std::sin(curAngle) * cornerRadius;

  // lower right corner
  translation = vm::vec2f{(width / 2.0f - cornerRadius), -(height / 2.0f - cornerRadius)};
  for (size_t i = 0; i < cornerSegments; ++i)
  {
    vertices.push_back(center);
    vertices.push_back(translation + vm::vec2f{x, y});

    curAngle -= angle;
    x = std::cos(curAngle) * cornerRadius;
    y = std::sin(curAngle) * cornerRadius;
    vertices.push_back(translation + vm::vec2f{x, y});
  }

  // lower left corner
  translation =
    vm::vec2f{-(width / 2.0f - cornerRadius), -(height / 2.0f - cornerRadius)};
  for (size_t i = 0; i < cornerSegments; ++i)
  {
    vertices.push_back(center);
    vertices.push_back(translation + vm::vec2f{x, y});

    curAngle -= angle;
    x = std::cos(curAngle) * cornerRadius;
    y = std::sin(curAngle) * cornerRadius;
    vertices.push_back(translation + vm::vec2f{x, y});
  }

  // upper left corner
  translation = vm::vec2f{-(width / 2.0f - cornerRadius), (height / 2.0f - cornerRadius)};
  for (size_t i = 0; i < cornerSegments; ++i)
  {
    vertices.push_back(center);
    vertices.push_back(translation + vm::vec2f{x, y});

    curAngle -= angle;
    x = std::cos(curAngle) * cornerRadius;
    y = std::sin(curAngle) * cornerRadius;
    vertices.push_back(translation + vm::vec2f{x, y});
  }

  // upper right corner
  translation = vm::vec2f{(width / 2.0f - cornerRadius), (height / 2.0f - cornerRadius)};
  for (size_t i = 0; i < cornerSegments; ++i)
  {
    vertices.push_back(center);
    vertices.push_back(translation + vm::vec2f{x, y});

    curAngle -= angle;
    x = std::cos(curAngle) * cornerRadius;
    y = std::sin(curAngle) * cornerRadius;
    vertices.push_back(translation + vm::vec2f{x, y});
  }

  // upper body triangle
  vertices.push_back(center);
  vertices.emplace_back(-(width / 2.0f - cornerRadius), height / 2.0f);
  vertices.emplace_back((width / 2.0f - cornerRadius), height / 2.0f);

  // right body triangle
  vertices.push_back(center);
  vertices.emplace_back(width / 2.0f, (height / 2.0f - cornerRadius));
  vertices.emplace_back(width / 2.0f, -(height / 2.0f - cornerRadius));

  // lower body triangle
  vertices.push_back(center);
  vertices.emplace_back((width / 2.0f - cornerRadius), -height / 2.0f);
  vertices.emplace_back(-(width / 2.0f - cornerRadius), -height / 2.0f);

  // left body triangle
  vertices.push_back(center);
  vertices.emplace_back(-width / 2.0f, -(height / 2.0f - cornerRadius));
  vertices.emplace_back(-width / 2.0f, (height / 2.0f - cornerRadius));

  return vertices;
}

namespace
{
using Triangle = std::array<size_t, 3>;
using MidPointIndex = std::tuple<size_t, size_t>;
using MidPointCache = std::map<MidPointIndex, size_t>;

size_t midPoint(
  std::vector<vm::vec3f>& vertices, MidPointCache& cache, size_t index1, size_t index2);

size_t midPoint(
  std::vector<vm::vec3f>& vertices, MidPointCache& cache, size_t index1, size_t index2)
{
  if (auto it = cache.find(MidPointIndex{index1, index2}); it != cache.end())
  {
    return it->second;
  }

  const auto& vertex1 = vertices[index1];
  const auto& vertex2 = vertices[index2];
  auto midPoint = (vertex1 + vertex2) / 2.0f;
  vertices.push_back(vm::normalize(midPoint));

  auto midPointIndex = vertices.size() - 1;
  cache[MidPointIndex{index1, index2}] = midPointIndex;
  cache[MidPointIndex{index2, index1}] = midPointIndex;
  return midPointIndex;
}
} // namespace

std::vector<vm::vec3f> sphere3D(const float radius, const size_t iterations)
{
  assert(radius > 0.0f);
  assert(iterations > 0);

  auto vertices = std::vector<vm::vec3f>{};
  auto triangles = std::vector<Triangle>{};

  // build initial icosahedron
  const auto t = float((1.0 + std::sqrt(5.0)) / 2.0);
  vertices.push_back(vm::normalize(vm::vec3f{-1.0f, t, 0.0f}));
  vertices.push_back(vm::normalize(vm::vec3f{1.0f, t, 0.0f}));
  vertices.push_back(vm::normalize(vm::vec3f{-1.0f, -t, 0.0f}));
  vertices.push_back(vm::normalize(vm::vec3f{1.0f, -t, 0.0f}));

  vertices.push_back(vm::normalize(vm::vec3f{0.0f, -1.0f, t}));
  vertices.push_back(vm::normalize(vm::vec3f{0.0f, 1.0f, t}));
  vertices.push_back(vm::normalize(vm::vec3f{0.0f, -1.0f, -t}));
  vertices.push_back(vm::normalize(vm::vec3f{0.0f, 1.0f, -t}));

  vertices.push_back(vm::normalize(vm::vec3f{t, 0.0f, -1.0f}));
  vertices.push_back(vm::normalize(vm::vec3f{t, 0.0f, 1.0f}));
  vertices.push_back(vm::normalize(vm::vec3f{-t, 0.0f, -1.0f}));
  vertices.push_back(vm::normalize(vm::vec3f{-t, 0.0f, 1.0f}));

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
  MidPointCache cache;
  for (size_t i = 0; i < iterations; ++i)
  {
    auto newTriangles = std::vector<Triangle>{};
    newTriangles.reserve(triangles.size() * 4);

    for (Triangle& triangle : triangles)
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

  auto allVertices = std::vector<vm::vec3f>{};
  allVertices.reserve(3 * triangles.size());

  for (const auto& triangle : triangles)
  {
    for (size_t i = 0; i < 3; ++i)
    {
      allVertices.push_back(radius * vertices[triangle[i]]);
    }
  }

  return allVertices;
}

VertsAndNormals circle3D(const float radius, const size_t segments)
{
  assert(radius > 0.0f);
  assert(segments > 2);

  auto vertices = std::vector<vm::vec3f>{};
  vertices.reserve(segments);

  auto normals = std::vector<vm::vec3f>{};
  normals.reserve(segments);

  auto a = 0.0f;
  const auto d = 2.0f * vm::Cf::pi() / float(segments);
  for (size_t i = 0; i < segments; i++)
  {
    vertices.emplace_back(radius * std::sin(a), radius * std::cos(a), 0.0f);
    normals.push_back(vm::vec3f::pos_z());

    a += d;
  }
  return {std::move(vertices), std::move(normals)};
}

VertsAndNormals cylinder3D(const float radius, const float length, const size_t segments)
{
  assert(radius > 0.0f);
  assert(length > 0.0f);
  assert(segments > 2);

  auto vertices = std::vector<vm::vec3f>{};
  vertices.reserve(2 * (segments + 1));

  auto normals = std::vector<vm::vec3f>{};
  normals.reserve(2 * (segments + 1));

  auto a = 0.0f;
  const auto d = 2.0f * vm::Cf::pi() / float(segments);
  for (size_t i = 0; i <= segments; ++i)
  {
    const auto s = std::sin(a);
    const auto c = std::cos(a);
    const auto x = radius * s;
    const auto y = radius * c;

    vertices.emplace_back(x, y, length);
    normals.emplace_back(s, c, 0.0f);

    vertices.emplace_back(x, y, 0.0f);
    normals.emplace_back(s, c, 0.0f);

    a += d;
  }

  return {std::move(vertices), std::move(normals)};
}

VertsAndNormals cone3D(const float radius, const float length, const size_t segments)
{
  assert(radius > 0.0f);
  assert(length > 0.0f);
  assert(segments > 2);

  auto vertices = std::vector<vm::vec3f>{};
  vertices.reserve(3 * (segments + 1));

  auto normals = std::vector<vm::vec3f>{};
  normals.reserve(3 * (segments + 1));

  const auto t = std::atan(length / radius);
  const auto n = std::cos(vm::Cf::half_pi() - t);

  auto a = 0.0f;
  const auto d = 2.0f * vm::Cf::pi() / float(segments);
  auto lastS = std::sin(a);
  auto lastC = std::cos(a);
  a += d;

  for (size_t i = 0; i <= segments; ++i)
  {
    const auto s = std::sin(a);
    const auto c = std::cos(a);

    vertices.emplace_back(0.0f, 0.0f, length);
    normals.push_back(
      vm::normalize(vm::vec3f{std::sin(a - d / 2.0f), std::cos(a - d / 2.0f), n}));

    vertices.emplace_back(radius * lastS, radius * lastC, 0.0f);
    normals.push_back(vm::normalize(vm::vec3f{lastS, lastC, n}));

    vertices.emplace_back(radius * s, radius * c, 0.0f);
    normals.push_back(vm::normalize(vm::vec3f{s, c, n}));

    lastS = s;
    lastC = c;
    a += d;
  }

  return {std::move(vertices), std::move(normals)};
}

} // namespace TrenchBroom::Renderer
