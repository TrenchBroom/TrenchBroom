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

#include "ui/RotateTool.h"

#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/TransactionScope.h"
#include "ui/MapDocument.h"
#include "ui/RotateHandle.h"
#include "ui/RotateToolPage.h"

namespace tb::ui
{
namespace
{

class RotateRingDragTracker : public RingDragTracker
{
private:
  RotateTool& m_tool;

public:
  explicit RotateRingDragTracker(RotateTool& tool)
    : m_tool{tool}
  {
    m_tool.beginRotation();
  }

  void apply(const vm::vec3d& center, const vm::vec3d& axis, const double angle) override
  {
    m_tool.applyRotation(center, axis, angle);
  }

  void end() override { m_tool.commitRotation(); }

  void cancel() override { m_tool.cancelRotation(); }
};

} // namespace

RotateTool::RotateTool(MapDocument& document)
  : Tool{false}
  , m_document{document}
{
}

bool RotateTool::doActivate()
{
  resetRotationCenter();
  return true;
}

void RotateTool::updateToolPageAxis(const RotateHandle::HitArea area)
{
  handleHitAreaDidChangeNotifier(area);
}

double RotateTool::angle() const
{
  return m_angle;
}

void RotateTool::setAngle(const double angle)
{
  m_angle = angle;
}

vm::vec3d RotateTool::rotationCenter() const
{
  return m_handle.position();
}

void RotateTool::setRotationCenter(const vm::vec3d& position)
{
  m_handle.setPosition(position);
  rotationCenterDidChangeNotifier(position);
  refreshViews();
}

void RotateTool::resetRotationCenter()
{
  auto& map = m_document.map();

  const auto& selection = map.selection();
  if (selection.hasOnlyEntities() && selection.entities.size() == 1)
  {
    const auto& entityNode = *selection.entities.front();
    setRotationCenter(entityNode.entity().origin());
  }
  else if (const auto& bounds = map.selectionBounds())
  {
    const auto position = map.grid().snap(bounds->center());
    setRotationCenter(position);
  }
}

double RotateTool::minorHandleRadius(const gl::Camera& camera) const
{
  return m_handle.minorHandleRadius(camera);
}

void RotateTool::beginRotation()
{
  m_document.map().startTransaction("Rotate Objects", mdl::TransactionScope::LongRunning);
}

void RotateTool::commitRotation()
{
  m_document.map().commitTransaction();
  rotationCenterWasUsedNotifier(rotationCenter());
}

void RotateTool::cancelRotation()
{
  m_document.map().cancelTransaction();
}

double RotateTool::snapRotationAngle(const double angle) const
{
  return m_document.map().grid().snapAngle(angle);
}

void RotateTool::applyRotation(
  const vm::vec3d& center, const vm::vec3d& axis, const double angle)
{
  auto& map = m_document.map();
  map.rollbackTransaction();
  rotateSelection(map, center, axis, angle);
}

QWidget* RotateTool::doCreatePage(QWidget* parent)
{
  return new RotateToolPage{m_document, *this, parent};
}

Tool& RotateTool::tool()
{
  return *this;
}

const Tool& RotateTool::tool() const
{
  return *this;
}

const mdl::Grid& RotateTool::grid() const
{
  return m_document.map().grid();
}

RotateHandle& RotateTool::handle()
{
  return m_handle;
}

double RotateTool::handleSnapAngle() const
{
  return angle();
}

void RotateTool::handleClicked(const RotateHandle::HitArea area)
{
  updateToolPageAxis(area);
}

vm::vec3d RotateTool::handleCenter() const
{
  return rotationCenter();
}

void RotateTool::setHandleCenter(const vm::vec3d& position)
{
  setRotationCenter(position);
}

std::unique_ptr<RingDragTracker> RotateTool::beginRingDrag()
{
  return std::make_unique<RotateRingDragTracker>(*this);
}

} // namespace tb::ui
