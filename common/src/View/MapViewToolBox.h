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
class CreateComplexBrushTool;
class CreatePrimitiveBrushTool;
class CreateEntityTool;
class CreateSimpleBrushTool;
class MoveObjectsTool;
class ExtrudeTool;
class RotateObjectsTool;
class ScaleObjectsTool;
class ShearObjectsTool;
class VertexTool;
class EdgeTool;
class FaceTool;
class MapDocument;

class MapViewToolBox : public ToolBox
{
private:
  std::weak_ptr<MapDocument> m_document;

  std::unique_ptr<ClipTool> m_clipTool;
  std::unique_ptr<CreateComplexBrushTool> m_createComplexBrushTool;
  std::unique_ptr<CreatePrimitiveBrushTool> m_createPrimitiveBrushTool;
  std::unique_ptr<CreateEntityTool> m_createEntityTool;
  std::unique_ptr<CreateSimpleBrushTool> m_createSimpleBrushTool;
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
  CreateComplexBrushTool& createComplexBrushTool();
  CreatePrimitiveBrushTool &createPrimitiveBrushTool();
  CreateEntityTool& createEntityTool();
  CreateSimpleBrushTool& createSimpleBrushTool();
  MoveObjectsTool& moveObjectsTool();
  ExtrudeTool& extrudeTool();
  RotateObjectsTool& rotateObjectsTool();
  ScaleObjectsTool& scaleObjectsTool();
  ShearObjectsTool& shearObjectsTool();
  VertexTool& vertexTool();
  EdgeTool& edgeTool();
  FaceTool& faceTool();

  void toggleCreateComplexBrushTool();
  bool createComplexBrushToolActive() const;
  void performCreateComplexBrush();

  void toggleCreatePrimitiveBrushTool();
  bool createPrimitiveBrushToolActive() const;
  void performCreatePrimitiveBrush();

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
};
} // namespace View
} // namespace TrenchBroom
