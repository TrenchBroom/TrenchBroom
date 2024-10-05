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

#include "Model/BrushGeometry.h"
#include "Result.h"

#include "kdl/reflection_decl.h"

#include "vm/forward.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom::Model
{
template <typename P>
class PolyhedronMatcher;

enum class MapFormat;

class Brush
{
private:
  class CopyCallback;

  /**
   * Epsilon value to use when finding a vertex after applying a vertex operation
   */
  constexpr static double CloseVertexEpsilon = static_cast<double>(0.01);

public:
  using VertexList = BrushVertexList;
  using EdgeList = BrushEdgeList;

private:
  std::vector<BrushFace> m_faces;
  std::unique_ptr<BrushGeometry> m_geometry;

  kdl_reflect_decl(Brush, m_faces);

public:
  Brush();

  Brush(const Brush& other);
  Brush(Brush&& other) noexcept;

  Brush& operator=(const Brush& other);
  Brush& operator=(Brush&& other) noexcept;

  ~Brush();

  static Result<Brush> create(
    const vm::bbox3d& worldBounds, std::vector<BrushFace> faces);

private:
  explicit Brush(std::vector<BrushFace> faces);

  Result<void> updateGeometryFromFaces(const vm::bbox3d& worldBounds);

public:
  const vm::bbox3d& bounds() const;

public: // face management:
  std::optional<size_t> findFace(const std::string& materialName) const;
  std::optional<size_t> findFace(const vm::vec3d& normal) const;
  std::optional<size_t> findFace(const vm::plane3d& boundary) const;
  std::optional<size_t> findFace(
    const vm::polygon3d& vertices, double epsilon = static_cast<double>(0.0)) const;
  std::optional<size_t> findFace(
    const std::vector<vm::polygon3d>& candidates,
    double epsilon = static_cast<double>(0.0)) const;

  const BrushFace& face(size_t index) const;
  BrushFace& face(size_t index);
  size_t faceCount() const;
  const std::vector<BrushFace>& faces() const;
  std::vector<BrushFace>& faces();

  bool closed() const;
  bool fullySpecified() const;

public: // clone face attributes from matching faces of other brushes
  void cloneFaceAttributesFrom(const Brush& brush);
  void cloneFaceAttributesFrom(const std::vector<const Brush*>& brushes);
  void cloneInvertedFaceAttributesFrom(const Brush& brush);

public: // clipping
  Result<void> clip(const vm::bbox3d& worldBounds, BrushFace face);

public: // move face along normal
  /**
   * Translates a face by the given delta.
   *
   * If the brush becomes invalid, an error is returned.
   *
   * @param worldBounds the world bounds
   * @param faceIndex the index of the face to translate
   * @param delta the vector by which to translate the face
   * @param lockMaterial whether material alignment should be locked
   *
   * @return a void result or an error
   */
  Result<void> moveBoundary(
    const vm::bbox3d& worldBounds,
    size_t faceIndex,
    const vm::vec3d& delta,
    bool lockMaterial);

  /**
   * Moves all faces by `delta` units along their normals; negative values shrink the
   * brush. If the brush becomes invalid, an error is returned.
   *
   * @param worldBounds the world bounds
   * @param delta the distance by which to move the faces
   * @param lockMaterial whether material alignment should be locked
   *
   * @return a void result or an error
   */
  Result<void> expand(const vm::bbox3d& worldBounds, double delta, bool lockMaterial);

public:
  // geometry access
  size_t vertexCount() const;
  const VertexList& vertices() const;
  const std::vector<vm::vec3d> vertexPositions() const;

  vm::vec3d findClosestVertexPosition(const vm::vec3d& position) const;
  std::vector<vm::vec3d> findClosestVertexPositions(
    const std::vector<vm::vec3d>& positions) const;
  std::vector<vm::segment3d> findClosestEdgePositions(
    const std::vector<vm::segment3d>& positions) const;
  std::vector<vm::polygon3d> findClosestFacePositions(
    const std::vector<vm::polygon3d>& positions) const;

  bool hasVertex(
    const vm::vec3d& position, double epsilon = static_cast<double>(0.0)) const;
  bool hasEdge(
    const vm::segment3d& edge, double epsilon = static_cast<double>(0.0)) const;
  bool hasFace(
    const vm::polygon3d& face, double epsilon = static_cast<double>(0.0)) const;

  size_t edgeCount() const;
  const EdgeList& edges() const;
  bool containsPoint(const vm::vec3d& point) const;

  std::vector<const BrushFace*> incidentFaces(const BrushVertex* vertex) const;

  // vertex operations
  bool canMoveVertices(
    const vm::bbox3d& worldBounds,
    const std::vector<vm::vec3d>& vertices,
    const vm::vec3d& delta) const;
  Result<void> moveVertices(
    const vm::bbox3d& worldBounds,
    const std::vector<vm::vec3d>& vertexPositions,
    const vm::vec3d& delta,
    bool uvLock = false);

  bool canAddVertex(const vm::bbox3d& worldBounds, const vm::vec3d& position) const;
  Result<void> addVertex(const vm::bbox3d& worldBounds, const vm::vec3d& position);

  bool canRemoveVertices(
    const vm::bbox3d& worldBounds, const std::vector<vm::vec3d>& vertexPositions) const;
  Result<void> removeVertices(
    const vm::bbox3d& worldBounds, const std::vector<vm::vec3d>& vertexPositions);

  bool canSnapVertices(const vm::bbox3d& worldBounds, double snapTo) const;
  Result<void> snapVertices(
    const vm::bbox3d& worldBounds, double snapTo, bool uvLock = false);

  // edge operations
  bool canMoveEdges(
    const vm::bbox3d& worldBounds,
    const std::vector<vm::segment3d>& edgePositions,
    const vm::vec3d& delta) const;
  Result<void> moveEdges(
    const vm::bbox3d& worldBounds,
    const std::vector<vm::segment3d>& edgePositions,
    const vm::vec3d& delta,
    bool uvLock = false);

  // face operations
  bool canMoveFaces(
    const vm::bbox3d& worldBounds,
    const std::vector<vm::polygon3d>& facePositions,
    const vm::vec3d& delta) const;
  Result<void> moveFaces(
    const vm::bbox3d& worldBounds,
    const std::vector<vm::polygon3d>& facePositions,
    const vm::vec3d& delta,
    bool uvLock = false);

private:
  struct CanMoveVerticesResult
  {
  public:
    bool success;
    std::unique_ptr<BrushGeometry> geometry;

  private:
    CanMoveVerticesResult(bool s, BrushGeometry&& g);

  public:
    static CanMoveVerticesResult rejectVertexMove();
    static CanMoveVerticesResult acceptVertexMove(BrushGeometry&& result);
  };

  CanMoveVerticesResult doCanMoveVertices(
    const vm::bbox3d& worldBounds,
    const std::vector<vm::vec3d>& vertexPositions,
    vm::vec3d delta,
    bool allowVertexRemoval) const;
  Result<void> doMoveVertices(
    const vm::bbox3d& worldBounds,
    const std::vector<vm::vec3d>& vertexPositions,
    const vm::vec3d& delta,
    bool lockMaterial);
  /**
   * Tries to find 3 vertices in `left` and `right` that are related according to the
   * PolyhedronMatcher, and generates an affine transform for them which can then be used
   * to implement UV lock.
   *
   * @param matcher a polyhedron matcher which is used to identify related vertices
   * @param left the face of the left polyhedron
   * @param right the face of the right polyhedron
   * @return the transformation matrix or nullopt if it cannot be found
   */
  static std::optional<vm::mat4x4d> findTransformForUVLock(
    const PolyhedronMatcher<BrushGeometry>& matcher,
    BrushFaceGeometry* left,
    BrushFaceGeometry* right);

  /**
   * Helper function to apply UV lock to the face `right`.
   *
   * It's assumed that `left` and `right` have already been identified as "matching" faces
   * for a vertex move where `left` is a face from the polyhedron before vertex
   * manipulation, and `right` is from the newly modified brush.
   *
   * This function tries to pick 3 vertices from `left` and `right` to generate a
   * transform (using findTransformForUVLock), and updates the texturing of `right` using
   * that transform applied to `left`. If it can't perform UV lock, `right` remains
   * unmodified.
   *
   * This is only meant to be called in the matcher callback in
   * Brush::createBrushWithNewGeometry
   *
   * @param matcher a polyhedron matcher which is used to identify related vertices
   * @param leftFace the face of the left polyhedron
   * @param rightFace the face of the right polyhedron
   */
  static void applyUVLock(
    const PolyhedronMatcher<BrushGeometry>& matcher,
    const BrushFace& leftFace,
    BrushFace& rightFace);
  Result<void> updateFacesFromGeometry(
    const vm::bbox3d& worldBounds,
    const PolyhedronMatcher<BrushGeometry>& matcher,
    const BrushGeometry& newGeometry,
    bool uvLock = false);

public:
  // CSG operations
  /**
   * Subtracts the given subtrahends from `this`, returning the result but without
   * modifying `this`.
   *
   * @param subtrahends brushes to subtract from `this`. The passed-in brushes are not
   * modified.
   * @return the subtraction result framents as Brushes, or Errors for any fragments
   * which were invalid. Note, the subtraction result should still be usable even if some
   * Errors are returned. It's a hint to the user to double check the result, and
   * potentially report a bug.
   */
  std::vector<Result<Brush>> subtract(
    MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    const std::string& defaultMaterialName,
    const std::vector<const Brush*>& subtrahends) const;
  std::vector<Result<Brush>> subtract(
    MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    const std::string& defaultMaterialName,
    const Brush& subtrahend) const;

  /**
   * Intersects this brush with the given brush.
   *
   * If the resulting brush is invalid, an error is returned.
   *
   * @param worldBounds the world bounds
   * @param brush the brush to intersect this brush with
   * @return a void result or an error if the operation fails
   */
  Result<void> intersect(const vm::bbox3d& worldBounds, const Brush& brush);

  /**
   * Applies the given transformation to this brush.
   *
   * If the brush becomes invalid, an error is returned.
   *
   * @param worldBounds the world bounds
   * @param transformation the transformation to apply
   * @param lockMaterials whether material alignment should be locked
   * @return a void result or an error if the operation fails
   */
  Result<void> transform(
    const vm::bbox3d& worldBounds, const vm::mat4x4d& transformation, bool lockMaterials);

public:
  bool contains(const vm::bbox3d& bounds) const;
  bool contains(const Brush& brush) const;
  bool intersects(const vm::bbox3d& bounds) const;
  bool intersects(const Brush& brush) const;

private:
  /**
   * Final step of CSG subtraction; takes the geometry that is the result of the
   * subtraction, and turns it into a Brush by copying materials from `this` (for
   * un-clipped faces) or the brushes in `subtrahends` (for clipped faces).
   *
   * @param mapFormat the map format
   * @param worldBounds the world bounds
   * @param defaultMaterialName default material name
   * @param geometry the geometry for the newly created brush
   * @param subtrahends used as a source of material alignment only
   * @return the newly created brush
   */
  Result<Brush> createBrush(
    MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    const std::string& defaultMaterialName,
    const BrushGeometry& geometry,
    const std::vector<const Brush*>& subtrahends) const;

public: // UV format conversion
  Brush convertToParaxial() const;
  Brush convertToParallel() const;

private:
  bool checkFaceLinks() const;
};

bool operator==(const Brush& lhs, const Brush& rhs);
bool operator!=(const Brush& lhs, const Brush& rhs);

} // namespace TrenchBroom::Model
