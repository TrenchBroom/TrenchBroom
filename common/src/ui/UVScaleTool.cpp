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

#include "UVScaleTool.h"

#include "gl/VertexType.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushGeometry.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/PickResult.h"
#include "mdl/TransactionScope.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "render/EdgeRenderer.h"
#include "render/PrimType.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/UVOriginTool.h"
#include "ui/UVViewHelper.h"

#include "kd/contracts.h"
#include "kd/optional_utils.h"

#include "vm/intersection.h"
#include "vm/ray.h"
#include "vm/vec.h"

#include <numeric>
#include <vector>

namespace tb::ui
{
namespace
{

vm::vec2i getScaleHandle(const mdl::Hit& xHit, const mdl::Hit& yHit)
{
  const auto x = xHit.isMatch() ? xHit.target<int>() : 0;
  const auto y = yHit.isMatch() ? yHit.target<int>() : 0;
  return vm::vec2i{x, y};
}

std::tuple<vm::vec2i, vm::vec2b> getHandleAndSelector(const InputState& inputState)
{
  using namespace mdl::HitFilters;

  const auto& xHit = inputState.pickResult().first(type(UVScaleTool::XHandleHitType));
  const auto& yHit = inputState.pickResult().first(type(UVScaleTool::YHandleHitType));

  return {getScaleHandle(xHit, yHit), vm::vec2b{xHit.isMatch(), yHit.isMatch()}};
}

std::optional<vm::vec2f> getHitPoint(const UVViewHelper& helper, const vm::ray3d& pickRay)
{
  const auto& boundary = helper.face()->boundary();
  return vm::intersect_ray_plane(pickRay, boundary)
         | kdl::optional_transform([&](const auto facePointDist) {
             const auto facePoint = vm::point_at_distance(pickRay, facePointDist);
             const auto toTex =
               helper.face()->toUVCoordSystemMatrix({0, 0}, {1, 1}, true);

             return vm::vec2f{toTex * facePoint};
           });
}

vm::vec2f getScaledTranslatedHandlePos(const UVViewHelper& helper, const vm::vec2i handle)
{
  return vm::vec2f{handle} * vm::vec2f{helper.stripeSize()};
}

vm::vec2f getHandlePos(const UVViewHelper& helper, const vm::vec2i handle)
{
  const auto toWorld = helper.face()->fromUVCoordSystemMatrix(
    helper.face()->attributes().offset(), helper.face()->attributes().scale(), true);
  const auto toTex =
    helper.face()->toUVCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1}, true);

  return vm::vec2f{
    toTex * toWorld * vm::vec3d{getScaledTranslatedHandlePos(helper, handle)}};
}

vm::vec2f snap(const UVViewHelper& helper, const vm::vec2f& position)
{
  const auto toTex =
    helper.face()->toUVCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1}, true);

  const auto vertices = helper.face()->vertices();
  auto distance = std::accumulate(
    std::begin(vertices),
    std::end(vertices),
    vm::vec2f::max(),
    [&](const vm::vec2f& current, const mdl::BrushVertex* vertex) {
      const auto vertex2 = vm::vec2f{toTex * vertex->position()};
      return vm::abs_min(current, position - vertex2);
    });

  for (size_t i = 0; i < 2; ++i)
  {
    if (vm::abs(distance[i]) > 8.0f / helper.cameraZoom())
    {
      distance[i] = 0.0f;
    }
  }

  return position - distance;
}

using EdgeVertex = gl::VertexTypes::P3::Vertex;

std::vector<EdgeVertex> getHandleVertices(
  const UVViewHelper& helper, const vm::vec2i& handle, const vm::vec2b& selector)
{
  const auto stripeSize = helper.stripeSize();
  const auto pos = stripeSize * vm::vec2d{handle};

  vm::vec3d h1, h2, v1, v2;
  helper.computeScaleHandleVertices(pos, v1, v2, h1, h2);

  auto vertices = std::vector<EdgeVertex>{};
  vertices.reserve(4);

  if (selector.x())
  {
    vertices.emplace_back(vm::vec3f{v1});
    vertices.emplace_back(vm::vec3f{v2});
  }

  if (selector.y())
  {
    vertices.emplace_back(vm::vec3f{h1});
    vertices.emplace_back(vm::vec3f{h2});
  }

  return vertices;
}

void renderHighlight(
  const UVViewHelper& helper,
  const vm::vec2i& handle,
  const vm::vec2b& selector,
  render::RenderBatch& renderBatch)
{
  static const auto color = RgbaF{1.0f, 0.0f, 0.0f, 1.0f};

  auto handleRenderer = render::DirectEdgeRenderer{
    render::VertexArray::move(getHandleVertices(helper, handle, selector)),
    render::PrimType::Lines};
  handleRenderer.render(renderBatch, color, 1.0f);
}

class UVScaleDragTracker : public GestureTracker
{
private:
  mdl::Map& m_map;
  const UVViewHelper& m_helper;
  vm::vec2i m_handle;
  vm::vec2b m_selector;
  vm::vec2f m_lastHitPoint; // in non-scaled, non-translated UV coordinates
public:
  UVScaleDragTracker(
    mdl::Map& map,
    const UVViewHelper& helper,
    const vm::vec2i& handle,
    const vm::vec2b& selector,
    const vm::vec2f& initialHitPoint)
    : m_map{map}
    , m_helper{helper}
    , m_handle{handle}
    , m_selector{selector}
    , m_lastHitPoint{initialHitPoint}
  {
    m_map.startTransaction("Scale UV", mdl::TransactionScope::LongRunning);
  }

