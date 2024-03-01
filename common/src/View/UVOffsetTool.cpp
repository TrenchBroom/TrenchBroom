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

#include "UVOffsetTool.h"

#include "Ensure.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Polyhedron.h"
#include "View/DragTracker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/TransactionScope.h"
#include "View/UVView.h"

#include "kdl/memory_utils.h"
#include "kdl/optional_utils.h"

#include "vm/forward.h"
#include "vm/intersection.h"
#include "vm/mat.h"
#include "vm/vec.h"

#include <cassert>

namespace TrenchBroom::View
{

namespace
{

vm::vec2f computeHitPoint(const UVViewHelper& helper, const vm::ray3& ray)
{
  const auto& boundary = helper.face()->boundary();
  return *kdl::optional_transform(
    vm::intersect_ray_plane(ray, boundary), [&](const auto distance) {
      const auto hitPoint = vm::point_at_distance(ray, distance);
      const auto transform = helper.face()->toTexCoordSystemMatrix(
        vm::vec2f::zero(), helper.face()->attributes().scale(), true);
      return vm::vec2f{transform * hitPoint};
    });
}

vm::vec2f snapDelta(const UVViewHelper& helper, const vm::vec2f& delta)
{
  assert(helper.valid());

  if (helper.texture())
  {
    const auto transform = helper.face()->toTexCoordSystemMatrix(
      helper.face()->attributes().offset() - delta,
      helper.face()->attributes().scale(),
      true);

    auto distance = vm::vec2f::max();
    for (const auto* vertex : helper.face()->vertices())
    {
      const auto temp =
        helper.computeDistanceFromTextureGrid(transform * vertex->position());
      distance = vm::abs_min(distance, temp);
    }

    return helper.snapDelta(delta, -distance);
  }

  return vm::round(delta);
}

class UVOffsetDragTracker : public DragTracker
{
private:
  MapDocument& m_document;
  const UVViewHelper& m_helper;
  vm::vec2f m_lastPoint;

public:
  UVOffsetDragTracker(
    MapDocument& document, const UVViewHelper& helper, const InputState& inputState)
    : m_document{document}
    , m_helper{helper}
    , m_lastPoint{computeHitPoint(m_helper, inputState.pickRay())}
  {
    m_document.startTransaction("Move Texture", TransactionScope::LongRunning);
  }

  bool drag(const InputState& inputState) override
  {
    assert(m_helper.valid());

    const auto curPoint = computeHitPoint(m_helper, inputState.pickRay());
    const auto delta = curPoint - m_lastPoint;
    const auto snapped = snapDelta(m_helper, delta);

    const auto corrected =
      vm::correct(m_helper.face()->attributes().offset() - snapped, 4, 0.0f);

    if (corrected == m_helper.face()->attributes().offset())
    {
      return true;
    }

    auto request = Model::ChangeBrushFaceAttributesRequest{};
    request.setOffset(corrected);

    m_document.setFaceAttributes(request);

    m_lastPoint = m_lastPoint + snapped;
    return true;
  }

  void end(const InputState&) override { m_document.commitTransaction(); }

  void cancel() override { m_document.cancelTransaction(); }
};

} // namespace

UVOffsetTool::UVOffsetTool(
  std::weak_ptr<MapDocument> document, const UVViewHelper& helper)
  : ToolController{}
  , Tool{true}
  , m_document{std::move(document)}
  , m_helper{helper}
{
}

Tool& UVOffsetTool::tool()
{
  return *this;
}

const Tool& UVOffsetTool::tool() const
{
  return *this;
}

std::unique_ptr<DragTracker> UVOffsetTool::acceptMouseDrag(const InputState& inputState)
{
  assert(m_helper.valid());

  if (
    !inputState.modifierKeysPressed(ModifierKeys::MKNone)
    || !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
  {
    return nullptr;
  }

  return std::make_unique<UVOffsetDragTracker>(
    *kdl::mem_lock(m_document), m_helper, inputState);
}

bool UVOffsetTool::cancel()
{
  return false;
}

} // namespace TrenchBroom::View
