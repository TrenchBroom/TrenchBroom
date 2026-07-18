/*
 Copyright (C) 2026 Jackson Palmer
 Copyright (C) 2026 Kristian Duske

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

#include "base/NotifierConnection.h"
#include "mdl/Hit.h"
#include "mdl/HitType.h"
#include "ui/RotateHandle.h"
#include "ui/RotateHandleController.h"
#include "ui/SweepToolUtils.h"
#include "ui/Tool.h"

#include "vm/ray.h"
#include "vm/vec.h"

#include <map>
#include <memory>
#include <vector>

namespace tb
{
namespace gl
{
class Camera;
}

namespace mdl
{
class BrushNode;
class Grid;
class Node;
} // namespace mdl

namespace render
{
class BrushRenderer;
class RenderBatch;
class RenderContext;
} // namespace render

namespace ui
{
class MapDocument;

/**
 * Sweeps the selected brush faces into a run of brushes. A ghost copy of the faces (the
 * destination cap) is dragged into place with a gizmo, and the gap is filled with one
 * brush per segment.
 */
class SweepTool : public Tool, public RotateHandleDelegate, public PointHandleDelegate
{
public:
  static constexpr double MinScaleFactor = 0.01;

  static const mdl::HitType::Type ScaleHitType;

private:
  MapDocument& m_document;
  RotateHandle m_handle;

  SweepSource m_source;
  SweepTransform m_transform;
  SweepParameters m_parameters;

  std::map<mdl::Node*, std::vector<std::unique_ptr<mdl::BrushNode>>> m_previewBrushes;
  std::unique_ptr<render::BrushRenderer> m_brushRenderer;

  NotifierConnection m_notifierConnection;

public:
  explicit SweepTool(MapDocument& document);
  ~SweepTool() override;

  bool doActivate() override;
  bool doDeactivate() override;

  bool applies() const;

  const SweepTransform& transform() const;
  void setTransform(const SweepTransform& transform);

  const SweepParameters& parameters() const;
  void setParameters(const SweepParameters& parameters);

  vm::vec3d destinationCenter() const;
  void setDestinationCenter(const vm::vec3d& position);

  void reset();
  bool cancel();

  double minorHandleRadius(const gl::Camera& camera) const;

  void updateBrushes();
  void commitSweep();

  void renderDestinationGhost(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const;
  void renderPreview(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const;

  bool hasScaleHandle() const;
  vm::vec3d scaleHandlePosition() const;
  void dragScaleHandleTo(const vm::vec3d& position);
  mdl::Hit pickScaleHandle(const vm::ray3d& pickRay, const gl::Camera& camera) const;
  void renderScaleHandle(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const;
  void renderScaleHighlight(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const;

  // RotateHandleDelegate
  Tool& tool() override;
  const Tool& tool() const override;
  const mdl::Grid& grid() const override;
  RotateHandle& handle() override;
  double handleSnapAngle() const override;
  vm::vec3d handleCenter() const override;
  void setHandleCenter(const vm::vec3d& position) override;
  std::unique_ptr<RingDragTracker> beginRingDrag() override;

  // PointHandleDelegate (scale handle)
  vm::vec3d handlePosition() const override;
  void setHandlePosition(const vm::vec3d& position) override;
  void renderHighlight(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) const override;

private:
  void connectObservers();
  void nodesWereRemoved(const std::vector<mdl::Node*>& nodes);

  QWidget* doCreatePage(QWidget* parent) override;
};

} // namespace ui
} // namespace tb
