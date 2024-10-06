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

#include "EntityRenderer.h"

#include "AttrString.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityModelManager.h"
#include "mdl/EntityNode.h"
#include "render/Camera.h"
#include "render/GLVertexType.h"
#include "render/PrimType.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/RenderService.h"
#include "render/TextAnchor.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <vector>

namespace tb::render
{
namespace
{

class EntityClassnameAnchor : public TextAnchor3D
{
private:
  const mdl::EntityNode* m_entity;

public:
  explicit EntityClassnameAnchor(const mdl::EntityNode* entity)
    : m_entity{entity}
  {
  }

private:
  vm::vec3f basePosition() const override
  {
    return vm::vec3f{
      m_entity->logicalBounds().center().xy(), m_entity->logicalBounds().max.z() + 2.0};
  }

  TextAlignment::Type alignment() const override { return TextAlignment::Bottom; }
};

} // namespace

EntityRenderer::EntityRenderer(
  Logger& logger,
  mdl::EntityModelManager& entityModelManager,
  const mdl::EditorContext& editorContext)
  : m_entityModelManager{entityModelManager}
  , m_editorContext{editorContext}
  , m_modelRenderer{logger, m_entityModelManager, m_editorContext}
{
}

void EntityRenderer::invalidate()
{
  invalidateBounds();
  reloadModels();
}

void EntityRenderer::clear()
{
  m_entities.clear();
  m_pointEntityWireframeBoundsRenderer = DirectEdgeRenderer();
  m_brushEntityWireframeBoundsRenderer = DirectEdgeRenderer();
  m_solidBoundsRenderer = TriangleRenderer();
  m_modelRenderer.clear();
}

void EntityRenderer::reloadModels()
{
  m_modelRenderer.updateEntities(std::begin(m_entities), std::end(m_entities));
}

void EntityRenderer::addEntity(const mdl::EntityNode* entity)
{
  if (m_entities.insert(entity).second)
  {
    m_modelRenderer.addEntity(entity);
    invalidateBounds();
  }
}

void EntityRenderer::removeEntity(const mdl::EntityNode* entity)
{
  if (auto it = m_entities.find(entity); it != std::end(m_entities))
  {
    m_entities.erase(it);
    m_modelRenderer.removeEntity(entity);
    invalidateBounds();
  }
}

void EntityRenderer::invalidateEntity(const mdl::EntityNode* entity)
{
  m_modelRenderer.updateEntity(entity);
  invalidateBounds();
}

void EntityRenderer::invalidateEntityModels(
  const std::vector<const mdl::EntityModel*>& entityModels)
{
  const auto entityModelSet =
    std::unordered_set<const mdl::EntityModel*>{entityModels.begin(), entityModels.end()};
  for (const auto* entity : m_entities)
  {
    if (entityModelSet.contains(entity->entity().model()))
    {
      invalidateEntity(entity);
    }
  }
}

void EntityRenderer::setShowOverlays(const bool showOverlays)
{
  m_showOverlays = showOverlays;
}

void EntityRenderer::setOverlayTextColor(const Color& overlayTextColor)
{
  m_overlayTextColor = overlayTextColor;
}

void EntityRenderer::setOverlayBackgroundColor(const Color& overlayBackgroundColor)
{
  m_overlayBackgroundColor = overlayBackgroundColor;
}

void EntityRenderer::setShowOccludedOverlays(const bool showOccludedOverlays)
{
  m_showOccludedOverlays = showOccludedOverlays;
}

void EntityRenderer::setTint(const bool tint)
{
  m_tint = tint;
}

void EntityRenderer::setTintColor(const Color& tintColor)
{
  m_tintColor = tintColor;
}

void EntityRenderer::setOverrideBoundsColor(const bool overrideBoundsColor)
{
  m_overrideBoundsColor = overrideBoundsColor;
}

void EntityRenderer::setBoundsColor(const Color& boundsColor)
{
  m_boundsColor = boundsColor;
}

void EntityRenderer::setShowOccludedBounds(const bool showOccludedBounds)
{
  m_showOccludedBounds = showOccludedBounds;
}

void EntityRenderer::setOccludedBoundsColor(const Color& occludedBoundsColor)
{
  m_occludedBoundsColor = occludedBoundsColor;
}

void EntityRenderer::setShowAngles(const bool showAngles)
{
  m_showAngles = showAngles;
}

void EntityRenderer::setAngleColor(const Color& angleColor)
{
  m_angleColor = angleColor;
}

void EntityRenderer::setShowHiddenEntities(const bool showHiddenEntities)
{
  m_showHiddenEntities = showHiddenEntities;
}

void EntityRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch)
{
  if (!m_entities.empty())
  {
    renderBounds(renderContext, renderBatch);
    renderModels(renderContext, renderBatch);
    renderClassnames(renderContext, renderBatch);
    renderAngles(renderContext, renderBatch);
  }
}

