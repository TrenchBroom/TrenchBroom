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

#include "base/Color.h"
#include "base/Result.h"
#include "mdl/AssetReference.h"
#include "mdl/BrushGeometry.h"
#include "mdl/SurfaceAttributes.h"
#include "mdl/Tag.h"
#include "mdl/UvAttributes.h"
#include "mdl/UvCoordSystem.h"

#include "kd/reflection_decl.h"

#include "vm/plane.h"
#include "vm/polygon.h"
#include "vm/ray.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <array>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace tb
{
namespace gl
{
class Material;
}

namespace mdl
{
enum class MapFormat;

class BrushFace : public Taggable
{
public:
  static const std::string NoMaterialName;

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

private:
  BrushFace::Points m_points;
  vm::plane3d m_boundary;

  std::string m_materialName;
  UvCoordSystem m_uvCoordSystem;
  SurfaceAttributes m_surfaceAttributes;

  AssetReference<gl::Material> m_materialReference;
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

  kdl_reflect_decl(
    BrushFace,
    m_points,
    m_boundary,
    m_materialName,
    m_surfaceAttributes,
    m_materialReference,
    m_uvCoordSystem);

  /**
   * Creates a face using TB's default UV projection for the given map format and the
   * given plane.
   *
   * Used when creating new faces when we don't have a particular alignment to request.
   * On Valve format maps, this differs from createFromStandard() by creating a
   * face-aligned UV projection, whereas createFromStandard() creates an axis-aligned
   * UV projection.
   *
   * The returned face has a UvCoordSystem matching the given format.
   */
  static Result<BrushFace> create(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    std::string materialName,
    const UvAttributes& uvAttributes,
    const SurfaceAttributes& surfaceAttributes,
    MapFormat mapFormat);

  /**
   * Creates a face from a Standard UV projection, converting it to Valve if
   * necessary.
   *
   * Used when loading/pasting a Standard format map.
   *
   * The returned face has a UvCoordSystem matching the given format.
   */
  static Result<BrushFace> createFromStandard(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    std::string materialName,
    const UvAttributes& uvAttributes,
    const SurfaceAttributes& surfaceAttributes,
    MapFormat mapFormat);

  /**
   * Creates a face from a Valve UV projection, converting it to Standard if
   * necessary.
   *
   * Used when loading/pasting a Valve format map.
   *
   * The returned face has a UvCoordSystem matching the given format.
   */
  static Result<BrushFace> createFromValve(
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const vm::vec3d& point3,
    std::string materialName,
    const UvAttributes& uvAttributes,
    const SurfaceAttributes& surfaceAttributes,
    const vm::vec3d& uAxis,
    const vm::vec3d& vAxis,
    MapFormat mapFormat);

  /**
   * Creates a face with the given UV coordinate system, which carries the face's UV
   * attributes.
   */
  static Result<BrushFace> create(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    std::string materialName,
    UvCoordSystem uvCoordSystem,
    const SurfaceAttributes& surfaceAttributes);

  BrushFace(
    const BrushFace::Points& points,
    const vm::plane3d& boundary,
    std::string materialName,
    UvCoordSystem uvCoordSystem,
    const SurfaceAttributes& surfaceAttributes);

  static void sortFaces(std::vector<BrushFace>& faces);

  std::optional<UvCoordSystemSnapshot> takeUvCoordSystemSnapshot() const;
  void restoreUvCoordSystemSnapshot(const UvCoordSystemSnapshot& coordSystemSnapshot);
  void copyUvCoordSystemFromFace(
    const UvCoordSystemSnapshot& coordSystemSnapshot,
    const UvAttributes& uvAttributes,
    const vm::plane3d& sourceFacePlane,
    WrapStyle wrapStyle);

  const BrushFace::Points& points() const;
  const vm::plane3d& boundary() const;
  const vm::vec3d& normal() const;
  vm::vec3d center() const;
  vm::bbox3d bounds() const;
  vm::vec3d boundsCenter() const;
  double projectedArea(vm::axis::type axis) const;
  double area() const;
  bool coplanarWith(const vm::plane3d& plane) const;

  const std::string& materialName() const;
  bool setMaterialName(std::string materialName);

  UvAttributes uvAttributes() const;
  void setUvAttributes(const UvAttributes& uvAttributes);

  const SurfaceAttributes& surfaceAttributes() const;
  void setSurfaceAttributes(const SurfaceAttributes& surfaceAttributes);

  /**
   * Copies the material name and the UV and surface attributes from the given face.
   * Unlike setUvAttributes, this does not update the rotation of the UV coordinate
   * system.
   *
   * @return true if any attribute changed
   */
  bool copyAttributes(const BrushFace& other);

  int resolvedSurfaceContents() const;
  int resolvedSurfaceFlags() const;
  float resolvedSurfaceValue() const;
  std::optional<Color> resolvedColor() const;

  void resetUvCoordSystemCache();
  const UvCoordSystem& uvCoordSystem() const;

  const gl::Material* material() const;
  vm::vec2f textureSize() const;
  vm::vec2f modOffset(const vm::vec2f& offset) const;

  bool setMaterial(gl::Material* material);

  vm::vec3d uAxis() const;
  vm::vec3d vAxis() const;
  void resetUvAxes();
  void resetUvAxesToParaxial();

  void convertToParaxial();
  void convertToParallel();

  void translateUv(const vm::vec3d& up, const vm::vec3d& right, const vm::vec2f& offset);
  void rotateUv(float angle);
  void shearUv(const vm::vec2f& factors);
  void flipUv(
    const vm::vec3d& cameraUp,
    const vm::vec3d& cameraRight,
    vm::direction cameraRelativeFlipDirection);

  Result<void> transform(const vm::mat4x4d& transform, bool lockAlignment);
  void invert();

  Result<void> updatePointsFromVertices();

  vm::mat4x4d projectToBoundaryMatrix() const;
  vm::mat4x4d toUvCoordSystemMatrix(
    const vm::vec2f& offset, const vm::vec2f& scale) const;
  vm::mat4x4d fromUvCoordSystemMatrix(
    const vm::vec2f& offset, const vm::vec2f& scale) const;
  float measureUvAngle(const vm::vec2f& center, const vm::vec2f& point) const;

  size_t vertexCount() const;

  auto edges() const
  {
    return m_geometry->boundary() | std::views::transform(TransformHalfEdgeToEdge{});
  }

  auto vertices() const
  {
    return m_geometry->boundary() | std::views::transform(TransformHalfEdgeToVertex{});
  }

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

} // namespace mdl
} // namespace tb
