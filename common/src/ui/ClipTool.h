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

#include "NotifierConnection.h"
#include "mdl/HitType.h"
#include "ui/Tool.h"

#include "vm/ray.h"
#include "vm/vec.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace tb
{
namespace mdl
{
class BrushFace;
class BrushFaceHandle;
class Grid;
class Map;
class Node;
class PickResult;

struct SelectionChange;
} // namespace mdl

namespace render
{
class BrushRenderer;
class Camera;
class RenderBatch;
class RenderContext;
} // namespace render

namespace ui
{
class ClipStrategy;

class ClipTool : public Tool
{
public:
  static const mdl::HitType::Type PointHitType;

private:
  enum class ClipSide
  {
    Front,
    Both,
    Back
  };

private:
  mdl::Map& m_map;

  ClipSide m_clipSide = ClipSide::Front;
  std::unique_ptr<ClipStrategy> m_strategy;

  std::map<mdl::Node*, std::vector<mdl::Node*>> m_frontBrushes;
  std::map<mdl::Node*, std::vector<mdl::Node*>> m_backBrushes;

  std::unique_ptr<render::BrushRenderer> m_remainingBrushRenderer;
  std::unique_ptr<render::BrushRenderer> m_clippedBrushRenderer;

  bool m_ignoreNotifications = false;
  bool m_dragging = false;

  NotifierConnection m_notifierConnection;

public:
  explicit ClipTool(mdl::Map& map);
  ~ClipTool() override;

  const mdl::Grid& grid() const;

  void toggleSide();

  void pick(
    const vm::ray3d& pickRay, const render::Camera& camera, mdl::PickResult& pickResult);

  void render(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const mdl::PickResult& pickResult);

private:
  void renderBrushes(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  void renderStrategy(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const mdl::PickResult& pickResult);

public:
  void renderFeedback(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const vm::vec3d& point) const;

public:
  bool hasBrushes() const;
  bool canClip() const;
  void performClip();

private:
  std::map<mdl::Node*, std::vector<mdl::Node*>> clipBrushes();

public:
  std::optional<vm::vec3d> defaultClipPointPos() const;

  bool canAddPoint(const vm::vec3d& point) const;
  bool hasPoints() const;
  void addPoint(const vm::vec3d& point, const std::vector<vm::vec3d>& helpVectors);
  bool canRemoveLastPoint() const;
  bool removeLastPoint();

  std::optional<std::tuple<vm::vec3d, vm::vec3d>> beginDragPoint(
    const mdl::PickResult& pickResult);
  void beginDragLastPoint();
  bool dragPoint(const vm::vec3d& newPosition, const std::vector<vm::vec3d>& helpVectors);
  void endDragPoint();
  void cancelDragPoint();

  void setFace(const mdl::BrushFaceHandle& face);
  bool reset();

private:
  void resetStrategy();
  void update();

  void clearBrushes();
  void updateBrushes();

  void setFaceAttributes(
    const std::vector<mdl::BrushFace>& faces, mdl::BrushFace& toSet) const;

  void clearRenderers();
  void updateRenderers();
  void addBrushesToRenderer(
    const std::map<mdl::Node*, std::vector<mdl::Node*>>& map,
    render::BrushRenderer& renderer);

  bool keepFrontBrushes() const;
  bool keepBackBrushes() const;

private:
  bool doActivate() override;
  bool doDeactivate() override;

  bool doRemove();

  void connectObservers();
  void selectionDidChange(const mdl::SelectionChange& selectionChange);
  void nodesWillChange(const std::vector<mdl::Node*>& nodes);
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);
  void brushFacesDidChange(const std::vector<mdl::BrushFaceHandle>& nodes);
};

} // namespace ui
} // namespace tb
