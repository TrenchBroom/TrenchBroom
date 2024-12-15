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

#include "BrushBuilder.h"

#include "Ensure.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "render/RenderUtils.h"

#include "kdl/range_to_vector.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"

#include "vm/intersection.h"
#include "vm/line.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"

#include <cassert>
#include <ranges>
#include <string>
#include <utility>

namespace tb::mdl
{

EdgeAlignedCircle::EdgeAlignedCircle() = default;

EdgeAlignedCircle::EdgeAlignedCircle(const size_t numSides_)
  : numSides{numSides_}
{
}

EdgeAlignedCircle::EdgeAlignedCircle(const VertexAlignedCircle& circleShape)
  : EdgeAlignedCircle{circleShape.numSides}
{
}

VertexAlignedCircle::VertexAlignedCircle() = default;

VertexAlignedCircle::VertexAlignedCircle(const size_t numSides_)
  : numSides{numSides_}
{
}

VertexAlignedCircle::VertexAlignedCircle(const EdgeAlignedCircle& circleShape)
  : VertexAlignedCircle{circleShape.numSides}
{
}

BrushBuilder::BrushBuilder(const MapFormat mapFormat, const vm::bbox3d& worldBounds)
  : m_mapFormat{mapFormat}
  , m_worldBounds{worldBounds}
  , m_defaultAttribs{BrushFaceAttributes::NoMaterialName}
{
}

BrushBuilder::BrushBuilder(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  BrushFaceAttributes defaultAttribs)
  : m_mapFormat{mapFormat}
  , m_worldBounds{worldBounds}
  , m_defaultAttribs{std::move(defaultAttribs)}
{
}

Result<Brush> BrushBuilder::createCube(
  const double size, const std::string& materialName) const
{
  return createCuboid(
    vm::bbox3d{size / 2.0},
    materialName,
    materialName,
    materialName,
    materialName,
    materialName,
    materialName);
}

Result<Brush> BrushBuilder::createCube(
  double size,
  const std::string& leftMaterial,
  const std::string& rightMaterial,
  const std::string& frontMaterial,
  const std::string& backMaterial,
  const std::string& topMaterial,
  const std::string& bottomMaterial) const
{
  return createCuboid(
    vm::bbox3d{size / 2.0},
    leftMaterial,
    rightMaterial,
    frontMaterial,
    backMaterial,
    topMaterial,
    bottomMaterial);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::vec3d& size, const std::string& materialName) const
{
  return createCuboid(
    vm::bbox3d{-size / 2.0, size / 2.0},
    materialName,
    materialName,
    materialName,
    materialName,
    materialName,
    materialName);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::vec3d& size,
  const std::string& leftMaterial,
  const std::string& rightMaterial,
  const std::string& frontMaterial,
  const std::string& backMaterial,
  const std::string& topMaterial,
  const std::string& bottomMaterial) const
{
  return createCuboid(
    vm::bbox3d{-size / 2.0, size / 2.0},
    leftMaterial,
    rightMaterial,
    frontMaterial,
    backMaterial,
    topMaterial,
    bottomMaterial);
}

Result<Brush> BrushBuilder::createCuboid(
  const vm::bbox3d& bounds, const std::string& materialName) const
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
  const vm::bbox3d& bounds,
  const std::string& leftMaterial,
  const std::string& rightMaterial,
  const std::string& frontMaterial,
  const std::string& backMaterial,
  const std::string& topMaterial,
  const std::string& bottomMaterial) const
{
  return std::vector{
           BrushFace::create(
             bounds.min,
             bounds.min + vm::vec3d{0, 1, 0},
             bounds.min + vm::vec3d{0, 0, 1},
             {leftMaterial, m_defaultAttribs},
             m_mapFormat), // left
           BrushFace::create(
             bounds.max,
             bounds.max + vm::vec3d{0, 0, 1},
             bounds.max + vm::vec3d{0, 1, 0},
             {rightMaterial, m_defaultAttribs},
             m_mapFormat), // right
           BrushFace::create(
             bounds.min,
             bounds.min + vm::vec3d{0, 0, 1},
             bounds.min + vm::vec3d{1, 0, 0},
             {frontMaterial, m_defaultAttribs},
             m_mapFormat), // front
           BrushFace::create(
             bounds.max,
             bounds.max + vm::vec3d{1, 0, 0},
             bounds.max + vm::vec3d{0, 0, 1},
             {backMaterial, m_defaultAttribs},
             m_mapFormat), // back
           BrushFace::create(
             bounds.max,
             bounds.max + vm::vec3d{0, 1, 0},
             bounds.max + vm::vec3d{1, 0, 0},
             {topMaterial, m_defaultAttribs},
             m_mapFormat), // top
           BrushFace::create(
             bounds.min,
             bounds.min + vm::vec3d{1, 0, 0},
             bounds.min + vm::vec3d{0, 1, 0},
             {bottomMaterial, m_defaultAttribs},
             m_mapFormat), // bottom
         }
         | kdl::fold | kdl::and_then([&](auto faces) {
             return Brush::create(m_worldBounds, std::move(faces));
           });
}

