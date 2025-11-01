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

#include "UVOriginTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushFace.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "mdl/Polyhedron.h"
#include "render/ActiveShader.h"
#include "render/Circle.h"
#include "render/EdgeRenderer.h"
#include "render/GLVertexType.h"
#include "render/PrimType.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/Renderable.h"
#include "render/Shaders.h"
#include "render/Transformation.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/UVViewHelper.h"

#include "vm/distance.h"
#include "vm/intersection.h"
#include "vm/line.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <vector>

namespace tb::ui
{
namespace
{

std::tuple<vm::line3d, vm::line3d> computeOriginHandles(const UVViewHelper& helper)
{
  const auto toWorld =
    helper.face()->fromUVCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1}, true);

  const auto origin = vm::vec3d{helper.originInFaceCoords()};
  const auto linePoint = toWorld * origin;
  return {
    vm::line3d{
      linePoint, vm::normalize(toWorld * (origin + vm::vec3d{0, 1, 0}) - linePoint)},
    vm::line3d{linePoint, (toWorld * (origin + vm::vec3d{1, 0, 0}) - linePoint)},
  };
}

vm::vec2f getSelector(const InputState& inputState)
{
  using namespace mdl::HitFilters;

  const auto& xHandleHit =
    inputState.pickResult().first(type(UVOriginTool::XHandleHitType));
  const auto& yHandleHit =
    inputState.pickResult().first(type(UVOriginTool::YHandleHitType));

  return vm::vec2f{
    xHandleHit.isMatch() ? 1.0f : 0.0f, yHandleHit.isMatch() ? 1.0f : 0.0f};
}

vm::vec2f computeHitPoint(const UVViewHelper& helper, const vm::ray3d& ray)
{
  const auto& boundary = helper.face()->boundary();
  const auto distance = *vm::intersect_ray_plane(ray, boundary);
  const auto hitPoint = vm::point_at_distance(ray, distance);
  const auto transform =
    helper.face()->toUVCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1}, true);
  return vm::vec2f{transform * hitPoint};
}

vm::vec2f snapDelta(const UVViewHelper& helper, const vm::vec2f& delta)
{
  assert(helper.valid());

  if (vm::is_zero(delta, vm::Cf::almost_zero()))
  {
    return delta;
  }

  // The delta is given in non-translated and non-scaled UV coordinates because that's how
  // the origin is stored. We have to convert to translated and scaled UV coordinates to
  // do our snapping because that's how the helper computes the distance to the UV grid.
  // Finally, we will convert the distance back to non-translated and non-scaled UV
  // coordinates and snap the delta to the distance.

  const auto w2fTransform =
    helper.face()->toUVCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1}, true);
  const auto w2tTransform = helper.face()->toUVCoordSystemMatrix(
    helper.face()->attributes().offset(), helper.face()->attributes().scale(), true);
  const auto f2wTransform =
    helper.face()->fromUVCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1}, true);
  const auto t2wTransform = helper.face()->fromUVCoordSystemMatrix(
    helper.face()->attributes().offset(), helper.face()->attributes().scale(), true);
  const auto f2tTransform = w2tTransform * f2wTransform;
  const auto t2fTransform = w2fTransform * t2wTransform;

  const auto newOriginInFaceCoords = helper.originInFaceCoords() + delta;
  const auto newOriginInUVCoords =
    vm::vec2f{f2tTransform * vm::vec3d{newOriginInFaceCoords}};

  // now snap to the vertices
  // TODO: this actually doesn't work because we're snapping to the X or Y coordinate of
  // the vertices instead, we must snap to the edges!
  auto distanceInUVCoords = vm::vec2f::max();
  for (const auto* vertex : helper.face()->vertices())
  {
    distanceInUVCoords = vm::abs_min(
      distanceInUVCoords,
      vm::vec2f{w2tTransform * vertex->position()} - newOriginInUVCoords);
  }

  // and to the UV grid
  if (helper.face()->material())
  {
    distanceInUVCoords = vm::abs_min(
      distanceInUVCoords,
      helper.computeDistanceFromUVGrid(vm::vec3d{newOriginInUVCoords}));
  }

  // finally snap to the face center
  const auto faceCenter = vm::vec2f{w2tTransform * helper.face()->boundsCenter()};
  distanceInUVCoords = vm::abs_min(distanceInUVCoords, faceCenter - newOriginInUVCoords);

  // now we have a distance in the scaled and translated UV coordinate system so we
  // transform the new position plus distance back to the unscaled and untranslated UV
  // coordinate system and take the actual distance
  const auto distanceInFaceCoords =
    newOriginInFaceCoords
    - vm::vec2f{t2fTransform * vm::vec3d{newOriginInUVCoords + distanceInUVCoords}};
  return helper.snapDelta(delta, -distanceInFaceCoords);
}