void EntityRenderer::renderBounds(RenderContext& renderContext, RenderBatch& renderBatch)
{
  if (!m_boundsValid)
  {
    validateBounds();
  }

  if (renderContext.showPointEntityBounds())
  {
    renderPointEntityWireframeBounds(renderBatch);
  }

  if (renderContext.showBrushEntityBounds())
  {
    renderBrushEntityWireframeBounds(renderBatch);
  }

  if (m_showHiddenEntities || renderContext.showPointEntities())
  {
    renderSolidBounds(renderBatch);
  }
}

void EntityRenderer::renderPointEntityWireframeBounds(RenderBatch& renderBatch)
{
  if (m_showOccludedBounds)
  {
    m_pointEntityWireframeBoundsRenderer.renderOnTop(
      renderBatch, m_overrideBoundsColor, m_occludedBoundsColor);
  }

  m_pointEntityWireframeBoundsRenderer.render(
    renderBatch, m_overrideBoundsColor, m_boundsColor);
}

void EntityRenderer::renderBrushEntityWireframeBounds(RenderBatch& renderBatch)
{
  if (m_showOccludedBounds)
  {
    m_brushEntityWireframeBoundsRenderer.renderOnTop(
      renderBatch, m_overrideBoundsColor, m_occludedBoundsColor);
  }

  m_brushEntityWireframeBoundsRenderer.render(
    renderBatch, m_overrideBoundsColor, m_boundsColor);
}

void EntityRenderer::renderSolidBounds(RenderBatch& renderBatch)
{
  m_solidBoundsRenderer.setApplyTinting(m_tint);
  m_solidBoundsRenderer.setTintColor(m_tintColor);
  renderBatch.add(&m_solidBoundsRenderer);
}

void EntityRenderer::renderModels(RenderContext& renderContext, RenderBatch& renderBatch)
{
  if (
    m_showHiddenEntities
    || (renderContext.showPointEntities() && renderContext.showPointEntityModels()))
  {
    m_modelRenderer.setApplyTinting(m_tint);
    m_modelRenderer.setTintColor(m_tintColor);
    m_modelRenderer.setShowHiddenEntities(m_showHiddenEntities);
    m_modelRenderer.render(renderBatch);
  }
}

void EntityRenderer::renderClassnames(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  if (m_showOverlays && renderContext.showEntityClassnames())
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(m_overlayTextColor);
    renderService.setBackgroundColor(m_overlayBackgroundColor);

    for (const auto* entity : m_entities)
    {
      if (m_showHiddenEntities || m_editorContext.visible(entity))
      {
        if (
          !entity->containingGroup()
          || entity->containingGroup() == m_editorContext.currentGroup())
        {
          if (m_showOccludedOverlays)
          {
            renderService.setShowOccludedObjects();
          }
          else
          {
            renderService.setHideOccludedObjects();
          }

          renderService.renderString(entityString(entity), EntityClassnameAnchor{entity});
        }
      }
    }
  }
}

