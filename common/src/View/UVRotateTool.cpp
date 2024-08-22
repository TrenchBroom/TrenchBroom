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

#include "UVRotateTool.h"

#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Circle.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Renderable.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/Transformation.h"
#include "Renderer/VboManager.h"
#include "View/DragTracker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/TransactionScope.h"
#include "View/UVViewHelper.h"

#include "kdl/memory_utils.h"
#include "kdl/optional_utils.h"

#include "vm/forward.h"
#include "vm/intersection.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <optional>

namespace TrenchBroom::View
{

namespace
{

constexpr auto CenterHandleRadius = 2.5;
constexpr auto RotateHandleRadius = 32.0;
constexpr auto RotateHandleWidth = 5.0;

float measureAngle(const UVViewHelper& helper, const vm::vec2f& point)
{
  const auto origin = helper.originInFaceCoords();
  return vm::mod(helper.face()->measureUVAngle(origin, point), 360.0f);
}

float snapAngle(const UVViewHelper& helper, const float angle)
{
  const float angles[] = {
    vm::mod(angle + 0.0f, 360.0f),
    vm::mod(angle + 90.0f, 360.0f),
    vm::mod(angle + 180.0f, 360.0f),
    vm::mod(angle + 270.0f, 360.0f),
  };
  auto minDelta = std::numeric_limits<float>::max();

  const auto toFace =
    helper.face()->toUVCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
  for (const auto* edge : helper.face()->edges())
  {
    const auto startInFaceCoords = vm::vec2f{toFace * edge->firstVertex()->position()};
    const auto endInFaceCoords = vm::vec2f{toFace * edge->secondVertex()->position()};
    const auto edgeAngle =
      vm::mod(helper.face()->measureUVAngle(startInFaceCoords, endInFaceCoords), 360.0f);

    for (size_t i = 0; i < 4; ++i)
    {
      if (std::abs(angles[i] - edgeAngle) < std::abs(minDelta))
      {
        minDelta = angles[i] - edgeAngle;
      }
    }
  }

  if (std::abs(minDelta) < 3.0f)
  {
    return angle - minDelta;
  }
  return angle;
}

Renderer::Circle makeCircle(
  const UVViewHelper& helper, const float radius, const size_t segments, const bool fill)
{
  const auto zoom = helper.cameraZoom();
  return Renderer::Circle{radius / zoom, segments, fill};
}

class Render : public Renderer::DirectRenderable
{
private:
  const UVViewHelper& m_helper;
  bool m_highlight;
  Renderer::Circle m_center;
  Renderer::Circle m_outer;

public:
  Render(
    const UVViewHelper& helper,
    const float centerRadius,
    const float outerRadius,
    const bool highlight)
    : m_helper{helper}
    , m_highlight{highlight}
    , m_center{makeCircle(helper, centerRadius, 10, true)}
    , m_outer{makeCircle(helper, outerRadius, 32, false)}
  {
  }

private:
  void doPrepareVertices(Renderer::VboManager& vboManager) override
  {
    m_center.prepare(vboManager);
    m_outer.prepare(vboManager);
  }

  void doRender(Renderer::RenderContext& renderContext) override
  {
    const auto fromFace =
      m_helper.face()->fromUVCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

    const auto& boundary = m_helper.face()->boundary();
    const auto toPlane = vm::plane_projection_matrix(boundary.distance, boundary.normal);
    const auto fromPlane = vm::invert(toPlane);

    const auto originPosition =
      toPlane * fromFace * vm::vec3{m_helper.originInFaceCoords()};
    const auto faceCenterPosition = toPlane * m_helper.face()->boundsCenter();

    const auto& handleColor = pref(Preferences::HandleColor);
    const auto& highlightColor = pref(Preferences::SelectedHandleColor);

    auto shader = Renderer::ActiveShader{
      renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader};
    const auto toWorldTransform = Renderer::MultiplyModelMatrix{
      renderContext.transformation(), vm::mat4x4f{*fromPlane}};
    {
      const auto translation = vm::translation_matrix(vm::vec3{originPosition});
      const auto centerTransform = Renderer::MultiplyModelMatrix{
        renderContext.transformation(), vm::mat4x4f{translation}};
      if (m_highlight)
      {
        shader.set("Color", highlightColor);
      }
      else
      {
        shader.set("Color", handleColor);
      }
      m_outer.render();
    }

    {
      const auto translation = vm::translation_matrix(vm::vec3{faceCenterPosition});
      const auto centerTransform = Renderer::MultiplyModelMatrix{
        renderContext.transformation(), vm::mat4x4f{translation}};
      shader.set("Color", highlightColor);
      m_center.render();
    }
  }
};

class UVRotateDragTracker : public DragTracker
{
private:
  MapDocument& m_document;
  const UVViewHelper& m_helper;
  float m_initialAngle;

public:
  UVRotateDragTracker(
    MapDocument& document, const UVViewHelper& helper, const float initialAngle)
    : m_document{document}
    , m_helper{helper}
    , m_initialAngle{initialAngle}
  {
    document.startTransaction("Rotate UV", TransactionScope::LongRunning);
  }

