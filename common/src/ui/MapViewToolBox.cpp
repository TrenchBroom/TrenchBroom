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

#include "MapViewToolBox.h"

#include "mdl/EditorContext.h"
#include "ui/AssembleBrushTool.h"
#include "ui/ClipTool.h"
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

AssembleBrushTool& MapViewToolBox::assembleBrushTool()
{
  return *m_assembleBrushTool;
}

CreateEntityTool& MapViewToolBox::createEntityTool()
{
  return *m_createEntityTool;
}

DrawShapeTool& MapViewToolBox::drawShapeTool()
{
  return *m_drawShapeTool;
}

MoveObjectsTool& MapViewToolBox::moveObjectsTool()
{
  return *m_moveObjectsTool;
}

ExtrudeTool& MapViewToolBox::extrudeTool()
{
  return *m_extrudeTool;
}

RotateTool& MapViewToolBox::rotateTool()
{
  return *m_rotateTool;
}

ScaleTool& MapViewToolBox::scaleTool()
{
  return *m_scaleTool;
}

ShearTool& MapViewToolBox::shearTool()
{
  return *m_shearTool;
}

VertexTool& MapViewToolBox::vertexTool()
{
  return *m_vertexTool;
}

EdgeTool& MapViewToolBox::edgeTool()
{
  return *m_edgeTool;
}

FaceTool& MapViewToolBox::faceTool()
{
  return *m_faceTool;
}

void MapViewToolBox::toggleAssembleBrushTool()
{
  toggleTool(assembleBrushTool());
}

bool MapViewToolBox::assembleBrushToolActive() const
{
  return m_assembleBrushTool->active();
}

void MapViewToolBox::performAssembleBrush()
{
  m_assembleBrushTool->createBrushes();
}

void MapViewToolBox::toggleClipTool()
{
  toggleTool(clipTool());
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

void MapViewToolBox::toggleRotateTool()
{
  toggleTool(rotateTool());
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

void MapViewToolBox::toggleScaleTool()
{
  toggleTool(scaleTool());
}

bool MapViewToolBox::scaleToolActive() const
{
  return m_scaleTool->active();
}

void MapViewToolBox::toggleShearTool()
{
  toggleTool(shearTool());
}

bool MapViewToolBox::shearToolActive() const
{
  return m_shearTool->active();
}

bool MapViewToolBox::anyVertexToolActive() const
{
  return vertexToolActive() || edgeToolActive() || faceToolActive();
}

void MapViewToolBox::toggleVertexTool()
{
  toggleTool(vertexTool());
}

bool MapViewToolBox::vertexToolActive() const
{
  return m_vertexTool->active();
}

void MapViewToolBox::toggleEdgeTool()
{
  toggleTool(edgeTool());
}

bool MapViewToolBox::edgeToolActive() const
{
  return m_edgeTool->active();
}

void MapViewToolBox::toggleFaceTool()
{
  toggleTool(faceTool());
}

bool MapViewToolBox::faceToolActive() const
{
  return m_faceTool->active();
}

bool MapViewToolBox::anyModalToolActive() const
{
  return rotateToolActive() || scaleToolActive() || shearToolActive()
         || anyVertexToolActive();
}

void MapViewToolBox::moveVertices(const vm::vec3d& delta)
{
  contract_pre(anyVertexToolActive());

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
  m_scaleTool = std::make_unique<ScaleTool>(m_document);
  m_shearTool = std::make_unique<ShearTool>(m_document);
  m_vertexTool = std::make_unique<VertexTool>(m_document);
  m_edgeTool = std::make_unique<EdgeTool>(m_document);
  m_faceTool = std::make_unique<FaceTool>(m_document);

  addExclusiveToolGroup(
    assembleBrushTool(),
    rotateTool(),
    scaleTool(),
    shearTool(),
    edgeTool(),
    faceTool(),
    clipTool());

  addExclusiveToolGroup(
    assembleBrushTool(), vertexTool(), edgeTool(), faceTool(), clipTool());

  suppressWhileActive(
    assembleBrushTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(rotateTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(scaleTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(shearTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(vertexTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(edgeTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(faceTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());
  suppressWhileActive(clipTool(), moveObjectsTool(), extrudeTool(), drawShapeTool());

  registerTool(moveObjectsTool(), bookCtrl);
  registerTool(rotateTool(), bookCtrl);
  registerTool(scaleTool(), bookCtrl);
  registerTool(shearTool(), bookCtrl);
  registerTool(extrudeTool(), bookCtrl);
  registerTool(assembleBrushTool(), bookCtrl);
  registerTool(clipTool(), bookCtrl);
  registerTool(vertexTool(), bookCtrl);
  registerTool(edgeTool(), bookCtrl);
  registerTool(faceTool(), bookCtrl);
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

  m_notifierConnection += m_document.documentWasCreatedNotifier.connect(
    this, &MapViewToolBox::documentWasCreated);
  m_notifierConnection += m_document.documentWasLoadedNotifier.connect(
    this, &MapViewToolBox::documentWasLoaded);
  m_notifierConnection += m_document.documentWasClearedNotifier.connect(
    this, &MapViewToolBox::documentWasCleared);

  m_notifierConnection += m_document.map().selectionDidChangeNotifier.connect(
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

void MapViewToolBox::documentWasCreated()
{
  deactivateAllTools();
}

void MapViewToolBox::documentWasLoaded()
{
  deactivateAllTools();
}

void MapViewToolBox::documentWasCleared()
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
