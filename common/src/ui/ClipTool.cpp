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

#include "ClipTool.h"

#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PickResult.h"
#include "mdl/SelectionChange.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "render/BrushRenderer.h"
#include "render/Camera.h"
#include "render/RenderService.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"

#include "kd/map_utils.h"
#include "kd/optional_utils.h"
#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/set_temp.h"
#include "kd/vector_utils.h"

#include "vm/ray.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <optional>
#include <ranges>

namespace tb::ui
{

const mdl::HitType::Type ClipTool::PointHitType = mdl::HitType::freeType();

class ClipStrategy
{
public:
  virtual ~ClipStrategy() = default;

  virtual void pick(
    const vm::ray3d& pickRay,
    const render::Camera& camera,
    mdl::PickResult& pickResult) const = 0;
  virtual void render(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const mdl::PickResult& pickResult) = 0;
  virtual void renderFeedback(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const vm::vec3d& point) const = 0;

  virtual std::optional<vm::vec3d> computeThirdPoint() const = 0;

  virtual bool canClip() const = 0;
  virtual bool hasPoints() const = 0;
  virtual bool canAddPoint(const vm::vec3d& point) const = 0;
  virtual void addPoint(const vm::vec3d& point, std::vector<vm::vec3d> helpVectors) = 0;
  virtual bool canRemoveLastPoint() const = 0;
  virtual void removeLastPoint() = 0;

  virtual std::optional<std::tuple<vm::vec3d, vm::vec3d>> canDragPoint(
    const mdl::PickResult& pickResult) const = 0;
  virtual void beginDragPoint(const mdl::PickResult& pickResult) = 0;
  virtual void beginDragLastPoint() = 0;
  virtual bool dragPoint(
    const vm::vec3d& newPosition, const std::vector<vm::vec3d>& helpVectors) = 0;
  virtual void endDragPoint() = 0;
  virtual void cancelDragPoint() = 0;

  virtual bool setFace(const mdl::BrushFaceHandle& faceHandle) = 0;
  virtual void reset() = 0;
  virtual std::vector<vm::vec3d> getPoints() const = 0;
};

namespace
{

class PointClipStrategy : public ClipStrategy
{
private:
  struct ClipPoint
  {
    vm::vec3d point;
    std::vector<vm::vec3d> helpVectors;
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
    const vm::ray3d& pickRay,
    const render::Camera& camera,
    mdl::PickResult& pickResult) const override
  {
    for (size_t i = 0; i < m_points.size(); ++i)
    {
      const auto& point = m_points[i].point;
      if (
        const auto distance = camera.pickPointHandle(
          pickRay, point, static_cast<double>(pref(Preferences::HandleRadius))))
      {
        const auto hitPoint = vm::point_at_distance(pickRay, *distance);
        pickResult.addHit(mdl::Hit{ClipTool::PointHitType, *distance, hitPoint, i});
      }
    }
  }

  void render(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const mdl::PickResult& pickResult) override
  {
    renderPoints(renderContext, renderBatch);
    renderHighlight(renderContext, renderBatch, pickResult);
  }

  void renderFeedback(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const vm::vec3d& point) const override
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
    renderService.renderHandle(vm::vec3f{point});
  }

  std::optional<vm::vec3d> computeThirdPoint() const override
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

  vm::vec3d computeHelpVector() const
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

    const auto first = std::ranges::max_element(counts);
    const auto firstIndex = std::distance(std::begin(counts), first);

    const auto next = std::ranges::max_element(std::next(first), std::end(counts));
    if (next == std::end(counts) || *first > *next)
    {
      return vm::vec3d::axis(size_t(firstIndex % 3));
    }

    const auto nextIndex = std::distance(std::begin(counts), next);

    // two counts are equal
    if (firstIndex % 3 == 2 || nextIndex % 3 == 2)
    {
      // prefer the Z axis if possible:
      return vm::vec3d{0, 0, 1};
    }

    // Z axis cannot win, so X and Y axis are a tie, prefer the X axis:
    return vm::vec3d{1, 0, 0};
  }

  std::vector<vm::vec3d> combineHelpVectors() const
  {
    return m_points
           | std::views::transform([](const auto& point) { return point.helpVectors; })
           | std::views::join | kdl::ranges::to<std::vector>();
  }

  bool canClip() const override { return m_points.size() == 3 || computeThirdPoint(); }

  bool hasPoints() const override { return !m_points.empty(); }

