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

namespace tb::mdl
{
class Map;
}

namespace tb::render
{
class Camera;
class RenderBatch;
class RenderContext;
} // namespace tb::render

namespace tb::ui
{
class ScaleTool;

class ScaleToolController : public ToolController
{
protected:
  ScaleTool& m_tool;

private:
  mdl::Map& m_map;

public:
  explicit ScaleToolController(ScaleTool& tool, mdl::Map& map);
  ~ScaleToolController() override;

private:
  Tool& tool() override;
  const Tool& tool() const override;

  void pick(const InputState& inputState, mdl::PickResult& pickResult) override;

  void modifierKeyChange(const InputState& inputState) override;

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
  virtual void doPick(
    const vm::ray3d& pickRay,
    const render::Camera& camera,
    mdl::PickResult& pickResult) const = 0;
};

class ScaleToolController2D : public ScaleToolController
{
public:
  explicit ScaleToolController2D(ScaleTool& tool, mdl::Map& map);

private:
  void doPick(
    const vm::ray3d& pickRay,
    const render::Camera& camera,
    mdl::PickResult& pickResult) const override;
};

class ScaleToolController3D : public ScaleToolController
{
public:
  explicit ScaleToolController3D(ScaleTool& tool, mdl::Map& map);

private:
  void doPick(
    const vm::ray3d& pickRay,
    const render::Camera& camera,
    mdl::PickResult& pickResult) const override;
};

} // namespace tb::ui
