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

#pragma once

#include "FloatType.h"
#include "Model/BrushFaceHandle.h"
#include "Model/HitType.h"

#include <vecmath/vec.h>

#include <optional>

namespace TrenchBroom {
namespace Assets {
class Texture;
}

namespace Renderer {
class ActiveShader;
class Camera;
class OrthographicCamera;
class RenderContext;
} // namespace Renderer

namespace Model {
class BrushFace;
class PickResult;
} // namespace Model

namespace View {
class UVViewHelper {
private:
  Renderer::OrthographicCamera& m_camera;
  bool m_zoomValid;

  std::optional<Model::BrushFaceHandle> m_faceHandle;

  vm::vec2i m_subDivisions;

  /**
   The position of the scaling origin / rotation center handle in world coords.
   */
  vm::vec3 m_origin;

public:
  explicit UVViewHelper(Renderer::OrthographicCamera& camera);

  bool valid() const;
  const Model::BrushFace* face() const;
  const Assets::Texture* texture() const;
  void setFaceHandle(std::optional<Model::BrushFaceHandle> faceHandle);
  void cameraViewportChanged();

  const vm::vec2i& subDivisions() const;
  vm::vec2 stripeSize() const;
  void setSubDivisions(const vm::vec2i& subDivisions);

  const vm::vec3 origin() const;
  const vm::vec2f originInFaceCoords() const;
  const vm::vec2f originInTexCoords() const;
  void setOriginInFaceCoords(const vm::vec2f& originInFaceCoords);

  const Renderer::OrthographicCamera& camera() const;
  float cameraZoom() const;

  void pickTextureGrid(
    const vm::ray3& ray, const Model::HitType::Type hitTypes[2],
    Model::PickResult& pickResult) const;

  vm::vec2f snapDelta(const vm::vec2f& delta, const vm::vec2f& distance) const;
  vm::vec2f computeDistanceFromTextureGrid(const vm::vec3& position) const;

  void computeOriginHandleVertices(vm::vec3& x1, vm::vec3& x2, vm::vec3& y1, vm::vec3& y2) const;
  void computeScaleHandleVertices(
    const vm::vec2& pos, vm::vec3& x1, vm::vec3& x2, vm::vec3& y1, vm::vec3& y2) const;
  void computeLineVertices(
    const vm::vec2& pos, vm::vec3& x1, vm::vec3& x2, vm::vec3& y1, vm::vec3& y2,
    const vm::mat4x4& toTex, const vm::mat4x4& toWorld) const;

  /**
   * Converts texture space to view space (pixels in the UV viewport).
   */
  vm::vec2f texToViewCoords(const vm::vec2f& pos) const;

private:
  void resetOrigin();
  void resetCamera();
  void resetZoom();

  vm::bbox3 computeFaceBoundsInCameraCoords() const;
  vm::vec3 transformToCamera(const vm::vec3& point) const;
  vm::vec3 transformFromCamera(const vm::vec3& point) const;
};
} // namespace View
} // namespace TrenchBroom
