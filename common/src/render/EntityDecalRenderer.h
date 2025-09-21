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

#pragma once

#include "Color.h"
#include "render/AllocationTracker.h"
#include "render/EdgeRenderer.h"
#include "render/FaceRenderer.h"
#include "render/GLVertexType.h"
#include "render/Renderable.h"

#include <unordered_map>
#include <vector>

namespace tb::mdl
{
class BrushFace;
class BrushNode;
class EntityNode;
class Map;
class Material;
class Node;
struct DecalSpecification;
} // namespace tb::mdl

namespace tb::render
{

class EntityDecalRenderer
{
private:
  struct EntityDecalData
  {
    std::vector<const mdl::BrushNode*> brushes;

    /* will only be true if the brushes array has been calculated since the last change
     * and the decal geometry is stored in the VBO */
    bool validated = false;

    mdl::Material* material = nullptr;

    AllocationTracker::Block* vertexHolderKey = nullptr;
    AllocationTracker::Block* faceIndicesKey = nullptr;
  };

  using EntityWithDependenciesMap =
    std::unordered_map<const mdl::EntityNode*, EntityDecalData>;

  mdl::Map& m_map;
  EntityWithDependenciesMap m_entities;

  using Vertex = render::GLVertexTypes::P3NT2::Vertex;
  using MaterialToBrushIndicesMap =
    std::unordered_map<const mdl::Material*, std::shared_ptr<BrushIndexArray>>;

  std::shared_ptr<MaterialToBrushIndicesMap> m_faces;
  std::shared_ptr<BrushVertexArray> m_vertexArray;
  FaceRenderer m_faceRenderer;
  Color m_faceColor;

public:
  explicit EntityDecalRenderer(mdl::Map& map);

  /**
   * Equivalent to updateNode() on all added nodes.
   */
  void invalidate();

  /**
   * Equivalent to removeNode() on all added nodes.
   */
  void clear();

  /**
   * Adds a node if its not already present and invalidates it.
   */
  void updateNode(mdl::Node* node);

  /**
   * Removes a node. Calling with an unknown node is allowed, but ignored.
   */
  void removeNode(mdl::Node* node);

private:
  void updateEntity(const mdl::EntityNode* entityNode);
  void removeEntity(const mdl::EntityNode* entityNode);
  void updateBrush(const mdl::BrushNode* brushNode);
  void removeBrush(const mdl::BrushNode* brushNode);

  void invalidateDecalData(EntityDecalData& data) const;

  void validateDecalData(const mdl::EntityNode* entityNode, EntityDecalData& data) const;

public: // rendering
  void render(RenderContext& renderContext, RenderBatch& renderBatch);

  deleteCopy(EntityDecalRenderer);
};

} // namespace tb::render