namespace
{
auto makeUnitCircle(const CircleShape& circleShape)
{

  return std::visit(
    kdl::overload(
      [](const EdgeAlignedCircle& edgeAligned) {
        ensure(edgeAligned.numSides > 2, "shape has at least three sides");
        auto vertices = std::vector<vm::vec2d>{};
        for (size_t i = 0; i < edgeAligned.numSides; ++i)
        {
          const auto angle =
            (double(i) + 0.5) * vm::Cd::two_pi() / double(edgeAligned.numSides)
            - vm::Cd::half_pi();
          const auto a = vm::Cd::pi() / double(edgeAligned.numSides); // Half angle
          const auto ca = std::cos(a);
          const auto x = std::cos(angle) / ca;
          const auto y = std::sin(angle) / ca;
          vertices.emplace_back(x, y);
        }
        return vertices;
      },
      [](const VertexAlignedCircle& vertexAligned) {
        ensure(vertexAligned.numSides > 2, "shape has at least three sides");
        auto vertices = std::vector<vm::vec2d>{};
        for (size_t i = 0; i < vertexAligned.numSides; ++i)
        {
          const auto angle = double(i) * vm::Cd::two_pi() / double(vertexAligned.numSides)
                             - vm::Cd::half_pi();
          const auto x = std::cos(angle);
          const auto y = std::sin(angle);
          vertices.emplace_back(x, y);
        }
        return vertices;
      }),
    circleShape);
}

auto makeUnitCylinder(const CircleShape& circleShape)
{
  auto vertices = std::vector<vm::vec3d>{};
  for (const auto& v : makeUnitCircle(circleShape))
  {
    vertices.emplace_back(v.x(), v.y(), -1.0);
    vertices.emplace_back(v.x(), v.y(), +1.0);
  }
  return vertices;
}
} // namespace

Result<Brush> BrushBuilder::createCylinder(
  const vm::bbox3d& bounds,
  const CircleShape& circleShape,
  const vm::axis::type axis,
  const std::string& textureName) const
{
  const auto transform = vm::translation_matrix(bounds.min)
                         * vm::scaling_matrix(bounds.size())
                         * vm::translation_matrix(vm::vec3d{0.5, 0.5, 0.5})
                         * vm::scaling_matrix(vm::vec3d{0.5, 0.5, 0.5})
                         * vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::vec3d::axis(axis));

  const auto cylinder = makeUnitCylinder(circleShape);
  const auto vertices =
    cylinder | std::views::transform([&](const auto& v) { return transform * v; })
    | kdl::to_vector;

  return createBrush(vertices, textureName);
}

namespace
{

auto makeHollowCylinderOuterCircle(const vm::vec2d& size, const CircleShape& circleShape)
{
  const auto unitCircle = makeUnitCircle(circleShape);
  return unitCircle
         | std::views::transform(
           [t = vm::scaling_matrix(size / 2.0)](const auto& v) { return t * v; })
         | kdl::to_vector;
}

Result<std::vector<vm::vec2d>> makeHollowCylinderInnerCircle(
  const std::vector<vm::vec2d>& outerCircle, const double thickness)
{
  const auto numSides = outerCircle.size();

  auto outerLines = std::vector<vm::line2d>{};
  outerLines.reserve(numSides);
  for (size_t i = 0; i < numSides; ++i)
  {
    const auto p1 = outerCircle[i];
    const auto p2 = outerCircle[(i + 1) % numSides];
    outerLines.emplace_back(p1, vm::normalize(p2 - p1));
  }

  const auto innerLines =
    outerLines | std::views::transform([&](const auto& l) {
      const auto offsetDir = vm::vec2d{-l.direction.y(), l.direction.x()};
      return vm::line2d{l.point + offsetDir * thickness, l.direction};
    })
    | kdl::to_vector;

  auto innerCircle = std::vector<vm::vec2d>{};
  innerCircle.reserve(numSides);
  for (size_t i = 0; i < numSides; ++i)
  {
    const auto l1 = innerLines[(i + numSides - 1) % numSides];
    const auto l2 = innerLines[i];
    const auto d = vm::intersect_line_line(l1, l2);
    if (!d)
    {
      return Error{"Failed to intersect lines"};
    }

    innerCircle.push_back(vm::point_at_distance(l1, *d));
  }

  return innerCircle;
}

auto makeHollowCylinderFragmentVertices(
  const std::vector<vm::vec2d>& outerCircle,
  const std::vector<vm::vec2d>& innerCircle,
  const size_t i,
  const double sz)
{
  assert(outerCircle.size() == innerCircle.size());
  const auto numSides = outerCircle.size();

  const auto po = outerCircle[(i + 0) % numSides];
  const auto pi = innerCircle[(i + 0) % numSides];
  const auto no = outerCircle[(i + 1) % numSides];
  const auto ni = innerCircle[(i + 1) % numSides];

  const auto brushVertices = std::vector<vm::vec3d>{
    {po, -sz},
    {po, +sz},
    {pi, -sz},
    {pi, +sz},
    {no, -sz},
    {no, +sz},
    {ni, -sz},
    {ni, +sz},
  };

  return brushVertices;
}

} // namespace

