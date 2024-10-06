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

#include "Renderer/PointGuideRenderer.h"
#include "View/VertexHandleManager.h"
#include "View/VertexToolBase.h"

#include <memory>
#include <string>
#include <vector>

namespace tb::mdl
{
class PickResult;
}

namespace tb::Renderer
{
class Camera;
class RenderContext;
class RenderBatch;
class RenderService;
} // namespace tb::Renderer

namespace tb::View
{
class BrushVertexCommandBase;
class Grid;
class Lasso;
class Selection;

class VertexTool : public VertexToolBase<vm::vec3d>
{
private:
  enum class Mode
  {
    Move,
    SplitEdge,
    SplitFace
  };

  Mode m_mode;

  std::unique_ptr<VertexHandleManager> m_vertexHandles;
  std::unique_ptr<EdgeHandleManager> m_edgeHandles;
  std::unique_ptr<FaceHandleManager> m_faceHandles;

  mutable Renderer::PointGuideRenderer m_guideRenderer;

public:
  explicit VertexTool(std::weak_ptr<MapDocument> document);

public:
  std::vector<mdl::BrushNode*> findIncidentBrushes(const vm::vec3d& handle) const;
  std::vector<mdl::BrushNode*> findIncidentBrushes(const vm::segment3d& handle) const;
  std::vector<mdl::BrushNode*> findIncidentBrushes(const vm::polygon3d& handle) const;

private:
  using VertexToolBase::findIncidentBrushes;

public:
  void pick(
    const vm::ray3d& pickRay,
    const Renderer::Camera& camera,
    mdl::PickResult& pickResult) const override;

public: // Handle selection
  bool deselectAll() override;

public:
  VertexHandleManager& handleManager() override;
  const VertexHandleManager& handleManager() const override;

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
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const vm::vec3d& position) const override;

private: // Tool interface
  bool doActivate() override;
  bool doDeactivate() override;

private:
  void addHandles(const std::vector<mdl::Node*>& nodes) override;
  void removeHandles(const std::vector<mdl::Node*>& nodes) override;

  void addHandles(BrushVertexCommandBase* command) override;
  void removeHandles(BrushVertexCommandBase* command) override;

private: // General helper methods
  void resetModeAfterDeselection();
};

} // namespace tb::View