void EntityRenderer::renderAngles(RenderContext& renderContext, RenderBatch& renderBatch)
{
  static constexpr auto maxDistance2 = 500.0f * 500.0f;

  if (m_showAngles)
  {
    const auto arrow = arrowHead(9.0f, 6.0f);

    auto renderService = RenderService{renderContext, renderBatch};
    renderService.setShowOccludedObjectsTransparent();
    renderService.setForegroundColor(m_angleColor);

    for (const auto* entityNode : m_entities)
    {
      if (!m_showHiddenEntities && !m_editorContext.visible(entityNode))
      {
        continue;
      }

      const auto rotation = vm::mat4x4f{entityNode->entity().rotation()};
      const auto direction = rotation * vm::vec3f{0, 0, 1};
      const auto center = vm::vec3f{entityNode->logicalBounds().center()};

      const auto toCam = renderContext.camera().position() - center;
      // only distance cull for perspective camera, since the 2D one is always very far
      // from the level
      if (
        renderContext.camera().perspectiveProjection()
        && vm::squared_length(toCam) > maxDistance2)
      {
        continue;
      }

      auto onPlane = toCam - vm::dot(toCam, direction) * direction;
      if (vm::is_zero(onPlane, vm::Cf::almost_zero()))
      {
        continue;
      }

      onPlane = vm::normalize(onPlane);

      const auto rotZ = rotation * vm::vec3f{0, 0, 1};
      const auto angle = -vm::measure_angle(rotZ, onPlane, direction);
      const auto matrix = vm::translation_matrix(center)
                          * vm::rotation_matrix(direction, angle) * rotation
                          * vm::translation_matrix(16.0f * vm::vec3f{0, 0, 1});


      const auto vertices =
        kdl::vec_transform(arrow, [&](const auto& x) { return matrix * x; });
      renderService.renderPolygonOutline(vertices);
    }
  }
}

std::vector<vm::vec3f> EntityRenderer::arrowHead(
  const float length, const float width) const
{
  // clockwise winding
  return std::vector{
    vm::vec3f{0.0f, width / 2.0f, 0.0f},
    vm::vec3f{length, 0.0f, 0.0f},
    vm::vec3f{0.0f, -width / 2.0f, 0.0f},
  };
}

void EntityRenderer::invalidateBounds()
{
  m_boundsValid = false;
}

namespace
{

auto makeWireFrameBoundsVertexBuilder(std::vector<GLVertexTypes::P3::Vertex>& vertices)
{
  return [&](const vm::vec3d& v1, const vm::vec3d& v2) {
    vertices.emplace_back(vm::vec3f{v1});
    vertices.emplace_back(vm::vec3f{v2});
  };
}

auto makeColoredWireFrameBoundsVertexBuilder(
  std::vector<GLVertexTypes::P3C4::Vertex>& vertices, const Color& color)
{
  return [&](const vm::vec3d& v1, const vm::vec3d& v2) {
    vertices.emplace_back(vm::vec3f{v1}, color);
    vertices.emplace_back(vm::vec3f{v2}, color);
  };
}

auto makeColoredSolidBoundsVertexBuilder(
  std::vector<GLVertexTypes::P3NC4::Vertex>& vertices, const Color& color)
{
  return [&](
           const vm::vec3d& v1,
           const vm::vec3d& v2,
           const vm::vec3d& v3,
           const vm::vec3d& v4,
           const vm::vec3d& n) {
    vertices.emplace_back(vm::vec3f{v1}, vm::vec3f{n}, color);
    vertices.emplace_back(vm::vec3f{v2}, vm::vec3f{n}, color);
    vertices.emplace_back(vm::vec3f{v3}, vm::vec3f{n}, color);
    vertices.emplace_back(vm::vec3f{v4}, vm::vec3f{n}, color);
  };
}

} // namespace

