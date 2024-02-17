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

#include "ClipTool.h"

#include "Error.h"
#include "FloatType.h"
#include "Macros.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "View/MapDocument.h"
#include "View/Selection.h"

#include "kdl/map_utils.h"
#include "kdl/memory_utils.h"
#include "kdl/overload.h"
#include "kdl/set_temp.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

#include "vm/ray.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <optional>

namespace TrenchBroom::View
{

const Model::HitType::Type ClipTool::PointHitType = Model::HitType::freeType();

class ClipStrategy
{
public:
  virtual ~ClipStrategy() = default;

  virtual void pick(
    const vm::ray3& pickRay,
    const Renderer::Camera& camera,
    Model::PickResult& pickResult) const = 0;
  virtual void render(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const Model::PickResult& pickResult) = 0;
  virtual void renderFeedback(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const vm::vec3& point) const = 0;

  virtual std::optional<vm::vec3> computeThirdPoint() const = 0;

  virtual bool canClip() const = 0;
  virtual bool hasPoints() const = 0;
  virtual bool canAddPoint(const vm::vec3& point) const = 0;
  virtual void addPoint(const vm::vec3& point, std::vector<vm::vec3> helpVectors) = 0;
  virtual bool canRemoveLastPoint() const = 0;
  virtual void removeLastPoint() = 0;

  virtual std::optional<std::tuple<vm::vec3, vm::vec3>> canDragPoint(
    const Model::PickResult& pickResult) const = 0;
  virtual void beginDragPoint(const Model::PickResult& pickResult) = 0;
  virtual void beginDragLastPoint() = 0;
  virtual bool dragPoint(
    const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors) = 0;
  virtual void endDragPoint() = 0;
  virtual void cancelDragPoint() = 0;

  virtual bool setFace(const Model::BrushFaceHandle& faceHandle) = 0;
  virtual void reset() = 0;
  virtual std::vector<vm::vec3> getPoints() const = 0;
};

namespace
{

class PointClipStrategy : public ClipStrategy
{
private:
  struct ClipPoint
  {
    vm::vec3 point;
    std::vector<vm::vec3> helpVectors;
  };

  struct DragState
  {
    size_t index;
    ClipPoint originalPoint;
  };

  std::vector<ClipPoint> m_points;
  std::optional<DragState> m_dragState;

public:
  void pick(
    const vm::ray3& pickRay,
    const Renderer::Camera& camera,
    Model::PickResult& pickResult) const override
  {
    for (size_t i = 0; i < m_points.size(); ++i)
    {
      const auto& point = m_points[i].point;
      const auto distance = camera.pickPointHandle(
        pickRay, point, static_cast<FloatType>(pref(Preferences::HandleRadius)));
      if (!vm::is_nan(distance))
      {
        const auto hitPoint = vm::point_at_distance(pickRay, distance);
        pickResult.addHit(Model::Hit{ClipTool::PointHitType, distance, hitPoint, i});
      }
    }
  }

  void render(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const Model::PickResult& pickResult) override
  {
    renderPoints(renderContext, renderBatch);
    renderHighlight(renderContext, renderBatch, pickResult);
  }

  void renderFeedback(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const vm::vec3& point) const override
  {
    auto renderService = Renderer::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
    renderService.renderHandle(vm::vec3f{point});
  }

  std::optional<vm::vec3> computeThirdPoint() const override
  {
    if (m_points.size() == 2)
    {
      const auto point = m_points[1].point + 128.0 * computeHelpVector();
      if (!vm::is_colinear(m_points[0].point, m_points[1].point, point))
      {
        return point;
      }
    }
    return std::nullopt;
  }

  vm::vec3 computeHelpVector() const
  {
    size_t counts[6];
    counts[0] = counts[1] = counts[2] = counts[3] = counts[4] = counts[5] = 0;

    const auto helpVectors = combineHelpVectors();
    for (size_t i = 0; i < helpVectors.size(); ++i)
    {
      const auto axis = vm::find_abs_max_component(helpVectors[i]);
      const auto index = helpVectors[i][axis] > 0.0 ? axis : axis + 3;
      counts[index]++;
    }

    const auto first = std::max_element(std::begin(counts), std::end(counts));
    const auto next = std::max_element(std::next(first), std::end(counts));

    const auto firstIndex = first - std::begin(counts);
    const auto nextIndex = next - std::begin(counts);

    if (counts[firstIndex] > counts[nextIndex])
    {
      return vm::vec3::axis(size_t(firstIndex % 3));
    }

    // two counts are equal
    if (firstIndex % 3 == 2 || nextIndex % 3 == 2)
    {
      // prefer the Z axis if possible:
      return vm::vec3::pos_z();
    }

    // Z axis cannot win, so X and Y axis are a tie, prefer the X axis:
    return vm::vec3::pos_x();
  }

