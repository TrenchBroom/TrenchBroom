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

#include "Brush.h"

#include "Polyhedron.h"
#include "Polyhedron_Matcher.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushGeometry.h"
#include "mdl/MapFormat.h"
#include "mdl/UVCoordSystem.h"

#include "kdl/range_utils.h"
#include "kdl/ranges/to.h"
#include "kdl/reflection_impl.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/vector_utils.h"

#include "vm/mat_ext.h"
#include "vm/polygon.h"
#include "vm/ray.h"
#include "vm/segment.h"
#include "vm/util.h"

#include <iterator>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace tb::mdl
{

kdl_reflect_impl(Brush);

class Brush::CopyCallback : public BrushGeometry::CopyCallback
{
public:
  void faceWasCopied(
    const BrushFaceGeometry* original, BrushFaceGeometry* copy) const override
  {
    copy->setPayload(original->payload());
  }
};

Brush::Brush() {}

Brush::Brush(const Brush& other)
  : m_faces{other.m_faces}
  , m_geometry{
      other.m_geometry
        ? std::make_unique<BrushGeometry>(*other.m_geometry, CopyCallback())
        : nullptr}
{
  if (m_geometry)
  {
    for (BrushFaceGeometry* faceGeometry : m_geometry->faces())
    {
      if (const auto faceIndex = faceGeometry->payload())
      {
        BrushFace& face = m_faces[*faceIndex];
        face.setGeometry(faceGeometry);
      }
    }
  }
}

Brush::Brush(Brush&& other) noexcept = default;

Brush& Brush::operator=(const Brush& other)
{
  *this = Brush{other};
  return *this;
}

Brush& Brush::operator=(Brush&& other) noexcept = default;

Brush::~Brush() = default;

Brush::Brush(std::vector<BrushFace> faces)
  : m_faces{std::move(faces)}
{
}

Result<Brush> Brush::create(const vm::bbox3d& worldBounds, std::vector<BrushFace> faces)
{
  auto brush = Brush{std::move(faces)};
  return brush.updateGeometryFromFaces(worldBounds)
         | kdl::transform([&]() { return std::move(brush); });
}

Result<void> Brush::updateGeometryFromFaces(const vm::bbox3d& worldBounds)
{
  // First, add all faces to the brush geometry
  BrushFace::sortFaces(m_faces);

  auto geometry = std::make_unique<BrushGeometry>(worldBounds);

  for (size_t i = 0u; i < m_faces.size(); ++i)
  {
    BrushFace& face = m_faces[i];
    const auto result = geometry->clip(face.boundary());
    if (result.success())
    {
      BrushFaceGeometry* faceGeometry = result.face();
      face.setGeometry(faceGeometry);
      faceGeometry->setPayload(i);
    }
    else if (result.empty())
    {
      return Error{"Brush is empty"};
    }
  }

  // Correct vertex positions and heal short edges
  geometry->correctVertexPositions();
  if (!geometry->healEdges())
  {
    return Error{"Brush is invalid"};
  }

  // Now collect all faces which still remain
  std::vector<BrushFace> remainingFaces;
  remainingFaces.reserve(m_faces.size());

  for (BrushFaceGeometry* faceGeometry : geometry->faces())
  {
    if (const auto faceIndex = faceGeometry->payload())
    {
      remainingFaces.push_back(std::move(m_faces[*faceIndex]));
      faceGeometry->setPayload(remainingFaces.size() - 1u);
    }
    else
    {
      return Error{"Brush is incomplete"};
    }
  }

  m_faces = std::move(remainingFaces);
  m_geometry = std::move(geometry);

  assert(checkFaceLinks());

  return kdl::void_success;
}

const vm::bbox3d& Brush::bounds() const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->bounds();
}

std::optional<size_t> Brush::findFace(const std::string& materialName) const
{
  return kdl::index_of(m_faces, [&](const BrushFace& face) {
    return face.attributes().materialName() == materialName;
  });
}

std::optional<size_t> Brush::findFace(const vm::vec3d& normal) const
{
  return kdl::index_of(m_faces, [&](const BrushFace& face) {
    return vm::is_equal(face.boundary().normal, normal, vm::Cd::almost_zero());
  });
}

std::optional<size_t> Brush::findFace(const vm::plane3d& boundary) const
{
  return kdl::index_of(m_faces, [&](const BrushFace& face) {
    return vm::is_equal(face.boundary(), boundary, vm::Cd::almost_zero());
  });
}

std::optional<size_t> Brush::findFace(
  const vm::polygon3d& vertices, const double epsilon) const
{
  return kdl::index_of(
    m_faces, [&](const BrushFace& face) { return face.hasVertices(vertices, epsilon); });
}

