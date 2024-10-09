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

#include "UVOffsetTool.h"

#include "mdl/BrushFace.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/TransactionScope.h"
#include "ui/UVView.h"

#include "kdl/memory_utils.h"
#include "kdl/optional_utils.h"
#include "kdl/range_fold.h"

#include "vm/intersection.h"
#include "vm/vec.h"

#include <cassert>
#include <ranges>

namespace tb::ui
{
namespace
{

vm::vec2f computeHitPoint(const UVViewHelper& helper, const vm::ray3d& ray)
{
  const auto& boundary = helper.face()->boundary();
  return *kdl::optional_transform(
    vm::intersect_ray_plane(ray, boundary), [&](const auto distance) {
      const auto hitPoint = vm::point_at_distance(ray, distance);
      const auto transform = helper.face()->toUVCoordSystemMatrix(
        vm::vec2f{0, 0}, helper.face()->attributes().scale(), true);
      return vm::vec2f{transform * hitPoint};
    });
}

vm::vec2f snapDelta(const UVViewHelper& helper, const vm::vec2f& delta)
{
  assert(helper.valid());

  if (helper.material())
  {
    const auto transform = helper.face()->toUVCoordSystemMatrix(
      helper.face()->attributes().offset() - delta,
      helper.face()->attributes().scale(),
      true);

    const auto distance = kdl::fold_left_first(
      helper.face()->vertices() | std::views::transform([&](const auto& vertex) {
        return helper.computeDistanceFromUVGrid(transform * vertex->position());
      }),
      [&](const auto lhs, const auto rhs) { return vm::abs_min(lhs, rhs); });

    return helper.snapDelta(delta, -distance);
  }

  return vm::round(delta);
}

class UVOffsetDragTracker : public GestureTracker
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
    m_document.startTransaction("Move UV", TransactionScope::LongRunning);
  }

  bool update(const InputState& inputState) override
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

    auto request = mdl::ChangeBrushFaceAttributesRequest{};
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

std::unique_ptr<GestureTracker> UVOffsetTool::acceptMouseDrag(
  const InputState& inputState)
{
  assert(m_helper.valid());

  if (
    !inputState.modifierKeysPressed(ModifierKeys::None)
    || !inputState.mouseButtonsPressed(MouseButtons::Left))
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

} // namespace tb::ui
