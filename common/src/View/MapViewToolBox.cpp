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

#include "MapViewToolBox.h"

#include "Model/EditorContext.h"
#include "View/AssembleBrushTool.h"
#include "View/ClipTool.h"
#include "View/CreateEntityTool.h"
#include "View/DrawShapeTool.h"
#include "View/EdgeTool.h"
#include "View/ExtrudeTool.h"
#include "View/FaceTool.h"
#include "View/MapDocument.h"
#include "View/MoveObjectsTool.h"
#include "View/RotateObjectsTool.h"
#include "View/ScaleObjectsTool.h"
#include "View/ShearObjectsTool.h"
#include "View/VertexTool.h"

namespace TrenchBroom
{
namespace View
{
MapViewToolBox::MapViewToolBox(
  std::weak_ptr<MapDocument> document, QStackedLayout* bookCtrl)
  : m_document(document)
{
  createTools(document, bookCtrl);
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

RotateObjectsTool& MapViewToolBox::rotateObjectsTool()
{
  return *m_rotateObjectsTool;
}

ScaleObjectsTool& MapViewToolBox::scaleObjectsTool()
{
  return *m_scaleObjectsTool;
}

ShearObjectsTool& MapViewToolBox::shearObjectsTool()
{
  return *m_shearObjectsTool;
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
  assert(clipToolActive());
  m_clipTool->toggleSide();
}

void MapViewToolBox::performClip()
{
  assert(clipToolActive());
  m_clipTool->performClip();
}

void MapViewToolBox::removeLastClipPoint()
{
  assert(clipToolActive());
  m_clipTool->removeLastPoint();
}

void MapViewToolBox::toggleRotateObjectsTool()
{
  toggleTool(rotateObjectsTool());
}

bool MapViewToolBox::rotateObjectsToolActive() const
{
  return m_rotateObjectsTool->active();
}

double MapViewToolBox::rotateToolAngle() const
{
  assert(rotateObjectsToolActive());
  return m_rotateObjectsTool->angle();
}

vm::vec3 MapViewToolBox::rotateToolCenter() const
{
  assert(rotateObjectsToolActive());
  return m_rotateObjectsTool->rotationCenter();
}

void MapViewToolBox::moveRotationCenter(const vm::vec3& delta)
{
  assert(rotateObjectsToolActive());
  const vm::vec3 center = m_rotateObjectsTool->rotationCenter();
  m_rotateObjectsTool->setRotationCenter(center + delta);
}

void MapViewToolBox::toggleScaleObjectsTool()
{
  toggleTool(scaleObjectsTool());
}

bool MapViewToolBox::scaleObjectsToolActive() const
{
  return m_scaleObjectsTool->active();
}

void MapViewToolBox::toggleShearObjectsTool()
{
  toggleTool(shearObjectsTool());
}

bool MapViewToolBox::shearObjectsToolActive() const
{
  return m_shearObjectsTool->active();
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

void MapViewToolBox::moveVertices(const vm::vec3& delta)
{
  assert(anyVertexToolActive());
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

void MapViewToolBox::createTools(
  std::weak_ptr<MapDocument> document, QStackedLayout* bookCtrl)
{
  m_clipTool = std::make_unique<ClipTool>(document);
  m_assembleBrushTool = std::make_unique<AssembleBrushTool>(document);
  m_createEntityTool = std::make_unique<CreateEntityTool>(document);
  m_drawShapeTool = std::make_unique<DrawShapeTool>(document);
  m_moveObjectsTool = std::make_unique<MoveObjectsTool>(document);
  m_extrudeTool = std::make_unique<ExtrudeTool>(document);
  m_rotateObjectsTool = std::make_unique<RotateObjectsTool>(document);
  m_scaleObjectsTool = std::make_unique<ScaleObjectsTool>(document);
  m_shearObjectsTool = std::make_unique<ShearObjectsTool>(document);
  m_vertexTool = std::make_unique<VertexTool>(document);
  m_edgeTool = std::make_unique<EdgeTool>(document);
  m_faceTool = std::make_unique<FaceTool>(document);

  suppressWhileActive(moveObjectsTool(), assembleBrushTool());
  suppressWhileActive(extrudeTool(), assembleBrushTool());
  suppressWhileActive(drawShapeTool(), assembleBrushTool());
  suppressWhileActive(moveObjectsTool(), rotateObjectsTool());
  suppressWhileActive(extrudeTool(), rotateObjectsTool());
  suppressWhileActive(drawShapeTool(), rotateObjectsTool());
  suppressWhileActive(moveObjectsTool(), scaleObjectsTool());
  suppressWhileActive(extrudeTool(), scaleObjectsTool());
  suppressWhileActive(drawShapeTool(), scaleObjectsTool());
  suppressWhileActive(moveObjectsTool(), shearObjectsTool());
  suppressWhileActive(extrudeTool(), shearObjectsTool());
  suppressWhileActive(drawShapeTool(), shearObjectsTool());
  suppressWhileActive(moveObjectsTool(), vertexTool());
  suppressWhileActive(extrudeTool(), vertexTool());
  suppressWhileActive(drawShapeTool(), vertexTool());
  suppressWhileActive(moveObjectsTool(), edgeTool());
  suppressWhileActive(extrudeTool(), edgeTool());
  suppressWhileActive(drawShapeTool(), edgeTool());
  suppressWhileActive(moveObjectsTool(), faceTool());
  suppressWhileActive(extrudeTool(), faceTool());
  suppressWhileActive(drawShapeTool(), faceTool());
  suppressWhileActive(moveObjectsTool(), clipTool());
  suppressWhileActive(extrudeTool(), clipTool());
  suppressWhileActive(drawShapeTool(), clipTool());

  registerTool(moveObjectsTool(), bookCtrl);
  registerTool(rotateObjectsTool(), bookCtrl);
  registerTool(scaleObjectsTool(), bookCtrl);
  registerTool(shearObjectsTool(), bookCtrl);
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

  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->documentWasNewedNotifier.connect(
    this, &MapViewToolBox::documentWasNewedOrLoaded);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &MapViewToolBox::documentWasNewedOrLoaded);
  m_notifierConnection += document->selectionDidChangeNotifier.connect(
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
  auto document = kdl::mem_lock(m_document);
  Model::EditorContext& editorContext = document->editorContext();
  editorContext.setBlockSelection(assembleBrushToolActive());
}

void MapViewToolBox::documentWasNewedOrLoaded(MapDocument*)
{
  deactivateAllTools();
}

void MapViewToolBox::selectionDidChange(const Selection&)
{
  updateToolPage();
}

void MapViewToolBox::updateToolPage()
{
  auto document = kdl::mem_lock(m_document);
  if (auto* activeTool = this->activeTool())
  {
    activeTool->showPage();
  }
  else if (document->hasSelection())
  {
    moveObjectsTool().showPage();
  }
  else
  {
    drawShapeTool().showPage();
  }
}

} // namespace View
} // namespace TrenchBroom