std::optional<size_t> Brush::findFace(
  const std::vector<vm::polygon3d>& candidates, const double epsilon) const
{
  for (const auto& candidate : candidates)
  {
    if (const auto faceIndex = findFace(candidate, epsilon))
    {
      return faceIndex;
    }
  }
  return std::nullopt;
}

const BrushFace& Brush::face(const size_t index) const
{
  assert(index < faceCount());
  return m_faces[index];
}

BrushFace& Brush::face(const size_t index)
{
  assert(index < faceCount());
  return m_faces[index];
}

size_t Brush::faceCount() const
{
  return m_faces.size();
}

const std::vector<BrushFace>& Brush::faces() const
{
  return m_faces;
}

std::vector<BrushFace>& Brush::faces()
{
  return m_faces;
}

bool Brush::closed() const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->closed();
}

bool Brush::fullySpecified() const
{
  ensure(m_geometry != nullptr, "geometry is null");

  for (auto* current : m_geometry->faces())
  {
    if (!current->payload().has_value())
    {
      return false;
    }
  }
  return true;
}

void Brush::cloneFaceAttributesFrom(const Brush& brush)
{
  for (auto& destination : m_faces)
  {
    if (const auto sourceIndex = brush.findFace(destination.boundary()))
    {
      const auto& source = brush.face(*sourceIndex);
      destination.setAttributes(source.attributes());

      auto snapshot = source.takeUVCoordSystemSnapshot();
      if (snapshot != nullptr)
      {
        destination.copyUVCoordSystemFromFace(
          *snapshot, source.attributes(), source.boundary(), WrapStyle::Projection);
      }
    }
  }
}

static const BrushFace* findBestMatchingFace(
  const BrushFace& face, const std::vector<const BrushFace*>& candidates)
{
  if (candidates.empty())
  {
    return nullptr;
  }

  // First, look for coplanar candidates
  const auto coplanarCandidates = kdl::vec_filter(
    candidates,
    [&](const BrushFace* candidate) { return candidate->coplanarWith(face.boundary()); });
  ;

  if (!coplanarCandidates.empty())
  {
    // Return the largest coplanar face
    return *std::max_element(
      std::begin(coplanarCandidates),
      std::end(coplanarCandidates),
      [](const BrushFace* lhs, const BrushFace* rhs) {
        return lhs->area() < rhs->area();
      });
  }

  // No coplanar faces. Return the one with the smallest "face center off reference
  // plane" distance.
  const auto faceCenterOffPlaneDist = [&](const BrushFace* candidate) -> double {
    return vm::abs(face.boundary().point_distance(candidate->center()));
  };

  return *std::min_element(
    std::begin(candidates),
    std::end(candidates),
    [&](const BrushFace* lhs, const BrushFace* rhs) {
      return faceCenterOffPlaneDist(lhs) < faceCenterOffPlaneDist(rhs);
    });
}

void Brush::cloneFaceAttributesFrom(const std::vector<const Brush*>& brushes)
{
  auto candidates = std::vector<const BrushFace*>{};
  for (const auto* candidateBrush : brushes)
  {
    for (const auto& candidateFace : candidateBrush->faces())
    {
      candidates.push_back(&candidateFace);
    }
  }

  for (auto& face : m_faces)
  {
    if (const auto* bestMatch = findBestMatchingFace(face, candidates))
    {
      face.setAttributes(bestMatch->attributes());

      auto snapshot = bestMatch->takeUVCoordSystemSnapshot();
      if (snapshot != nullptr)
      {
        face.copyUVCoordSystemFromFace(
          *snapshot, bestMatch->attributes(), face.boundary(), WrapStyle::Projection);
      }
    }
  }
}

void Brush::cloneInvertedFaceAttributesFrom(const Brush& brush)
{
  for (auto& destination : m_faces)
  {
    if (const auto sourceIndex = brush.findFace(destination.boundary().flip()))
    {
      const auto& source = brush.face(*sourceIndex);
      // Todo: invert the face attributes?
      destination.setAttributes(source.attributes());

      auto snapshot = source.takeUVCoordSystemSnapshot();
      if (snapshot != nullptr)
      {
        destination.copyUVCoordSystemFromFace(
          *snapshot, source.attributes(), destination.boundary(), WrapStyle::Projection);
      }
    }
  }
}

Result<void> Brush::clip(const vm::bbox3d& worldBounds, BrushFace face)
{
  m_faces.push_back(std::move(face));
  return updateGeometryFromFaces(worldBounds);
}

Result<void> Brush::moveBoundary(
  const vm::bbox3d& worldBounds,
  const size_t faceIndex,
  const vm::vec3d& delta,
  const bool lockMaterial)
{
  assert(faceIndex < faceCount());

  return m_faces[faceIndex].transform(vm::translation_matrix(delta), lockMaterial)
         | kdl::and_then([&]() { return updateGeometryFromFaces(worldBounds); });
}

