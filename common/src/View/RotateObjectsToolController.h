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

#include "View/RotateObjectsHandle.h"
#include "View/ToolController.h"

namespace TrenchBroom {
namespace Renderer {
class RenderBatch;
class RenderContext;
} // namespace Renderer

namespace View {
class RotateObjectsTool;

class RotateObjectsToolController : public ToolControllerGroup {
protected:
  RotateObjectsTool& m_tool;

protected:
  explicit RotateObjectsToolController(RotateObjectsTool& tool);

public:
  ~RotateObjectsToolController() override;

private:
  Tool& tool() override;
  const Tool& tool() const override;

  void pick(const InputState& inputState, Model::PickResult& pickResult) override;

  void setRenderOptions(
    const InputState& inputState, Renderer::RenderContext& renderContext) const override;
  void render(
    const InputState& inputState, Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) override;

  bool cancel() override;

private: // subclassing interface
  virtual Model::Hit doPick(const InputState& inputState) = 0;
  virtual void doRenderHandle(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
};

class RotateObjectsToolController2D : public RotateObjectsToolController {
public:
  explicit RotateObjectsToolController2D(RotateObjectsTool& tool);

private:
  Model::Hit doPick(const InputState& inputState) override;
  void doRenderHandle(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
};

class RotateObjectsToolController3D : public RotateObjectsToolController {
public:
  explicit RotateObjectsToolController3D(RotateObjectsTool& tool);

private:
  Model::Hit doPick(const InputState& inputState) override;
  void doRenderHandle(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
};
} // namespace View
} // namespace TrenchBroom