  bool canAddPoint(const vm::vec3d& point) const override
  {
    return (m_points.size() < 2
            || (m_points.size() == 2 && !vm::is_colinear(m_points[0].point, m_points[1].point, point)))
           && std::ranges::none_of(m_points, [&](const auto& p) {
                return vm::is_equal(p.point, point, vm::Cd::almost_zero());
              });
  }

  void addPoint(const vm::vec3d& point, std::vector<vm::vec3d> helpVectors) override
  {
    m_points.push_back(ClipPoint{point, std::move(helpVectors)});
  }

  bool canRemoveLastPoint() const override { return hasPoints(); }

  void removeLastPoint() override
  {
    contract_pre(canRemoveLastPoint());

    m_points.pop_back();
  }

  std::optional<std::tuple<vm::vec3d, vm::vec3d>> canDragPoint(
    const mdl::PickResult& pickResult) const override
  {
    using namespace mdl::HitFilters;

    const auto& hit = pickResult.first(type(ClipTool::PointHitType));
    if (!hit.isMatch())
    {
      return std::nullopt;
    }

    const auto index = hit.target<size_t>();
    const auto position = m_points[index].point;
    return {{position, hit.hitPoint()}};
  }

  void beginDragPoint(const mdl::PickResult& pickResult) override
  {
    using namespace mdl::HitFilters;

    const auto& hit = pickResult.first(type(ClipTool::PointHitType));
    contract_assert(hit.isMatch());

    const auto dragIndex = hit.target<size_t>();
    m_dragState = DragState{dragIndex, m_points[dragIndex]};
  }

  void beginDragLastPoint() override
  {
    contract_pre(hasPoints());

    m_dragState = DragState{m_points.size() - 1, m_points.back()};
  }

  bool dragPoint(
    const vm::vec3d& newPosition, const std::vector<vm::vec3d>& helpVectors) override
  {
    contract_pre(m_dragState != std::nullopt);

    // Don't allow to drag a point onto another point!
    for (size_t i = 0; i < m_points.size(); ++i)
    {
      if (
        m_dragState->index != i
        && vm::is_equal(m_points[i].point, newPosition, vm::Cd::almost_zero()))
      {
        return false;
      }
    }

    if (m_points.size() == 3)
    {
      const auto index0 = (m_dragState->index + 1) % 3;
      const auto index1 = (m_dragState->index + 2) % 3;
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
    contract_pre(m_dragState != std::nullopt);

    m_points[m_dragState->index] = m_dragState->originalPoint;
    m_dragState = std::nullopt;
  }

  bool setFace(const mdl::BrushFaceHandle& /* faceHandle */) override { return false; }

  void reset() override { m_points.clear(); }

  std::vector<vm::vec3d> getPoints() const override
  {
    auto result = m_points | std::views::transform([](const auto& p) { return p.point; })
                  | kdl::ranges::to<std::vector>();
    if (const auto thirdPoint = computeThirdPoint())
    {
      result = kdl::vec_push_back(std::move(result), *thirdPoint);
    }
    return result;
  }

private:
  void renderPoints(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch)
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
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
      renderService.renderString(toString(point).toStdString(), vm::vec3f{point});
    }
  }

  void renderHighlight(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const mdl::PickResult& pickResult)
  {
    if (m_dragState)
    {
      renderHighlight(renderContext, renderBatch, m_dragState->index);
    }
    else
    {
      using namespace mdl::HitFilters;

      const auto& hit = pickResult.first(type(ClipTool::PointHitType));
      if (hit.isMatch())
      {
        const auto index = hit.target<size_t>();
        renderHighlight(renderContext, renderBatch, index);
      }
    }
  }

  void renderHighlight(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const size_t index)
  {
    if (index < m_points.size())
    {
      auto renderService = render::RenderService{renderContext, renderBatch};
      renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
      renderService.renderHandleHighlight(vm::vec3f(m_points[index].point));
    }
  }
};

class FaceClipStrategy : public ClipStrategy
{
private:
  std::optional<mdl::BrushFaceHandle> m_faceHandle;

public:
  void pick(const vm::ray3d&, const render::Camera&, mdl::PickResult&) const override {}

  void render(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const mdl::PickResult&) override
  {
    if (m_faceHandle)
    {
      auto renderService = render::RenderService{renderContext, renderBatch};

      const auto positions = m_faceHandle->face().vertices()
                             | std::views::transform([](const auto& vertex) {
                                 return vm::vec3f{vertex->position()};
                               })
                             | kdl::ranges::to<std::vector>();

      renderService.setForegroundColor(pref(Preferences::ClipHandleColor));
      renderService.renderPolygonOutline(positions);

      renderService.setForegroundColor(pref(Preferences::ClipFaceColor));
      renderService.renderFilledPolygon(positions);
    }
  }

