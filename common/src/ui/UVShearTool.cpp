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
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/PickResult.h"
#include "mdl/TransactionScope.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/UVViewHelper.h"

#include "kdl/optional_utils.h"
#include "kdl/ranges/to.h"

#include "vm/intersection.h"
#include "vm/vec.h"

#include <ranges>

namespace tb::ui
{
namespace
{

std::optional<vm::vec2f> getHit(
  const UVViewHelper& helper,
  const vm::vec3d& uAxis,
  const vm::vec3d& vAxis,
  const vm::ray3d& pickRay)
{
  const auto& boundary = helper.face()->boundary();
  return vm::intersect_ray_plane(pickRay, boundary)
         | kdl::optional_transform([&](const auto distance) {
             const auto hitPoint = vm::point_at_distance(pickRay, distance);
             const auto hitVec = hitPoint - helper.origin();
             return vm::vec2f{
               float(vm::dot(hitVec, uAxis)), float(vm::dot(hitVec, vAxis))};
           });
}

/**
 * Return the face's edge vectors in UV coordinates.
 */
std::vector<vm::vec2f> getEdgeVectorsUV(const UVViewHelper& helper)
{
  if (const auto* face = helper.face())
  {
    const auto toUV =
      helper.face()->toUVCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{0, 0}, true);
    return face->edges() | std::views::transform([&](const auto* edge) {
             const auto& segment3d = edge->segment();
             return vm::vec2f{toUV * segment3d.end()}
                    - vm::vec2f{toUV * segment3d.start()};
           })
           | kdl::ranges::to<std::vector>();
  }
  return {};
}

std::vector<float> getSnappedShearFactors(
  const std::vector<vm::vec2f> edgeVectors, const vm::axis::type axis)
{
  return edgeVectors | std::views::filter([&](const auto& v) {
           return !vm::is_zero(v[axis], vm::Cf::almost_zero());
         })
         | std::views::transform([&](const auto& v) { return -v[1 - axis] / v[axis]; })
         | kdl::ranges::to<std::vector>();
}

float snapShearFactors(
  const float factor,
  const float orthogonalOffset,
  const vm::axis::type axis,
  const UVViewHelper& helper)
{
  const auto edgeVectors = getEdgeVectorsUV(helper);
  const auto snappedFactors = getSnappedShearFactors(edgeVectors, axis);

  const auto absDiff = [&](const auto& x) { return vm::abs(x - factor); };

  const auto it = std::ranges::min_element(
    snappedFactors,
    [&](const auto& lhs, const auto& rhs) { return absDiff(lhs) < absDiff(rhs); });

  const auto threshold = 10.0f / vm::abs(orthogonalOffset) / helper.cameraZoom();
  return it != snappedFactors.end() && absDiff(*it) < threshold ? *it : factor;
}

vm::vec2f snapShearFactors(
  const vm::vec2f& factors, const vm::vec2f& offset, const UVViewHelper& helper)
{
  return vm::vec2f{
    snapShearFactors(factors.x(), offset.x(), vm::axis::x, helper),
    snapShearFactors(factors.y(), offset.y(), vm::axis::y, helper),
  };
}

vm::vec2f selectShearFactors(const vm::vec2f& factors, const vm::vec2b& selector)
{
  return vm::vec2f{
    selector.x() ? factors.x() : 0.0f,
    selector.y() ? factors.y() : 0.0f,
  };
}

class UVShearDragTracker : public GestureTracker
{
private:
  mdl::Map& m_map;
  const UVViewHelper& m_helper;
  vm::vec2b m_selector;
  vm::vec3d m_uAxis;
  vm::vec3d m_vAxis;
  vm::vec2f m_initialHit;

public:
  UVShearDragTracker(
    mdl::Map& map,
    const UVViewHelper& helper,
    const vm::vec2b& selector,
    const vm::vec3d& uAxis,
    const vm::vec3d& vAxis,
    const vm::vec2f& initialHit)
    : m_map{map}
    , m_helper{helper}
    , m_selector{selector}
    , m_uAxis{uAxis}
    , m_vAxis{vAxis}
    , m_initialHit{initialHit}
  {
    m_map.startTransaction("Shear UV", mdl::TransactionScope::LongRunning);
  }

  bool update(const InputState& inputState) override
  {
    m_map.rollbackTransaction();

    const auto currentHit = getHit(m_helper, m_uAxis, m_vAxis, inputState.pickRay());
    if (!currentHit)
    {
      return false;
    }

    const auto delta = *currentHit - m_initialHit;

    const auto factors = vm::vec2f{
      -delta.y() / m_initialHit.x(),
      -delta.x() / m_initialHit.y(),
    };

    const auto snappedFactors = selectShearFactors(
      !inputState.modifierKeysDown(ModifierKeys::CtrlCmd)
        ? snapShearFactors(factors, *currentHit, m_helper)
        : factors,
      m_selector);

    if (!vm::is_zero(snappedFactors, vm::Cf::almost_zero()))
    {
      const auto origin = m_helper.origin();
      const auto oldOriginUV = vm::vec2f{
        m_helper.face()->toUVCoordSystemMatrix(
          vm::vec2f{0, 0}, m_helper.face()->attributes().scale(), true)
        * origin};

      shearUV(m_map, snappedFactors);

      const auto newOriginUV = vm::vec2f{
        m_helper.face()->toUVCoordSystemMatrix(
          vm::vec2f{0, 0}, m_helper.face()->attributes().scale(), true)
        * origin};
      const auto newOffset =
        m_helper.face()->attributes().offset() + oldOriginUV - newOriginUV;

      setBrushFaceAttributes(
        m_map,
        {
          .xOffset = mdl::SetValue{newOffset.x()},
          .yOffset = mdl::SetValue{newOffset.y()},
        });
    }

    return true;
  }

  void end(const InputState&) override { m_map.commitTransaction(); }

  void cancel() override { m_map.cancelTransaction(); }
};

} // namespace

const mdl::HitType::Type UVShearTool::XHandleHitType = mdl::HitType::freeType();
const mdl::HitType::Type UVShearTool::YHandleHitType = mdl::HitType::freeType();

UVShearTool::UVShearTool(MapDocument& document, UVViewHelper& helper)
  : ToolController{}
  , Tool{true}
  , m_document{document}
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
    !(inputState.modifierKeysPressed(ModifierKeys::Alt)
      || inputState.modifierKeysPressed(ModifierKeys::Alt | ModifierKeys::CtrlCmd))
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

  if (!xHit.isMatch() && !yHit.isMatch())
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
    m_document.map(), m_helper, selector, xAxis, yAxis, *initialHit);
}

bool UVShearTool::cancel()
{
  return false;
}

} // namespace tb::ui
