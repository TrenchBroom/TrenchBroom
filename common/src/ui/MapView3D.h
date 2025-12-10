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

#include <filesystem>
#include <vector>

class QKeyEvent;

namespace tb
{
namespace render
{
class PerspectiveCamera;
}

namespace ui
{
class FlyModeHelper;

class MapView3D : public MapViewBase
{
  Q_OBJECT
private:
  std::unique_ptr<render::PerspectiveCamera> m_camera;
  std::unique_ptr<FlyModeHelper> m_flyModeHelper;
  bool m_ignoreCameraChangeEvents = false;

  NotifierConnection m_notifierConnection;

public:
  MapView3D(
    MapDocument& document, MapViewToolBox& toolBox, GLContextManager& contextManager);
  ~MapView3D() override;

private:
  void initializeCamera();
  void initializeToolChain(MapViewToolBox& toolBox);

private: // notification
  void connectObservers();
  void cameraDidChange(const render::Camera* camera);
  void preferenceDidChange(const std::filesystem::path& path);

protected: // QWidget overrides
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;

protected: // QOpenGLWidget overrides
  void initializeGL() override;

private: // interaction events
  void bindEvents();

private: // other events
  void updateFlyMode();
  void resetFlyModeKeys();

private: // implement ToolBoxConnector interface
  PickRequest pickRequest(float x, float y) const override;
  mdl::PickResult pick(const vm::ray3d& pickRay) const override;

private: // implement RenderView interface
  void updateViewport(int x, int y, int width, int height) override;

private: // implement MapView interface
  vm::vec3d pasteObjectsDelta(
    const vm::bbox3d& bounds, const vm::bbox3d& referenceBounds) const override;

  bool canSelectTall() override;
  void selectTall() override;

  void reset2dCameras(const render::Camera& masterCamera, bool animate) override;
  void focusCameraOnSelection(bool animate) override;

  vm::vec3f focusCameraOnObjectsPosition(const std::vector<mdl::Node*>& nodes);

  void moveCameraToPosition(const vm::vec3f& position, bool animate) override;
  void animateCamera(
    const vm::vec3f& position,
    const vm::vec3f& direction,
    const vm::vec3f& up,
    float zoom,
    int duration = DefaultCameraAnimationDuration);

  void moveCameraToCurrentTracePoint() override;

private: // implement MapViewBase interface
  render::Camera& camera() override;

  vm::vec3d moveDirection(vm::direction direction) const override;
  size_t flipAxis(vm::direction direction) const override;
  vm::vec3d computePointEntityPosition(const vm::bbox3d& bounds) const override;

  ActionContext::Type viewActionContext() const override;

  void preRender() override;
  render::RenderMode renderMode() override;

  void renderMap(
    render::MapRenderer& renderer,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override;
  void renderTools(
    MapViewToolBox& toolBox,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override;

  void beforePopupMenu() override;

public: // implement CameraLinkableView interface
  void linkCamera(CameraLinkHelper& linkHelper) override;
};

} // namespace ui
} // namespace tb
