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
#include "Macros.h"
#include "Model/HitType.h"

#include <vecmath/vec.h>

namespace TrenchBroom {
namespace Model {
class Hit;
}

namespace Renderer {
class Camera;
class RenderBatch;
class RenderContext;
} // namespace Renderer

namespace View {
class RotateObjectsHandle {
public:
  static const Model::HitType::Type HandleHitType;

  enum class HitArea {
    None = 0,
    Center = 1,
    XAxis = 2,
    YAxis = 4,
    ZAxis = 8
  };

private:
  class Handle {
  protected:
    const vm::vec3& m_position;

  public:
    explicit Handle(const vm::vec3& position);
    virtual ~Handle();

    virtual Model::Hit pick(const vm::ray3& pickRay, const Renderer::Camera& camera) const = 0;
    virtual void renderHandle(
      Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const = 0;
    virtual void renderHighlight(
      Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch,
      HitArea area) const = 0;

    FloatType scalingFactor(const Renderer::Camera& camera) const;
    static FloatType majorRadius();
    static FloatType minorRadius();

  protected:
    Model::Hit pickCenterHandle(const vm::ray3& pickRay, const Renderer::Camera& camera) const;
    virtual Model::Hit pickRotateHandle(
      const vm::ray3& pickRay, const Renderer::Camera& camera, HitArea area) const;

    vm::mat4x4 handleTransform(const Renderer::Camera& camera, HitArea area) const;
  };

  class Handle2D : public Handle {
  public:
    using Handle::Handle;

    Model::Hit pick(const vm::ray3& pickRay, const Renderer::Camera& camera) const override;
    void renderHandle(
      Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const override;
    void renderHighlight(
      Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch,
      HitArea area) const override;

  protected:
    Model::Hit pickRotateHandle(
      const vm::ray3& pickRay, const Renderer::Camera& camera, HitArea area) const override;

    deleteCopyAndMove(Handle2D);
  };

  class Handle3D : public Handle {
  public:
    using Handle::Handle;

    Model::Hit pick(const vm::ray3& pickRay, const Renderer::Camera& camera) const override;
    void renderHandle(
      Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const override;
    void renderHighlight(
      Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch,
      HitArea area) const override;

  protected:
    Model::Hit pickRotateHandle(
      const vm::ray3& pickRay, const Renderer::Camera& camera, HitArea area) const override;

    deleteCopyAndMove(Handle3D);
  };

private:
  vm::vec3 m_position;
  Handle2D m_handle2D;
  Handle3D m_handle3D;

public:
  RotateObjectsHandle();

  const vm::vec3& position() const;
  void setPosition(const vm::vec3& position);

  Model::Hit pick2D(const vm::ray3& pickRay, const Renderer::Camera& camera) const;
  Model::Hit pick3D(const vm::ray3& pickRay, const Renderer::Camera& camera) const;

  FloatType majorHandleRadius(const Renderer::Camera& camera) const;
  FloatType minorHandleRadius(const Renderer::Camera& camera) const;

  vm::vec3 rotationAxis(HitArea area) const;

public:
  void renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void renderHighlight2D(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea area);
  void renderHighlight3D(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea area);
};
} // namespace View
} // namespace TrenchBroom
