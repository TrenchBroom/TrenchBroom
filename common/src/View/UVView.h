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
#include "Renderer/OrthographicCamera.h"
#include "View/RenderView.h"
#include "View/ToolBox.h"
#include "View/ToolBoxConnector.h"
#include "View/UVViewHelper.h"
#include "mdl/HitType.h"
#include "mdl/PickResult.h"

#include <filesystem>
#include <memory>
#include <vector>

class QWidget;

namespace tb::mdl
{
class BrushFaceHandle;
class Node;
} // namespace tb::mdl

namespace tb::Renderer
{
class ActiveShader;
class RenderBatch;
class RenderContext;
} // namespace tb::Renderer

namespace tb::View
{
class MapDocument;
class Selection;
class UVRotateTool;
class UVOriginTool;
class UVScaleTool;
class UVShearTool;
class UVOffsetTool;
class UVCameraTool;

/**
 A view which allows the user to manipulate the UV projection interactively with the
 mouse. The user can change UV offsets, scaling factors and rotation. If supported by the
 map format, the user can manipulate the UV axes as well.
 */
class UVView : public RenderView, public ToolBoxConnector
{
  Q_OBJECT
public:
  static const mdl::HitType::Type FaceHitType;

private:
  std::weak_ptr<MapDocument> m_document;

  Renderer::OrthographicCamera m_camera;
  UVViewHelper m_helper;

  ToolBox m_toolBox;

  NotifierConnection m_notifierConnection;

public:
  UVView(std::weak_ptr<MapDocument> document, GLContextManager& contextManager);

  void setSubDivisions(const vm::vec2i& subDivisions);

  bool event(QEvent* event) override;

private:
  void createTools();

  void connectObservers();

  void selectionDidChange(const Selection& selection);
  void documentWasCleared(MapDocument* document);
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);
  void brushFacesDidChange(const std::vector<mdl::BrushFaceHandle>& faces);
  void gridDidChange();
  void cameraDidChange(const Renderer::Camera* camera);
  void preferenceDidChange(const std::filesystem::path& path);

  void updateViewport(int x, int y, int width, int height) override;
  void renderContents() override;
  bool shouldRenderFocusIndicator() const override;
  const Color& getBackgroundColor() override;

  void setupGL(Renderer::RenderContext& renderContext);

  void renderMaterial(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

  void renderFace(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void renderUVAxes(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void renderToolBox(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

public: // implement InputEventProcessor interface
  void processEvent(const KeyEvent& event) override;
  void processEvent(const MouseEvent& event) override;
  void processEvent(const ScrollEvent& event) override;
  void processEvent(const GestureEvent& event) override;
  void processEvent(const CancelEvent& event) override;

private:
  PickRequest pickRequest(float x, float y) const override;
  mdl::PickResult pick(const vm::ray3d& pickRay) const override;
};

} // namespace tb::View