  std::vector<vm::vec3> combineHelpVectors() const
  {
    return kdl::vec_flatten(
      kdl::vec_transform(m_points, [](const auto& point) { return point.helpVectors; }));
  }

  bool canClip() const override { return m_points.size() == 3 || computeThirdPoint(); }

  bool hasPoints() const override { return !m_points.empty(); }

  bool canAddPoint(const vm::vec3& point) const override
  {
    return (m_points.size() < 2
            || (m_points.size() == 2 && !vm::is_colinear(m_points[0].point, m_points[1].point, point)))
           && kdl::none_of(m_points, [&](const auto& p) {
                return vm::is_equal(p.point, point, vm::C::almost_zero());
              });
  }

  void addPoint(const vm::vec3& point, std::vector<vm::vec3> helpVectors) override
  {
    m_points.push_back(ClipPoint{point, std::move(helpVectors)});
  }

  bool canRemoveLastPoint() const override { return hasPoints(); }

  void removeLastPoint() override
  {
    ensure(canRemoveLastPoint(), "can't remove last point");
    m_points.pop_back();
  }

  std::optional<std::tuple<vm::vec3, vm::vec3>> canDragPoint(
    const Model::PickResult& pickResult) const override
  {
    using namespace Model::HitFilters;

    const auto& hit = pickResult.first(type(ClipTool::PointHitType));
    if (!hit.isMatch())
    {
      return std::nullopt;
    }

    const auto index = hit.target<size_t>();
    const auto position = m_points[index].point;
    return {{position, hit.hitPoint() - position}};
  }

  void beginDragPoint(const Model::PickResult& pickResult) override
  {
    using namespace Model::HitFilters;

    const auto& hit = pickResult.first(type(ClipTool::PointHitType));
    assert(hit.isMatch());

    const auto dragIndex = hit.target<size_t>();
    m_dragState = DragState{dragIndex, m_points[dragIndex]};
  }

  void beginDragLastPoint() override
  {
    ensure(hasPoints(), "invalid numPoints");
    m_dragState = DragState{m_points.size() - 1, m_points.back()};
  }

  bool dragPoint(
    const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors) override
  {
    ensure(m_dragState, "Clip tool is dragging");

    // Don't allow to drag a point onto another point!
    for (size_t i = 0; i < m_points.size(); ++i)
    {
      if (
        m_dragState->index != i
        && vm::is_equal(m_points[i].point, newPosition, vm::C::almost_zero()))
      {
        return false;
      }
    }

    if (m_points.size() == 3)
    {
      const auto index0 = m_dragState->index + 1 % 3;
      const auto index1 = m_dragState->index + 2 % 3;
      if (vm::is_colinear(m_points[index0].point, m_points[index1].point, newPosition))
      {
        return false;
      }
    }

    if (helpVectors.empty())
    {
      m_points[m_dragState->index] =
        ClipPoint{newPosition, m_points[m_dragState->index].helpVectors};
    }
    else
    {
      m_points[m_dragState->index] = ClipPoint{newPosition, helpVectors};
    }
    return true;
  }

  void endDragPoint() override { m_dragState = std::nullopt; }

  void cancelDragPoint() override
  {
    ensure(m_dragState, "Clip tool is dragging");
    m_points[m_dragState->index] = m_dragState->originalPoint;
    m_dragState = std::nullopt;
  }

  bool setFace(const Model::BrushFaceHandle& /* faceHandle */) override { return false; }

  void reset() override { m_points.clear(); }