using EdgeVertex = render::GLVertexTypes::P3C4::Vertex;

std::vector<EdgeVertex> getHandleVertices(
  const UVViewHelper& helper, const vm::vec2b& highlightHandle)
{
  const auto xColor =
    highlightHandle.x() ? RgbaF{1.0f, 0.0f, 0.0f, 1.0f} : RgbaF{0.7f, 0.0f, 0.0f, 1.0f};
  const auto yColor =
    highlightHandle.y() ? RgbaF{1.0f, 0.0f, 0.0f, 1.0f} : RgbaF{0.7f, 0.0f, 0.0f, 1.0f};

  vm::vec3d x1, x2, y1, y2;
  helper.computeOriginHandleVertices(x1, x2, y1, y2);

  return {
    EdgeVertex{vm::vec3f{x1}, xColor},
    EdgeVertex{vm::vec3f{x2}, xColor},
    EdgeVertex{vm::vec3f{y1}, yColor},
    EdgeVertex{vm::vec3f{y2}, yColor}};
}

void renderLineHandles(
  const UVViewHelper& helper,
  const vm::vec2b& highlightHandles,
  render::RenderBatch& renderBatch)
{
  auto edgeRenderer = render::DirectEdgeRenderer{
    render::VertexArray::move(getHandleVertices(helper, highlightHandles)),
    render::PrimType::Lines};
  edgeRenderer.renderOnTop(renderBatch, 0.5f);
}

class RenderOrigin : public render::DirectRenderable
{
private:
  const UVViewHelper& m_helper;
  bool m_highlight;
  render::Circle m_originHandle;

public:
  RenderOrigin(const UVViewHelper& helper, const float originRadius, const bool highlight)
    : m_helper{helper}
    , m_highlight{highlight}
    , m_originHandle{makeCircle(m_helper, originRadius, 16, true)}
  {
  }

private:
  static render::Circle makeCircle(
    const UVViewHelper& helper,
    const float radius,
    const size_t segments,
    const bool fill)
  {
    const float zoom = helper.cameraZoom();
    return render::Circle{radius / zoom, segments, fill};
  }

private:
  void doPrepareVertices(render::VboManager& vboManager) override
  {
    m_originHandle.prepare(vboManager);
  }

  void doRender(render::RenderContext& renderContext) override
  {
    const auto fromFace =
      m_helper.face()->fromUVCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1}, true);

    const auto& boundary = m_helper.face()->boundary();
    const auto toPlane = vm::plane_projection_matrix(boundary.distance, boundary.normal);
    const auto fromPlane = vm::invert(toPlane);
    const auto originPosition(
      toPlane * fromFace * vm::vec3d{m_helper.originInFaceCoords()});

    const auto& handleColor = pref(Preferences::HandleColor);
    const auto& highlightColor = pref(Preferences::SelectedHandleColor);

    const auto toWorldTransform = render::MultiplyModelMatrix{
      renderContext.transformation(), vm::mat4x4f{*fromPlane}};
    const auto translation = vm::translation_matrix(vm::vec3d{originPosition});
    const auto centerTransform = render::MultiplyModelMatrix{
      renderContext.transformation(), vm::mat4x4f{translation}};

    auto shader = render::ActiveShader{
      renderContext.shaderManager(), render::Shaders::VaryingPUniformCShader};
    shader.set("Color", m_highlight ? highlightColor.toRgbaF() : handleColor);
    m_originHandle.render();
  }
};

void renderOriginHandle(
  const UVViewHelper& helper, const bool highlight, render::RenderBatch& renderBatch)
{
  renderBatch.addOneShot(
    new RenderOrigin{helper, UVOriginTool::OriginHandleRadius, highlight});
}

class UVOriginDragTracker : public GestureTracker
{
private:
  UVViewHelper& m_helper;
  vm::vec2f m_selector;
  vm::vec2f m_lastPoint;

public:
  UVOriginDragTracker(UVViewHelper& helper, const InputState& inputState)
    : m_helper{helper}
    , m_selector{getSelector(inputState)}
    , m_lastPoint{computeHitPoint(m_helper, inputState.pickRay())}
  {
  }

  bool update(const InputState& inputState) override
  {
    const auto curPoint = computeHitPoint(m_helper, inputState.pickRay());
    const auto delta = curPoint - m_lastPoint;

    const auto snapped = !inputState.modifierKeysDown(ModifierKeys::CtrlCmd)
                           ? snapDelta(m_helper, delta * m_selector)
                           : delta * m_selector;
    if (vm::is_zero(snapped, vm::Cf::almost_zero()))
    {
      return true;
    }

    m_helper.setOriginInFaceCoords(m_helper.originInFaceCoords() + snapped);
    m_lastPoint = m_lastPoint + snapped;

    return true;
  }

