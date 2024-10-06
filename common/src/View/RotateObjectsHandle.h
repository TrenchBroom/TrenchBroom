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

#include "Macros.h"
#include "Model/Hit.h"
#include "Model/HitType.h"

#include "vm/mat.h"
#include "vm/ray.h"
#include "vm/vec.h"

namespace tb::Renderer
{
class Camera;
class RenderBatch;
class RenderContext;
} // namespace tb::Renderer

namespace tb::View
{
class RotateObjectsHandle
{
public:
  static const Model::HitType::Type HandleHitType;

  enum class HitArea
  {
    None = 0,
    Center = 1,
    XAxis = 2,
    YAxis = 4,
    ZAxis = 8
  };

private:
  class Handle
  {
  protected:
    const vm::vec3d& m_position;

  public:
    explicit Handle(const vm::vec3d& position);
    virtual ~Handle();

    virtual Model::Hit pick(
      const vm::ray3d& pickRay, const Renderer::Camera& camera) const = 0;
    virtual void renderHandle(
      Renderer::RenderContext& renderContext,
      Renderer::RenderBatch& renderBatch) const = 0;
    virtual void renderHighlight(
      Renderer::RenderContext& renderContext,
      Renderer::RenderBatch& renderBatch,
      HitArea area) const = 0;

    double scalingFactor(const Renderer::Camera& camera) const;
    static double majorRadius();
    static double minorRadius();

  protected:
    Model::Hit pickCenterHandle(
      const vm::ray3d& pickRay, const Renderer::Camera& camera) const;
    virtual Model::Hit pickRotateHandle(
      const vm::ray3d& pickRay, const Renderer::Camera& camera, HitArea area) const;

    vm::mat4x4d handleTransform(const Renderer::Camera& camera, HitArea area) const;
  };

  class Handle2D : public Handle
  {
  public:
    using Handle::Handle;

    Model::Hit pick(
      const vm::ray3d& pickRay, const Renderer::Camera& camera) const override;
    void renderHandle(
      Renderer::RenderContext& renderContext,
      Renderer::RenderBatch& renderBatch) const override;
    void renderHighlight(
      Renderer::RenderContext& renderContext,
      Renderer::RenderBatch& renderBatch,
      HitArea area) const override;

  protected:
    Model::Hit pickRotateHandle(
      const vm::ray3d& pickRay,
      const Renderer::Camera& camera,
      HitArea area) const override;

    deleteCopyAndMove(Handle2D);
  };

  class Handle3D : public Handle
  {
  public:
    using Handle::Handle;

    Model::Hit pick(
      const vm::ray3d& pickRay, const Renderer::Camera& camera) const override;
    void renderHandle(
      Renderer::RenderContext& renderContext,
      Renderer::RenderBatch& renderBatch) const override;
    void renderHighlight(
      Renderer::RenderContext& renderContext,
      Renderer::RenderBatch& renderBatch,
      HitArea area) const override;

  protected:
    Model::Hit pickRotateHandle(
      const vm::ray3d& pickRay,
      const Renderer::Camera& camera,
      HitArea area) const override;

    deleteCopyAndMove(Handle3D);
  };

private:
  vm::vec3d m_position;
  Handle2D m_handle2D;
  Handle3D m_handle3D;

public:
  RotateObjectsHandle();

  const vm::vec3d& position() const;
  void setPosition(const vm::vec3d& position);

  Model::Hit pick2D(const vm::ray3d& pickRay, const Renderer::Camera& camera) const;
  Model::Hit pick3D(const vm::ray3d& pickRay, const Renderer::Camera& camera) const;

  double majorHandleRadius(const Renderer::Camera& camera) const;
  double minorHandleRadius(const Renderer::Camera& camera) const;

  vm::vec3d rotationAxis(HitArea area) const;

public:
  void renderHandle2D(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void renderHandle3D(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void renderHighlight2D(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    HitArea area);
  void renderHighlight3D(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    HitArea area);
};

} // namespace tb::View
