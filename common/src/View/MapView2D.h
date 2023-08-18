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

#include "NotifierConnection.h"
#include "View/MapViewBase.h"

#include <vecmath/forward.h>

#include <memory>

namespace TrenchBroom
{
class Logger;

namespace Model
{
class PickResult;
}

namespace Renderer
{
class MapRenderer;
class OrthographicCamera;
class RenderBatch;
class RenderContext;
enum class RenderMode;
} // namespace Renderer

namespace View
{
class MapView2D : public MapViewBase
{
  Q_OBJECT
public:
  typedef enum
  {
    ViewPlane_XY,
    ViewPlane_XZ,
    ViewPlane_YZ
  } ViewPlane;

private:
  std::unique_ptr<Renderer::OrthographicCamera> m_camera;

  NotifierConnection m_notifierConnection;

public:
  MapView2D(
    std::weak_ptr<MapDocument> document,
    MapViewToolBox& toolBox,
    Renderer::MapRenderer& renderer,
    GLContextManager& contextManager,
    ViewPlane viewPlane,
    Logger* logger);

private:
  void initializeCamera(ViewPlane viewPlane);
  void initializeToolChain(MapViewToolBox& toolBox);

private: // notification
  void connectObservers();
  void cameraDidChange(const Renderer::Camera* camera);

private: // implement ToolBoxConnector interface
  PickRequest doGetPickRequest(float x, float y) const override;
  Model::PickResult doPick(const vm::ray3& pickRay) const override;

protected: // QOpenGLWidget overrides
  void initializeGL() override;

private: // implement RenderView interface
  void doUpdateViewport(int x, int y, int width, int height) override;

private: // implement MapView interface
  vm::vec3 doGetPasteObjectsDelta(
    const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const override;
  bool doCanSelectTall() override;
  void doSelectTall() override;
  void doFocusCameraOnSelection(bool animate) override;

  void doMoveCameraToPosition(const vm::vec3& position, bool animate) override;
  void animateCamera(
    const vm::vec3f& position,
    const vm::vec3f& direction,
    const vm::vec3f& up,
    const int duration = DefaultCameraAnimationDuration);

  void doMoveCameraToCurrentTracePoint() override;

private: // implement MapViewBase interface
  vm::vec3 doGetMoveDirection(vm::direction direction) const override;
  size_t doGetFlipAxis(const vm::direction direction) const override;
  vm::vec3 doComputePointEntityPosition(const vm::bbox3& bounds) const override;

  ActionContext::Type doGetActionContext() const override;
  ActionView doGetActionView() const override;
  bool doCancel() override;

  Renderer::RenderMode doGetRenderMode() override;
  Renderer::Camera& doGetCamera() override;
  void doRenderGrid(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
  void doRenderMap(
    Renderer::MapRenderer& renderer,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) override;
  void doRenderTools(
    MapViewToolBox& toolBox,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) override;
  void doRenderExtras(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
  void doRenderSoftWorldBounds(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

public: // implement CameraLinkableView interface
  void linkCamera(CameraLinkHelper& linkHelper) override;
};
} // namespace View
} // namespace TrenchBroom
