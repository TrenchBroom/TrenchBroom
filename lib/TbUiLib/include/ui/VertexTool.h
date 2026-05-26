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

#include "mdl/BrushVertexCommands.h"
#include "render/PointGuideRenderer.h"
#include "ui/BrushHandleToolBase.h"

#include <string>
#include <vector>

namespace tb
{
namespace gl
{
class Camera;
}

namespace mdl
{
class PickResult;
} // namespace mdl

namespace render
{
class RenderContext;
class RenderBatch;
class RenderService;
} // namespace render

namespace ui
{
class Lasso;
class MapDocument;
class InputState;

class VertexTool : public BrushHandleToolBase<mdl::VertexHandle>
{
private:
  enum class Mode
  {
    Move,
    SplitEdge,
    SplitFace
  };

  Mode m_mode;

  mutable render::PointGuideRenderer m_guideRenderer;

public:
  explicit VertexTool(MapDocument& document);

  using BrushHandleToolBase::findIncidentNodes;

  void pick(
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    double handleRadius,
    mdl::PickResult& pickResult) const override;

public: // Handle selection
  bool deselectAll() override;

  mdl::Hit findDraggableHandle(
    const InputState& inputState, mdl::HitType::Type hitType) const override;

  std::vector<mdl::Hit> collectDraggableHandles(
    const InputState& inputState, mdl::HitType::Type hitType) const override;

public: // Vertex moving
  std::tuple<vm::vec3d, vm::vec3d> handlePositionAndHitPoint(
    const std::vector<mdl::Hit>& hits) const override;

  bool startMove(const std::vector<mdl::Hit>& hits) override;
  MoveResult move(const vm::vec3d& delta) override;
  void endMove() override;
  void cancelMove() override;

  bool allowAbsoluteSnapping() const override;

  vm::vec3d getHandlePosition(const mdl::Hit& hit) const override;
  std::string actionName() const override;

  void removeSelection();

public: // Rendering
  void renderGuide(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const vm::vec3d& position) const override;

private: // Tool interface
  bool doActivate() override;
  bool doDeactivate() override;

private:
  void addHandles(const std::vector<mdl::Node*>& nodes) override;
  void removeHandles(const std::vector<mdl::Node*>& nodes) override;

  void addHandles(mdl::BrushVertexCommand& command) override;
  void removeHandles(mdl::BrushVertexCommand& command) override;

private: // General helper methods
  void resetModeAfterDeselection();
};

} // namespace ui
} // namespace tb
