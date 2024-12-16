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
#include "kdl/vector_utils.h"

#include "vm/intersection.h"
#include "vm/line.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"

#include <cassert>
#include <cmath>
#include <ranges>
#include <string>
#include <utility>

namespace tb::mdl
{
namespace
{

auto numSidesToPrecision(const size_t numSides)
{
  return size_t(std::max(0.0, std::ceil(std::log2(double(numSides) / 12.0))));
}

auto precisionToNumSides(const size_t precision)
{
  return size_t(std::pow(2.0, precision) * 12);
}

} // namespace

EdgeAlignedCircle::EdgeAlignedCircle() = default;

EdgeAlignedCircle::EdgeAlignedCircle(const size_t numSides_)
  : numSides{numSides_}
{
}

EdgeAlignedCircle::EdgeAlignedCircle(const VertexAlignedCircle& circleShape)
  : EdgeAlignedCircle{circleShape.numSides}
{
}

EdgeAlignedCircle::EdgeAlignedCircle(const ScalableCircle& circleShape)
  : numSides{precisionToNumSides(circleShape.precision)}
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

VertexAlignedCircle::VertexAlignedCircle(const ScalableCircle& circleShape)
  : numSides{precisionToNumSides(circleShape.precision)}
{
}

ScalableCircle::ScalableCircle() = default;

ScalableCircle::ScalableCircle(const size_t precision_)
  : precision{precision_}
{
}

ScalableCircle::ScalableCircle(const VertexAlignedCircle& circleShape)
  : precision{numSidesToPrecision(circleShape.numSides)}
{
}

ScalableCircle::ScalableCircle(const EdgeAlignedCircle& circleShape)
  : precision{numSidesToPrecision(circleShape.numSides)}
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
auto makeEdgeAlignedCircle(const size_t numSides, const vm::bbox2d& bounds)
{
  ensure(numSides > 2, "shape has at least three sides");

  const auto transform = vm::translation_matrix(bounds.min)
                         * vm::scaling_matrix(bounds.size())
                         * vm::translation_matrix(vm::vec2d{0.5, 0.5})
                         * vm::scaling_matrix(vm::vec2d{0.5, 0.5});

  auto vertices = std::vector<vm::vec2d>{};
  for (size_t i = 0; i < numSides; ++i)
  {
    const auto angle =
      (double(i) + 0.5) * vm::Cd::two_pi() / double(numSides) - vm::Cd::half_pi();
    const auto a = vm::Cd::pi() / double(numSides); // Half angle
    const auto ca = std::cos(a);
    const auto x = std::cos(angle) / ca;
    const auto y = std::sin(angle) / ca;
    vertices.emplace_back(x, y);
  }
  return transform * vertices;
}

auto makeVertexAlignedCircle(const size_t numSides, const vm::bbox2d& bounds)
{
  ensure(numSides > 2, "shape has at least three sides");

  const auto transform = vm::translation_matrix(bounds.min)
                         * vm::scaling_matrix(bounds.size())
                         * vm::translation_matrix(vm::vec2d{0.5, 0.5})
                         * vm::scaling_matrix(vm::vec2d{0.5, 0.5});

  auto vertices = std::vector<vm::vec2d>{};
  for (size_t i = 0; i < numSides; ++i)
  {
    const auto angle =
      double(i) * vm::Cd::two_pi() / double(numSides) - vm::Cd::half_pi();
    const auto x = std::cos(angle);
    const auto y = std::sin(angle);
    vertices.emplace_back(x, y);
  }
  return transform * vertices;
}

auto makeScalableCircle(const size_t precision, const vm::bbox2d& bounds)
{
  auto vertices = std::vector<vm::vec2d>{
    {-0.25, +1.00},
    {-0.75, +0.75},
    {-1.00, +0.25},
    {-1.00, -0.25},
    {-0.75, -0.75},
    {-0.25, -1.00},
    {+0.25, -1.00},
    {+0.75, -0.75},
    {+1.00, -0.25},
    {+1.00, +0.25},
    {+0.75, +0.75},
    {+0.25, +1.00},
  };

  // Clip off each corner to get a scalable unit circle with double the vertices
  for (size_t i = 0; i < precision; ++i)
  {

    const auto previousVertices = std::exchange(vertices, std::vector<vm::vec2d>{});
    const auto count = previousVertices.size();
    for (size_t j = 0; j < previousVertices.size(); ++j)
    {
      const auto prev = previousVertices[(j + count - 1) % count];
      const auto cur = previousVertices[j];
      const auto next = previousVertices[(j + 1) % count];

      vertices.push_back(prev + (cur - prev) * 0.75);
      vertices.push_back(cur + (next - cur) * 0.25);
    }
  }

  const auto size = bounds.size();
  const auto minSize = vm::min(size.x(), size.y());
  const auto squareSize = vm::vec2d::fill(minSize);

  vertices = vm::scaling_matrix(squareSize) * vm::translation_matrix(vm::vec2d{0.5, 0.5})
             * vm::scaling_matrix(vm::vec2d{0.5, 0.5}) * vertices;

  // Stretch the circle to fit the bounds by moving the right half and the top half
  // instead of uniformly scaling all vertices
  const auto offset = vm::vec2d{
    vm::max(size.x() - size.y(), 0.0),
    vm::max(size.y() - size.x(), 0.0),
  };

  for (auto& v : vertices)
  {
    if (v.x() > minSize / 2.0)
    {
      v = vm::vec2d{v.x() + offset.x(), v.y()};
    }
    if (v.y() > minSize / 2.0)
    {
      v = vm::vec2d{v.x(), v.y() + offset.y()};
    }
  }

  return vm::translation_matrix(bounds.min) * vertices;
}

auto makeCircle(const CircleShape& circleShape, const vm::bbox2d& bounds)
{

  return std::visit(
    kdl::overload(
      [&](const EdgeAlignedCircle& edgeAligned) {
        return makeEdgeAlignedCircle(edgeAligned.numSides, bounds);
      },
      [&](const VertexAlignedCircle& vertexAligned) {
        return makeVertexAlignedCircle(vertexAligned.numSides, bounds);
      },
      [&](const ScalableCircle& scalable) {
        return makeScalableCircle(scalable.precision, bounds);
      }),
    circleShape);
}

auto makeCylinder(const CircleShape& circleShape, const vm::bbox3d& boundsXY)
{
  auto vertices = std::vector<vm::vec3d>{};
  for (const auto& v : makeCircle(circleShape, boundsXY.xy()))
  {
    vertices.emplace_back(v.x(), v.y(), boundsXY.min.z());
    vertices.emplace_back(v.x(), v.y(), boundsXY.max.z());
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
  const auto toXY = vm::rotation_matrix(vm::vec3d::axis(axis), vm::vec3d{0, 0, 1});
  const auto fromXY = vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::vec3d::axis(axis));

  const auto cylinder = makeCylinder(circleShape, bounds.transform(toXY));
  return createBrush(fromXY * cylinder, textureName);
}

namespace
{

auto makeVerticesForWedges(
  const std::vector<vm::vec2d>& outerCircle, const vm::bbox2d& bounds)
{
  // The bounds are too small to create an inner circle, but we can still create
  // wedges for the hollow cylinder.
  // Generate four points (here called corners) where the wedges should meet.
  // If the bounds are square, all corners coincide. If the bounds are
  // rectangular, two pairs of corners coincide.
  // Then map each vertex of the outer circle to the closest corner.
  const auto offset = vm::min(bounds.size().x(), bounds.size().y()) / 2.0;
  const auto corners = std::vector<vm::vec2d>{
    {bounds.min.x() + offset, bounds.min.y() + offset},
    {bounds.min.x() + offset, bounds.max.y() - offset},
    {bounds.max.x() - offset, bounds.min.y() + offset},
    {bounds.max.x() - offset, bounds.max.y() - offset},
  };
  return outerCircle | std::views::transform([&](const auto& v) {
           return *std::ranges::min_element(corners, [&](const auto& a, const auto& b) {
             return vm::squared_distance(v, a) < vm::squared_distance(v, b);
           });
         })
         | kdl::to_vector;
}

auto makeHollowCylinderInnerCircle(
  const std::vector<vm::vec2d>& outerCircle,
  const double thickness,
  const CircleShape& circleShape,
  const vm::bbox2d& bounds)
{
  if (bounds.size().x() <= thickness * 2.0 || bounds.size().y() <= thickness * 2.0)
  {
    return Result<std::vector<vm::vec2d>>{makeVerticesForWedges(outerCircle, bounds)};
  }

  return std::visit(
    kdl::overload(
      [&](const ScalableCircle& scalable) -> Result<std::vector<vm::vec2d>> {
        const auto delta = vm::vec2d{thickness, thickness};
        const auto innerBounds = vm::bbox2d{bounds.min + delta, bounds.max - delta};
        return makeScalableCircle(scalable.precision, innerBounds);
      },
      [&](const auto& axisOrVertexAligned) -> Result<std::vector<vm::vec2d>> {
        const auto numSides = axisOrVertexAligned.numSides;
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
      }),
    circleShape);
}

auto makeHollowCylinderFragmentVertices(
  const std::vector<vm::vec2d>& outerCircle,
  const std::vector<vm::vec2d>& innerCircle,
  const size_t i,
  const vm::bbox3d& boundsXY)
{
  assert(outerCircle.size() == innerCircle.size());
  const auto numSides = outerCircle.size();

  const auto po = outerCircle[(i + 0) % numSides];
  const auto pi = innerCircle[(i + 0) % numSides];
  const auto no = outerCircle[(i + 1) % numSides];
  const auto ni = innerCircle[(i + 1) % numSides];

  const auto brushVertices = std::vector<vm::vec3d>{
    {po, boundsXY.min.z()},
    {po, boundsXY.max.z()},
    {pi, boundsXY.min.z()},
    {pi, boundsXY.max.z()},
    {no, boundsXY.min.z()},
    {no, boundsXY.max.z()},
    {ni, boundsXY.min.z()},
    {ni, boundsXY.max.z()},
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
  const auto toXY = vm::rotation_matrix(vm::vec3d::axis(axis), vm::vec3d{0, 0, 1});
  const auto fromXY = vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::vec3d::axis(axis));
  const auto boundsXY = bounds.transform(toXY);

  const auto outerCircle = makeCircle(circleShape, boundsXY.xy());

  return makeHollowCylinderInnerCircle(outerCircle, thickness, circleShape, boundsXY.xy())
    .and_then([&](const auto& innerCircle) {
      ensure(
        innerCircle.size() == outerCircle.size(), "inner circle has same size as outer");

      const auto numFragments = outerCircle.size();

      auto brushes = std::vector<Result<Brush>>{};
      brushes.reserve(numFragments);

      for (size_t i = 0; i < numFragments; ++i)
      {
        const auto fragmentVertices =
          makeHollowCylinderFragmentVertices(outerCircle, innerCircle, i, boundsXY);
        const auto rotatedFragmentVertices = fromXY * fragmentVertices;

        brushes.push_back(createBrush(rotatedFragmentVertices, textureName));
      }

      return brushes | kdl::fold;
    });
}

namespace
{

auto setZ(const std::vector<vm::vec2d>& vertices, const double z)
{
  return vertices | std::views::transform([&](const auto& v) { return vm::vec3d{v, z}; })
         | kdl::to_vector;
}

/** If a scalable cone is stretched, it doesn't have one vertex as the tip. Instead, the
 * tip is an edge.
 */
auto makeScalableConeTip(const vm::bbox3d& boundsXY)
{
  const auto offset = vm::min(boundsXY.xy().size().x(), boundsXY.xy().size().y()) / 2.0;
  return kdl::vec_sort_and_remove_duplicates(std::vector<vm::vec2d>{
    {boundsXY.xy().min.x() + offset, boundsXY.xy().min.y() + offset},
    {boundsXY.xy().min.x() + offset, boundsXY.xy().max.y() - offset},
    {boundsXY.xy().max.x() - offset, boundsXY.xy().min.y() + offset},
    {boundsXY.xy().max.x() - offset, boundsXY.xy().max.y() - offset},
  });
}

auto makeCone(const CircleShape& circleShape, const vm::bbox3d& boundsXY)
{
  return std::visit(
    kdl::overload(
      [&](const ScalableCircle& scalableCircle) {
        return kdl::vec_concat(
          setZ(
            makeScalableCircle(scalableCircle.precision, boundsXY.xy()),
            boundsXY.min.z()),
          setZ(makeScalableConeTip(boundsXY), boundsXY.max.z()));
      },
      [&](const auto&) {
        return kdl::vec_concat(
          setZ(makeCircle(circleShape, boundsXY.xy()), boundsXY.min.z()),
          std::vector{vm::vec3d{boundsXY.xy().center(), boundsXY.max.z()}});
      }),
    circleShape);
}
} // namespace

Result<Brush> BrushBuilder::createCone(
  const vm::bbox3d& bounds,
  const CircleShape& circleShape,
  const vm::axis::type axis,
  const std::string& textureName) const
{
  const auto toXY = vm::rotation_matrix(vm::vec3d::axis(axis), vm::vec3d{0, 0, 1});
  const auto fromXY = vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::vec3d::axis(axis));
  const auto boundsXY = bounds.transform(toXY);