  void renderFeedback(
    render::RenderContext&, render::RenderBatch&, const vm::vec3d&) const override
  {
  }

  vm::vec3d getHelpVector() const { return vm::vec3d{0, 0, 0}; }

  std::optional<vm::vec3d> computeThirdPoint() const override { return std::nullopt; }

  bool canClip() const override { return m_faceHandle.has_value(); }
  bool hasPoints() const override { return false; }
  bool canAddPoint(const vm::vec3d&) const override { return false; }
  void addPoint(const vm::vec3d&, std::vector<vm::vec3d>) override {}
  bool canRemoveLastPoint() const override { return false; }
  void removeLastPoint() override {}

  std::optional<std::tuple<vm::vec3d, vm::vec3d>> canDragPoint(
    const mdl::PickResult&) const override
  {
    return std::nullopt;
  }
  void beginDragPoint(const mdl::PickResult&) override {}
  void beginDragLastPoint() override {}
  bool dragPoint(
    const vm::vec3d& /* newPosition */,
    const std::vector<vm::vec3d>& /* helpVectors */) override
  {
    return false;
  }
  void endDragPoint() override {}
  void cancelDragPoint() override {}

  bool setFace(const mdl::BrushFaceHandle& faceHandle) override
  {
    m_faceHandle = faceHandle;
    return true;
  }

  void reset() override { m_faceHandle = std::nullopt; }

  std::vector<vm::vec3d> getPoints() const override
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

ClipTool::ClipTool(MapDocument& document)
  : Tool{false}
  , m_document{document}
  , m_remainingBrushRenderer{std::make_unique<render::BrushRenderer>()}
  , m_clippedBrushRenderer{std::make_unique<render::BrushRenderer>()}
{
}

ClipTool::~ClipTool()
{
  kdl::map_clear_and_delete(m_frontBrushes);
  kdl::map_clear_and_delete(m_backBrushes);
}

const mdl::Grid& ClipTool::grid() const
{
  return m_document.map().grid();
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
  const vm::ray3d& pickRay, const render::Camera& camera, mdl::PickResult& pickResult)
{
  if (m_strategy)
  {
    m_strategy->pick(pickRay, camera, pickResult);
  }
}

void ClipTool::render(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const mdl::PickResult& pickResult)
{
  renderBrushes(renderContext, renderBatch);
  renderStrategy(renderContext, renderBatch, pickResult);
}

void ClipTool::renderBrushes(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_remainingBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
  m_remainingBrushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
  m_remainingBrushRenderer->setShowEdges(true);
  m_remainingBrushRenderer->setShowOccludedEdges(true);
  m_remainingBrushRenderer->setOccludedEdgeColor(RgbaF{
    pref(Preferences::SelectedEdgeColor).to<RgbF>(),
    pref(Preferences::OccludedSelectedEdgeAlpha)});
  m_remainingBrushRenderer->setTint(true);
  m_remainingBrushRenderer->setTintColor(pref(Preferences::SelectedFaceColor));
  m_remainingBrushRenderer->render(renderContext, renderBatch);

  m_clippedBrushRenderer->setFaceColor(pref(Preferences::FaceColor));
  m_clippedBrushRenderer->setEdgeColor(
    RgbaF{pref(Preferences::EdgeColor).to<RgbF>(), 0.5f});
  m_clippedBrushRenderer->setShowEdges(true);
  m_clippedBrushRenderer->setTint(false);
  m_clippedBrushRenderer->setForceTransparent(true);
  m_clippedBrushRenderer->setTransparencyAlpha(0.5f);
  m_clippedBrushRenderer->render(renderContext, renderBatch);
}

void ClipTool::renderStrategy(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const mdl::PickResult& pickResult)
{
  if (m_strategy)
  {
    m_strategy->render(renderContext, renderBatch, pickResult);
  }
}

void ClipTool::renderFeedback(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::vec3d& point) const
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
  return m_document.map().selection().hasBrushes();
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

    auto& map = m_document.map();
    auto transaction = mdl::Transaction{map, "Clip Brushes"};

    // need to make a copies here so that we are not affected by the deselection
    const auto toAdd = clipBrushes();
    const auto toRemove = map.selection().nodes;
    const auto addedNodes = addNodes(map, toAdd);

    deselectAll(map);
    removeNodes(map, toRemove);
    selectNodes(map, addedNodes);
    transaction.commit();

    update();
  }
}