Result<void> Brush::expand(
  const vm::bbox3d& worldBounds, const double delta, const bool lockMaterial)
{
  for (auto& face : m_faces)
  {
    const vm::vec3d moveAmount = face.boundary().normal * delta;
    if (!face.transform(vm::translation_matrix(moveAmount), lockMaterial))
    {
      return Error{"Brush has invalid face"};
    }
  }

  return updateGeometryFromFaces(worldBounds);
}

size_t Brush::vertexCount() const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->vertexCount();
}

const Brush::VertexList& Brush::vertices() const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->vertices();
}

const std::vector<vm::vec3d> Brush::vertexPositions() const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->vertexPositions();
}

bool Brush::hasVertex(const vm::vec3d& position, const double epsilon) const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->findVertexByPosition(position, epsilon) != nullptr;
}

vm::vec3d Brush::findClosestVertexPosition(const vm::vec3d& position) const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->findClosestVertex(position)->position();
}

std::vector<vm::vec3d> Brush::findClosestVertexPositions(
  const std::vector<vm::vec3d>& positions) const
{
  ensure(m_geometry != nullptr, "geometry is null");

  std::vector<vm::vec3d> result;
  result.reserve(positions.size());

  for (const auto& position : positions)
  {
    const auto* newVertex = m_geometry->findClosestVertex(position, CloseVertexEpsilon);
    if (newVertex != nullptr)
    {
      result.push_back(newVertex->position());
    }
  }

  return result;
}

std::vector<vm::segment3d> Brush::findClosestEdgePositions(
  const std::vector<vm::segment3d>& positions) const
{
  ensure(m_geometry != nullptr, "geometry is null");

  auto result = std::vector<vm::segment3d>{};
  result.reserve(positions.size());

  for (const auto& edgePosition : positions)
  {
    if (
      const auto* newEdge = m_geometry->findClosestEdge(
        edgePosition.start(), edgePosition.end(), CloseVertexEpsilon))
    {
      result.emplace_back(
        newEdge->firstVertex()->position(), newEdge->secondVertex()->position());
    }
  }

  return result;
}

std::vector<vm::polygon3d> Brush::findClosestFacePositions(
  const std::vector<vm::polygon3d>& positions) const
{
  ensure(m_geometry != nullptr, "geometry is null");

  auto result = std::vector<vm::polygon3d>{};
  result.reserve(positions.size());

  for (const auto& facePosition : positions)
  {
    if (
      const auto* newFace =
        m_geometry->findClosestFace(facePosition.vertices(), CloseVertexEpsilon))
    {
      result.emplace_back(newFace->vertexPositions());
    }
  }

  return result;
}

bool Brush::hasEdge(const vm::segment3d& edge, const double epsilon) const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->findEdgeByPositions(edge.start(), edge.end(), epsilon) != nullptr;
}

bool Brush::hasFace(const vm::polygon3d& face, const double epsilon) const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->hasFace(face.vertices(), epsilon);
}

size_t Brush::edgeCount() const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->edgeCount();
}

const Brush::EdgeList& Brush::edges() const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return m_geometry->edges();
}

bool Brush::containsPoint(const vm::vec3d& point) const
{
  if (!bounds().contains(point))
  {
    return false;
  }
  else
  {
    for (const auto& face : m_faces)
    {
      if (face.boundary().point_status(point) == vm::plane_status::above)
      {
        return false;
      }
    }
    return true;
  }
}

std::vector<const BrushFace*> Brush::incidentFaces(const BrushVertex* vertex) const
{
  std::vector<const BrushFace*> result;
  result.reserve(m_faces.size());

  auto* first = vertex->leaving();
  auto* current = first;
  do
  {
    if (const auto faceIndex = current->face()->payload())
    {
      result.push_back(&m_faces[*faceIndex]);
    }
    current = current->nextIncident();
  } while (current != first);

  return result;
}

bool Brush::canTransformVertices(
  const vm::bbox3d& worldBounds,
  const std::vector<vm::vec3d>& vertices,
  const vm::mat4x4d& transform) const
{
  return doCanTransformVertices(worldBounds, vertices, transform, true).success;
}

Result<void> Brush::transformVertices(
  const vm::bbox3d& worldBounds,
  const std::vector<vm::vec3d>& vertexPositions,
  const vm::mat4x4d& transform,
  const bool uvLock)
{
  return doTransformVertices(worldBounds, vertexPositions, transform, uvLock);
}

bool Brush::canAddVertex(const vm::bbox3d& worldBounds, const vm::vec3d& position) const
{
  ensure(m_geometry != nullptr, "geometry is null");
  if (!worldBounds.contains(position))
  {
    return false;
  }

  BrushGeometry newGeometry(
    kdl::vec_concat(m_geometry->vertexPositions(), std::vector<vm::vec3d>({position})));
  return newGeometry.hasVertex(position);
}

