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

#include "Renderer/Camera.h"

#include "vm/forward.h"

#include <vector>

namespace TrenchBroom
{
namespace Renderer
{
class OrthographicCamera : public Camera
{
private:
  Viewport m_zoomedViewport;

public:
  OrthographicCamera();
  OrthographicCamera(
    float nearPlane,
    float farPlane,
    const Viewport& viewport,
    const vm::vec3f& position,
    const vm::vec3f& direction,
    const vm::vec3f& up);

  const Viewport& zoomedViewport() const;
  std::vector<vm::vec3> viewportVertices() const;

private:
  ProjectionType doGetProjectionType() const override;

  void doValidateMatrices(
    vm::mat4x4f& projectionMatrix, vm::mat4x4f& viewMatrix) const override;
  vm::ray3f doGetPickRay(const vm::vec3f& point) const override;
  void doComputeFrustumPlanes(
    vm::plane3f& topPlane,
    vm::plane3f& rightPlane,
    vm::plane3f& bottomPlane,
    vm::plane3f& leftPlane) const override;

  void doRenderFrustum(
    RenderContext& renderContext,
    VboManager& vboManager,
    float size,
    const Color& color) const override;
  float doPickFrustum(float size, const vm::ray3f& ray) const override;
  float doGetPerspectiveScalingFactor(const vm::vec3f& position) const override;
  void doUpdateZoom() override;
};
} // namespace Renderer
} // namespace TrenchBroom
