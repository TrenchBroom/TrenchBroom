/*
 Copyright (C) 2026 Jackson Palmer
 Copyright (C) 2026 Kristian Duske

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

#include "ui/SweepTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/Camera.h"
#include "mdl/BrushFace.h" // IWYU pragma: keep
#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/Grid.h"
#include "mdl/Hit.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/Transaction.h"
#include "render/BrushRenderer.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/RenderService.h"
#include "ui/MapDocument.h"

#include "kd/ranges/concat_view.h"
#include "kd/ranges/to.h"
#include "kd/vector_utils.h"

#include "vm/bbox.h"
#include "vm/mat.h"
#include "vm/polygon_io.h" // IWYU pragma: keep
#include "vm/quat.h"
#include "vm/quat_io.h" // IWYU pragma: keep
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <array>
#include <map>
#include <numeric>
#include <optional>
#include <ranges>

namespace tb::ui
{
namespace
{

vm::vec3d clampScale(const vm::vec3d& factors)
{
  // a factor at zero collapses the profile, a negative one turns it inside-out
  return vm::max(factors, vm::vec3d::fill(SweepTool::MinScaleFactor));
}

auto initializeFaces(const auto& faces)
{
  return faces | std::views::transform([](const auto& faceHandle) {
           const auto& face = faceHandle.face();
           auto polygon = face.polygon();
           return SweepFace{std::move(polygon), faceHandle.node()->parent()};
         })
         | kdl::ranges::to<std::vector>();
}

auto initializeCenter(const auto& faces)
{
  return vm::bbox3d::merge_all(
           std::begin(faces),
           std::end(faces),
           [](const auto& faceHandle) { return faceHandle.face().bounds(); })
    .center();
}

auto initializeNormal(const auto& faces)
{
  const auto normals = faces | std::views::transform([](const auto& faceHandle) {
                         const auto& face = faceHandle.face();
                         return face.normal();
                       });

  const auto sumOfAllNormals =
    std::accumulate(normals.begin(), normals.end(), vm::vec3d{0, 0, 0});

  // cancelling normals leave no forward direction; S-bend falls back to straight
  return vm::squared_length(sumOfAllNormals) > vm::Cd::almost_zero()
           ? vm::normalize(sumOfAllNormals)
           : vm::vec3d{0, 0, 0};
}

vm::vec3d initializeScaleBaseVector(
  const std::vector<SweepFace>& sourceFaces, const vm::vec3d& center)
{
  if (sourceFaces.empty())
  {
    return vm::vec3d{0, 0, 0};
  }

  const auto getVertices = [](const auto& sourceFace) {
    return sourceFace.polygon.vertices();
  };

  const auto getScaleBaseVector = [&](const auto& vertex) { return vertex - center; };

  const auto scaleBaseVectors =
    sourceFaces | std::views::transform(getVertices) | std::views::join
    | std::views::transform(getScaleBaseVector) | kdl::ranges::to<std::vector>();

  const auto iLongestScaleBaseVector =
    std::ranges::max_element(scaleBaseVectors, std::less<double>{}, [](const auto& arm) {
      return vm::squared_length(arm);
    });

  contract_assert(iLongestScaleBaseVector != scaleBaseVectors.end());
  return *iLongestScaleBaseVector;
}

auto initializeSweepSource(const auto& faces)
{
  auto sourceFaces = initializeFaces(faces);
  const auto center = initializeCenter(faces);
  const auto normal = initializeNormal(faces);
  const auto scaleBaseVector = initializeScaleBaseVector(sourceFaces, center);

  return SweepSource{
    std::move(sourceFaces),
    center,
    normal,
    scaleBaseVector,
  };
}

vm::vec3d scaleArmAtCap(const vm::vec3d& scaleBase, const SweepTransform& transform)
{
  return transform.effectiveRotation() * scaleBase;
}

class SweepRingDragTracker : public RingDragTracker
{
private:
  SweepTool& m_tool;
  vm::quatd m_initialRotation;

public:
  explicit SweepRingDragTracker(SweepTool& tool)
    : m_tool{tool}
    , m_initialRotation{tool.transform().rotation}
  {
  }

  void apply(const vm::vec3d&, const vm::vec3d& axis, const double angle) override
  {
    auto transform = m_tool.transform();
    transform.rotation = vm::quatd{vm::normalize(axis), angle} * m_initialRotation;
    m_tool.setTransform(transform);
  }

  void end() override {}

  void cancel() override
  {
    auto transform = m_tool.transform();
    transform.rotation = m_initialRotation;
    m_tool.setTransform(transform);
  }
};

} // namespace

const mdl::HitType::Type SweepTool::ScaleHitType = mdl::HitType::freeType();

SweepTool::SweepTool(MapDocument& document)
  : Tool{false}
  , m_document{document}
  , m_brushRenderer{std::make_unique<render::BrushRenderer>()}
{
}

SweepTool::~SweepTool() = default;

bool SweepTool::doActivate()
{
  auto& map = m_document.map();
  const auto& faces = map.selection().brushFaces;
  if (faces.empty())
  {
    return false;
  }

  m_source = initializeSweepSource(faces);

  connectObservers();
  reset();

  return true;
}

bool SweepTool::doDeactivate()
{
  m_notifierConnection.disconnect();
  m_source = SweepSource{};

  m_previewBrushes.clear();
  m_brushRenderer->clear();

  refreshViews();

  return true;
}

bool SweepTool::applies() const
{
  return m_document.map().selection().hasBrushFaces();
}

const SweepTransform& SweepTool::transform() const
{
  return m_transform;
}

void SweepTool::setTransform(const SweepTransform& transform)
{
  m_transform = transform;
  m_handle.setPosition(destinationCenter());
  updateBrushes();
}

const SweepParameters& SweepTool::parameters() const
{
  return m_parameters;
}

void SweepTool::setParameters(const SweepParameters& parameters)
{
  m_parameters = parameters;
  updateBrushes();
}

vm::vec3d SweepTool::destinationCenter() const
{
  return m_transform.destinationCenter(m_source);
}

void SweepTool::setDestinationCenter(const vm::vec3d& position)
{
  auto transform = m_transform;
  transform.translation = position - m_source.center;
  setTransform(transform);
}

void SweepTool::reset()
{
  setTransform(SweepTransform{});
}

bool SweepTool::cancel()
{
  if (!m_transform.isNoOp())
  {
    reset();
    return true;
  }

  return false;
}

double SweepTool::minorHandleRadius(const gl::Camera& camera) const
{
  return m_handle.minorHandleRadius(camera);
}

void SweepTool::updateBrushes()
{
  if (active())
  {
    m_previewBrushes.clear();
    m_brushRenderer->clear();

    if (!m_source.faces.empty() && m_parameters.segments > 0 && !m_transform.isNoOp())
    {
      m_previewBrushes =
        generateSweepBrushes(m_document.map(), m_source, m_transform, m_parameters);

      for (const auto& [parent, brushNodes] : m_previewBrushes)
      {
        for (auto& brushNode : brushNodes)
        {
          m_brushRenderer->addBrush(*brushNode);
        }
      }
    }

    refreshViews();
  }
}

void SweepTool::commitSweep()
{
  if (!m_previewBrushes.empty())
  {
    auto nodesToAdd = m_previewBrushes | std::views::transform([](auto& entry) {
                        auto& [parent, childrenPtrs] = entry;
                        auto childrenRaw =
                          childrenPtrs | std::views::transform([](auto& childPtr) {
                            return static_cast<mdl::Node*>(childPtr.release());
                          })
                          | kdl::ranges::to<std::vector>();
                        return std::pair{parent, std::move(childrenRaw)};
                      })
                      | kdl::ranges::to<std::map>();

    m_previewBrushes.clear();
    m_brushRenderer->clear();

    auto& map = m_document.map();
    auto transaction = mdl::Transaction{map, "Sweep"};
    const auto addedNodes = mdl::addNodes(map, nodesToAdd);
    mdl::deselectAll(map);
    mdl::selectNodes(map, addedNodes);
    transaction.commit();
  }

  refreshViews();
}

void SweepTool::renderDestinationGhost(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
{
  if (m_source.faces.empty())
  {
    return;
  }

  auto renderService = render::RenderService{renderContext, renderBatch};
  renderService.setLineWidth(2.0f);

  const auto renderCaps = [&](const vm::mat4x4d& transform) {
    for (const auto& sourceFace : m_source.faces)
    {
      const auto& vertices = sourceFace.polygon.vertices();
      if (vertices.size() < 2)
      {
        continue;
      }

      const auto loop =
        kdl::views::concat(vertices, vertices | std::views::take(1))
        | std::views::transform([&](const auto& v) { return vm::vec3f{transform * v}; })
        | kdl::ranges::to<std::vector>();

      renderService.renderLineStrip(loop);
    }
  };

  // later iterations' caps are drawn as fainter echoes
  const auto capTransform = stationTransform(
    m_source, m_transform, m_parameters, 1.0, m_transform.effectiveRotation());

  auto transform = capTransform;
  for (size_t r = 0; r < m_parameters.iterations; ++r)
  {
    renderService.setForegroundColor(
      r == 0 ? pref(Preferences::HandleColor)
             : RgbaF{pref(Preferences::HandleColor).to<RgbF>(), 0.35f});
    renderCaps(transform);
    transform = transform * capTransform;
  }
}

void SweepTool::renderPreview(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
{
  m_brushRenderer->setFaceColor(pref(Preferences::FaceColor));
  m_brushRenderer->setEdgeColor(pref(Preferences::SelectedEdgeColor));
  m_brushRenderer->setShowEdges(true);
  m_brushRenderer->setShowOccludedEdges(true);
  m_brushRenderer->setOccludedEdgeColor(RgbaF{
    pref(Preferences::SelectedEdgeColor).to<RgbF>(),
    pref(Preferences::OccludedSelectedEdgeAlpha)});
  m_brushRenderer->setTint(true);
  m_brushRenderer->setTintColor(pref(Preferences::SelectedFaceColor));
  m_brushRenderer->render(renderContext, renderBatch);
}

bool SweepTool::hasScaleHandle() const
{
  return !m_source.faces.empty()
         && vm::squared_length(m_source.scaleBaseVector) > vm::Cd::almost_zero();
}

vm::vec3d SweepTool::scaleHandlePosition() const
{
  const auto scaledArm =
    m_transform.effectiveRotation() * (m_transform.scale * m_source.scaleBaseVector);
  return destinationCenter() + scaledArm;
}

void SweepTool::dragScaleHandleTo(const vm::vec3d& position)
{
  const auto arm = scaleArmAtCap(m_source.scaleBaseVector, m_transform);
  const auto armLengthSquared = vm::squared_length(arm);
  if (armLengthSquared < vm::Cd::almost_zero())
  {
    return;
  }

  // project the dragged position onto the arm to read off a uniform factor
  const auto factor = vm::dot(position - destinationCenter(), arm) / armLengthSquared;
  m_transform.scale = clampScale(vm::vec3d{factor, factor, factor});
  updateBrushes();
}

mdl::Hit SweepTool::pickScaleHandle(
  const vm::ray3d& pickRay, const gl::Camera& camera) const
{
  if (hasScaleHandle())
  {
    if (
      const auto distance = camera.pickPointHandle(
        pickRay, scaleHandlePosition(), double(pref(Preferences::HandleRadius))))
    {
      return {ScaleHitType, *distance, vm::point_at_distance(pickRay, *distance), 0};
    }
  }

  return mdl::Hit::NoHit;
}

void SweepTool::renderScaleHandle(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
{
  if (hasScaleHandle())
  {
    const auto center = destinationCenter();
    const auto position = scaleHandlePosition();

    auto renderService = render::RenderService{renderContext, renderBatch};
    // green like the Scale tool's handles
    renderService.setForegroundColor(pref(Preferences::ScaleHandleColor));
    renderService.renderLine(vm::vec3f{center}, vm::vec3f{position});
    renderService.renderHandle(vm::vec3f{position});
  }
}

void SweepTool::renderScaleHighlight(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
{
  if (hasScaleHandle())
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
    renderService.renderHandleHighlight(vm::vec3f{scaleHandlePosition()});
  }
}

Tool& SweepTool::tool()
{
  return *this;
}

const Tool& SweepTool::tool() const
{
  return *this;
}

const mdl::Grid& SweepTool::grid() const
{
  return m_document.map().grid();
}

RotateHandle& SweepTool::handle()
{
  return m_handle;
}

double SweepTool::handleSnapAngle() const
{
  return grid().angle();
}

vm::vec3d SweepTool::handleCenter() const
{
  return destinationCenter();
}

void SweepTool::setHandleCenter(const vm::vec3d& position)
{
  setDestinationCenter(position);
}

std::unique_ptr<RingDragTracker> SweepTool::beginRingDrag()
{
  return std::make_unique<SweepRingDragTracker>(*this);
}

vm::vec3d SweepTool::handlePosition() const
{
  return scaleHandlePosition();
}

void SweepTool::setHandlePosition(const vm::vec3d& position)
{
  dragScaleHandleTo(position);
}

void SweepTool::renderHighlight(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
{
  renderScaleHighlight(renderContext, renderBatch);
}

void SweepTool::connectObservers()
{
  m_notifierConnection += m_document.nodesWereRemovedNotifier.connect(
    [&](const auto& nodes) { nodesWereRemoved(nodes); });
}

void SweepTool::nodesWereRemoved(const std::vector<mdl::Node*>& nodes)
{
  // a source face's parent may be deleted while the tool is active; null it so the commit
  // falls back to the default parent
  auto mustRebuild = false;
  for (auto& sourceFace : m_source.faces)
  {
    if (
      sourceFace.parent
      && (kdl::vec_contains(nodes, sourceFace.parent) || sourceFace.parent->isDescendantOf(nodes)))
    {
      sourceFace.parent = nullptr;
      mustRebuild = true;
    }
  }
  if (mustRebuild)
  {
    updateBrushes();
  }
}

} // namespace tb::ui
