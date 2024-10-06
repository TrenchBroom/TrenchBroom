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

#include "Result.h"
#include "asset/AssetReference.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/BrushGeometry.h"
#include "mdl/Tag.h"

#include "kdl/reflection_decl.h"
#include "kdl/transform_range.h"

#include "vm/plane.h"
#include "vm/polygon.h"
#include "vm/ray.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <array>
#include <memory>
#include <optional>
#include <vector>

namespace tb::asset
{
class Material;
}

namespace tb::mdl
{
class UVCoordSystem;
class UVCoordSystemSnapshot;
enum class WrapStyle;
enum class MapFormat;

class BrushFace : public Taggable
{
public:
  /*
   * The order of points, when looking from outside the face:
   *
   * 1
   * |
   * |
   * |
   * |
   * 0-----------2
   */
  using Points = std::array<vm::vec3d, 3u>;

private:
  /**
   * For use in VertexList transformation below.
   */
  struct TransformHalfEdgeToVertex
  {
    const BrushVertex* operator()(const BrushHalfEdge* halfEdge) const;
  };

  /**
   * For use in EdgeList transformation below.
   */
  struct TransformHalfEdgeToEdge
  {
    const BrushEdge* operator()(const BrushHalfEdge* halfEdge) const;
  };

public:
  using VertexList = kdl::transform_adapter<BrushHalfEdgeList, TransformHalfEdgeToVertex>;
  using EdgeList = kdl::transform_adapter<BrushHalfEdgeList, TransformHalfEdgeToEdge>;

private:
  BrushFace::Points m_points;
  vm::plane3d m_boundary;
  BrushFaceAttributes m_attributes;

  asset::AssetReference<asset::Material> m_materialReference;
  std::unique_ptr<UVCoordSystem> m_uvCoordSystem;
  BrushFaceGeometry* m_geometry = nullptr;

  mutable size_t m_lineNumber = 0;
  mutable size_t m_lineCount = 0;
  bool m_selected = false;

  // brush renderer
  mutable bool m_markedToRenderFace = false;

public:
  BrushFace(const BrushFace& other);
  BrushFace(BrushFace&& other) noexcept;
  BrushFace& operator=(BrushFace other) noexcept;

  friend void swap(BrushFace& lhs, BrushFace& rhs) noexcept;

  ~BrushFace() override;

  kdl_reflect_decl(BrushFace, m_points, m_boundary, m_attributes, m_materialReference);

  /**
   * Creates a face using TB's default UV projection for the given map format and the
   * given plane.
   *
   * Used when creating new faces when we don't have a particular alignment to request.
   * On Valve format maps, this differs from createFromStandard() by creating a
   * face-aligned UV projection, whereas createFromStandard() creates an axis-aligned
   * UV projection.
   *
   * The returned face has a UVCoordSystem matching the given format.
   */
  static Result<BrushFace> create(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attributes,
    MapFormat mapFormat);

  /**
   * Creates a face from a Standard UV projection, converting it to Valve if
   * necessary.
   *
   * Used when loading/pasting a Standard format map.
   *
   * The returned face has a UVCoordSystem matching the given format.
   */
  static Result<BrushFace> createFromStandard(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attributes,
    MapFormat mapFormat);

  /**
   * Creates a face from a Valve UV projection, converting it to Standard if
   * necessary.
   *
   * Used when loading/pasting a Valve format map.
   *
   * The returned face has a UVCoordSystem matching the given format.
   */
  static Result<BrushFace> createFromValve(
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const vm::vec3d& point3,
    const BrushFaceAttributes& attributes,
    const vm::vec3d& uAxis,
    const vm::vec3d& vAxis,
    MapFormat mapFormat);

  static Result<BrushFace> create(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attributes,
    std::unique_ptr<UVCoordSystem> uvCoordSystem);

  BrushFace(
    const BrushFace::Points& points,
    const vm::plane3d& boundary,
    BrushFaceAttributes attributes,
    std::unique_ptr<UVCoordSystem> uvCoordSystem);