  std::vector<vm::vec3> getPoints() const override
  {
    auto result = kdl::vec_transform(m_points, [](const auto& p) { return p.point; });
    if (const auto thirdPoint = computeThirdPoint())
    {
      result = kdl::vec_push_back(std::move(result), *thirdPoint);
    }
    return result;
  }

private:
  void renderPoints(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
  {
    auto renderService = Renderer::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
    renderService.setShowOccludedObjects();

    if (m_points.size() > 1)
    {
      renderService.renderLine(
        vm::vec3f{m_points[0].point}, vm::vec3f{m_points[1].point});

      if (m_points.size() > 2)
      {
        renderService.renderLine(
          vm::vec3f{m_points[1].point}, vm::vec3f{m_points[2].point});
        renderService.renderLine(
          vm::vec3f{m_points[2].point}, vm::vec3f{m_points[0].point});
      }
    }

    renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
    renderService.setBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));

    for (size_t i = 0; i < m_points.size(); ++i)
    {
      const auto& point = m_points[i].point;
      renderService.renderHandle(vm::vec3f{point});
      renderService.renderString(
        fmt::format("{}: {}", i + 1, fmt::streamed(point)), vm::vec3f{point});
    }
  }

  void renderHighlight(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const Model::PickResult& pickResult)
  {
    if (m_dragState)
    {
      renderHighlight(renderContext, renderBatch, m_dragState->index);
    }
    else
    {
      using namespace Model::HitFilters;

      const auto& hit = pickResult.first(type(ClipTool::PointHitType));
      if (hit.isMatch())
      {
        const auto index = hit.target<size_t>();
        renderHighlight(renderContext, renderBatch, index);
      }
    }
  }

  void renderHighlight(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const size_t index)
  {
    auto renderService = Renderer::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
    renderService.renderHandleHighlight(vm::vec3f(m_points[index].point));
  }
};

class FaceClipStrategy : public ClipStrategy
{
private:
  std::optional<Model::BrushFaceHandle> m_faceHandle;

public:
  void pick(const vm::ray3&, const Renderer::Camera&, Model::PickResult&) const override
  {
  }

  void render(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const Model::PickResult&) override
  {
    if (m_faceHandle)
    {
      auto renderService = Renderer::RenderService{renderContext, renderBatch};

      const auto positions = kdl::vec_transform(
        m_faceHandle->face().vertices(),
        [](const auto& vertex) { return vm::vec3f{vertex->position()}; });

      renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
      renderService.renderPolygonOutline(positions);

      renderService.setForegroundColor(pref(Preferences::ClipFaceColor));
      renderService.renderFilledPolygon(positions);
    }
  }

  void renderFeedback(
    Renderer::RenderContext&, Renderer::RenderBatch&, const vm::vec3&) const override
  {
  }

  vm::vec3 getHelpVector() const { return vm::vec3::zero(); }

  std::optional<vm::vec3> computeThirdPoint() const override { return std::nullopt; }

  bool canClip() const override { return m_faceHandle.has_value(); }
  bool hasPoints() const override { return false; }
  bool canAddPoint(const vm::vec3&) const override { return false; }
  void addPoint(const vm::vec3&, std::vector<vm::vec3>) override {}
  bool canRemoveLastPoint() const override { return false; }
  void removeLastPoint() override {}

  std::optional<std::tuple<vm::vec3, vm::vec3>> canDragPoint(
    const Model::PickResult&) const override
  {
    return std::nullopt;
  }
  void beginDragPoint(const Model::PickResult&) override {}
  void beginDragLastPoint() override {}
  bool dragPoint(
    const vm::vec3& /* newPosition */,
    const std::vector<vm::vec3>& /* helpVectors */) override
  {
    return false;
  }
  void endDragPoint() override {}
  void cancelDragPoint() override {}

  bool setFace(const Model::BrushFaceHandle& faceHandle) override
  {
    m_faceHandle = faceHandle;
    return true;
  }

  void reset() override { m_faceHandle = std::nullopt; }

  std::vector<vm::vec3> getPoints() const override
  {
    if (m_faceHandle)
    {
      const auto& points = m_faceHandle->face().points();
      return {points.begin(), points.end()};
    }

    return {};
  }
};

} // namespace

ClipTool::ClipTool(std::weak_ptr<MapDocument> document)
  : Tool{false}
  , m_document{std::move(document)}
  , m_remainingBrushRenderer{std::make_unique<Renderer::BrushRenderer>()}
  , m_clippedBrushRenderer{std::make_unique<Renderer::BrushRenderer>()}
{
}