Result<void> Brush::addVertex(const vm::bbox3d& worldBounds, const vm::vec3d& position)
{
  assert(canAddVertex(worldBounds, position));

  BrushGeometry newGeometry(
    kdl::vec_concat(m_geometry->vertexPositions(), std::vector<vm::vec3d>({position})));
  const PolyhedronMatcher<BrushGeometry> matcher(*m_geometry, newGeometry);
  return updateFacesFromGeometry(worldBounds, matcher, newGeometry);
}

static BrushGeometry removeVerticesFromGeometry(
  const BrushGeometry& geometry, const std::vector<vm::vec3d>& vertexPositions)
{
  std::vector<vm::vec3d> points;
  points.reserve(geometry.vertexCount());

  for (const auto* vertex : geometry.vertices())
  {
    const auto& position = vertex->position();
    if (!kdl::vec_contains(vertexPositions, position))
    {
      points.push_back(position);
    }
  }

  return BrushGeometry(points);
}

bool Brush::canRemoveVertices(
  const vm::bbox3d& /* worldBounds */,
  const std::vector<vm::vec3d>& vertexPositions) const
{
  ensure(m_geometry != nullptr, "geometry is null");
  ensure(!vertexPositions.empty(), "no vertex positions");

  return removeVerticesFromGeometry(*m_geometry, vertexPositions).polyhedron();
}

Result<void> Brush::removeVertices(
  const vm::bbox3d& worldBounds, const std::vector<vm::vec3d>& vertexPositions)
{
  ensure(m_geometry != nullptr, "geometry is null");
  ensure(!vertexPositions.empty(), "no vertex positions");
  assert(canRemoveVertices(worldBounds, vertexPositions));

  const BrushGeometry newGeometry =
    removeVerticesFromGeometry(*m_geometry, vertexPositions);
  const PolyhedronMatcher<BrushGeometry> matcher(*m_geometry, newGeometry);
  return updateFacesFromGeometry(worldBounds, matcher, newGeometry);
}

static BrushGeometry snappedGeometry(const BrushGeometry& geometry, const double snapToF)
{
  std::vector<vm::vec3d> points;
  points.reserve(geometry.vertexCount());

  for (const auto* vertex : geometry.vertices())
  {
    points.push_back(snapToF * vm::round(vertex->position() / snapToF));
  }

  return BrushGeometry(std::move(points));
}

bool Brush::canSnapVertices(
  const vm::bbox3d& /* worldBounds */, const double snapToF) const
{
  ensure(m_geometry != nullptr, "geometry is null");
  return snappedGeometry(*m_geometry, snapToF).polyhedron();
}

Result<void> Brush::snapVertices(
  const vm::bbox3d& worldBounds, const double snapToF, const bool uvLock)
{
  ensure(m_geometry != nullptr, "geometry is null");

  const BrushGeometry newGeometry = snappedGeometry(*m_geometry, snapToF);

  std::map<vm::vec3d, vm::vec3d> vertexMapping;
  for (const auto* vertex : m_geometry->vertices())
  {
    const auto& origin = vertex->position();
    const auto destination = snapToF * round(origin / snapToF);
    if (newGeometry.hasVertex(destination))
    {
      vertexMapping.insert(std::make_pair(origin, destination));
    }
  }

  const PolyhedronMatcher<BrushGeometry> matcher(*m_geometry, newGeometry, vertexMapping);
  return updateFacesFromGeometry(worldBounds, matcher, newGeometry, uvLock);
}

bool Brush::canTransformEdges(
  const vm::bbox3d& worldBounds,
  const std::vector<vm::segment3d>& edgePositions,
  const vm::mat4x4d& transform) const
{
  ensure(m_geometry != nullptr, "geometry is null");
  ensure(!edgePositions.empty(), "no edge positions");

  std::vector<vm::vec3d> vertexPositions;
  vm::segment3d::get_vertices(
    std::begin(edgePositions),
    std::end(edgePositions),
    std::back_inserter(vertexPositions));
  const auto result =
    doCanTransformVertices(worldBounds, vertexPositions, transform, false);

  if (!result.success)
  {
    return false;
  }

  for (const auto& edge : edgePositions)
  {
    if (!result.geometry->hasEdge(transform * edge.start(), transform * edge.end()))
    {
      return false;
    }
  }

  return true;
}

Result<void> Brush::transformEdges(
  const vm::bbox3d& worldBounds,
  const std::vector<vm::segment3d>& edgePositions,
  const vm::mat4x4d& transform,
  const bool uvLock)
{
  assert(canTransformEdges(worldBounds, edgePositions, transform));

  std::vector<vm::vec3d> vertexPositions;
  vm::segment3d::get_vertices(
    std::begin(edgePositions),
    std::end(edgePositions),
    std::back_inserter(vertexPositions));
  return doTransformVertices(worldBounds, vertexPositions, transform, uvLock);
}

