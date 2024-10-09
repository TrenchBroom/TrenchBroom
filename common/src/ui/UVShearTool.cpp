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

#include "UVShearTool.h"

#include "mdl/BrushFace.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/TransactionScope.h"
#include "ui/UVViewHelper.h"

#include "kdl/memory_utils.h"
#include "kdl/optional_utils.h"

#include "vm/intersection.h"
#include "vm/vec.h"

namespace tb::ui
{
namespace
{

std::optional<vm::vec2f> getHit(
  const UVViewHelper& helper,
  const vm::vec3d& xAxis,
  const vm::vec3d& yAxis,
  const vm::ray3d& pickRay)
{
  const auto& boundary = helper.face()->boundary();
  return vm::intersect_ray_plane(pickRay, boundary)
         | kdl::optional_transform([&](const auto distance) {
             const auto hitPoint = vm::point_at_distance(pickRay, distance);
             const auto hitVec = hitPoint - helper.origin();
             return vm::vec2f{
               float(vm::dot(hitVec, xAxis)), float(vm::dot(hitVec, yAxis))};
           });
}

class UVShearDragTracker : public GestureTracker
{
private:
  MapDocument& m_document;
  const UVViewHelper& m_helper;
  vm::vec2b m_selector;
  vm::vec3d m_xAxis;
  vm::vec3d m_yAxis;
  vm::vec2f m_initialHit;
  vm::vec2f m_lastHit;

public:
  UVShearDragTracker(
    MapDocument& document,
    const UVViewHelper& helper,
    const vm::vec2b& selector,
    const vm::vec3d& xAxis,
    const vm::vec3d& yAxis,
    const vm::vec2f& initialHit)
    : m_document{document}
    , m_helper{helper}
    , m_selector{selector}
    , m_xAxis{xAxis}
    , m_yAxis{yAxis}
    , m_initialHit{initialHit}
    , m_lastHit{initialHit}
  {
    m_document.startTransaction("Shear UV", TransactionScope::LongRunning);
  }

  bool update(const InputState& inputState) override
  {
    const auto currentHit = getHit(m_helper, m_xAxis, m_yAxis, inputState.pickRay());
    if (!currentHit)
    {
      return false;
    }

    const auto delta = *currentHit - m_lastHit;

    const auto origin = m_helper.origin();
    const auto oldCoords = vm::vec2f{
      m_helper.face()->toUVCoordSystemMatrix(
        vm::vec2f{0, 0}, m_helper.face()->attributes().scale(), true)
      * origin};

    if (m_selector[0])
    {
      const auto factors = vm::vec2f{-delta.y() / m_initialHit.x(), 0.0f};
      if (!vm::is_zero(factors, vm::Cf::almost_zero()))
      {
        m_document.shearUV(factors);
      }
    }
    else if (m_selector[1])
    {
      const auto factors = vm::vec2f{0.0f, -delta.x() / m_initialHit.y()};
      if (!vm::is_zero(factors, vm::Cf::almost_zero()))
      {
        m_document.shearUV(factors);
      }
    }

    const auto newCoords = vm::vec2f{
      m_helper.face()->toUVCoordSystemMatrix(
        vm::vec2f{0, 0}, m_helper.face()->attributes().scale(), true)
      * origin};
    const auto newOffset = m_helper.face()->attributes().offset() + oldCoords - newCoords;

    auto request = mdl::ChangeBrushFaceAttributesRequest{};
    request.setOffset(newOffset);
    m_document.setFaceAttributes(request);

    m_lastHit = *currentHit;
    return true;
  }

  void end(const InputState&) override { m_document.commitTransaction(); }

  void cancel() override { m_document.cancelTransaction(); }
};

} // namespace

const mdl::HitType::Type UVShearTool::XHandleHitType = mdl::HitType::freeType();
const mdl::HitType::Type UVShearTool::YHandleHitType = mdl::HitType::freeType();

UVShearTool::UVShearTool(std::weak_ptr<MapDocument> document, UVViewHelper& helper)
  : ToolController{}
  , Tool{true}
  , m_document{std::move(document)}
  , m_helper{helper}
{
}

Tool& UVShearTool::tool()
{
  return *this;
}

const Tool& UVShearTool::tool() const
{
  return *this;
}

void UVShearTool::pick(const InputState& inputState, mdl::PickResult& pickResult)
{
  static const mdl::HitType::Type HitTypes[] = {XHandleHitType, YHandleHitType};
  if (m_helper.valid())
  {
    m_helper.pickUVGrid(inputState.pickRay(), HitTypes, pickResult);
  }
}

std::unique_ptr<GestureTracker> UVShearTool::acceptMouseDrag(const InputState& inputState)
{
  using namespace mdl::HitFilters;

  assert(m_helper.valid());

  if (
    !inputState.modifierKeysPressed(ModifierKeys::MKAlt)
    || !inputState.mouseButtonsPressed(MouseButtons::Left))
  {
    return nullptr;
  }

  if (!m_helper.face()->attributes().valid())
  {
    return nullptr;
  }

  const auto& xHit = inputState.pickResult().first(type(XHandleHitType));
  const auto& yHit = inputState.pickResult().first(type(YHandleHitType));

  if (!(xHit.isMatch() ^ yHit.isMatch()))
  {
    return nullptr;
  }

  const auto selector = vm::vec2b{xHit.isMatch(), yHit.isMatch()};

  const auto xAxis = m_helper.face()->uAxis();
  const auto yAxis = m_helper.face()->vAxis();
  const auto initialHit = getHit(m_helper, xAxis, yAxis, inputState.pickRay());
  if (!initialHit)
  {
    return nullptr;
  }

  // #1350: Don't allow shearing if the shear would result in very large changes. This
  // happens if the shear handle to be dragged is very close to one of the UV axes.
  if (vm::is_zero(initialHit->x(), 6.0f) || vm::is_zero(initialHit->y(), 6.0f))
  {
    return nullptr;
  }

  return std::make_unique<UVShearDragTracker>(
    *kdl::mem_lock(m_document), m_helper, selector, xAxis, yAxis, *initialHit);
}

bool UVShearTool::cancel()
{
  return false;
}

} // namespace tb::ui