  bool drag(const InputState& inputState) override
  {
    assert(m_helper.valid());

    const auto& boundary = m_helper.face()->boundary();
    const auto& pickRay = inputState.pickRay();
    const auto curPointDistance = vm::intersect_ray_plane(pickRay, boundary);
    const auto curPoint = vm::point_at_distance(pickRay, *curPointDistance);

    const auto toFaceOld =
      m_helper.face()->toUVCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
    const auto toWorld =
      m_helper.face()->fromUVCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

    const auto curPointInFaceCoords = vm::vec2f{toFaceOld * curPoint};
    const auto curAngle = measureAngle(m_helper, curPointInFaceCoords);

    const auto angle = curAngle - m_initialAngle;
    const auto snappedAngle = vm::correct(snapAngle(m_helper, angle), 4, 0.0f);

    const auto oldCenterInFaceCoords = m_helper.originInFaceCoords();
    const auto oldCenterInWorldCoords = toWorld * vm::vec3{oldCenterInFaceCoords};

    auto request = Model::ChangeBrushFaceAttributesRequest{};
    request.setRotation(snappedAngle);
    m_document.setFaceAttributes(request);

    // Correct the offsets.
    const auto toFaceNew =
      m_helper.face()->toUVCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
    const auto newCenterInFaceCoords = vm::vec2f{toFaceNew * oldCenterInWorldCoords};

    const auto delta = (oldCenterInFaceCoords - newCenterInFaceCoords)
                       / m_helper.face()->attributes().scale();
    const auto newOffset =
      vm::correct(m_helper.face()->attributes().offset() + delta, 4, 0.0f);

    request.clear();
    request.setOffset(newOffset);
    m_document.setFaceAttributes(request);

    return true;
  }

  void end(const InputState&) override { m_document.commitTransaction(); }

  void cancel() override { m_document.cancelTransaction(); }

  void render(
    const InputState&,
    Renderer::RenderContext&,
    Renderer::RenderBatch& renderBatch) const override
  {
    renderBatch.addOneShot(
      new Render{m_helper, float(CenterHandleRadius), float(RotateHandleRadius), true});
  }
};

std::optional<vm::vec2f> hitPointInFaceCoords(
  const UVViewHelper& helper, const InputState& inputState)
{
  using namespace Model::HitFilters;

  const auto toFace =
    helper.face()->toUVCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);

  const auto& angleHandleHit =
    inputState.pickResult().first(type(UVRotateTool::AngleHandleHitType));
  if (angleHandleHit.isMatch())
  {
    return vm::vec2f{toFace * angleHandleHit.hitPoint()};
  }

  if (inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd))
  {
    // If Ctrl is pressed, allow starting the drag anywhere, not just on the handle
    const auto& boundary = helper.face()->boundary();
    const auto& pickRay = inputState.pickRay();
    return kdl::optional_transform(
      vm::intersect_ray_plane(pickRay, boundary), [&](const auto distanceToFace) {
        const auto hitPoint = vm::point_at_distance(pickRay, distanceToFace);
        return vm::vec2f{toFace * hitPoint};
      });
  }

  return std::nullopt;
}

std::optional<float> computeInitialAngle(
  const UVViewHelper& helper, const InputState& inputState)
{
  return kdl::optional_transform(
    hitPointInFaceCoords(helper, inputState), [&](const auto& point) {
      return measureAngle(helper, point) - helper.face()->attributes().rotation();
    });
}

} // namespace

const Model::HitType::Type UVRotateTool::AngleHandleHitType = Model::HitType::freeType();

UVRotateTool::UVRotateTool(std::weak_ptr<MapDocument> document, UVViewHelper& helper)
  : ToolController{}
  , Tool{true}
  , m_document{std::move(document)}
  , m_helper{helper}
{
}

Tool& UVRotateTool::tool()
{
  return *this;
}

const Tool& UVRotateTool::tool() const
{
  return *this;
}

void UVRotateTool::pick(const InputState& inputState, Model::PickResult& pickResult)
{
  if (!m_helper.valid())
  {
    return;
  }

  const auto& boundary = m_helper.face()->boundary();

  const auto& pickRay = inputState.pickRay();
  if (const auto distanceToFace = vm::intersect_ray_plane(pickRay, boundary))
  {
    const auto hitPoint = vm::point_at_distance(pickRay, *distanceToFace);

    const auto fromFace =
      m_helper.face()->fromUVCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
    const auto toPlane = vm::plane_projection_matrix(boundary.distance, boundary.normal);

    const auto originOnPlane =
      toPlane * fromFace * vm::vec3{m_helper.originInFaceCoords()};
    const auto hitPointOnPlane = toPlane * hitPoint;

    const auto zoom = static_cast<FloatType>(m_helper.cameraZoom());
    const auto error =
      vm::abs(RotateHandleRadius / zoom - vm::distance(hitPointOnPlane, originOnPlane));
    if (error <= RotateHandleWidth / zoom)
    {
      pickResult.addHit(
        Model::Hit{AngleHandleHitType, *distanceToFace, hitPoint, 0, error});
    }
  }
}

std::unique_ptr<DragTracker> UVRotateTool::acceptMouseDrag(const InputState& inputState)
{
  assert(m_helper.valid());

  if (
    !(inputState.modifierKeysPressed(ModifierKeys::MKNone)
      || inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd))
    || !inputState.mouseButtonsPressed(MouseButtons::MBLeft))
  {
    return nullptr;
  }

  if (!m_helper.face()->attributes().valid())
  {
    return nullptr;
  }

  const auto initialAngle = computeInitialAngle(m_helper, inputState);
  if (!initialAngle)
  {
    return nullptr;
  }

  return std::make_unique<UVRotateDragTracker>(
    *kdl::mem_lock(m_document), m_helper, *initialAngle);
}

void UVRotateTool::render(
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

  const auto highlight =
    inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd)
    || inputState.pickResult().first(type(AngleHandleHitType)).isMatch();
  renderBatch.addOneShot(new Render{
    m_helper, float(CenterHandleRadius), float(RotateHandleRadius), highlight});
}

bool UVRotateTool::cancel()
{
  return false;
}

} // namespace TrenchBroom::View
