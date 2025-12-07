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
#include "ui/ToolBox.h"

#include "vm/vec.h"

#include <memory>

class QStackedLayout;

namespace tb
{
namespace mdl
{
class Map;

struct SelectionChange;
} // namespace mdl

namespace ui
{
class ClipTool;
class AssembleBrushTool;
class CreateEntityTool;
class DrawShapeTool;
class MapDocument;
class MoveObjectsTool;
class ExtrudeTool;
class RotateTool;
class ScaleTool;
class ShearTool;
class VertexTool;
class EdgeTool;
class FaceTool;

class MapViewToolBox : public ToolBox
{
private:
  MapDocument& m_document;

  std::unique_ptr<ClipTool> m_clipTool;
  std::unique_ptr<AssembleBrushTool> m_assembleBrushTool;
  std::unique_ptr<CreateEntityTool> m_createEntityTool;
  std::unique_ptr<DrawShapeTool> m_drawShapeTool;
  std::unique_ptr<MoveObjectsTool> m_moveObjectsTool;
  std::unique_ptr<ExtrudeTool> m_extrudeTool;
  std::unique_ptr<RotateTool> m_rotateTool;
  std::unique_ptr<ScaleTool> m_scaleTool;
  std::unique_ptr<ShearTool> m_shearTool;
  std::unique_ptr<VertexTool> m_vertexTool;
  std::unique_ptr<EdgeTool> m_edgeTool;
  std::unique_ptr<FaceTool> m_faceTool;

  NotifierConnection m_notifierConnection;

public:
  MapViewToolBox(MapDocument& map, QStackedLayout* bookCtrl);
  ~MapViewToolBox() override;

public: // tools
  ClipTool& clipTool();
  AssembleBrushTool& assembleBrushTool();
  CreateEntityTool& createEntityTool();
  DrawShapeTool& drawShapeTool();
  MoveObjectsTool& moveObjectsTool();
  ExtrudeTool& extrudeTool();
  RotateTool& rotateTool();
  ScaleTool& scaleTool();
  ShearTool& shearTool();
  VertexTool& vertexTool();
  EdgeTool& edgeTool();
  FaceTool& faceTool();

  void toggleAssembleBrushTool();
  bool assembleBrushToolActive() const;
  void performAssembleBrush();

  void toggleClipTool();
  bool clipToolActive() const;
  void toggleClipSide();
  void performClip();
  void removeLastClipPoint();

  void toggleRotateTool();
  bool rotateToolActive() const;
  double rotateToolAngle() const;
  vm::vec3d rotateToolCenter() const;
  void moveRotationCenter(const vm::vec3d& delta);

  void toggleScaleTool();
  bool scaleToolActive() const;

  void toggleShearTool();
  bool shearToolActive() const;

  bool anyVertexToolActive() const;

  void toggleVertexTool();
  bool vertexToolActive() const;

  void toggleEdgeTool();
  bool edgeToolActive() const;

  void toggleFaceTool();
  bool faceToolActive() const;

  bool anyModalToolActive() const;

  void moveVertices(const vm::vec3d& delta);

private: // Tool related methods
  void createTools(QStackedLayout* bookCtrl);

private: // notification
  void registerTool(Tool& tool, QStackedLayout* bookCtrl);
  void connectObservers();
  void toolActivated(Tool& tool);
  void toolDeactivated(Tool& tool);
  void updateEditorContext();
  void mapWasCreated(mdl::Map& map);
  void mapWasLoaded(mdl::Map& map);
  void selectionDidChange(const mdl::SelectionChange& selectionChange);

  void updateToolPage();
};

} // namespace ui
} // namespace tb