  static void sortFaces(std::vector<BrushFace>& faces);

  std::unique_ptr<UVCoordSystemSnapshot> takeUVCoordSystemSnapshot() const;
  void restoreUVCoordSystemSnapshot(const UVCoordSystemSnapshot& coordSystemSnapshot);
  void copyUVCoordSystemFromFace(
    const UVCoordSystemSnapshot& coordSystemSnapshot,
    const BrushFaceAttributes& attributes,
    const vm::plane3d& sourceFacePlane,
    WrapStyle wrapStyle);

  const BrushFace::Points& points() const;
  const vm::plane3d& boundary() const;
  const vm::vec3d& normal() const;
  vm::vec3d center() const;
  vm::vec3d boundsCenter() const;
  double projectedArea(vm::axis::type axis) const;
  double area() const;
  bool coplanarWith(const vm::plane3d& plane) const;

  const BrushFaceAttributes& attributes() const;
  void setAttributes(const BrushFaceAttributes& attributes);
  bool setAttributes(const BrushFace& other);

  int resolvedSurfaceContents() const;
  int resolvedSurfaceFlags() const;
  float resolvedSurfaceValue() const;
  Color resolvedColor() const;

  void resetUVCoordSystemCache();
  const UVCoordSystem& uvCoordSystem() const;

  const asset::Material* material() const;
  vm::vec2f textureSize() const;
  vm::vec2f modOffset(const vm::vec2f& offset) const;

  bool setMaterial(asset::Material* material);

  vm::vec3d uAxis() const;
  vm::vec3d vAxis() const;
  void resetUVAxes();
  void resetUVAxesToParaxial();

  void convertToParaxial();
  void convertToParallel();

  void moveUV(const vm::vec3d& up, const vm::vec3d& right, const vm::vec2f& offset);
  void rotateUV(float angle);
  void shearUV(const vm::vec2f& factors);
  void flipUV(
    const vm::vec3d& cameraUp,
    const vm::vec3d& cameraRight,
    vm::direction cameraRelativeFlipDirection);

  Result<void> transform(const vm::mat4x4d& transform, bool lockAlignment);
  void invert();

  Result<void> updatePointsFromVertices();

  vm::mat4x4d projectToBoundaryMatrix() const;
  vm::mat4x4d toUVCoordSystemMatrix(
    const vm::vec2f& offset, const vm::vec2f& scale, bool project) const;
  vm::mat4x4d fromUVCoordSystemMatrix(
    const vm::vec2f& offset, const vm::vec2f& scale, bool project) const;
  float measureUVAngle(const vm::vec2f& center, const vm::vec2f& point) const;

  size_t vertexCount() const;
  EdgeList edges() const;
  VertexList vertices() const;
  std::vector<vm::vec3d> vertexPositions() const;

  bool hasVertices(
    const vm::polygon3d& vertices, double epsilon = static_cast<double>(0.0)) const;
  vm::polygon3d polygon() const;

public:
  BrushFaceGeometry* geometry() const;
  void setGeometry(BrushFaceGeometry* geometry);

  size_t lineNumber() const;
  void setFilePosition(size_t lineNumber, size_t lineCount) const;

  bool selected() const;
  void select();
  void deselect();

  vm::vec2f uvCoords(const vm::vec3d& point) const;

  std::optional<double> intersectWithRay(const vm::ray3d& ray) const;

private:
  Result<void> setPoints(
    const vm::vec3d& point0, const vm::vec3d& point1, const vm::vec3d& point2);
  void correctPoints();

public: // brush renderer
  /**
   * This is used to cache results of evaluating the BrushRenderer Filter.
   * It's only valid within a call to `BrushRenderer::validateBrush`.
   *
   * @param marked    whether the face is going to be rendered.
   */
  void setMarked(bool marked) const;
  bool isMarked() const;

private: // implement Taggable interface
  void doAcceptTagVisitor(TagVisitor& visitor) override;
  void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
};

} // namespace tb::mdl
