/*
 Copyright (C) 2026 Jackson Palmer

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

namespace tb
{
namespace render
{
class RenderBatch;
class RenderContext;
} // namespace render

namespace ui
{
class SweepTool;

class SweepToolController : public ToolControllerGroup
{
protected:
  SweepTool& m_tool;

protected:
  explicit SweepToolController(SweepTool& tool);

public:
  ~SweepToolController() override;

private:
  Tool& tool() override;
  const Tool& tool() const override;

  void pick(const InputState& inputState, mdl::PickResult& pickResult) override;

  void setRenderOptions(
    const InputState& inputState, render::RenderContext& renderContext) const override;
  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override;

  bool cancel() override;

private: // subclassing interface
  virtual mdl::Hit doPick(const InputState& inputState) = 0;
  virtual void doRenderHandle(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) = 0;
};

class SweepToolController2D : public SweepToolController
{
public:
  explicit SweepToolController2D(SweepTool& tool);

private:
  mdl::Hit doPick(const InputState& inputState) override;
  void doRenderHandle(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) override;
};

class SweepToolController3D : public SweepToolController
{
public:
  explicit SweepToolController3D(SweepTool& tool);

private:
  mdl::Hit doPick(const InputState& inputState) override;
  void doRenderHandle(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) override;
};

} // namespace ui
} // namespace tb