void EntityRenderer::validateBounds()
{
  auto solidVertices = std::vector<GLVertexTypes::P3NC4::Vertex>{};
  solidVertices.reserve(36 * m_entities.size());

  if (m_overrideBoundsColor)
  {
    using Vertex = GLVertexTypes::P3::Vertex;
    auto pointEntityWireframeVertices = std::vector<Vertex>{};
    auto brushEntityWireframeVertices = std::vector<Vertex>{};

    pointEntityWireframeVertices.reserve(24 * m_entities.size());
    brushEntityWireframeVertices.reserve(24 * m_entities.size());

    for (const auto* entityNode : m_entities)
    {
      if (m_editorContext.visible(entityNode))
      {
        const auto pointEntity = !entityNode->hasChildren();
        if (pointEntity && !entityNode->entity().model())
        {
          entityNode->logicalBounds().for_each_face(
            makeColoredSolidBoundsVertexBuilder(solidVertices, boundsColor(entityNode)));
        }
        else if (pointEntity)
        {
          entityNode->logicalBounds().for_each_edge(
            makeWireFrameBoundsVertexBuilder(pointEntityWireframeVertices));
        }
        else
        {
          entityNode->logicalBounds().for_each_edge(
            makeWireFrameBoundsVertexBuilder(brushEntityWireframeVertices));
        }
      }
    }

    m_pointEntityWireframeBoundsRenderer = DirectEdgeRenderer{
      VertexArray::move(std::move(pointEntityWireframeVertices)), PrimType::Lines};
    m_brushEntityWireframeBoundsRenderer = DirectEdgeRenderer{
      VertexArray::move(std::move(brushEntityWireframeVertices)), PrimType::Lines};
  }
  else
  {
    using Vertex = GLVertexTypes::P3C4::Vertex;
    auto pointEntityWireframeVertices = std::vector<Vertex>{};
    auto brushEntityWireframeVertices = std::vector<Vertex>{};

    pointEntityWireframeVertices.reserve(24 * m_entities.size());
    brushEntityWireframeVertices.reserve(24 * m_entities.size());

    for (auto* entityNode : m_entities)
    {
      if (m_editorContext.visible(entityNode))
      {
        const auto pointEntity = !entityNode->hasChildren();
        if (pointEntity && !entityNode->entity().model())
        {
          entityNode->logicalBounds().for_each_face(
            makeColoredSolidBoundsVertexBuilder(solidVertices, boundsColor(entityNode)));
        }
        else if (pointEntity)
        {
          entityNode->logicalBounds().for_each_edge(
            makeColoredWireFrameBoundsVertexBuilder(
              pointEntityWireframeVertices, boundsColor(entityNode)));
        }
        else
        {
          entityNode->logicalBounds().for_each_edge(
            makeColoredWireFrameBoundsVertexBuilder(
              brushEntityWireframeVertices, boundsColor(entityNode)));
        }
      }
    }

    m_pointEntityWireframeBoundsRenderer = DirectEdgeRenderer(
      VertexArray::move(std::move(pointEntityWireframeVertices)), PrimType::Lines);
    m_brushEntityWireframeBoundsRenderer = DirectEdgeRenderer(
      VertexArray::move(std::move(brushEntityWireframeVertices)), PrimType::Lines);
  }

  m_solidBoundsRenderer =
    TriangleRenderer{VertexArray::move(std::move(solidVertices)), PrimType::Quads};
  m_boundsValid = true;
}

AttrString EntityRenderer::entityString(const mdl::EntityNode* entityNode) const
{
  const auto& classname = entityNode->entity().classname();
  // const mdl::AttributeValue& targetname =
  // entity->attribute(mdl::AttributeNames::Targetname);

  auto str = AttrString{};
  str.appendCentered(classname);
  // if (!targetname.empty())
  // str.appendCentered(targetname);
  return str;
}

const Color& EntityRenderer::boundsColor(const mdl::EntityNode* entityNode) const
{
  if (const auto* definition = entityNode->entity().definition())
  {
    return definition->color();
  }
  return m_boundsColor;
}

} // namespace tb::render