bool Brush::canTransformFaces(
  const vm::bbox3d& worldBounds,
  const std::vector<vm::polygon3d>& facePositions,
  const vm::mat4x4d& transform) const
{
  ensure(m_geometry != nullptr, "geometry is null");
  ensure(!facePositions.empty(), "no face positions");

  std::vector<vm::vec3d> vertexPositions;
  vm::polygon3d::get_vertices(
    std::begin(facePositions),
    std::end(facePositions),
    std::back_inserter(vertexPositions));
  const auto result =
    doCanTransformVertices(worldBounds, vertexPositions, transform, false);

  if (!result.success)
  {
    return false;
  }

  for (const auto& face : facePositions)
  {
    if (!result.geometry->hasFace(transform * face.vertices()))
    {
      return false;
    }
  }

  return true;
}

Result<void> Brush::transformFaces(
  const vm::bbox3d& worldBounds,
  const std::vector<vm::polygon3d>& facePositions,
  const vm::mat4x4d& transform,
  const bool uvLock)
{
  assert(canTransformFaces(worldBounds, facePositions, transform));

  std::vector<vm::vec3d> vertexPositions;
  vm::polygon3d::get_vertices(
    std::begin(facePositions),
    std::end(facePositions),
    std::back_inserter(vertexPositions));
  return doTransformVertices(worldBounds, vertexPositions, transform, uvLock);
}

Brush::CanTransformVerticesResult::CanTransformVerticesResult(
  const bool s, BrushGeometry&& g)
  : success{s}
  , geometry{std::make_unique<BrushGeometry>(std::move(g))}
{
}

Brush::CanTransformVerticesResult Brush::CanTransformVerticesResult::reject()
{
  return {false, BrushGeometry{}};
}

Brush::CanTransformVerticesResult Brush::CanTransformVerticesResult::accept(
  BrushGeometry&& result)
{
  return {true, std::move(result)};
}

/*
 We determine whether a transform is valid by considering the vertices being transformed
 and the vertices remaining at their positions as polyhedra. Depending on whether or not
 they really are polyhedra, polygons, edges, points, or empty, we have to consider the
 following cases.

 REMAINING  || Empty   | Point  | Edge   | Polygon | Polyhedron
 ===========||=========|========|========|=========|============
 MOVING     ||         |        |        |         |
 -----------||---------|--------|--------|---------|------------
 Empty      || n/a     | n/a    | n/a    | n/a     | no
 -----------||---------|--------|--------|---------|------------
 Point      || n/a     | n/a    | n/a    | ok      | check
 -----------||---------|--------|--------|---------|------------
 Edge       || n/a     | n/a    | ok     | check   | check
 -----------||---------|--------|--------|---------|------------
 Polygon    || n/a     | invert | invert | check   | check
 -----------||---------|--------|--------|---------|------------
 Polyhedron || ok      | invert | invert | invert  | check

 - n/a: This case can never occur.
 - ok: This case is always allowed unless the brush becomes invalid.
 - no: This case is always forbidden.
 - invert: This case is handled by swapping the remaining and the moving fragments and
   inverting the delta. This takes us from a cell at (column, row) to the cell at (row,
   column).
 - check: Check whether any of the moved vertices would travel through the remaining
   fragment, or vice versa if inverted case. Also check whether the brush would become
   invalid, i.e., not a polyhedron.

 If `allowVertexRemoval` is true, vertices can be moved inside a remaining polyhedron.

 */
