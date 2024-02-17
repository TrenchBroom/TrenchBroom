/*
 Copyright (C) 2023 Daniel Walder

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

#include "EntityDecalRenderer.h"

#include "Assets/DecalDefinition.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "BrushRendererArrays.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/ModelUtils.h"
#include "Model/Polyhedron_Face.h"
#include "Model/TexCoordSystem.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "octree.h"

#include "kdl/memory_utils.h"
#include "kdl/overload.h"

#include "vm/intersection.h"

#include <cstring>

namespace TrenchBroom::Renderer
{

namespace
{

std::optional<Assets::DecalSpecification> getDecalSpecification(
  const Model::EntityNode* entityNode)
{
  const auto decalSpec = entityNode->entity().decalSpecification();
  return decalSpec.textureName.empty() ? std::nullopt : std::make_optional(decalSpec);
}

using Vertex = Renderer::GLVertexTypes::P3NT2::Vertex;
std::vector<Vertex> createDecalBrushFace(
  const Model::EntityNode* entityNode,
  const Model::BrushNode* brush,
  const Model::BrushFace& face,
  const Assets::Texture* texture)
{
  assert(texture != nullptr);

  const auto textureSize = vm::vec2f{float(texture->width()), float(texture->height())};
  const auto textureName = texture->name();

  // copy the face properties, used to calculate the decal size and texture coords
  auto attrs = Model::BrushFaceAttributes{textureName, face.attributes()};
  auto texCoords = face.texCoordSystem().clone();
  const auto tex = texCoords.get();

  // create the geometry for the decal
  const auto plane = face.boundary();
  const auto origin = entityNode->physicalBounds().center();
  const auto center = plane.project_point(origin);

  // re-project the vertices in case the texture axes are not on the face plane
  const auto xShift = tex->xAxis() * double(attrs.xScale() * textureSize.x() / 2.0f);
  const auto yShift = tex->yAxis() * double(attrs.yScale() * textureSize.y() / 2.0f);

  // we want to shift every vertex by just a little bit to avoid z-fighting
  const auto offset = plane.normal * 0.1;

  // start with a rectangle
  auto verts = std::vector{
    plane.project_point(center + xShift - yShift) + offset, // Bottom Right
    plane.project_point(center + xShift + yShift) + offset, // Top Right
    plane.project_point(center - xShift + yShift) + offset, // Top Left
    plane.project_point(center - xShift - yShift) + offset  // Bottom Left
  };

  // because the texture axes don't have to align to the face, we might have a reversed
  // face here. if so, reverse the points to get a valid face for the plane
  const auto [_, vertPlane] = vm::from_points(verts[0], verts[1], verts[2]);
  if (!vm::is_equal(plane.normal, vertPlane.normal, vm::C::almost_zero()))
  {
    std::reverse(std::begin(verts), std::end(verts));
  }

  // calculate the texture offset based on the first vertex location
  const auto vtx = verts[0];
  const auto xOffs = -vm::dot(vtx, tex->xAxis()) / attrs.xScale();
  const auto yOffs = -vm::dot(vtx, tex->yAxis()) / attrs.yScale();
  attrs.setXOffset(float(xOffs));
  attrs.setYOffset(float(yOffs));

  // clip the decal geometry against every other plane in the brush
  for (const auto& f : brush->brush().faces())
  {
    if (&f == &face)
    {
      // skip the face that the decal is to be applied to, it's coplanar
      continue;
    }

    verts = vm::polygon_clip_by_plane(f.boundary(), std::begin(verts), std::end(verts));
    if (verts.empty())
    {
      // the polygon is completely outside the brush bounds, no decal will be placed
      return {};
    }
  }

  // convert the geometry into a list of vertices
  const auto norm = vm::vec3f{plane.normal};
  return kdl::vec_transform(verts, [&](const auto& v) {
    return Vertex{vm::vec3f{v}, norm, tex->getTexCoords(v, attrs, textureSize)};
  });
}

} // namespace

EntityDecalRenderer::EntityDecalRenderer(std::weak_ptr<View::MapDocument> document)
  : m_document{std::move(document)}
{
  clear();
}

void EntityDecalRenderer::invalidate()
{
  for (auto& [ent, data] : m_entities)
  {
    invalidateDecalData(data);
  }
}

void EntityDecalRenderer::clear()
{
  m_entities.clear();
  m_vertexArray = std::make_shared<BrushVertexArray>();
  m_faces = std::make_shared<TextureToBrushIndicesMap>();
  m_faceRenderer = FaceRenderer{m_vertexArray, m_faces, m_faceColor};
}

void EntityDecalRenderer::updateNode(Model::Node* node)
{
  node->accept(kdl::overload(
    [](Model::WorldNode*) {},
    [](Model::LayerNode*) {},
    [](Model::GroupNode*) {},
    [&](const Model::EntityNode* entity) { updateEntity(entity); },
    [&](const Model::BrushNode* brush) { updateBrush(brush); },
    [](Model::PatchNode*) {}));
}

void EntityDecalRenderer::removeNode(Model::Node* node)
{
  node->accept(kdl::overload(
    [](Model::WorldNode*) {},
    [](Model::LayerNode*) {},
    [](Model::GroupNode*) {},
    [&](const Model::EntityNode* entity) { removeEntity(entity); },
    [&](const Model::BrushNode* brush) { removeBrush(brush); },
    [](Model::PatchNode*) {}));
}

void EntityDecalRenderer::updateEntity(const Model::EntityNode* entityNode)
{
  // if the entity isn't visible, don't create decal geometry for it
  const auto& editorContext = kdl::mem_lock(m_document)->editorContext();

  // check if the entity has a decal specification
  const auto spec =
    editorContext.visible(entityNode) ? getDecalSpecification(entityNode) : std::nullopt;

  // see if we are tracking this entity
  const auto entity = m_entities.find(entityNode);
  const auto isTracking = entity != std::end(m_entities);
  if (isTracking && spec)
  {
    // entity is being tracked and has a decal specification, invalidate it
    invalidateDecalData(entity->second);
  }
  else if (isTracking)
  {
    // entity is being tracked but no longer has a decal specification
    removeEntity(entityNode);
  }
  else if (spec.has_value())
  {
    // entity is not being tracked and has a decal specification, start tracking it
    m_entities.insert({entityNode, EntityDecalData{}});
  }
}

void EntityDecalRenderer::removeEntity(const Model::EntityNode* entityNode)
{
  if (const auto it = m_entities.find(entityNode); it != std::end(m_entities))
  {
    // make sure the entity data is cleaned up
    invalidateDecalData(it->second);
    m_entities.erase(it);
  }
}

void EntityDecalRenderer::updateBrush(const Model::BrushNode* brushNode)
{
  // invalidate any entities that intersect this brush or are tracking this brush
  for (auto& [ent, data] : m_entities)
  {
    // skip entities that are going to be recomputed anyway
    if (!data.validated)
    {
      continue;
    }

    // if the brush is not visible, then it doesn't (currently) intersect
    const auto& editorContext = kdl::mem_lock(m_document)->editorContext();
    const auto intersects =
      editorContext.visible(brushNode) && brushNode->intersects(ent);
    const auto tracked = std::find(data.brushes.begin(), data.brushes.end(), brushNode)
                         != data.brushes.end();

    // if this brush is tracked by this entity or intersects, we'll need to
    // recalculate the geometry
    if (intersects || tracked)
    {
      invalidateDecalData(data);
    }
  }
}

void EntityDecalRenderer::removeBrush(const Model::BrushNode* brushNode)
{
  // invalidate any entities that are tracking this brush
  for (auto& [ent, data] : m_entities)
  {
    // skip entities that are going to be recomputed anyway
    if (!data.validated)
    {
      continue;
    }

    // if this brush is tracked by this entity, remove it and recalculate
    const auto tracked = std::find(data.brushes.begin(), data.brushes.end(), brushNode)
                         != data.brushes.end();
    if (tracked)
    {
      invalidateDecalData(data);
    }
  }
}

void EntityDecalRenderer::invalidateDecalData(EntityDecalData& data) const
{
  // do nothing if the brush data is already marked as invalidated
  if (!data.validated)
  {
    return;
  }

  data.validated = false;

  // if the texture doesn't exist, do nothing
  // also do nothing if the VBO storage fields are null, but it shouldn't happen
  if (!data.texture || !data.vertexHolderKey || !data.faceIndicesKey)
  {
    return;
  }

  // update the VBO
  m_vertexArray->deleteVerticesWithKey(data.vertexHolderKey);

  const auto faceIndexHolder = m_faces->at(data.texture);
  faceIndexHolder->zeroElementsWithKey(data.faceIndicesKey);

  if (!faceIndexHolder->hasValidIndices())
  {
    // there are no indices left to render for this texture
    m_faces->erase(data.texture);
  }

  data.vertexHolderKey = nullptr;
  data.faceIndicesKey = nullptr;
}

void EntityDecalRenderer::validateDecalData(
  const Model::EntityNode* entityNode, EntityDecalData& data) const
{
  if (data.validated)
  {
    // already validated, no need to check again
    return;
  }

  const auto spec = getDecalSpecification(entityNode);
  ensure(spec, "entity has a decal specification");

  const auto& document = kdl::mem_lock(m_document);
  const auto& editorContext = document->editorContext();
  const auto* world = document->world();

  // collect all the brush nodes that touch the entity's bbox
  const auto entityBounds = entityNode->physicalBounds();
  const auto intersectors = world->nodeTree().find_intersectors(entityBounds);

  // track them in the entity
  data.brushes.clear();
  for (const auto* node : intersectors)
  {
    const auto* brushNode = dynamic_cast<const Model::BrushNode*>(node);
    if (brushNode && editorContext.visible(brushNode))
    {
      data.brushes.push_back(brushNode);
    }
  }

  data.texture = document->textureManager().texture(spec->textureName);
  if (!data.texture)
  {
    // no decal texture was found, don't generate any geometry
    data.validated = true;
    return;
  }

  // `bbox` and methods in the veclib library perform inclusive intersection tests - that
  // is, if two polygons share an edge, plane, or vertex, then they are considered to be
  // intersecting. We need the opposite behaviour when placing decals: when the entity's
  // bounding box 'touches' but doesn't actually intersect through a face, we do not want
  // to place a decal on it. To achieve this logic, we shrink the bounds just a tiny bit
  // so adjacent faces that don't actually breach the entity's bounding box are excluded.
  const auto shrunkBounds = entityBounds.expand(-vm::C::almost_zero());

  // create geometry for the decal
  auto vertices = std::vector<Vertex>{};
  auto indices = std::vector<size_t>{};

  for (const auto& brush : data.brushes)
  {
    for (const auto& face : brush->brush().faces())
    {
      // see if this decal can be projected onto this face
      const auto facePolygon = face.geometry()->vertexPositions();
      if (vm::intersect_bbox_polygon(
            shrunkBounds, facePolygon.begin(), facePolygon.end()))
      {
        const auto decalPolygon =
          createDecalBrushFace(entityNode, brush, face, data.texture);
        if (!decalPolygon.empty())
        {
          // add the geometry to be uploaded into the VBO
          const auto vertexOffset = vertices.size();

          vertices.insert(vertices.end(), decalPolygon.begin(), decalPolygon.end());
          for (size_t i = 0; i < decalPolygon.size() - 2; ++i)
          {
            indices.push_back(vertexOffset);
            indices.push_back(vertexOffset + i + 1);
            indices.push_back(vertexOffset + i + 2);
          }
        }
      }
    }
  }

  if (!vertices.empty() && !indices.empty())
  {
    // upload the geometry into the VBO
    assert(m_vertexArray != nullptr);
    auto [vertBlock, vertDest] =
      m_vertexArray->getPointerToInsertVerticesAt(vertices.size());
    std::memcpy(vertDest, vertices.data(), vertices.size() * sizeof(*vertDest));
    data.vertexHolderKey = vertBlock;

    const auto brushVerticesStartIndex = GLuint(vertBlock->pos);

    auto& faceVboMap = *m_faces;
    auto& holderPtr = faceVboMap[data.texture];
    if (!holderPtr)
    {
      // inserts into map!
      holderPtr = std::make_shared<BrushIndexArray>();
    }
    auto [indexBlock, indexDest] =
      holderPtr->getPointerToInsertElementsAt(indices.size());
    auto* currentDest = indexDest;
    for (const auto& i : indices)
    {
      *(currentDest++) = GLuint(brushVerticesStartIndex + i);
    }
    data.faceIndicesKey = indexBlock;
  }

  data.validated = true;
}

void EntityDecalRenderer::render(RenderContext&, RenderBatch& renderBatch)
{
  // update any invalidated entities if required
  for (auto& [ent, data] : m_entities)
  {
    validateDecalData(ent, data);
  }

  m_faceRenderer.render(renderBatch);
}

} // namespace TrenchBroom::Renderer
