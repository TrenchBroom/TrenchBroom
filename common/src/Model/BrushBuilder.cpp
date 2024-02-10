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
#include "Error.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Polyhedron.h"

#include "kdl/vector_utils.h"
#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/result_fold.h>
#include <kdl/string_utils.h>

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>

#include <cassert>
#include <string>

namespace TrenchBroom::Model
{
BrushBuilder::BrushBuilder(const MapFormat mapFormat, const vm::bbox3& worldBounds)
  : m_mapFormat(mapFormat)
  , m_worldBounds(worldBounds)
  , m_defaultAttribs(BrushFaceAttributes::NoTextureName)
{
}

BrushBuilder::BrushBuilder(
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  const BrushFaceAttributes& defaultAttribs)
  : m_mapFormat(mapFormat)
  , m_worldBounds(worldBounds)
  , m_defaultAttribs(defaultAttribs)
{
}

Result<Brush> BrushBuilder::createCube(
  const FloatType size, const std::string& textureName) const
{
  return createCuboid(
    vm::bbox3(size / 2.0),
    textureName,
    textureName,
    textureName,
    textureName,
    textureName,
    textureName);
}

Result<Brush> BrushBuilder::createCube(
  FloatType size,
  const std::string& leftTexture,
  const std::string& rightTexture,
  const std::string& frontTexture,
  const std::string& backTexture,
  const std::string& topTexture,
  const std::string& bottomTexture) const
{
  return createCuboid(
    vm::bbox3(size / 2.0),
    leftTexture,
    rightTexture,
    frontTexture,
    backTexture,
    topTexture,
    bottomTexture);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::vec3& size, const std::string& textureName) const
{
  return createCuboid(
    vm::bbox3(-size / 2.0, size / 2.0),
    textureName,
    textureName,
    textureName,
    textureName,
    textureName,
    textureName);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::vec3& size,
  const std::string& leftTexture,
  const std::string& rightTexture,
  const std::string& frontTexture,
  const std::string& backTexture,
  const std::string& topTexture,
  const std::string& bottomTexture) const
{
  return createCuboid(
    vm::bbox3(-size / 2.0, size / 2.0),
    leftTexture,
    rightTexture,
    frontTexture,
    backTexture,
    topTexture,
    bottomTexture);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::bbox3& bounds, const std::string& textureName) const
{
  return createCuboid(
    bounds, textureName, textureName, textureName, textureName, textureName, textureName);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::bbox3& bounds,
  const std::string& leftTexture,
  const std::string& rightTexture,
  const std::string& frontTexture,
  const std::string& backTexture,
  const std::string& topTexture,
  const std::string& bottomTexture) const
{
  const auto specs =
    std::vector<std::tuple<vm::vec3, vm::vec3, vm::vec3, BrushFaceAttributes>>({
      {bounds.min,
       bounds.min + vm::vec3::pos_y(),
       bounds.min + vm::vec3::pos_z(),
       BrushFaceAttributes(leftTexture, m_defaultAttribs)}, // left
      {bounds.max,
       bounds.max + vm::vec3::pos_z(),
       bounds.max + vm::vec3::pos_y(),
       BrushFaceAttributes(rightTexture, m_defaultAttribs)}, // right
      {bounds.min,
       bounds.min + vm::vec3::pos_z(),
       bounds.min + vm::vec3::pos_x(),
       BrushFaceAttributes(frontTexture, m_defaultAttribs)}, // front
      {bounds.max,
       bounds.max + vm::vec3::pos_x(),
       bounds.max + vm::vec3::pos_z(),
       BrushFaceAttributes(backTexture, m_defaultAttribs)}, // back
      {bounds.max,
       bounds.max + vm::vec3::pos_y(),
       bounds.max + vm::vec3::pos_x(),
       BrushFaceAttributes(topTexture, m_defaultAttribs)}, // top
      {bounds.min,
       bounds.min + vm::vec3::pos_x(),
       bounds.min + vm::vec3::pos_y(),
       BrushFaceAttributes(bottomTexture, m_defaultAttribs)}, // bottom
    });

  return kdl::fold_results(kdl::vec_transform(
                             specs,
                             [&](const auto spec) {
                               const auto& [p1, p2, p3, attrs] = spec;
                               return BrushFace::create(p1, p2, p3, attrs, m_mapFormat);
                             }))
    .and_then([&](auto faces) { return Brush::create(m_worldBounds, std::move(faces)); });
}

namespace
{
auto makeUnitCylinder(const size_t numSides, const RadiusMode radiusMode)
{
  auto vertices = std::vector<vm::vec3>{};

  switch (radiusMode)
  {
  case RadiusMode::ToEdge:
    for (size_t i = 0; i < numSides; ++i)
    {
      const auto angle =
        (FloatType(i) + 0.5) * vm::C::two_pi() / FloatType(numSides) - vm::C::half_pi();
      const auto a = vm::C::pi() / FloatType(numSides); // Half angle
      const auto ca = std::cos(a);
      const auto x = std::cos(angle) * 0.5 / ca + 0.5;
      const auto y = std::sin(angle) * 0.5 / ca + 0.5;
      vertices.emplace_back(x, y, 0.0);
      vertices.emplace_back(x, y, 1.0);
    }
    break;
  case RadiusMode::ToVertex:
    for (size_t i = 0; i < numSides; ++i)
    {
      const auto angle =
        FloatType(i) * vm::C::two_pi() / FloatType(numSides) - vm::C::half_pi();
      const auto x = std::cos(angle) * 0.5 + 0.5;
      const auto y = std::sin(angle) * 0.5 + 0.5;
      vertices.emplace_back(x, y, 0.0);
      vertices.emplace_back(x, y, 1.0);
    }
    break;
    switchDefault();
  }

  return vertices;
}
} // namespace

std::vector<Result<Brush>> BrushBuilder::createCylinder(
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
                         * vm::rotation_matrix(vm::vec3::pos_z(), vm::vec3::axis(axis))
                         * vm::translation_matrix(vm::vec3{-0.5, -0.5, -0.5});
//  const auto vertices = kdl::vec_transform(
//    makeUnitCylinder(numSides, radiusMode), [&](const auto& v) { return transform * v; });
  auto verts_vec = std::vector<std::vector<vm::vec3>> {};
  verts_vec.push_back(makeUnitCylinder(numSides, radiusMode)); // TODO :Call this for each side or break it up or whatever.
  return kdl::vec_transform(verts_vec, [&](const auto& verts) {
    return createBrush(transform * verts, textureName);
  });
}

namespace
{
auto makeUnitCone(const size_t numSides, const RadiusMode radiusMode)
{
  auto vertices = std::vector<vm::vec3>{};

  switch (radiusMode)
  {
  case RadiusMode::ToEdge:
    for (size_t i = 0; i < numSides; ++i)
    {
      const auto angle =
        (FloatType(i) + 0.5) * vm::C::two_pi() / FloatType(numSides) - vm::C::half_pi();
      const auto a = vm::C::pi() / FloatType(numSides); // Half angle
      const auto ca = std::cos(a);
      const auto x = std::cos(angle) * 0.5 / ca + 0.5;
      const auto y = std::sin(angle) * 0.5 / ca + 0.5;
      vertices.emplace_back(x, y, 0.0);
    }
    break;
  case RadiusMode::ToVertex:
    for (size_t i = 0; i < numSides; ++i)
    {
      const auto angle =
        FloatType(i) * vm::C::two_pi() / FloatType(numSides) - vm::C::half_pi();
      const auto x = std::cos(angle) * 0.5 + 0.5;
      const auto y = std::sin(angle) * 0.5 + 0.5;
      vertices.emplace_back(x, y, 0.0);
    }
    break;
    switchDefault();
  }

  vertices.emplace_back(0.5, 0.5, 1.0);
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
                         * vm::rotation_matrix(vm::vec3::pos_z(), vm::vec3::axis(axis))
                         * vm::translation_matrix(vm::vec3{-0.5, -0.5, -0.5});
  const auto vertices = kdl::vec_transform(
    makeUnitCone(numSides, radiusMode), [&](const auto& v) { return transform * v; });

  return createBrush(vertices, textureName);
}

Result<Brush> BrushBuilder::createBrush(
  const std::vector<vm::vec3>& points, const std::string& textureName) const
{
  return createBrush(Polyhedron3(points), textureName);
}

Result<Brush> BrushBuilder::createBrush(
  const Polyhedron3& polyhedron, const std::string& textureName) const
{
  assert(polyhedron.closed());

  return kdl::fold_results(
           kdl::vec_transform(
             polyhedron.faces(),
             [&](const auto* face) {
               const auto& boundary = face->boundary();

               auto bIt = std::begin(boundary);
               const auto* edge1 = *bIt++;
               const auto* edge2 = *bIt++;
               const auto* edge3 = *bIt++;

               const auto& p1 = edge1->origin()->position();
               const auto& p2 = edge2->origin()->position();
               const auto& p3 = edge3->origin()->position();

               return BrushFace::create(
                 p1, p3, p2, Model::BrushFaceAttributes(textureName), m_mapFormat);
             }))
    .and_then(
      [&](auto&& faces) { return Brush::create(m_worldBounds, std::move(faces)); });
}
} // namespace TrenchBroom::Model