ClipTool::~ClipTool()
{
  kdl::map_clear_and_delete(m_frontBrushes);
  kdl::map_clear_and_delete(m_backBrushes);
}

const Grid& ClipTool::grid() const
{
  return kdl::mem_lock(m_document)->grid();
}

void ClipTool::toggleSide()
{
  if (canClip())
  {
    switch (m_clipSide)
    {
    case ClipSide::Front:
      m_clipSide = ClipSide::Both;
      break;
    case ClipSide::Both:
      m_clipSide = ClipSide::Back;
      break;
    case ClipSide::Back:
      m_clipSide = ClipSide::Front;
      break;
    }
    update();
  }
}

void ClipTool::pick(
  const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult)
{
  if (m_strategy)
  {
    m_strategy->pick(pickRay, camera, pickResult);
  }
}

void ClipTool::render(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const Model::PickResult& pickResult)
{
  renderBrushes(renderContext, renderBatch);
  renderStrategy(renderContext, renderBatch, pickResult);
}

void ClipTool::renderBrushes(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
{
  m_remainingBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
  m_remainingBrushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
  m_remainingBrushRenderer->setShowEdges(true);
  m_remainingBrushRenderer->setShowOccludedEdges(true);
  m_remainingBrushRenderer->setOccludedEdgeColor(Color(
    pref(Preferences::SelectedEdgeColor), pref(Preferences::OccludedSelectedEdgeAlpha)));
  m_remainingBrushRenderer->setTint(true);
  m_remainingBrushRenderer->setTintColor(pref(Preferences::SelectedFaceColor));
  m_remainingBrushRenderer->render(renderContext, renderBatch);

  m_clippedBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
  m_clippedBrushRenderer->setEdgeColor(Color(pref(Preferences::EdgeColor), 0.5f));
  m_clippedBrushRenderer->setShowEdges(true);
  m_clippedBrushRenderer->setTint(false);
  m_clippedBrushRenderer->setForceTransparent(true);
  m_clippedBrushRenderer->setTransparencyAlpha(0.5f);
  m_clippedBrushRenderer->render(renderContext, renderBatch);
}

void ClipTool::renderStrategy(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const Model::PickResult& pickResult)
{
  if (m_strategy)
  {
    m_strategy->render(renderContext, renderBatch, pickResult);
  }
}

void ClipTool::renderFeedback(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const vm::vec3& point) const
{
  if (m_strategy)
  {
    m_strategy->renderFeedback(renderContext, renderBatch, point);
  }
  else
  {
    PointClipStrategy{}.renderFeedback(renderContext, renderBatch, point);
  }
}

bool ClipTool::hasBrushes() const
{
  const auto document = kdl::mem_lock(m_document);
  return document->selectedNodes().hasBrushes();
}

bool ClipTool::canClip() const
{
  return m_strategy && m_strategy->canClip();
}

void ClipTool::performClip()
{
  if (!m_dragging && canClip())
  {
    const auto ignoreNotifications = kdl::set_temp{m_ignoreNotifications};

    auto document = kdl::mem_lock(m_document);
    auto transaction = Transaction{document, "Clip Brushes"};

    // need to make a copies here so that we are not affected by the deselection
    const auto toAdd = clipBrushes();
    const auto toRemove = document->selectedNodes().nodes();
    const auto addedNodes = document->addNodes(toAdd);

    document->deselectAll();
    document->removeNodes(toRemove);
    document->selectNodes(addedNodes);
    transaction.commit();

    update();
  }
}

std::map<Model::Node*, std::vector<Model::Node*>> ClipTool::clipBrushes()
{
  auto result = std::map<Model::Node*, std::vector<Model::Node*>>{};
  if (!m_frontBrushes.empty())
  {
    if (keepFrontBrushes())
    {
      result = kdl::map_merge(result, m_frontBrushes);
      m_frontBrushes.clear();
    }
    else
    {
      kdl::map_clear_and_delete(m_frontBrushes);
    }
  }

  if (!m_backBrushes.empty())
  {
    if (keepBackBrushes())
    {
      result = kdl::map_merge(result, m_backBrushes);
      m_backBrushes.clear();
    }
    else
    {
      kdl::map_clear_and_delete(m_backBrushes);
    }
  }

  resetStrategy();
  return result;
}

vm::vec3 ClipTool::defaultClipPointPos() const
{
  auto document = kdl::mem_lock(m_document);
  return document->selectionBounds().center();
}

bool ClipTool::canAddPoint(const vm::vec3& point) const
{
  return !m_strategy || m_strategy->canAddPoint(point);
}

bool ClipTool::hasPoints() const
{
  return m_strategy && m_strategy->hasPoints();
}

void ClipTool::addPoint(const vm::vec3& point, const std::vector<vm::vec3>& helpVectors)
{
  assert(canAddPoint(point));
  if (!m_strategy)
  {
    m_strategy = std::make_unique<PointClipStrategy>();
  }

  m_strategy->addPoint(point, helpVectors);
  update();
}

bool ClipTool::canRemoveLastPoint() const
{
  return m_strategy && m_strategy->canRemoveLastPoint();
}

bool ClipTool::removeLastPoint()
{
  if (canRemoveLastPoint())
  {
    m_strategy->removeLastPoint();
    update();
    return true;
  }

  return false;
}

std::optional<std::tuple<vm::vec3, vm::vec3>> ClipTool::beginDragPoint(
  const Model::PickResult& pickResult)
{
  assert(!m_dragging);
  if (m_strategy)
  {
    const auto pointAndOffset = m_strategy->canDragPoint(pickResult);
    if (pointAndOffset)
    {
      m_strategy->beginDragPoint(pickResult);
      m_dragging = true;
      return pointAndOffset;
    }
  }

  return std::nullopt;
}

void ClipTool::beginDragLastPoint()
{
  assert(!m_dragging);
  ensure(m_strategy, "strategy is not null");
  m_strategy->beginDragLastPoint();
  m_dragging = true;
}

bool ClipTool::dragPoint(
  const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors)
{
  assert(m_dragging);
  ensure(m_strategy, "strategy is not null");
  if (!m_strategy->dragPoint(newPosition, helpVectors))
  {
    return false;
  }

  update();
  return true;
}

void ClipTool::endDragPoint()
{
  assert(m_dragging);
  ensure(m_strategy, "strategy is not null");
  m_strategy->endDragPoint();
  m_dragging = false;
  refreshViews();
}

void ClipTool::cancelDragPoint()
{
  assert(m_dragging);
  ensure(m_strategy, "strategy is not null");
  m_strategy->cancelDragPoint();
  m_dragging = false;
  refreshViews();
}

void ClipTool::setFace(const Model::BrushFaceHandle& faceHandle)
{
  m_strategy = std::make_unique<FaceClipStrategy>();
  m_strategy->setFace(faceHandle);
  update();
}

bool ClipTool::reset()
{
  if (m_strategy)
  {
    resetStrategy();
    return true;
  }

  return false;
}

void ClipTool::resetStrategy()
{
  m_strategy.reset();
  update();
}

void ClipTool::update()
{
  clearRenderers();
  clearBrushes();

  updateBrushes();
  updateRenderers();

  refreshViews();
}

void ClipTool::clearBrushes()
{
  kdl::map_clear_and_delete(m_frontBrushes);
  kdl::map_clear_and_delete(m_backBrushes);
}

void ClipTool::updateBrushes()
{
  auto document = kdl::mem_lock(m_document);

  const auto& brushNodes = document->selectedNodes().brushes();
  const auto& worldBounds = document->worldBounds();

  const auto clip =
    [&](auto* node, const auto& p1, const auto& p2, const auto& p3, auto& brushMap) {
      auto brush = node->brush();
      Model::BrushFace::create(
        p1,
        p2,
        p3,
        Model::BrushFaceAttributes(document->currentTextureName()),
        document->world()->mapFormat())
        .and_then([&](Model::BrushFace&& clipFace) {
          setFaceAttributes(brush.faces(), clipFace);
          return brush.clip(worldBounds, std::move(clipFace));
        })
        .transform([&]() {
          brushMap[node->parent()].push_back(new Model::BrushNode(std::move(brush)));
        })
        .transform_error(
          [&](auto e) { document->error() << "Could not clip brush: " << e.msg; });
    };

  if (canClip())
  {
    const auto points = m_strategy->getPoints();
    ensure(points.size() == 3, "invalid number of points");

    for (auto* brushNode : brushNodes)
    {
      clip(brushNode, points[0], points[1], points[2], m_frontBrushes);
      clip(brushNode, points[0], points[2], points[1], m_backBrushes);
    }
  }
  else
  {
    for (auto* brushNode : brushNodes)
    {
      auto* parent = brushNode->parent();
      m_frontBrushes[parent].push_back(new Model::BrushNode{brushNode->brush()});
    }
  }
}

void ClipTool::setFaceAttributes(
  const std::vector<Model::BrushFace>& faces, Model::BrushFace& toSet) const
{
  ensure(!faces.empty(), "no faces");

  auto faceIt = std::begin(faces);
  auto faceEnd = std::end(faces);
  auto bestMatch = faceIt++;

  while (faceIt != faceEnd)
  {
    const auto& face = *faceIt;

    const auto bestDiff = bestMatch->boundary().normal - toSet.boundary().normal;
    const auto curDiff = face.boundary().normal - toSet.boundary().normal;
    if (vm::squared_length(curDiff) < vm::squared_length(bestDiff))
    {
      bestMatch = faceIt;
    }

    ++faceIt;
  }

  toSet.setAttributes(*bestMatch);
}

void ClipTool::clearRenderers()
{
  m_remainingBrushRenderer->clear();
  m_clippedBrushRenderer->clear();
}

void ClipTool::updateRenderers()
{
  if (canClip())
  {
    if (keepFrontBrushes())
    {
      addBrushesToRenderer(m_frontBrushes, *m_remainingBrushRenderer);
    }
    else
    {
      addBrushesToRenderer(m_frontBrushes, *m_clippedBrushRenderer);
    }

    if (keepBackBrushes())
    {
      addBrushesToRenderer(m_backBrushes, *m_remainingBrushRenderer);
    }
    else
    {
      addBrushesToRenderer(m_backBrushes, *m_clippedBrushRenderer);
    }
  }
  else
  {
    addBrushesToRenderer(m_frontBrushes, *m_remainingBrushRenderer);
    addBrushesToRenderer(m_backBrushes, *m_remainingBrushRenderer);
  }
}

void ClipTool::addBrushesToRenderer(
  const std::map<Model::Node*, std::vector<Model::Node*>>& map,
  Renderer::BrushRenderer& renderer)
{
  for (const auto& [parent, nodes] : map)
  {
    for (auto* node : nodes)
    {
      node->accept(kdl::overload(
        [](const Model::WorldNode*) {},
        [](const Model::LayerNode*) {},
        [](const Model::GroupNode*) {},
        [](const Model::EntityNode*) {},
        [&](Model::BrushNode* brush) { renderer.addBrush(brush); },
        [](Model::PatchNode*) {}));
    }
  }
}

bool ClipTool::keepFrontBrushes() const
{
  return m_clipSide != ClipSide::Back;
}

bool ClipTool::keepBackBrushes() const
{
  return m_clipSide != ClipSide::Front;
}

bool ClipTool::doActivate()
{
  auto document = kdl::mem_lock(m_document);
  if (!document->selectedNodes().hasOnlyBrushes())
  {
    return false;
  }

  connectObservers();
  resetStrategy();
  return true;
}

bool ClipTool::doDeactivate()
{
  m_notifierConnection.disconnect();

  m_strategy.reset();
  clearRenderers();
  clearBrushes();

  return true;
}

bool ClipTool::doRemove()
{
  return removeLastPoint();
}

void ClipTool::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection +=
    document->selectionDidChangeNotifier.connect(this, &ClipTool::selectionDidChange);
  m_notifierConnection +=
    document->nodesWillChangeNotifier.connect(this, &ClipTool::nodesWillChange);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &ClipTool::nodesDidChange);
  m_notifierConnection +=
    document->brushFacesDidChangeNotifier.connect(this, &ClipTool::brushFacesDidChange);
}

void ClipTool::selectionDidChange(const Selection&)
{
  if (!m_ignoreNotifications)
  {
    update();
  }
}

void ClipTool::nodesWillChange(const std::vector<Model::Node*>&)
{
  if (!m_ignoreNotifications)
  {
    update();
  }
}

void ClipTool::nodesDidChange(const std::vector<Model::Node*>&)
{
  if (!m_ignoreNotifications)
  {
    update();
  }
}

void ClipTool::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>&)
{
  if (!m_ignoreNotifications)
  {
    update();
  }
}

} // namespace TrenchBroom::View