Brush::CanTransformVerticesResult Brush::doCanTransformVertices(
  const vm::bbox3d& worldBounds,
  const std::vector<vm::vec3d>& vertexPositions,
  vm::mat4x4d transform,
  const bool allowVertexRemoval) const
{
  // Should never occur, takes care of the first row.
  if (
    vertexPositions.empty()
    || vm::is_equal(transform, vm::mat4x4d::identity(), vm::Cd::almost_zero()))
  {
    return CanTransformVerticesResult::reject();
  }

  const auto vertexSet =
    std::set<vm::vec3d>(std::begin(vertexPositions), std::end(vertexPositions));

  std::vector<vm::vec3d> remainingPoints;
  remainingPoints.reserve(vertexCount());

  std::vector<vm::vec3d> transformedPoints;
  transformedPoints.reserve(vertexCount());

  std::vector<vm::vec3d> resultPoints;
  resultPoints.reserve(vertexCount());

  for (const auto* vertex : m_geometry->vertices())
  {
    const auto& position = vertex->position();
    if (!vertexSet.count(position))
    {
      // the vertex is not transformed
      remainingPoints.push_back(position);
      resultPoints.push_back(position);
    }
    else
    {
      // the vertex is transformed
      transformedPoints.push_back(position);
      resultPoints.push_back(transform * position);
    }
  }

  BrushGeometry remaining(remainingPoints);
  BrushGeometry transformed(transformedPoints);
  BrushGeometry result(resultPoints);

  // Will the result go out of world bounds?
  if (!worldBounds.contains(result.bounds()))
  {
    return CanTransformVerticesResult::reject();
  }

  // Special case, takes care of the first column.
  if (transformed.vertexCount() == vertexCount())
  {
    return CanTransformVerticesResult::accept(std::move(result));
  }

  // Will vertices be removed?
  if (!allowVertexRemoval)
  {
    // All moving vertices must still be present in the result
    for (const auto& movingVertex : transformed.vertexPositions())
    {
      if (!result.hasVertex(transform * movingVertex))
      {
        return CanTransformVerticesResult::reject();
      }
    }
  }

  // Will the brush become invalid?
  if (!result.polyhedron())
  {
    return CanTransformVerticesResult::reject();
  }

  // One of the remaining two ok cases?
  if (
    (transformed.point() && remaining.polygon())
    || (transformed.edge() && remaining.edge()))
  {
    return CanTransformVerticesResult::accept(std::move(result));
  }

  // Invert if necessary.
  if (
    remaining.point() || remaining.edge()
    || (remaining.polygon() && transformed.polyhedron()))
  {
    const auto inverted = vm::invert(transform);
    if (!inverted)
    {
      return CanTransformVerticesResult::reject();
    }

    using std::swap;
    swap(remaining, transformed);
    transform = *inverted;
  }

  // Now check if any of the moving vertices would travel through the remaining fragment
  // and out the other side.
  for (const auto* vertex : transformed.vertices())
  {
    const auto& oldPos = vertex->position();
    const auto newPos = transform * oldPos;

    for (const auto* face : remaining.faces())
    {
      if (
        face->pointStatus(oldPos, vm::constants<double>::point_status_epsilon())
          == vm::plane_status::below
        && face->pointStatus(newPos, vm::constants<double>::point_status_epsilon())
             == vm::plane_status::above)
      {
        const auto ray = vm::ray3d(oldPos, normalize(newPos - oldPos));
        if (face->intersectWithRay(ray, vm::side::back))
        {
          return CanTransformVerticesResult::reject();
        }
      }
    }
  }

  return CanTransformVerticesResult::accept(std::move(result));
}

Result<void> Brush::doTransformVertices(
  const vm::bbox3d& worldBounds,
  const std::vector<vm::vec3d>& vertexPositions,
  const vm::mat4x4d& transform,
  const bool uvLock)
{
  ensure(m_geometry != nullptr, "geometry is null");
  ensure(!vertexPositions.empty(), "no vertex positions");
  assert(canTransformVertices(worldBounds, vertexPositions, transform));

  std::vector<vm::vec3d> newVertices;
  newVertices.reserve(vertexCount());

  for (const auto* vertex : m_geometry->vertices())
  {
    const auto& position = vertex->position();
    if (kdl::vec_contains(vertexPositions, position))
    {
      newVertices.push_back(transform * position);
    }
    else
    {
      newVertices.push_back(position);
    }
  }

  BrushGeometry newGeometry(newVertices);

  using VecMap = std::map<vm::vec3d, vm::vec3d>;
  VecMap vertexMapping;
  for (auto* oldVertex : m_geometry->vertices())
  {
    const auto& oldPosition = oldVertex->position();
    const auto transformed = kdl::vec_contains(vertexPositions, oldPosition);
    const auto newPosition = transformed ? transform * oldPosition : oldPosition;
    const auto* newVertex =
      newGeometry.findClosestVertex(newPosition, CloseVertexEpsilon);
    if (newVertex != nullptr)
    {
      vertexMapping.insert(std::make_pair(oldPosition, newVertex->position()));
    }
  }

  const PolyhedronMatcher<BrushGeometry> matcher(*m_geometry, newGeometry, vertexMapping);
  return updateFacesFromGeometry(worldBounds, matcher, newGeometry, uvLock);
}