  const auto cone = makeCone(circleShape, boundsXY);
  return createBrush(fromXY * cone, textureName);
}

namespace
{

auto subDivideRatios(const std::vector<double>& ratios)
{
  auto newRatios = std::vector<double>{};
  newRatios.push_back(0.0);
  for (size_t j = 1; j < ratios.size(); ++j)
  {
    const auto previousSize = ratios[j - 1];
    const auto currentSize = ratios[j];
    newRatios.push_back((previousSize + currentSize) / 2.0);
  }
  newRatios.push_back(1.0);
  return newRatios;
}

auto makeSizeRatiosPerRing(const size_t precision)
{
  auto sizeRatios = std::vector<double>{0.0, 1.0 / 2.0, 7.0 / 8.0, 1.0};
  for (size_t i = 0; i < precision; ++i)
  {
    sizeRatios = subDivideRatios(sizeRatios);
  }

  const auto n = sizeRatios.size();
  for (size_t i = 0; i < n - 1; ++i)
  {
    sizeRatios.push_back(sizeRatios[n - i - 2]);
  }

  return sizeRatios;
}

auto makeZRatiosPerRing(const size_t precision)
{
  auto zRatios = std::vector<double>{1.0, 7.0 / 8.0, 1.0 / 2.0, 0.0};
  for (size_t i = 0; i < precision; ++i)
  {
    zRatios = subDivideRatios(zRatios);
  }

  const auto n = zRatios.size();
  for (size_t i = 0; i < n - 1; ++i)
  {
    zRatios.push_back(-zRatios[n - i - 2]);
  }

  return zRatios;
}

auto makeScalableUVSphere(const vm::bbox3d& boundsXY, const size_t precision)
{
  const auto zRatios = makeZRatiosPerRing(precision);
  const auto getZ = [&](const size_t i) {
    const auto center = boundsXY.center();
    const auto size = boundsXY.size() / 2.0;
    return center.z() + size.z() * zRatios[i];
  };

  const auto sizeRatios = makeSizeRatiosPerRing(precision);
  const auto getBounds = [&](const size_t i) {
    const auto s = vm::min(boundsXY.size().x(), boundsXY.size().y()) / 2.0;
    return boundsXY.xy().expand(-s * (1 - sizeRatios[i]));
  };

  const auto numRings = size_t(std::pow(2, precision)) * 12 / 2 - 1;

  auto vertices = std::vector<vm::vec3d>{};
  vertices =
    kdl::vec_concat(std::move(vertices), setZ(makeScalableConeTip(boundsXY), getZ(0)));
  for (size_t i = 1; i <= numRings; ++i)
  {
    vertices = kdl::vec_concat(
      std::move(vertices), setZ(makeScalableCircle(precision, getBounds(i)), getZ(i)));
  }
  vertices = kdl::vec_concat(
    std::move(vertices), setZ(makeScalableConeTip(boundsXY), getZ(numRings + 1)));

  return vertices;
}

auto makeRing(
  const double angle, const CircleShape& circleShape, const vm::bbox3d& boundsXY)
{
  const auto r = std::sin(angle);
  const auto z = boundsXY.center().z() + std::cos(angle) * boundsXY.size().z() / 2.0;
  const auto t = vm::translation_matrix(boundsXY.xy().center())
                 * vm::scaling_matrix(vm::vec2d{r, r})
                 * vm::translation_matrix(-boundsXY.xy().center());
  const auto circle = t * makeCircle(circleShape, boundsXY.xy());
  return circle | std::views::transform([&](const auto& v) { return vm::vec3d{v, z}; })
         | kdl::to_vector;
}

auto makeAlignedUVSphere(
  const vm::bbox3d& boundsXY, const CircleShape& circleShape, const size_t numRings)
{
  const auto angleDelta = vm::Cd::pi() / (double(numRings) + 1.0);

  auto vertices = std::vector<vm::vec3d>{};
  vertices.emplace_back(boundsXY.xy().center(), boundsXY.max.z());

  for (size_t i = 0; i < numRings; ++i)
  {
    vertices = kdl::vec_concat(
      std::move(vertices), makeRing(double(i) * angleDelta, circleShape, boundsXY));
  }

  vertices.emplace_back(boundsXY.xy().center(), boundsXY.min.z());

  // ensure that the sphere fills the bounds when number of rings is equal
  const auto centerRingRadius = std::sin(angleDelta * double(numRings / 2));
  const auto extraScale = numRings % 2 == 0 ? 1.0 / centerRingRadius : 1.0;
  const auto transform = vm::translation_matrix(boundsXY.center())
                         * vm::scaling_matrix(vm::vec3d{extraScale, extraScale, 1.0})
                         * vm::translation_matrix(-boundsXY.center());

  return transform * vertices;
}

} // namespace

Result<Brush> BrushBuilder::createUVSphere(
  const vm::bbox3d& bounds,
  const CircleShape& circleShape,
  const size_t numRings,
  const vm::axis::type axis,
  const std::string& textureName) const
{
  const auto fromXY = vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::vec3d::axis(axis));
  const auto toXY = vm::rotation_matrix(vm::vec3d::axis(axis), vm::vec3d{0, 0, 1});
  const auto boundsXY = bounds.transform(toXY);

  const auto sphere = std::visit(
    kdl::overload(
      [&](const ScalableCircle& scalable) {
        return makeScalableUVSphere(boundsXY, scalable.precision);
      },
      [&](const auto& edgeOrVertexAligned) {
        return makeAlignedUVSphere(boundsXY, edgeOrVertexAligned, numRings);
      }),
    circleShape);

  return createBrush(fromXY * sphere, textureName);
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
