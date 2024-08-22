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

#include "UVScaleTool.h"

#include "Assets/Material.h"
#include "FloatType.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "View/DragTracker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/TransactionScope.h"
#include "View/UVOriginTool.h"
#include "View/UVViewHelper.h"

#include "kdl/memory_utils.h"
#include "kdl/optional_utils.h"

#include "vm/intersection.h"
#include "vm/ray.h"
#include "vm/vec.h"

#include <numeric>
#include <vector>

namespace TrenchBroom::View
{

namespace
{

vm::vec2i getScaleHandle(const Model::Hit& xHit, const Model::Hit& yHit)
{
  const auto x = xHit.isMatch() ? xHit.target<int>() : 0;
  const auto y = yHit.isMatch() ? yHit.target<int>() : 0;
  return vm::vec2i{x, y};
}

std::tuple<vm::vec2i, vm::vec2b> getHandleAndSelector(const InputState& inputState)
{
  using namespace Model::HitFilters;

  const auto& xHit = inputState.pickResult().first(type(UVScaleTool::XHandleHitType));
  const auto& yHit = inputState.pickResult().first(type(UVScaleTool::YHandleHitType));

  return {getScaleHandle(xHit, yHit), vm::vec2b{xHit.isMatch(), yHit.isMatch()}};
}

std::optional<vm::vec2f> getHitPoint(const UVViewHelper& helper, const vm::ray3& pickRay)
{
  const auto& boundary = helper.face()->boundary();
  return kdl::optional_transform(
    vm::intersect_ray_plane(pickRay, boundary), [&](const auto facePointDist) {
      const auto facePoint = vm::point_at_distance(pickRay, facePointDist);
      const auto toTex = helper.face()->toUVCoordSystemMatrix({0, 0}, {1, 1}, true);

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
    helper.face()->toUVCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

  return vm::vec2f{
    toTex * toWorld * vm::vec3{getScaledTranslatedHandlePos(helper, handle)}};
}

vm::vec2f snap(const UVViewHelper& helper, const vm::vec2f& position)
{
  const auto toTex =
    helper.face()->toUVCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

  const auto vertices = helper.face()->vertices();
  auto distance = std::accumulate(
    std::begin(vertices),
    std::end(vertices),
    vm::vec2f::max(),
    [&](const vm::vec2f& current, const Model::BrushVertex* vertex) {
      const auto vertex2 = vm::vec2f{toTex * vertex->position()};
      return vm::abs_min(current, position - vertex2);
    });

  for (size_t i = 0; i < 2; ++i)
  {
    if (vm::abs(distance[i]) > 4.0f / helper.cameraZoom())
    {
      distance[i] = 0.0f;
    }
  }

  return position - distance;
}

using EdgeVertex = Renderer::GLVertexTypes::P3::Vertex;

std::vector<EdgeVertex> getHandleVertices(
  const UVViewHelper& helper, const vm::vec2i& handle, const vm::vec2b& selector)
{
  const auto stripeSize = helper.stripeSize();
  const auto pos = stripeSize * vm::vec2{handle};

  vm::vec3 h1, h2, v1, v2;
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
  Renderer::RenderBatch& renderBatch)
{
  static const auto color = Color{1.0f, 0.0f, 0.0f, 1.0f};

  auto handleRenderer = Renderer::DirectEdgeRenderer{
    Renderer::VertexArray::move(getHandleVertices(helper, handle, selector)),
    Renderer::PrimType::Lines};
  handleRenderer.render(renderBatch, color, 1.0f);
}

class UVScaleDragTracker : public DragTracker
{
private:
  MapDocument& m_document;
  const UVViewHelper& m_helper;
  vm::vec2i m_handle;
  vm::vec2b m_selector;
  vm::vec2f m_lastHitPoint; // in non-scaled, non-translated UV coordinates
public:
  UVScaleDragTracker(
    MapDocument& document,
    const UVViewHelper& helper,
    const vm::vec2i& handle,
    const vm::vec2b& selector,
    const vm::vec2f& initialHitPoint)
    : m_document{document}
    , m_helper{helper}
    , m_handle{handle}
    , m_selector{selector}
    , m_lastHitPoint{initialHitPoint}
  {
    document.startTransaction("Scale UV", TransactionScope::LongRunning);
  }

  bool drag(const InputState& inputState) override
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
    const auto newHandlePosSnapped = snap(m_helper, newHandlePosFaceCoords);

    const auto originHandlePosFaceCoords = m_helper.originInFaceCoords();
    const auto originHandlePosUVCoords = m_helper.originInUVCoords();

    const auto newHandleDistFaceCoords = newHandlePosSnapped - originHandlePosFaceCoords;
    const auto curHandleDistUVCoords = curHandlePosUVCoords - originHandlePosUVCoords;

    auto newScale = m_helper.face()->attributes().scale();
    for (size_t i = 0; i < 2; ++i)
    {
      if (m_selector[i])
      {
        const auto value = newHandleDistFaceCoords[i] / curHandleDistUVCoords[i];
        if (value != 0.0f)
        {
          newScale[i] = value;
        }
      }
    }
    newScale = vm::correct(newScale, 4, 0.0f);

    auto request = Model::ChangeBrushFaceAttributesRequest{};
    request.setScale(newScale);
    m_document.setFaceAttributes(request);

    const auto newOriginInUVCoords = vm::correct(m_helper.originInUVCoords(), 4, 0.0f);
    const auto originDelta = originHandlePosUVCoords - newOriginInUVCoords;

    request.clear();
    request.addOffset(originDelta);
    m_document.setFaceAttributes(request);

    m_lastHitPoint =
      m_lastHitPoint
      + (dragDeltaFaceCoords - newHandlePosFaceCoords + newHandlePosSnapped);
    return true;
  }

  void end(const InputState&) override { m_document.commitTransaction(); }

  void cancel() override { m_document.cancelTransaction(); }

  void render(
    const InputState&,
    Renderer::RenderContext&,
    Renderer::RenderBatch& renderBatch) const override
  {
    renderHighlight(m_helper, m_handle, m_selector, renderBatch);
  }
};

} // namespace

const Model::HitType::Type UVScaleTool::XHandleHitType = Model::HitType::freeType();
const Model::HitType::Type UVScaleTool::YHandleHitType = Model::HitType::freeType();

UVScaleTool::UVScaleTool(std::weak_ptr<MapDocument> document, UVViewHelper& helper)
  : ToolController{}
  , Tool{true}
  , m_document{std::move(document)}
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

void UVScaleTool::pick(const InputState& inputState, Model::PickResult& pickResult)
{
  static const Model::HitType::Type HitTypes[] = {XHandleHitType, YHandleHitType};
  if (m_helper.valid())
  {
    m_helper.pickUVGrid(inputState.pickRay(), HitTypes, pickResult);
  }
}

std::unique_ptr<DragTracker> UVScaleTool::acceptMouseDrag(const InputState& inputState)
{
  using namespace Model::HitFilters;

  assert(m_helper.valid());

  if (
    !inputState.modifierKeysPressed(ModifierKeys::MKNone)
    || !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
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
    *kdl::mem_lock(m_document), m_helper, handle, selector, *initialHitPoint);
}

void UVScaleTool::render(
  const InputState& inputState,
  Renderer::RenderContext&,
  Renderer::RenderBatch& renderBatch)
{
  using namespace Model::HitFilters;

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

} // namespace TrenchBroom::View