std::optional<vm::mat4x4d> Brush::findTransformForUVLock(
  const PolyhedronMatcher<BrushGeometry>& matcher,
  BrushFaceGeometry* left,
  BrushFaceGeometry* right)
{
  std::vector<vm::vec3d> unmovedVerts;
  std::vector<std::pair<vm::vec3d, vm::vec3d>> movedVerts;

  matcher.visitMatchingVertexPairs(
    left, right, [&](BrushVertex* leftVertex, BrushVertex* rightVertex) {
      const auto leftPosition = leftVertex->position();
      const auto rightPosition = rightVertex->position();

      if (vm::is_equal(leftPosition, rightPosition, vm::constants<double>::almost_zero()))
      {
        unmovedVerts.push_back(leftPosition);
      }
      else
      {
        movedVerts.emplace_back(leftPosition, rightPosition);
      }
    });

  // If 3 or more are unmoving, give up.
  // (Picture a square with one corner being moved, we can't possibly lock the UV's of
  // all 4 corners.)
  if (unmovedVerts.size() >= 3)
  {
    return std::nullopt;
  }

  std::vector<std::pair<vm::vec3d, vm::vec3d>> referenceVerts;

  // Use unmoving, then moving
  for (const auto& unmovedVert : unmovedVerts)
  {
    referenceVerts.emplace_back(unmovedVert, unmovedVert);
  }
  // TODO: When there are multiple choices of moving verts (unmovedVerts.size() +
  // movedVerts.size() > 3) we should sort them somehow. This can be seen if you select
  // and move 3/5 verts of a pentagon; which of the 3 moving verts currently gets UV
  // lock is arbitrary.
  referenceVerts = kdl::vec_concat(std::move(referenceVerts), movedVerts);

  if (referenceVerts.size() < 3)
  {
    // Can't create a transform as there are not enough verts
    return std::nullopt;
  }

  const auto M = vm::points_transformation_matrix(
    referenceVerts[0].first,
    referenceVerts[1].first,
    referenceVerts[2].first,
    referenceVerts[0].second,
    referenceVerts[1].second,
    referenceVerts[2].second);

  if (!(M == M))
  {
    // Transform contains nan
    return std::nullopt;
  }

  return M;
}

void Brush::applyUVLock(
  const PolyhedronMatcher<BrushGeometry>& matcher,
  const BrushFace& leftFace,
  BrushFace& rightFace)
{
  if (
    const auto M =
      findTransformForUVLock(matcher, leftFace.geometry(), rightFace.geometry()))
  {

    // We want to re-set the alignment of `rightFace` using the alignment from M *
    // leftFace. We don't want to disturb the actual geometry of `rightFace` which is
    // already finalized. So the idea is, clone `leftFace`, transform it by M with the
    // material alignment locked, then copy the UV attributes from the transformed clone
    // (which should have an identical plane to `rightFace` within FP error) to
    // `rightFace`.
    BrushFace leftClone = leftFace;
    leftClone.transform(*M, true) | kdl::transform([&]() {
      auto snapshot =
        std::unique_ptr<UVCoordSystemSnapshot>(leftClone.takeUVCoordSystemSnapshot());
      rightFace.setAttributes(leftClone.attributes());
      if (snapshot)
      {
        // Note, the wrap style doesn't matter because the source and destination faces
        // should have the same plane
        rightFace.copyUVCoordSystemFromFace(
          *snapshot, leftClone.attributes(), leftClone.boundary(), WrapStyle::Rotation);
      }
      rightFace.resetUVCoordSystemCache();
    }) | kdl::transform_error([](auto) {
      // do nothing
    });
  }
}

Result<void> Brush::updateFacesFromGeometry(
  const vm::bbox3d& worldBounds,
  const PolyhedronMatcher<BrushGeometry>& matcher,
  const BrushGeometry& newGeometry,
  const bool uvLock)
{
  std::vector<BrushFace> newFaces;
  newFaces.reserve(newGeometry.faces().size());

  auto error = std::optional<Error>{};
  matcher.processRightFaces([&](BrushFaceGeometry* left, BrushFaceGeometry* right) {
    if (const auto leftFaceIndex = left->payload())
    {
      const auto& leftFace = m_faces[*leftFaceIndex];
      auto& rightFace = newFaces.emplace_back(leftFace);

      rightFace.setGeometry(right);
      rightFace.updatePointsFromVertices() | kdl::transform([&]() {
        if (uvLock)
        {
          applyUVLock(matcher, leftFace, rightFace);
        }
      }) | kdl::transform_error([&](auto e) {
        if (!error)
        {
          error = e;
        }
      });
    }
  });

  if (error)
  {
    return *error;
  }

  m_faces = std::move(newFaces);
  return updateGeometryFromFaces(worldBounds);
}

std::vector<Result<Brush>> Brush::subtract(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::string& defaultMaterialName,
  const std::vector<const Brush*>& subtrahends) const
{
  auto result = std::vector<BrushGeometry>{*m_geometry};

  for (const auto* subtrahend : subtrahends)
  {
    auto nextResults = std::vector<BrushGeometry>{};

    for (const BrushGeometry& fragment : result)
    {
      auto subFragments = fragment.subtract(*subtrahend->m_geometry);
      nextResults = kdl::vec_concat(std::move(nextResults), std::move(subFragments));
    }

    result = std::move(nextResults);
  }

  return result | std::views::transform([&](const auto& geometry) {
           return createBrush(
             mapFormat, worldBounds, defaultMaterialName, geometry, subtrahends);
         })
         | kdl::ranges::to<std::vector>();
}

