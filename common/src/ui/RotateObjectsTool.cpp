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

#include "RotateObjectsTool.h"

#include "mdl/Hit.h"
#include "ui/Grid.h"
#include "ui/MapDocument.h"
#include "ui/RotateObjectsHandle.h"
#include "ui/RotateObjectsToolPage.h"
#include "ui/TransactionScope.h"

#include "kdl/memory_utils.h"

namespace tb::ui
{

RotateObjectsTool::RotateObjectsTool(std::weak_ptr<MapDocument> document)
  : Tool{false}
  , m_document{std::move(document)}
{
}

bool RotateObjectsTool::doActivate()
{
  resetRotationCenter();
  return true;
}

const Grid& RotateObjectsTool::grid() const
{
  return kdl::mem_lock(m_document)->grid();
}

void RotateObjectsTool::updateToolPageAxis(const RotateObjectsHandle::HitArea area)
{
  handleHitAreaDidChangeNotifier(area);
}

double RotateObjectsTool::angle() const
{
  return m_angle;
}

void RotateObjectsTool::setAngle(const double angle)
{
  m_angle = angle;
}

vm::vec3d RotateObjectsTool::rotationCenter() const
{
  return m_handle.position();
}

void RotateObjectsTool::setRotationCenter(const vm::vec3d& position)
{
  m_handle.setPosition(position);
  rotationCenterDidChangeNotifier(position);
  refreshViews();
}

void RotateObjectsTool::resetRotationCenter()
{
  auto document = kdl::mem_lock(m_document);
  const auto& bounds = document->selectionBounds();
  const auto position = document->grid().snap(bounds.center());
  setRotationCenter(position);
}

double RotateObjectsTool::majorHandleRadius(const render::Camera& camera) const
{
  return m_handle.majorHandleRadius(camera);
}

double RotateObjectsTool::minorHandleRadius(const render::Camera& camera) const
{
  return m_handle.minorHandleRadius(camera);
}

void RotateObjectsTool::beginRotation()
{
  auto document = kdl::mem_lock(m_document);
  document->startTransaction("Rotate Objects", TransactionScope::LongRunning);
}

void RotateObjectsTool::commitRotation()
{
  auto document = kdl::mem_lock(m_document);
  document->commitTransaction();
  rotationCenterWasUsedNotifier(rotationCenter());
}

void RotateObjectsTool::cancelRotation()
{
  auto document = kdl::mem_lock(m_document);
  document->cancelTransaction();
}

double RotateObjectsTool::snapRotationAngle(const double angle) const
{
  auto document = kdl::mem_lock(m_document);
  return document->grid().snapAngle(angle);
}

void RotateObjectsTool::applyRotation(
  const vm::vec3d& center, const vm::vec3d& axis, const double angle)
{
  auto document = kdl::mem_lock(m_document);
  document->rollbackTransaction();
  document->rotateObjects(center, axis, angle);
}

mdl::Hit RotateObjectsTool::pick2D(const vm::ray3d& pickRay, const render::Camera& camera)
{
  return m_handle.pick2D(pickRay, camera);
}

mdl::Hit RotateObjectsTool::pick3D(const vm::ray3d& pickRay, const render::Camera& camera)
{
  return m_handle.pick3D(pickRay, camera);
}

vm::vec3d RotateObjectsTool::rotationAxis(const RotateObjectsHandle::HitArea area) const
{
  return m_handle.rotationAxis(area);
}

void RotateObjectsTool::renderHandle2D(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_handle.renderHandle2D(renderContext, renderBatch);
}

void RotateObjectsTool::renderHandle3D(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_handle.renderHandle3D(renderContext, renderBatch);
}

void RotateObjectsTool::renderHighlight2D(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const RotateObjectsHandle::HitArea area)
{
  m_handle.renderHighlight2D(renderContext, renderBatch, area);
}
void RotateObjectsTool::renderHighlight3D(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const RotateObjectsHandle::HitArea area)
{
  m_handle.renderHighlight3D(renderContext, renderBatch, area);
}

QWidget* RotateObjectsTool::doCreatePage(QWidget* parent)
{
  return new RotateObjectsToolPage{m_document, *this, parent};
}

} // namespace tb::ui
