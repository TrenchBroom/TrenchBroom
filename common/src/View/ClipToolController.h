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
#include "View/ToolController.h"

#include "vecmath/forward.h"

#include <memory>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class BrushFace;
class BrushNode;
class PickResult;
} // namespace Model

namespace Renderer
{
class RenderBatch;
class RenderContext;
} // namespace Renderer

namespace View
{
class ClipTool;

class ClipToolControllerBase : public ToolControllerGroup
{
protected:
  ClipTool& m_tool;

protected:
  explicit ClipToolControllerBase(ClipTool& tool);
  virtual ~ClipToolControllerBase() override;

private:
  Tool& tool() override;
  const Tool& tool() const override;

  void pick(const InputState& inputState, Model::PickResult& pickResult) override;

  void setRenderOptions(
    const InputState& inputState, Renderer::RenderContext& renderContext) const override;
  void render(
    const InputState& inputState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) override;

  bool cancel() override;
};

class ClipToolController2D : public ClipToolControllerBase
{
public:
  explicit ClipToolController2D(ClipTool& tool);
};

class ClipToolController3D : public ClipToolControllerBase
{
public:
  explicit ClipToolController3D(ClipTool& tool);
};
} // namespace View
} // namespace TrenchBroom