std::vector<Result<Brush>> Brush::subtract(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::string& defaultMaterialName,
  const Brush& subtrahend) const
{
  return subtract(
    mapFormat, worldBounds, defaultMaterialName, std::vector<const Brush*>{&subtrahend});
}

Result<void> Brush::intersect(const vm::bbox3d& worldBounds, const Brush& brush)
{
  m_faces = kdl::vec_concat(std::move(m_faces), brush.faces());
  return updateGeometryFromFaces(worldBounds);
}

Result<void> Brush::transform(
  const vm::bbox3d& worldBounds,
  const vm::mat4x4d& transformation,
  const bool lockMaterials)
{
  for (auto& face : m_faces)
  {
    if (!face.transform(transformation, lockMaterials))
    {
      return Error{"Brush has invalid face"};
    }
  }

  return updateGeometryFromFaces(worldBounds);
}

bool Brush::contains(const vm::bbox3d& bounds) const
{
  if (!this->bounds().contains(bounds))
  {
    return false;
  }

  for (const auto& vertex : bounds.vertices())
  {
    if (!containsPoint(vertex))
    {
      return false;
    }
  }

  return true;
}

bool Brush::contains(const Brush& brush) const
{
  return m_geometry->contains(*brush.m_geometry);
}

bool Brush::intersects(const vm::bbox3d& bounds) const
{
  return this->bounds().intersects(bounds);
}

bool Brush::intersects(const Brush& brush) const
{
  return m_geometry->intersects(*brush.m_geometry);
}

Result<Brush> Brush::createBrush(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::string& defaultMaterialName,
  const BrushGeometry& geometry,
  const std::vector<const Brush*>& subtrahends) const
{
  return geometry.faces() | std::views::transform([&](const auto* face) {
           const auto* h1 = face->boundary().front();
           const auto* h0 = h1->next();
           const auto* h2 = h0->next();

           const auto& p0 = h0->origin()->position();
           const auto& p1 = h1->origin()->position();
           const auto& p2 = h2->origin()->position();

           return BrushFace::create(
             p0, p1, p2, BrushFaceAttributes(defaultMaterialName), mapFormat);
         })
         | kdl::fold | kdl::and_then([&](std::vector<BrushFace>&& faces) {
             return Brush::create(worldBounds, std::move(faces));
           })
         | kdl::transform([&](Brush&& brush) {
             brush.cloneFaceAttributesFrom(*this);
             for (const auto* subtrahend : subtrahends)
             {
               brush.cloneFaceAttributesFrom(*subtrahend);
               brush.cloneInvertedFaceAttributesFrom(*subtrahend);
             }
             return std::move(brush);
           });
}

Brush Brush::convertToParaxial() const
{
  Brush result(*this);
  for (auto& face : result.m_faces)
  {
    face.convertToParaxial();
  }
  return result;
}

Brush Brush::convertToParallel() const
{
  Brush result(*this);
  for (auto& face : result.m_faces)
  {
    face.convertToParallel();
  }
  return result;
}

bool Brush::checkFaceLinks() const
{
  if (faceCount() != m_geometry->faceCount())
  {
    return false;
  }

  const auto findFaceGeometry = [&](const BrushFaceGeometry* g) {
    for (const auto* fg : m_geometry->faces())
    {
      if (fg == g)
      {
        return true;
      }
    }
    return false;
  };

  for (const auto* faceGeometry : m_geometry->faces())
  {
    if (const auto faceIndex = faceGeometry->payload())
    {
      if (*faceIndex >= m_faces.size())
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }

  std::set<const BrushFaceGeometry*> faceGeometries;
  for (const auto& face : m_faces)
  {
    const auto* faceGeometry = face.geometry();
    if (faceGeometry == nullptr)
    {
      return false;
    }
    if (!findFaceGeometry(faceGeometry))
    {
      return false;
    }
    if (const auto faceIndex = faceGeometry->payload())
    {
      if (*faceIndex >= m_faces.size())
      {
        return false;
      }
      if (&m_faces[*faceIndex] != &face)
      {
        return false;
      }
    }
    else
    {
      return false;
    }
    if (!faceGeometries.insert(faceGeometry).second)
    {
      return false;
    }
  }

  return true;
}

bool operator==(const Brush& lhs, const Brush& rhs)
{
  return lhs.faces() == rhs.faces();
}

bool operator!=(const Brush& lhs, const Brush& rhs)
{
  return !(lhs == rhs);
}

} // namespace tb::mdl
