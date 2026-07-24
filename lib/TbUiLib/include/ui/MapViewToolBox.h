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

#include "base/NotifierConnection.h"
#include "ui/ToolBox.h"

#include "vm/vec.h"

#include <memory>

class QStackedLayout;

namespace tb
{
namespace mdl
{
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
class SweepTool;
class ScaleTool;
class ShearTool;
class VertexTool;
class EdgeTool;
class FaceTool;
class ControlPointTool;

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
  std::unique_ptr<SweepTool> m_sweepTool;
  std::unique_ptr<ScaleTool> m_scaleTool;
  std::unique_ptr<ShearTool> m_shearTool;
  std::unique_ptr<VertexTool> m_vertexTool;
  std::unique_ptr<EdgeTool> m_edgeTool;
  std::unique_ptr<FaceTool> m_faceTool;
  std::unique_ptr<ControlPointTool> m_controlPointTool;

  NotifierConnection m_notifierConnection;

public:
  MapViewToolBox(MapDocument& map, QStackedLayout* bookCtrl);
  ~MapViewToolBox() override;

public: // tools
  const ClipTool& clipTool() const;
  ClipTool& clipTool();

  const AssembleBrushTool& assembleBrushTool() const;
  AssembleBrushTool& assembleBrushTool();

  const CreateEntityTool& createEntityTool() const;
  CreateEntityTool& createEntityTool();

  const DrawShapeTool& drawShapeTool() const;
  DrawShapeTool& drawShapeTool();

  const MoveObjectsTool& moveObjectsTool() const;
  MoveObjectsTool& moveObjectsTool();

  const ExtrudeTool& extrudeTool() const;
  ExtrudeTool& extrudeTool();

  const RotateTool& rotateTool() const;
  RotateTool& rotateTool();

  const SweepTool& sweepTool() const;
  SweepTool& sweepTool();

  const ScaleTool& scaleTool() const;
  ScaleTool& scaleTool();

  const ShearTool& shearTool() const;
  ShearTool& shearTool();

  const VertexTool& vertexTool() const;
  VertexTool& vertexTool();

  const EdgeTool& edgeTool() const;
  EdgeTool& edgeTool();

  const FaceTool& faceTool() const;
  FaceTool& faceTool();

  const ControlPointTool& controlPointTool() const;
  ControlPointTool& controlPointTool();

  bool canToggleAssembleBrushTool() const;
  void toggleAssembleBrushTool();
  bool assembleBrushToolActive() const;
  void performAssembleBrush();

  bool canToggleClipTool() const;
  void toggleClipTool();
  bool clipToolActive() const;
  void toggleClipSide();
  void performClip();
  void removeLastClipPoint();

  bool canToggleRotateTool() const;
  void toggleRotateTool();
  bool rotateToolActive() const;
  double rotateToolAngle() const;
  vm::vec3d rotateToolCenter() const;
  void moveRotationCenter(const vm::vec3d& delta);

  bool canToggleSweepTool() const;
  void toggleSweepTool();
  bool sweepToolActive() const;
  void performSweep();

  bool canToggleScaleTool() const;
  void toggleScaleTool();
  bool scaleToolActive() const;

  bool canToggleShearTool() const;
  void toggleShearTool();
  bool shearToolActive() const;

  bool canToggleAnyVertexTool() const;
  bool anyVertexToolActive() const;
  bool anyNodeHandleToolActive() const;

  void toggleVertexTool();
  bool vertexToolActive() const;

  void toggleEdgeTool();
  bool edgeToolActive() const;

  void toggleFaceTool();
  bool faceToolActive() const;

  bool canToggleControlPointTool() const;
  void toggleControlPointTool();
  bool controlPointToolActive() const;

  bool anyModalToolActive() const;

  void moveNodeHandles(const vm::vec3d& delta);

private: // Tool related methods
  void createTools(QStackedLayout* bookCtrl);

private: // notification
  void registerTool(Tool& tool, QStackedLayout* bookCtrl);
  void connectObservers();
  void toolActivated(Tool& tool);
  void toolDeactivated(Tool& tool);
  void updateEditorContext();
  void documentWasLoaded();
  void selectionDidChange(const mdl::SelectionChange& selectionChange);

  void updateToolPage();
};

} // namespace ui
} // namespace tb
