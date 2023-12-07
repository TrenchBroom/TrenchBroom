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
#include "NotifierConnection.h"
#include "View/ToolBox.h"

#include <memory>

class QStackedLayout;

namespace TrenchBroom
{
namespace View
{
class ClipTool;
class AssembleBrushTool;
class CreateEntityTool;
class DrawShapeTool;
class MoveObjectsTool;
class ExtrudeTool;
class RotateObjectsTool;
class ScaleObjectsTool;
class ShearObjectsTool;
class VertexTool;
class EdgeTool;
class FaceTool;
class MapDocument;
class Selection;

class MapViewToolBox : public ToolBox
{
private:
  std::weak_ptr<MapDocument> m_document;

  std::unique_ptr<ClipTool> m_clipTool;
  std::unique_ptr<AssembleBrushTool> m_assembleBrushTool;
  std::unique_ptr<CreateEntityTool> m_createEntityTool;
  std::unique_ptr<DrawShapeTool> m_drawShapeTool;
  std::unique_ptr<MoveObjectsTool> m_moveObjectsTool;
  std::unique_ptr<ExtrudeTool> m_extrudeTool;
  std::unique_ptr<RotateObjectsTool> m_rotateObjectsTool;
  std::unique_ptr<ScaleObjectsTool> m_scaleObjectsTool;
  std::unique_ptr<ShearObjectsTool> m_shearObjectsTool;
  std::unique_ptr<VertexTool> m_vertexTool;
  std::unique_ptr<EdgeTool> m_edgeTool;
  std::unique_ptr<FaceTool> m_faceTool;

  NotifierConnection m_notifierConnection;

public:
  MapViewToolBox(std::weak_ptr<MapDocument> document, QStackedLayout* bookCtrl);
  ~MapViewToolBox() override;

public: // tools
  ClipTool& clipTool();
  AssembleBrushTool& assembleBrushTool();
  CreateEntityTool& createEntityTool();
  DrawShapeTool& drawShapeTool();
  MoveObjectsTool& moveObjectsTool();
  ExtrudeTool& extrudeTool();
  RotateObjectsTool& rotateObjectsTool();
  ScaleObjectsTool& scaleObjectsTool();
  ShearObjectsTool& shearObjectsTool();
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

  void toggleRotateObjectsTool();
  bool rotateObjectsToolActive() const;
  double rotateToolAngle() const;
  vm::vec3 rotateToolCenter() const;
  void moveRotationCenter(const vm::vec3& delta);

  void toggleScaleObjectsTool();
  bool scaleObjectsToolActive() const;

  void toggleShearObjectsTool();
  bool shearObjectsToolActive() const;

  bool anyVertexToolActive() const;

  void toggleVertexTool();
  bool vertexToolActive() const;

  void toggleEdgeTool();
  bool edgeToolActive() const;

  void toggleFaceTool();
  bool faceToolActive() const;

  void moveVertices(const vm::vec3& delta);

private: // Tool related methods
  void createTools(std::weak_ptr<MapDocument> document, QStackedLayout* bookCtrl);

private: // notification
  void registerTool(Tool& tool, QStackedLayout* bookCtrl);
  void connectObservers();
  void toolActivated(Tool& tool);
  void toolDeactivated(Tool& tool);
  void updateEditorContext();
  void documentWasNewedOrLoaded(MapDocument* document);
  void selectionDidChange(const Selection& selection);

  void updateToolPage();
};
} // namespace View
} // namespace TrenchBroom
