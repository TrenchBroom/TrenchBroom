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

#include "Renderer/EdgeRenderer.h"
#include "View/ToolController.h"

namespace TrenchBroom
{
namespace Renderer
{
class DirectEdgeRenderer;
class RenderBatch;
class RenderContext;
} // namespace Renderer

namespace View
{
class DragTracker;
class ExtrudeTool;

class ExtrudeToolController : public ToolController
{
protected:
  ExtrudeTool& m_tool;

protected:
  explicit ExtrudeToolController(ExtrudeTool& tool);

public:
  ~ExtrudeToolController() override;

private:
  Tool& tool() override;
  const Tool& tool() const override;

  void pick(const InputState& inputState, Model::PickResult& pickResult) override;

  void modifierKeyChange(const InputState& inputState) override;

  void mouseMove(const InputState& inputState) override;

  std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override;

  void render(
    const InputState& inputState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) override;

  bool cancel() override;

  bool handleInput(const InputState& inputState) const;

private:
  virtual bool doHandleInput(const InputState& inputState) const = 0;
  virtual Model::Hit doPick(
    const vm::ray3& pickRay, const Model::PickResult& pickResult) = 0;
};

class ExtrudeToolController2D : public ExtrudeToolController
{
public:
  explicit ExtrudeToolController2D(ExtrudeTool& tool);

private:
  Model::Hit doPick(
    const vm::ray3& pickRay, const Model::PickResult& pickResult) override;
  bool doHandleInput(const InputState& inputState) const override;
};

class ExtrudeToolController3D : public ExtrudeToolController
{
public:
  explicit ExtrudeToolController3D(ExtrudeTool& tool);

private:
  Model::Hit doPick(
    const vm::ray3& pickRay, const Model::PickResult& pickResult) override;
  bool doHandleInput(const InputState& inputState) const override;
};
} // namespace View
} // namespace TrenchBroom
