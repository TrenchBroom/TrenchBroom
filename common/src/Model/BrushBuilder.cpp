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

#include "BrushBuilder.h"

#include "Ensure.h"
#include "Error.h" // IWYU pragma: keep
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Polyhedron.h"
#include "Renderer/RenderUtils.h"

#include "kdl/range_utils.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"

#include <cassert>
#include <ranges>
#include <string>
#include <utility>

namespace TrenchBroom::Model
{
namespace
{
auto createFromFaces(
  const std::vector<std::tuple<vm::vec3, vm::vec3, vm::vec3, BrushFaceAttributes>>& specs,
  const vm::bbox3& worldBounds,
  const MapFormat mapFormat)
{
  return specs | std::views::transform([&](const auto spec) {
           const auto& [p1, p2, p3, attrs] = spec;
           return BrushFace::create(p1, p2, p3, attrs, mapFormat);
         })
         | kdl::fold | kdl::and_then([&](auto faces) {
             return Brush::create(worldBounds, std::move(faces));
           });
}
} // namespace

BrushBuilder::BrushBuilder(const MapFormat mapFormat, const vm::bbox3& worldBounds)
  : m_mapFormat{mapFormat}
  , m_worldBounds{worldBounds}
  , m_defaultAttribs{BrushFaceAttributes::NoMaterialName}
{
}

BrushBuilder::BrushBuilder(
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  BrushFaceAttributes defaultAttribs)
  : m_mapFormat{mapFormat}
  , m_worldBounds{worldBounds}
  , m_defaultAttribs{std::move(defaultAttribs)}
{
}

Result<Brush> BrushBuilder::createCube(
  const FloatType size, const std::string& materialName) const
{
  return createCuboid(
    vm::bbox3{size / 2.0},
    materialName,
    materialName,
    materialName,
    materialName,
    materialName,
    materialName);
}

Result<Brush> BrushBuilder::createCube(
  FloatType size,
  const std::string& leftMaterial,
  const std::string& rightMaterial,
  const std::string& frontMaterial,
  const std::string& backMaterial,
  const std::string& topMaterial,
  const std::string& bottomMaterial) const
{
  return createCuboid(
    vm::bbox3{size / 2.0},
    leftMaterial,
    rightMaterial,
    frontMaterial,
    backMaterial,
    topMaterial,
    bottomMaterial);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::vec3& size, const std::string& materialName) const
{
  return createCuboid(
    vm::bbox3{-size / 2.0, size / 2.0},
    materialName,
    materialName,
    materialName,
    materialName,
    materialName,
    materialName);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::vec3& size,
  const std::string& leftMaterial,
  const std::string& rightMaterial,
  const std::string& frontMaterial,
  const std::string& backMaterial,
  const std::string& topMaterial,
  const std::string& bottomMaterial) const
{
  return createCuboid(
    vm::bbox3{-size / 2.0, size / 2.0},
    leftMaterial,
    rightMaterial,
    frontMaterial,
    backMaterial,
    topMaterial,
    bottomMaterial);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::bbox3& bounds, const std::string& materialName) const
{
  return createCuboid(
    bounds,
    materialName,
    materialName,
    materialName,
    materialName,
    materialName,
    materialName);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::bbox3& bounds,
  const std::string& leftMaterial,
  const std::string& rightMaterial,
  const std::string& frontMaterial,
  const std::string& backMaterial,
  const std::string& topMaterial,
  const std::string& bottomMaterial) const
{
  return createFromFaces(
    {
      {bounds.min,
       bounds.min + vm::vec3::pos_y(),
       bounds.min + vm::vec3::pos_z(),
       {leftMaterial, m_defaultAttribs}}, // left
      {bounds.max,
       bounds.max + vm::vec3::pos_z(),
       bounds.max + vm::vec3::pos_y(),
       {rightMaterial, m_defaultAttribs}}, // right
      {bounds.min,
       bounds.min + vm::vec3::pos_z(),
       bounds.min + vm::vec3::pos_x(),
       {frontMaterial, m_defaultAttribs}}, // front
      {bounds.max,
       bounds.max + vm::vec3::pos_x(),
       bounds.max + vm::vec3::pos_z(),
       {backMaterial, m_defaultAttribs}}, // back
      {bounds.max,
       bounds.max + vm::vec3::pos_y(),
       bounds.max + vm::vec3::pos_x(),
       {topMaterial, m_defaultAttribs}}, // top
      {bounds.min,
       bounds.min + vm::vec3::pos_x(),
       bounds.min + vm::vec3::pos_y(),
       {bottomMaterial, m_defaultAttribs}}, // bottom
    },
    m_worldBounds,
    m_mapFormat);
}

namespace
{
auto makeUnitCircle(const size_t numSides, const RadiusMode radiusMode)
{
  auto vertices = std::vector<vm::vec2>{};

  switch (radiusMode)
  {
  case RadiusMode::ToEdge:
    for (size_t i = 0; i < numSides; ++i)
    {
      const auto angle =
        (FloatType(i) + 0.5) * vm::C::two_pi() / FloatType(numSides) - vm::C::half_pi();
      const auto a = vm::C::pi() / FloatType(numSides); // Half angle
      const auto ca = std::cos(a);
      const auto x = std::cos(angle) / ca;
      const auto y = std::sin(angle) / ca;
      vertices.emplace_back(x, y);
    }
    break;
  case RadiusMode::ToVertex:
    for (size_t i = 0; i < numSides; ++i)
    {
      const auto angle =
        FloatType(i) * vm::C::two_pi() / FloatType(numSides) - vm::C::half_pi();
      const auto x = std::cos(angle);
      const auto y = std::sin(angle);
      vertices.emplace_back(x, y);
    }
    break;
    switchDefault();
  }

  return vertices;
}

auto makeUnitCylinder(const size_t numSides, const RadiusMode radiusMode)
{
  auto vertices = std::vector<vm::vec3>{};
  for (const auto& v : makeUnitCircle(numSides, radiusMode))
  {
    vertices.emplace_back(v.x(), v.y(), -1.0);
    vertices.emplace_back(v.x(), v.y(), +1.0);
  }
  return vertices;
}
} // namespace

Result<Brush> BrushBuilder::createCylinder(
  const vm::bbox3& bounds,
  const size_t numSides,
  const RadiusMode radiusMode,
  const vm::axis::type axis,
  const std::string& textureName) const
{
  ensure(numSides > 2, "cylinder has at least three sides");

  const auto transform = vm::translation_matrix(bounds.min)
                         * vm::scaling_matrix(bounds.size())
                         * vm::translation_matrix(vm::vec3{0.5, 0.5, 0.5})
                         * vm::scaling_matrix(vm::vec3{0.5, 0.5, 0.5})
                         * vm::rotation_matrix(vm::vec3::pos_z(), vm::vec3::axis(axis));

  const auto cylinder = makeUnitCylinder(numSides, radiusMode);
  const auto vertices =
    cylinder | std::views::transform([&](const auto& v) { return transform * v; })
    | kdl::to_vector;

  return createBrush(vertices, textureName);
}

namespace
{
auto makeUnitCone(const size_t numSides, const RadiusMode radiusMode)
{
  auto vertices = std::vector<vm::vec3>{};
  for (const auto& v : makeUnitCircle(numSides, radiusMode))
  {
    vertices.emplace_back(v.x(), v.y(), -1.0);
  }
  vertices.emplace_back(0.0, 0.0, 1.0);
  return vertices;
}
} // namespace

Result<Brush> BrushBuilder::createCone(
  const vm::bbox3& bounds,
  const size_t numSides,
  const RadiusMode radiusMode,
  const vm::axis::type axis,
  const std::string& textureName) const
{
  ensure(numSides > 2, "cylinder has at least three sides");

  const auto transform = vm::translation_matrix(bounds.min)
                         * vm::scaling_matrix(bounds.size())
                         * vm::translation_matrix(vm::vec3{0.5, 0.5, 0.5})
                         * vm::scaling_matrix(vm::vec3{0.5, 0.5, 0.5})
                         * vm::rotation_matrix(vm::vec3::pos_z(), vm::vec3::axis(axis));

  const auto cone = makeUnitCone(numSides, radiusMode);
  const auto vertices =
    cone | std::views::transform([&](const auto& v) { return transform * v; })
    | kdl::to_vector;

  return createBrush(vertices, textureName);
}

Result<Brush> BrushBuilder::createIcoSphere(
  const vm::bbox3& bounds, const size_t iterations, const std::string& textureName) const
{
  const auto [sphereVertices, sphereIndices] =
    Renderer::sphereMesh<FloatType>(iterations);

  const auto specs =
    sphereIndices
    | std::views::transform(
      [sphereVertices = sphereVertices, &textureName, this](const auto& face) {
        const auto& p1 = sphereVertices[face[0]];
        const auto& p2 = sphereVertices[face[1]];
        const auto& p3 = sphereVertices[face[2]];
        return std::tuple{p1, p2, p3, BrushFaceAttributes{textureName, m_defaultAttribs}};
      })
    | kdl::to_vector;

  return createFromFaces(specs, m_worldBounds, m_mapFormat)
         | kdl::and_then([&](auto brush) {
             const auto transform = vm::translation_matrix(bounds.min)
                                    * vm::scaling_matrix(bounds.size())
                                    * vm::scaling_matrix(vm::vec3{0.5, 0.5, 0.5})
                                    * vm::translation_matrix(vm::vec3{1, 1, 1});
             return brush.transform(m_worldBounds, transform, false)
                    | kdl::transform([&]() { return std::move(brush); });
           });
}

Result<Brush> BrushBuilder::createBrush(
  const std::vector<vm::vec3>& points, const std::string& materialName) const
{
  return createBrush(Polyhedron3{points}, materialName);
}

Result<Brush> BrushBuilder::createBrush(
  const Polyhedron3& polyhedron, const std::string& materialName) const
{
  assert(polyhedron.closed());

  return polyhedron.faces() | std::views::transform([&](const auto* face) {
           const auto& boundary = face->boundary();

           auto bIt = std::begin(boundary);
           const auto* edge1 = *bIt++;
           const auto* edge2 = *bIt++;
           const auto* edge3 = *bIt++;

           const auto& p1 = edge1->origin()->position();
           const auto& p2 = edge2->origin()->position();
           const auto& p3 = edge3->origin()->position();

           return BrushFace::create(
             p1,
             p3,
             p2,
             BrushFaceAttributes{materialName, m_defaultAttribs},
             m_mapFormat);
         })
         | kdl::fold | kdl::and_then([&](auto faces) {
             return Brush::create(m_worldBounds, std::move(faces));
           });
}
} // namespace TrenchBroom::Model