Result<std::vector<Brush>> BrushBuilder::createHollowCylinder(
  const vm::bbox3d& bounds,
  const double thickness,
  const CircleShape& circleShape,
  const vm::axis::type axis,
  const std::string& textureName) const
{
  const auto rotation = vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::vec3d::axis(axis));
  const auto rotatedSize = rotation * bounds.size();

  const auto outerCircle = makeHollowCylinderOuterCircle(rotatedSize.xy(), circleShape);

  return makeHollowCylinderInnerCircle(outerCircle, thickness)
    .and_then([&](const auto& innerCircle) {
      ensure(
        innerCircle.size() == outerCircle.size(), "inner circle has same size as outer");

      const auto numFragments = outerCircle.size();

      const auto transform =
        vm::translation_matrix(bounds.min + bounds.size() / 2.0) * rotation;

      auto brushes = std::vector<Result<Brush>>{};
      brushes.reserve(numFragments);

      const auto sz = rotatedSize.z() / 2.0;
      for (size_t i = 0; i < numFragments; ++i)
      {
        const auto fragmentVertices =
          makeHollowCylinderFragmentVertices(outerCircle, innerCircle, i, sz);
        const auto transformedBrushVertices =
          fragmentVertices
          | std::views::transform([&](const auto& v) { return transform * v; })
          | kdl::to_vector;

        brushes.push_back(createBrush(transformedBrushVertices, textureName));
      }

      return brushes | kdl::fold;
    });
}

namespace
{
auto makeUnitCone(const CircleShape& circleShape)
{
  auto vertices = std::vector<vm::vec3d>{};
  for (const auto& v : makeUnitCircle(circleShape))
  {
    vertices.emplace_back(v.x(), v.y(), -1.0);
  }
  vertices.emplace_back(0.0, 0.0, 1.0);
  return vertices;
}
} // namespace

Result<Brush> BrushBuilder::createCone(
  const vm::bbox3d& bounds,
  const CircleShape& circleShape,
  const vm::axis::type axis,
  const std::string& textureName) const
{
  const auto transform = vm::translation_matrix(bounds.min)
                         * vm::scaling_matrix(bounds.size())
                         * vm::translation_matrix(vm::vec3d{0.5, 0.5, 0.5})
                         * vm::scaling_matrix(vm::vec3d{0.5, 0.5, 0.5})
                         * vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::vec3d::axis(axis));

  const auto cone = makeUnitCone(circleShape);
  const auto vertices =
    cone | std::views::transform([&](const auto& v) { return transform * v; })
    | kdl::to_vector;

  return createBrush(vertices, textureName);
}

namespace
{
auto makeRing(const double angle, const CircleShape& circleShape)
{
  const auto r = std::sin(angle);
  const auto z = std::cos(angle);
  const auto circle = makeUnitCircle(circleShape);
  return circle
         | std::views::transform([&, t = vm::scaling_matrix(vm::vec2d{r, r})](
                                   const auto& v) { return vm::vec3d{t * v, z}; })
         | kdl::to_vector;
}

} // namespace

