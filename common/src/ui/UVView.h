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
#include "mdl/PickResult.h"
#include "render/OrthographicCamera.h"
#include "ui/RenderView.h"
#include "ui/ToolBox.h"
#include "ui/ToolBoxConnector.h"
#include "ui/UVViewHelper.h"

#include <filesystem>
#include <vector>

class QWidget;

namespace tb
{
namespace mdl
{
class BrushFaceHandle;
class Map;
class Node;

struct SelectionChange;
} // namespace mdl

namespace render
{
class ActiveShader;
class RenderBatch;
class RenderContext;
} // namespace render

namespace ui
{
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
  mdl::Map& m_map;

  render::OrthographicCamera m_camera;
  UVViewHelper m_helper;

  ToolBox m_toolBox;

  NotifierConnection m_notifierConnection;

public:
  UVView(mdl::Map& map, GLContextManager& contextManager);

  void setSubDivisions(const vm::vec2i& subDivisions);

  bool event(QEvent* event) override;

private:
  void createTools();

  void connectObservers();

  void selectionDidChange(const mdl::SelectionChange& selectionChange);
  void mapWasCleared(mdl::Map& map);
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);
  void brushFacesDidChange(const std::vector<mdl::BrushFaceHandle>& faces);
  void gridDidChange();
  void cameraDidChange(const render::Camera* camera);
  void preferenceDidChange(const std::filesystem::path& path);

  void updateViewport(int x, int y, int width, int height) override;
  void renderContents() override;
  bool shouldRenderFocusIndicator() const override;
  const Color& getBackgroundColor() override;

  void setupGL(render::RenderContext& renderContext);

  void renderMaterial(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);

  void renderFace(render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  void renderUVAxes(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  void renderToolBox(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);

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

} // namespace ui
} // namespace tb
