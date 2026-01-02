/*
Copyright (C) 2010 Kristian Duske
Copyright (C) 2018 Eric Wasylishen

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

#include "ui/ToolController.h"

#include <memory>

namespace tb
{
namespace gl
{
class Camera;
}

namespace render
{
class RenderBatch;
class RenderContext;
} // namespace render

namespace ui
{
class GestureTracker;
class ShearTool;

class ShearToolController : public ToolController
{
protected:
  ShearTool& m_tool;

public:
  explicit ShearToolController(ShearTool& tool);
  ~ShearToolController() override;

private:
  Tool& tool() override;
  const Tool& tool() const override;

  void pick(const InputState& inputState, mdl::PickResult& pickResult) override;
  virtual void doPick(
    const vm::ray3d& pickRay, const gl::Camera& camera, mdl::PickResult& pickResult) = 0;

  void mouseMove(const InputState& inputState) override;

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override;

  void setRenderOptions(
    const InputState& inputState, render::RenderContext& renderContext) const override;

  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override;

  bool cancel() override;

private:
  void renderBox(render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  void renderHandle(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);
};

class ShearToolController2D : public ShearToolController
{
public:
  explicit ShearToolController2D(ShearTool& tool);

private:
  void doPick(
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    mdl::PickResult& pickResult) override;
};

class ShearToolController3D : public ShearToolController
{
public:
  explicit ShearToolController3D(ShearTool& tool);

private:
  void doPick(
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    mdl::PickResult& pickResult) override;
};

} // namespace ui
} // namespace tb
