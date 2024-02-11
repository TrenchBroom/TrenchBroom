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
#include "Model/HitType.h"
#include "NotifierConnection.h"
#include "View/Tool.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace TrenchBroom::Model
{
class BrushFace;
class BrushFaceHandle;
class Node;
class PickResult;
} // namespace TrenchBroom::Model

namespace TrenchBroom::Renderer
{
class BrushRenderer;
class Camera;
class RenderBatch;
class RenderContext;
} // namespace TrenchBroom::Renderer

namespace TrenchBroom::View
{
class Grid;
class MapDocument;
class Selection;

class ClipStrategy;

class ClipTool : public Tool
{
public:
  static const Model::HitType::Type PointHitType;

private:
  enum class ClipSide
  {
    Front,
    Both,
    Back
  };

private:
  std::weak_ptr<MapDocument> m_document;

  ClipSide m_clipSide = ClipSide::Front;
  std::unique_ptr<ClipStrategy> m_strategy;

  std::map<Model::Node*, std::vector<Model::Node*>> m_frontBrushes;
  std::map<Model::Node*, std::vector<Model::Node*>> m_backBrushes;

  std::unique_ptr<Renderer::BrushRenderer> m_remainingBrushRenderer;
  std::unique_ptr<Renderer::BrushRenderer> m_clippedBrushRenderer;

  bool m_ignoreNotifications = false;
  bool m_dragging = false;

  NotifierConnection m_notifierConnection;

public:
  explicit ClipTool(std::weak_ptr<MapDocument> document);
  ~ClipTool() override;

  const Grid& grid() const;

  void toggleSide();

  void pick(
    const vm::ray3& pickRay,
    const Renderer::Camera& camera,
    Model::PickResult& pickResult);

  void render(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const Model::PickResult& pickResult);

private:
  void renderBrushes(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void renderStrategy(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const Model::PickResult& pickResult);

public:
  void renderFeedback(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const vm::vec3& point) const;

public:
  bool hasBrushes() const;
  bool canClip() const;
  void performClip();

private:
  std::map<Model::Node*, std::vector<Model::Node*>> clipBrushes();

public:
  vm::vec3 defaultClipPointPos() const;

  bool canAddPoint(const vm::vec3& point) const;
  bool hasPoints() const;
  void addPoint(const vm::vec3& point, const std::vector<vm::vec3>& helpVectors);
  bool canRemoveLastPoint() const;
  bool removeLastPoint();

  std::optional<std::tuple<vm::vec3, vm::vec3>> beginDragPoint(
    const Model::PickResult& pickResult);
  void beginDragLastPoint();
  bool dragPoint(const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors);
  void endDragPoint();
  void cancelDragPoint();

  void setFace(const Model::BrushFaceHandle& face);
  bool reset();

private:
  void resetStrategy();
  void update();

  void clearBrushes();
  void updateBrushes();

  void setFaceAttributes(
    const std::vector<Model::BrushFace>& faces, Model::BrushFace& toSet) const;

  void clearRenderers();
  void updateRenderers();
  void addBrushesToRenderer(
    const std::map<Model::Node*, std::vector<Model::Node*>>& map,
    Renderer::BrushRenderer& renderer);

  bool keepFrontBrushes() const;
  bool keepBackBrushes() const;

private:
  bool doActivate() override;
  bool doDeactivate() override;

  bool doRemove();

  void connectObservers();
  void selectionDidChange(const Selection& selection);
  void nodesWillChange(const std::vector<Model::Node*>& nodes);
  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& nodes);
};

} // namespace TrenchBroom::View