Result<Brush> BrushBuilder::createUVSphere(
  const vm::bbox3d& bounds,
  const CircleShape& circleShape,
  const size_t numRings,
  const vm::axis::type axis,
  const std::string& textureName) const
{
  const auto angleDelta = vm::Cd::pi() / (double(numRings) + 1.0);
  auto previousRing = makeRing(angleDelta, circleShape);
  const auto numSides = previousRing.size();

  auto faces = std::vector<Result<BrushFace>>{};

  // top cone
  for (size_t i = 0; i < numSides; ++i)
  {
    const auto p1 = vm::vec3d{0, 0, 1};
    const auto p2 = previousRing[(i + 1) % numSides];
    const auto p3 = previousRing[(i + 0) % numSides];

    faces.push_back(BrushFace::create(
      p1, p2, p3, BrushFaceAttributes{textureName, m_defaultAttribs}, m_mapFormat));
  }

  // // quad rings
  for (size_t i = 0; i < numRings - 1; ++i)
  {
    auto currentRing = makeRing(double(i + 2) * angleDelta, circleShape);
    for (size_t j = 0; j < numSides; ++j)
    {
      const auto p1 = currentRing[(j + 1) % numSides];
      const auto p2 = currentRing[(j + 0) % numSides];
      const auto p3 = previousRing[(j + 0) % numSides];

      faces.push_back(BrushFace::create(
        p1, p2, p3, BrushFaceAttributes{textureName, m_defaultAttribs}, m_mapFormat));
    }
    previousRing = std::move(currentRing);
  }

  // bottom cone
  for (size_t i = 0; i < numSides; ++i)
  {
    const auto p1 = vm::vec3d{0, 0, -1};
    const auto p2 = previousRing[(i + 0) % numSides];
    const auto p3 = previousRing[(i + 1) % numSides];

    faces.push_back(BrushFace::create(
      p1, p2, p3, BrushFaceAttributes{textureName, m_defaultAttribs}, m_mapFormat));
  }

  // ensure that the sphere fills the bounds when number or rings is equal
  const auto centerRingRadius = std::sin(angleDelta * double(numRings / 2));
  const auto extraScale = numRings % 2 == 0 ? 1.0 / centerRingRadius : 1.0;

  return std::move(faces) | kdl::fold | kdl::and_then([&](auto f) {
           return Brush::create(m_worldBounds, std::move(f));
         })
         | kdl::and_then([&](auto b) {
             const auto transform =
               vm::translation_matrix(bounds.min) * vm::scaling_matrix(bounds.size())
               * vm::scaling_matrix(vm::vec3d{0.5, 0.5, 0.5})
               * vm::translation_matrix(vm::vec3d{1, 1, 1})
               * vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::vec3d::axis(axis))
               * vm::scaling_matrix(vm::vec3d{extraScale, extraScale, 1.0});
             return b.transform(m_worldBounds, transform, false)
                    | kdl::transform([&]() { return std::move(b); });
           });
}

Result<Brush> BrushBuilder::createIcoSphere(
  const vm::bbox3d& bounds, const size_t iterations, const std::string& textureName) const
{
  const auto [sphereVertices_, sphereIndices] = render::sphereMesh<double>(iterations);

  return sphereIndices
         | std::views::transform(
           [sphereVertices = sphereVertices_, &textureName, this](const auto& face) {
             const auto& p1 = sphereVertices[face[0]];
             const auto& p2 = sphereVertices[face[1]];
             const auto& p3 = sphereVertices[face[2]];
             return BrushFace::create(
               p1,
               p2,
               p3,
               BrushFaceAttributes{textureName, m_defaultAttribs},
               m_mapFormat);
           })
         | kdl::fold | kdl::and_then([&](auto f) {
             return Brush::create(m_worldBounds, std::move(f));
           })
         | kdl::and_then([&](auto b) {
             const auto transform = vm::translation_matrix(bounds.min)
                                    * vm::scaling_matrix(bounds.size())
                                    * vm::scaling_matrix(vm::vec3d{0.5, 0.5, 0.5})
                                    * vm::translation_matrix(vm::vec3d{1, 1, 1});
             return b.transform(m_worldBounds, transform, false)
                    | kdl::transform([&]() { return std::move(b); });
           });
}

Result<Brush> BrushBuilder::createBrush(
  const std::vector<vm::vec3d>& points, const std::string& materialName) const
{
  return createBrush(Polyhedron3{points}, materialName);
}

Result<Brush> BrushBuilder::createBrush(
  const Polyhedron3& polyhedron, const std::string& materialName) const
{
  if (polyhedron.empty())
  {
    return Error{"Cannot create brush from empty polyhedron"};
  }

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
} // namespace tb::mdl
