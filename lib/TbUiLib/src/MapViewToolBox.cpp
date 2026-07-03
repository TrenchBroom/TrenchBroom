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

#include "ui/MapViewToolBox.h"

#include "mdl/EditorContext.h"
#include "mdl/Map.h"
#include "mdl/Selection.h"
#include "ui/AssembleBrushTool.h"
#include "ui/ClipTool.h"
#include "ui/ControlPointTool.h"
#include "ui/CreateEntityTool.h"
#include "ui/DrawShapeTool.h"
#include "ui/EdgeTool.h"
#include "ui/ExtrudeTool.h"
#include "ui/FaceTool.h"
#include "ui/MapDocument.h"
#include "ui/MoveObjectsTool.h"
#include "ui/RotateTool.h"
#include "ui/ScaleTool.h"
#include "ui/ShearTool.h"
#include "ui/SweepTool.h"
#include "ui/VertexTool.h"

#include "kd/contracts.h"

namespace tb::ui
{

MapViewToolBox::MapViewToolBox(MapDocument& document, QStackedLayout* bookCtrl)
  : m_document{document}
{
  createTools(bookCtrl);
  connectObservers();
}

MapViewToolBox::~MapViewToolBox() = default;

ClipTool& MapViewToolBox::clipTool()
{
  return *m_clipTool;
}

const AssembleBrushTool& MapViewToolBox::assembleBrushTool() const
{
  return *m_assembleBrushTool;
}

AssembleBrushTool& MapViewToolBox::assembleBrushTool()
{
  return KDL_CONST_OVERLOAD(assembleBrushTool());
}

const CreateEntityTool& MapViewToolBox::createEntityTool() const
{
  return *m_createEntityTool;
}

CreateEntityTool& MapViewToolBox::createEntityTool()
{
  return KDL_CONST_OVERLOAD(createEntityTool());
}

const DrawShapeTool& MapViewToolBox::drawShapeTool() const
{
  return *m_drawShapeTool;
}

DrawShapeTool& MapViewToolBox::drawShapeTool()
{
  return KDL_CONST_OVERLOAD(drawShapeTool());
}

const MoveObjectsTool& MapViewToolBox::moveObjectsTool() const
{
  return *m_moveObjectsTool;
}

MoveObjectsTool& MapViewToolBox::moveObjectsTool()
{
  return KDL_CONST_OVERLOAD(moveObjectsTool());
}

const ExtrudeTool& MapViewToolBox::extrudeTool() const
{
  return *m_extrudeTool;
}

ExtrudeTool& MapViewToolBox::extrudeTool()
{
  return KDL_CONST_OVERLOAD(extrudeTool());
}

const RotateTool& MapViewToolBox::rotateTool() const
{
  return *m_rotateTool;
}

RotateTool& MapViewToolBox::rotateTool()
{
  return KDL_CONST_OVERLOAD(rotateTool());
}

const SweepTool& MapViewToolBox::sweepTool() const
{
  return *m_sweepTool;
}

SweepTool& MapViewToolBox::sweepTool()
{
  return KDL_CONST_OVERLOAD(sweepTool());
}

const ScaleTool& MapViewToolBox::scaleTool() const
{
  return *m_scaleTool;
}

ScaleTool& MapViewToolBox::scaleTool()
{
  return KDL_CONST_OVERLOAD(scaleTool());
}

const ShearTool& MapViewToolBox::shearTool() const
{
  return *m_shearTool;
}

ShearTool& MapViewToolBox::shearTool()
{
  return KDL_CONST_OVERLOAD(shearTool());
}

const VertexTool& MapViewToolBox::vertexTool() const
{
  return *m_vertexTool;
}

VertexTool& MapViewToolBox::vertexTool()
{
  return KDL_CONST_OVERLOAD(vertexTool());
}

const EdgeTool& MapViewToolBox::edgeTool() const
{
  return *m_edgeTool;
}

EdgeTool& MapViewToolBox::edgeTool()
{
  return KDL_CONST_OVERLOAD(edgeTool());
}

const FaceTool& MapViewToolBox::faceTool() const
{
  return *m_faceTool;
}

FaceTool& MapViewToolBox::faceTool()
{
  return KDL_CONST_OVERLOAD(faceTool());
}

const ControlPointTool& MapViewToolBox::controlPointTool() const
{
  return *m_controlPointTool;
}

ControlPointTool& MapViewToolBox::controlPointTool()
{
  return KDL_CONST_OVERLOAD(controlPointTool());
}

bool MapViewToolBox::canToggleAssembleBrushTool() const
{
  return true;
}

void MapViewToolBox::toggleAssembleBrushTool()
{
  if (canToggleAssembleBrushTool())
  {
    toggleTool(assembleBrushTool());
  }
}

bool MapViewToolBox::assembleBrushToolActive() const
{
  return m_assembleBrushTool->active();
}

void MapViewToolBox::performAssembleBrush()
{
  contract_pre(assembleBrushToolActive());

  m_assembleBrushTool->createBrushes();
}

bool MapViewToolBox::canToggleClipTool() const
{
  const auto& map = m_document.map();
  return clipToolActive() || map.selection().hasOnlyBrushes();
}

void MapViewToolBox::toggleClipTool()
{
  if (canToggleClipTool())
  {
    toggleTool(clipTool());
  }
}

bool MapViewToolBox::clipToolActive() const
{
  return m_clipTool->active();
}

void MapViewToolBox::toggleClipSide()
{
  contract_pre(clipToolActive());

  m_clipTool->toggleSide();
}

void MapViewToolBox::performClip()
{
  contract_pre(clipToolActive());

  m_clipTool->performClip();
}

void MapViewToolBox::removeLastClipPoint()
{
  contract_pre(clipToolActive());

  m_clipTool->removeLastPoint();
}

bool MapViewToolBox::canToggleRotateTool() const
{
  const auto& map = m_document.map();
  return rotateToolActive() || map.selection().hasNodes();
}

void MapViewToolBox::toggleRotateTool()
{
  if (canToggleRotateTool())
  {
    toggleTool(rotateTool());
  }
}

bool MapViewToolBox::rotateToolActive() const
{
  return m_rotateTool->active();
}

double MapViewToolBox::rotateToolAngle() const
{
  contract_pre(rotateToolActive());

  return m_rotateTool->angle();
}

vm::vec3d MapViewToolBox::rotateToolCenter() const
{
  contract_pre(rotateToolActive());

  return m_rotateTool->rotationCenter();
}

void MapViewToolBox::moveRotationCenter(const vm::vec3d& delta)
{
  contract_pre(rotateToolActive());

  const vm::vec3d center = m_rotateTool->rotationCenter();
  m_rotateTool->setRotationCenter(center + delta);
}

bool MapViewToolBox::canToggleSweepTool() const
{
  const auto& map = m_document.map();
  return sweepToolActive() || map.selection().hasBrushFaces();
}

void MapViewToolBox::toggleSweepTool()
{
  if (canToggleSweepTool())
  {
    toggleTool(sweepTool());
  }
}

bool MapViewToolBox::sweepToolActive() const
{
  return m_sweepTool->active();
}

void MapViewToolBox::performSweep()
{
  contract_pre(sweepToolActive());

  m_sweepTool->commitSweep();
  deactivateCurrentTool();
}

bool MapViewToolBox::canToggleScaleTool() const
{
  const auto& map = m_document.map();
  return scaleToolActive() || map.selection().hasNodes();
}

void MapViewToolBox::toggleScaleTool()
{
  if (canToggleScaleTool())
  {
    toggleTool(scaleTool());
  }
}

bool MapViewToolBox::scaleToolActive() const
{
  return m_scaleTool->active();
}

bool MapViewToolBox::canToggleShearTool() const
{
  const auto& map = m_document.map();
  return shearToolActive() || map.selection().hasNodes();
}

void MapViewToolBox::toggleShearTool()
{
  if (canToggleShearTool())
  {
    toggleTool(shearTool());
  }
}

bool MapViewToolBox::shearToolActive() const
{
  return m_shearTool->active();
}

bool MapViewToolBox::canToggleAnyVertexTool() const
{
  const auto& map = m_document.map();
  return vertexToolActive() || edgeToolActive() || faceToolActive()
         || map.selection().hasOnlyBrushes();
}

bool MapViewToolBox::anyVertexToolActive() const
{
  return vertexToolActive() || edgeToolActive() || faceToolActive();
}

bool MapViewToolBox::anyNodeHandleToolActive() const
{
  return anyVertexToolActive() || controlPointToolActive();
}

void MapViewToolBox::toggleVertexTool()
{
  if (canToggleAnyVertexTool())
  {
    toggleTool(vertexTool());
  }
}

bool MapViewToolBox::vertexToolActive() const
{
  return m_vertexTool->active();
}

void MapViewToolBox::toggleEdgeTool()
{
  if (canToggleAnyVertexTool())
  {
    toggleTool(edgeTool());
  }
}

bool MapViewToolBox::edgeToolActive() const
{
  return m_edgeTool->active();
}

void MapViewToolBox::toggleFaceTool()
{
  if (canToggleAnyVertexTool())
  {
    toggleTool(faceTool());
  }
}

bool MapViewToolBox::faceToolActive() const
{
  return m_faceTool->active();
}

bool MapViewToolBox::canToggleControlPointTool() const
{
  const auto& map = m_document.map();
  return controlPointToolActive() || map.selection().hasOnlyPatches();
}

void MapViewToolBox::toggleControlPointTool()
{
  if (canToggleControlPointTool())
  {
    toggleTool(controlPointTool());
  }
}

bool MapViewToolBox::controlPointToolActive() const
{
  return m_controlPointTool->active();
}

bool MapViewToolBox::anyModalToolActive() const
{
  return rotateToolActive() || sweepToolActive() || scaleToolActive() || shearToolActive()
         || anyNodeHandleToolActive();
}

void MapViewToolBox::moveNodeHandles(const vm::vec3d& delta)
{
  contract_pre(anyNodeHandleToolActive());

  if (vertexToolActive())
  {
    vertexTool().moveSelection(delta);
  }
  else if (edgeToolActive())
  {
    edgeTool().moveSelection(delta);
  }
  else if (faceToolActive())
  {
    faceTool().moveSelection(delta);
  }
  else if (controlPointToolActive())
  {
    controlPointTool().moveSelection(delta);
  }
}

void MapViewToolBox::createTools(QStackedLayout* bookCtrl)
{
  m_clipTool = std::make_unique<ClipTool>(m_document);
  m_assembleBrushTool = std::make_unique<AssembleBrushTool>(m_document);
  m_createEntityTool = std::make_unique<CreateEntityTool>(m_document);
  m_drawShapeTool = std::make_unique<DrawShapeTool>(m_document);
  m_moveObjectsTool = std::make_unique<MoveObjectsTool>(m_document);
  m_extrudeTool = std::make_unique<ExtrudeTool>(m_document);
  m_rotateTool = std::make_unique<RotateTool>(m_document);
  m_sweepTool = std::make_unique<SweepTool>(m_document);
  m_scaleTool = std::make_unique<ScaleTool>(m_document);
  m_shearTool = std::make_unique<ShearTool>(m_document);
  m_vertexTool = std::make_unique<VertexTool>(m_document);
  m_edgeTool = std::make_unique<EdgeTool>(m_document);
  m_faceTool = std::make_unique<FaceTool>(m_document);
  m_controlPointTool = std::make_unique<ControlPointTool>(m_document);

  addExclusiveToolGroup(
    assembleBrushTool(),
    rotateTool(),
    sweepTool(),
    scaleTool(),
    shearTool(),
    controlPointTool(),
    edgeTool(),
    faceTool(),
    clipTool());

  addExclusiveToolGroup(
    assembleBrushTool(),
    vertexTool(),
    edgeTool(),
    faceTool(),
    controlPointTool(),
    clipTool());

  suppressWhileActive(
    assembleBrushTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(rotateTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(sweepTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(scaleTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(shearTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(vertexTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(edgeTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(faceTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(
    controlPointTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(clipTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());

  registerTool(moveObjectsTool(), bookCtrl);
  registerTool(rotateTool(), bookCtrl);
  registerTool(sweepTool(), bookCtrl);
  registerTool(scaleTool(), bookCtrl);
  registerTool(shearTool(), bookCtrl);
  registerTool(extrudeTool(), bookCtrl);
  registerTool(assembleBrushTool(), bookCtrl);
  registerTool(clipTool(), bookCtrl);
  registerTool(vertexTool(), bookCtrl);
  registerTool(edgeTool(), bookCtrl);
  registerTool(faceTool(), bookCtrl);
  registerTool(controlPointTool(), bookCtrl);
  registerTool(createEntityTool(), bookCtrl);
  registerTool(drawShapeTool(), bookCtrl);

  updateToolPage();
}

void MapViewToolBox::registerTool(Tool& tool, QStackedLayout* bookCtrl)
{
  tool.createPage(bookCtrl);
  addTool(tool);
}

void MapViewToolBox::connectObservers()
{
  m_notifierConnection +=
    toolActivatedNotifier.connect(this, &MapViewToolBox::toolActivated);
  m_notifierConnection +=
    toolDeactivatedNotifier.connect(this, &MapViewToolBox::toolDeactivated);

  m_notifierConnection += m_document.documentWasLoadedNotifier.connect(
    this, &MapViewToolBox::documentWasLoaded);

  m_notifierConnection += m_document.selectionDidChangeNotifier.connect(
    this, &MapViewToolBox::selectionDidChange);
}

void MapViewToolBox::toolActivated(Tool&)
{
  updateEditorContext();
  updateToolPage();
}

void MapViewToolBox::toolDeactivated(Tool&)
{
  updateEditorContext();
  updateToolPage();
}

void MapViewToolBox::updateEditorContext()
{
  auto& editorContext = m_document.map().editorContext();
  editorContext.setBlockSelection(assembleBrushToolActive());
}

void MapViewToolBox::documentWasLoaded()
{
  deactivateAllTools();
}

void MapViewToolBox::selectionDidChange(const mdl::SelectionChange&)
{
  updateToolPage();
}

void MapViewToolBox::updateToolPage()
{
  if (rotateToolActive())
  {
    rotateTool().showPage();
  }
  else if (sweepToolActive())
  {
    sweepTool().showPage();
  }
  else if (scaleToolActive())
  {
    scaleTool().showPage();
  }
  else if (shearToolActive())
  {
    shearTool().showPage();
  }
  else if (vertexToolActive())
  {
    vertexTool().showPage();
  }
  else if (edgeToolActive())
  {
    edgeTool().showPage();
  }
  else if (faceToolActive())
  {
    faceTool().showPage();
  }
  else if (controlPointToolActive())
  {
    controlPointTool().showPage();
  }
  else if (clipToolActive())
  {
    clipTool().showPage();
  }
  else
  {
    drawShapeTool().showPage();
  }
}

} // namespace tb::ui
