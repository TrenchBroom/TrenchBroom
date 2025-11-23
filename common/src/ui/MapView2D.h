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
#include "ui/MapViewBase.h"

#include "vm/vec.h"

namespace tb
{
namespace mdl
{
class PickResult;
}

namespace render
{
class MapRenderer;
class OrthographicCamera;
class RenderBatch;
class RenderContext;
enum class RenderMode;
} // namespace render

namespace ui
{

class MapView2D : public MapViewBase
{
  Q_OBJECT
public:
  enum class ViewPlane
  {
    XY,
    XZ,
    YZ,
  };

private:
  std::unique_ptr<render::OrthographicCamera> m_camera;

  NotifierConnection m_notifierConnection;

public:
  MapView2D(
    MapDocument& document,
    MapViewToolBox& toolBox,
    render::MapRenderer& renderer,
    GLContextManager& contextManager,
    ViewPlane viewPlane);

private:
  void initializeCamera(ViewPlane viewPlane);
  void initializeToolChain(MapViewToolBox& toolBox);

private: // notification
  void connectObservers();
  void cameraDidChange(const render::Camera* camera);

private: // implement ToolBoxConnector interface
  PickRequest pickRequest(float x, float y) const override;
  mdl::PickResult pick(const vm::ray3d& pickRay) const override;

protected: // QOpenGLWidget overrides
  void initializeGL() override;

private: // implement RenderView interface
  void updateViewport(int x, int y, int width, int height) override;

private: // implement MapView interface
  vm::vec3d pasteObjectsDelta(
    const vm::bbox3d& bounds, const vm::bbox3d& referenceBounds) const override;
  bool canSelectTall() override;
  void selectTall() override;
  void reset2dCameras(const render::Camera& masterCamera, bool animate) override;
  void focusCameraOnSelection(bool animate) override;

  void moveCameraToPosition(const vm::vec3f& position, bool animate) override;
  void animateCamera(
    const vm::vec3f& position,
    const vm::vec3f& direction,
    const vm::vec3f& up,
    float zoom,
    int duration = DefaultCameraAnimationDuration);

  void moveCameraToCurrentTracePoint() override;

public: // implement MapViewBase interface
  render::Camera& camera() override;

  vm::vec3d moveDirection(vm::direction direction) const override;
  size_t flipAxis(vm::direction direction) const override;
  vm::vec3d computePointEntityPosition(const vm::bbox3d& bounds) const override;

  ActionContext::Type viewActionContext() const override;

private:
  render::RenderMode renderMode() override;

  void renderGrid(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) override;
  void renderMap(
    render::MapRenderer& renderer,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override;
  void renderTools(
    MapViewToolBox& toolBox,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override;
  void renderSoftWorldBounds(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) override;

public: // implement CameraLinkableView interface
  void linkCamera(CameraLinkHelper& linkHelper) override;
};

} // namespace ui
} // namespace tb