  void end(const InputState&) override {}
  void cancel() override {}

  void render(const InputState&, render::RenderContext&, render::RenderBatch& renderBatch)
    const override
  {
    const auto highlightHandles = vm::vec2b{m_selector.x() > 0.0, m_selector.y() > 0.0};

    renderLineHandles(m_helper, highlightHandles, renderBatch);
    renderOriginHandle(m_helper, true, renderBatch);
  }
};

} // namespace

const mdl::HitType::Type UVOriginTool::XHandleHitType = mdl::HitType::freeType();
const mdl::HitType::Type UVOriginTool::YHandleHitType = mdl::HitType::freeType();
const double UVOriginTool::MaxPickDistance = 5.0;
const float UVOriginTool::OriginHandleRadius = 5.0f;

UVOriginTool::UVOriginTool(UVViewHelper& helper)
  : ToolController{}
  , Tool{true}
  , m_helper{helper}
{
}

Tool& UVOriginTool::tool()
{
  return *this;
}

const Tool& UVOriginTool::tool() const
{
  return *this;
}

void UVOriginTool::pick(const InputState& inputState, mdl::PickResult& pickResult)
{
  if (m_helper.valid())
  {
    const auto [xHandle, yHandle] = computeOriginHandles(m_helper);

    const auto fromTex =
      m_helper.face()->fromUVCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1}, true);
    const auto origin = fromTex * vm::vec3d{m_helper.originInFaceCoords()};

    const auto& pickRay = inputState.pickRay();
    const auto oDistance = vm::distance(pickRay, origin);
    if (
      oDistance.distance
      <= static_cast<double>(OriginHandleRadius / m_helper.cameraZoom()))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, oDistance.position);
      pickResult.addHit(mdl::Hit{
        XHandleHitType, oDistance.position, hitPoint, xHandle, oDistance.distance});
      pickResult.addHit(mdl::Hit{
        YHandleHitType, oDistance.position, hitPoint, xHandle, oDistance.distance});
    }
    else
    {
      const auto xDistance = vm::distance(pickRay, xHandle);
      const auto yDistance = vm::distance(pickRay, yHandle);

      assert(!xDistance.parallel);
      assert(!yDistance.parallel);

      const auto maxDistance =
        MaxPickDistance / static_cast<double>(m_helper.cameraZoom());
      if (xDistance.distance <= maxDistance)
      {
        const auto hitPoint = vm::point_at_distance(pickRay, xDistance.position1);
        pickResult.addHit(mdl::Hit{
          XHandleHitType, xDistance.position1, hitPoint, xHandle, xDistance.distance});
      }

      if (yDistance.distance <= maxDistance)
      {
        const auto hitPoint = vm::point_at_distance(pickRay, yDistance.position1);
        pickResult.addHit(mdl::Hit{
          YHandleHitType, yDistance.position1, hitPoint, yHandle, yDistance.distance});
      }
    }
  }
}

std::unique_ptr<GestureTracker> UVOriginTool::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace mdl::HitFilters;

  assert(m_helper.valid());

  if (
    !inputState.modifierKeysPressed(ModifierKeys::None)
    || !inputState.mouseButtonsPressed(MouseButtons::Left))
  {
    return nullptr;
  }

  const auto& xHandleHit = inputState.pickResult().first(type(XHandleHitType));
  const auto& yHandleHit = inputState.pickResult().first(type(YHandleHitType));

  if (!xHandleHit.isMatch() && !yHandleHit.isMatch())
  {
    return nullptr;
  }

  return std::make_unique<UVOriginDragTracker>(m_helper, inputState);
}

void UVOriginTool::render(
  const InputState& inputState, render::RenderContext&, render::RenderBatch& renderBatch)
{
  using namespace mdl::HitFilters;

  if (m_helper.valid() && !inputState.anyToolDragging())
  {
    const auto& xHandleHit =
      inputState.pickResult().first(type(UVOriginTool::XHandleHitType));
    const auto& yHandleHit =
      inputState.pickResult().first(type(UVOriginTool::YHandleHitType));

    const auto highlightHandles = vm::vec2b{xHandleHit.isMatch(), yHandleHit.isMatch()};

    renderLineHandles(m_helper, highlightHandles, renderBatch);
    renderOriginHandle(
      m_helper, xHandleHit.isMatch() || yHandleHit.isMatch(), renderBatch);
  }
}

bool UVOriginTool::cancel()
{
  return false;
}

} // namespace tb::ui