  bool update(const InputState& inputState) override
  {
    const auto curPoint = getHitPoint(m_helper, inputState.pickRay());
    if (!curPoint)
    {
      return false;
    }

    const auto dragDeltaFaceCoords = *curPoint - m_lastHitPoint;

    const auto curHandlePosUVCoords = getScaledTranslatedHandlePos(m_helper, m_handle);
    const auto newHandlePosFaceCoords =
      getHandlePos(m_helper, m_handle) + dragDeltaFaceCoords;
    const auto newHandlePosSnapped = !inputState.modifierKeysDown(ModifierKeys::CtrlCmd)
                                       ? snap(m_helper, newHandlePosFaceCoords)
                                       : newHandlePosFaceCoords;

    const auto originHandlePosFaceCoords = m_helper.originInFaceCoords();
    const auto originHandlePosUVCoords = m_helper.originInUVCoords();

    const auto newHandleDistFaceCoords = newHandlePosSnapped - originHandlePosFaceCoords;
    const auto curHandleDistUVCoords = curHandlePosUVCoords - originHandlePosUVCoords;

    auto newScale = m_helper.face()->attributes().scale();
    for (size_t i = 0; i < 2; ++i)
    {
      if (m_selector[i])
      {
        if (const auto value = newHandleDistFaceCoords[i] / curHandleDistUVCoords[i];
            value != 0.0f)
        {
          newScale[i] = value;
        }
      }
    }
    newScale = vm::correct(newScale, 4, 0.0f);

    setBrushFaceAttributes(
      m_map,
      {
        .xScale = mdl::SetValue{newScale.x()},
        .yScale = mdl::SetValue{newScale.y()},
      });

    const auto newOriginInUVCoords = vm::correct(m_helper.originInUVCoords(), 4, 0.0f);
    const auto originDelta = originHandlePosUVCoords - newOriginInUVCoords;

    setBrushFaceAttributes(
      m_map,
      {
        .xOffset = mdl::AddValue{originDelta.x()},
        .yOffset = mdl::AddValue{originDelta.y()},
      });

    m_lastHitPoint =
      m_lastHitPoint
      + (dragDeltaFaceCoords - newHandlePosFaceCoords + newHandlePosSnapped);
    return true;
  }

  void end(const InputState&) override { m_map.commitTransaction(); }

  void cancel() override { m_map.cancelTransaction(); }

  void render(const InputState&, render::RenderContext&, render::RenderBatch& renderBatch)
    const override
  {
    renderHighlight(m_helper, m_handle, m_selector, renderBatch);
  }
};

} // namespace

const mdl::HitType::Type UVScaleTool::XHandleHitType = mdl::HitType::freeType();
const mdl::HitType::Type UVScaleTool::YHandleHitType = mdl::HitType::freeType();

UVScaleTool::UVScaleTool(MapDocument& document, UVViewHelper& helper)
  : ToolController{}
  , Tool{true}
  , m_document{document}
  , m_helper{helper}
{
}

Tool& UVScaleTool::tool()
{
  return *this;
}

const Tool& UVScaleTool::tool() const
{
  return *this;
}

void UVScaleTool::pick(const InputState& inputState, mdl::PickResult& pickResult)
{
  static const mdl::HitType::Type HitTypes[] = {XHandleHitType, YHandleHitType};
  if (m_helper.valid())
  {
    m_helper.pickUVGrid(inputState.pickRay(), HitTypes, pickResult);
  }
}

std::unique_ptr<GestureTracker> UVScaleTool::acceptMouseDrag(const InputState& inputState)
{
  using namespace mdl::HitFilters;

  contract_pre(m_helper.valid());

  if (
    !inputState.modifierKeysPressed(ModifierKeys::None)
    || !inputState.mouseButtonsPressed(MouseButtons::Left))
  {
    return nullptr;
  }

  if (!m_helper.face()->attributes().valid())
  {
    return nullptr;
  }

  const auto [handle, selector] = getHandleAndSelector(inputState);
  if (!selector.x() && !selector.y())
  {
    return nullptr;
  }

  const auto initialHitPoint = getHitPoint(m_helper, inputState.pickRay());
  if (!initialHitPoint)
  {
    return nullptr;
  }

  return std::make_unique<UVScaleDragTracker>(
    m_document.map(), m_helper, handle, selector, *initialHitPoint);
}

void UVScaleTool::render(
  const InputState& inputState, render::RenderContext&, render::RenderBatch& renderBatch)
{
  using namespace mdl::HitFilters;

  if (
    inputState.anyToolDragging() || !m_helper.valid()
    || !m_helper.face()->attributes().valid())
  {
    return;
  }

  const auto& pickResult = inputState.pickResult();

  // don't overdraw the origin handles
  const auto& handleHit =
    pickResult.first(type(UVOriginTool::XHandleHitType | UVOriginTool::YHandleHitType));
  if (handleHit.isMatch())
  {
    return;
  }

  const auto [handle, selector] = getHandleAndSelector(inputState);
  if (!selector.x() && !selector.y())
  {
    return;
  }

  renderHighlight(m_helper, handle, selector, renderBatch);
}

bool UVScaleTool::cancel()
{
  return false;
}

} // namespace tb::ui