std::map<mdl::Node*, std::vector<mdl::Node*>> ClipTool::clipBrushes()
{
  auto result = std::map<mdl::Node*, std::vector<mdl::Node*>>{};
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

std::optional<vm::vec3d> ClipTool::defaultClipPointPos() const
{
  return m_document.map().selectionBounds()
         | kdl::optional_transform([](const auto& bounds) { return bounds.center(); });
}

bool ClipTool::canAddPoint(const vm::vec3d& point) const
{
  return !m_strategy || m_strategy->canAddPoint(point);
}

bool ClipTool::hasPoints() const
{
  return m_strategy && m_strategy->hasPoints();
}

void ClipTool::addPoint(const vm::vec3d& point, const std::vector<vm::vec3d>& helpVectors)
{
  contract_pre(canAddPoint(point));

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

std::optional<std::tuple<vm::vec3d, vm::vec3d>> ClipTool::beginDragPoint(
  const mdl::PickResult& pickResult)
{
  contract_pre(!m_dragging);

  if (m_strategy)
  {
    const auto handlePositionAndHitPoint = m_strategy->canDragPoint(pickResult);
    if (handlePositionAndHitPoint)
    {
      m_strategy->beginDragPoint(pickResult);
      m_dragging = true;
      return handlePositionAndHitPoint;
    }
  }

  return std::nullopt;
}

void ClipTool::beginDragLastPoint()
{
  contract_pre(!m_dragging);
  contract_pre(m_strategy != nullptr);

  m_strategy->beginDragLastPoint();
  m_dragging = true;
}

bool ClipTool::dragPoint(
  const vm::vec3d& newPosition, const std::vector<vm::vec3d>& helpVectors)
{
  contract_pre(m_dragging);
  contract_pre(m_strategy != nullptr);

  if (!m_strategy->dragPoint(newPosition, helpVectors))
  {
    return false;
  }

  update();
  return true;
}

void ClipTool::endDragPoint()
{
  contract_pre(m_dragging);
  contract_pre(m_strategy != nullptr);

  m_strategy->endDragPoint();
  m_dragging = false;
  refreshViews();
}

void ClipTool::cancelDragPoint()
{
  contract_pre(m_dragging);
  contract_pre(m_strategy != nullptr);

  m_strategy->cancelDragPoint();
  m_dragging = false;
  refreshViews();
}

void ClipTool::setFace(const mdl::BrushFaceHandle& faceHandle)
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
  auto& map = m_document.map();
  const auto& brushNodes = map.selection().brushes;
  const auto& worldBounds = map.worldBounds();

  const auto clip =
    [&](auto* node, const auto& p1, const auto& p2, const auto& p3, auto& brushMap) {
      auto brush = node->brush();
      mdl::BrushFace::create(
        p1,
        p2,
        p3,
        mdl::BrushFaceAttributes(map.currentMaterialName()),
        map.world()->mapFormat())
        | kdl::and_then([&](mdl::BrushFace&& clipFace) {
            setFaceAttributes(brush.faces(), clipFace);
            return brush.clip(worldBounds, std::move(clipFace));
          })
        | kdl::transform([&]() {
            brushMap[node->parent()].push_back(new mdl::BrushNode(std::move(brush)));
          })
        | kdl::transform_error(
          [&](auto e) { map.logger().error() << "Could not clip brush: " << e.msg; });
    };

  if (canClip())
  {
    const auto points = m_strategy->getPoints();
    contract_assert(points.size() == 3);

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
      m_frontBrushes[parent].push_back(new mdl::BrushNode{brushNode->brush()});
    }
  }
}

void ClipTool::setFaceAttributes(
  const std::vector<mdl::BrushFace>& faces, mdl::BrushFace& toSet) const
{
  contract_pre(!faces.empty());

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
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& map,
  render::BrushRenderer& renderer)
{
  for (const auto& [parent, nodes] : map)
  {
    for (auto* node : nodes)
    {
      node->accept(kdl::overload(
        [](const mdl::WorldNode*) {},
        [](const mdl::LayerNode*) {},
        [](const mdl::GroupNode*) {},
        [](const mdl::EntityNode*) {},
        [&](mdl::BrushNode* brush) { renderer.addBrush(brush); },
        [](mdl::PatchNode*) {}));
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
  if (!m_document.map().selection().hasOnlyBrushes())
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
  m_notifierConnection +=
    m_document.documentDidChangeNotifier.connect(this, &ClipTool::documentDidChange);
}

void ClipTool::documentDidChange()
{
  if (!m_ignoreNotifications)
  {
    update();
  }
}

} // namespace tb::ui
