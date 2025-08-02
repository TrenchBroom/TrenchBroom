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

#include "ui/RotateHandle.h"
#include "ui/Tool.h"

#include "vm/scalar.h"

namespace tb::mdl
{
class Grid;
class Map;
} // namespace tb::mdl

namespace tb::render
{
class Camera;
class RenderBatch;
class RenderContext;
} // namespace tb::render

namespace tb::ui
{

class RotateTool : public Tool
{
private:
  mdl::Map& m_map;
  RotateHandle m_handle;
  double m_angle = vm::to_radians(15.0);

public:
  Notifier<vm::vec3d> rotationCenterDidChangeNotifier;
  Notifier<vm::vec3d> rotationCenterWasUsedNotifier;
  Notifier<RotateHandle::HitArea> handleHitAreaDidChangeNotifier;

  explicit RotateTool(mdl::Map& map);

  bool doActivate() override;

  const mdl::Grid& grid() const;

  void updateToolPageAxis(RotateHandle::HitArea area);

  double angle() const;
  void setAngle(double angle);

  vm::vec3d rotationCenter() const;
  void setRotationCenter(const vm::vec3d& position);
  void resetRotationCenter();

  double majorHandleRadius(const render::Camera& camera) const;
  double minorHandleRadius(const render::Camera& camera) const;

  void beginRotation();
  void commitRotation();
  void cancelRotation();

  double snapRotationAngle(double angle) const;
  void applyRotation(const vm::vec3d& center, const vm::vec3d& axis, double angle);

  mdl::Hit pick2D(const vm::ray3d& pickRay, const render::Camera& camera);
  mdl::Hit pick3D(const vm::ray3d& pickRay, const render::Camera& camera);

  vm::vec3d rotationAxis(RotateHandle::HitArea area) const;

  void renderHandle2D(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  void renderHandle3D(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  void renderHighlight2D(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area);
  void renderHighlight3D(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area);

private:
  QWidget* doCreatePage(QWidget* parent) override;
};

} // namespace tb::ui
